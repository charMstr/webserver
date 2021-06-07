#include "../includes/Service.hpp"
#include "../includes/Service_generator.hpp"
#include "../includes/put_service.hpp"
#include "../includes/get_service.hpp"
#include "../includes/delete_service.hpp"
#include "../includes/request_reader.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/utils.hpp"
#include "../includes/cgi_service.hpp"
#include "../includes/Error_service.hpp"
#include <new>
#include <string>

extern std::map<std::string, std::string> mime_types;

/*
** constructor
** parameters:	- request_reader: so that we can access the client_addr, the
** 				client_addr_size and the fd_client in which we will write our
** 				answers.
** 				-  request_reader: so that we can understand what method we
** 				have to deal with, what file to open etc...
*/
Service_generator::Service_generator(Request_reader &request_reader):
	reader(request_reader),
	location_block_to_use(NULL),
	use_alias(false),
	uri_is_a_directory(false),
	error_pages_to_use(),
	autoindex_activated(false)
{
	memset(&file_stat, 0, sizeof(file_stat));
	//std::cout << "\033[33mconstructor called for request_reader\033[0m" << std::endl;
}

//DESTRUCTOR
Service_generator::~Service_generator()
{
	//std::cout << "\033[31m[~]\033[33mdestructor called for request_reader\033[0m" << std::endl;
}

/*
** This public function is the interface between service_generator object, and
** the Server object in which it lives. It will create on the heap a derivated
** object of the Service class, and open the required filescriptors. All that
** is done accordingly to the reader's informations.
**
** NOTE: if within this function an exception of type bad_alloc occured, it is
** important to close any freshly opened fd (except for the fd_client that was
** already opened and will still be required to adress our error response to
** the client). Then rethrow the bad_alloc exception as is.
**
** NOTE: if an fd fails to open, we should just consider that all of our 1024
** fd are being opened/busy, we still can either respond with an error message
** of the family 5xx or wait another loop of select maybe?
**
** NOTE: for any Service* generated object, the reader.get_client_fd() should
** 	be placed into the Service::fd_client member variable, and not into the
** 	list_fd_write linked list.
*/

Service *Service_generator::generate()
{
	int res;

	//DEBUG std::cout << "\033[36mmethod = " << \
	// debug_return_method_string(reader.get_method(), \
	// reader.is_a_cgi_request()) << "\033[m" << std::endl;

	if (200 != (res = preliminary_set_up_to_service_build(reader.get_uri())))
		return (generate_error_service(res));
	if (200 != reader.get_status())
		return (generate_error_service(reader.get_status()));

	else if (reader.is_a_cgi_request())
		return (generate_cgi_service());
	else switch (reader.get_method())
	{
		case GET: return (generate_get_service(false));
		case HEAD: return (generate_get_service(true));
		case PUT: return (generate_put_service());
		case DELETE: return (generate_delete_service());
		case POST: return (generate_cgi_service());
		//default means we fucked up... just a safety net.
		default: return (generate_error_service(500));
	}
}

/*
** This function will return a string that is the concatenation of two pieces
** of path.
** EDGE CASES:
** - If right bit is empty, we return left bit as is.
** - If left bit is empty, we return right
** - If both are empty, we return some empty string.
*/
std::string Service_generator::concatenate_path(std::string const &left, std::string const &right)
{
	std::string full_path;
	if (!right.length()) //return the left part as is.
		return (left);
	else if (!left.length())
		return (right);
	else if (left[left.length() - 1] == '/')
	{
		if (right[0] == '/')
		{
			full_path = left;
			full_path.erase(full_path.length() - 1);
			full_path += right;
		}
		else { full_path = left + right; }
	}
	else
	{
		if (right[0] == '/') { full_path = left + right; }
		else { full_path = left + '/' + right; }
	}
	return (full_path);
}

/*
** This function will build the real full path of the resource we are trying to access (see file_path in nginx documentation).
** This is done after we matched a location_block (if possible), and after we
** selected the most appropriate root_to_use string (according to t_srv struct
** containing the configuration directives). The boolean use_alias is already
** set to true or false at this stage.
**
** NOTE: if the boolean use_alias is set to true, the root_to_use is already
** set to the "<path>" given in the "alias <path>" directive.
**
** A full resource path is made by the concatenation of:
** 1) +/- server.directory string (current working directory where ./webserv is
** run), only used if the root_to_use is a relative path.
** 2) root string: the most appropriate root, selected depending on:
** 	matched a location_block? an alias directive was given?
** 3) uri string: the location->path disapears from uri is use_alias is true.
*/
void
Service_generator::set_resource_path(std::string const &uri, \
		std::string const  &directory)
{
	if (use_alias)
	{ //modify uri, make the location->path disapear from the URI.
		std::string trimed_uri = std::string(uri).erase(0, \
				location_block_to_use->path.length());
		resource_path = concatenate_path(root_to_use, trimed_uri);
	}
	else
		resource_path = concatenate_path(root_to_use, uri);

	//only prepend directory to root_to_use is not an absolute path already.
	if (root_to_use.length() && root_to_use[0] != '/')
		resource_path = concatenate_path(directory, resource_path);
}

/*
** This function will return the appropriate Content-Type string. It will be
** used in the constructrion of the header_for_client as follow:
** "Content-Type: " + returned string + CRLF;
*/
const std::string
Service_generator::get_content_type(const std::string & uri) const
{
	if (uri.find('.') != std::string::npos)
	{
		std::string file_extension = uri.substr(uri.find_last_of("."));
//		std::cout << "FILE EXTENSION : |" << file_extension << "|\n";
		std::map<std::string, std::string>::iterator i;
		for (i = mime_types.begin(); i != mime_types.end(); ++i)
		{
//			std::cout << "testing mime_types = " << i->first << "\n";
			if (i->first == file_extension)
				return (std::string(i->second));
		}
	}
	return (std::string("application/octet-stream"));
}

/*
** Thid function will generate an object of type Cgi_service (derivated from
** base class Service).
**
** note: should throw if some bad allocation occures.
*/

Service *
Service_generator::generate_cgi_service()
{
	//DEBUG debug_entering_generate_xxx_service("generate_cgi_service()",\
	//		reader.get_client_fd(), reader.get_method(), reader.get_uri());

	//note: if this fails, the request_reader becomes a bad_alloc_handler.
	Cgi_service *svc = new Cgi_service(reader.get_client_infos(), \
			error_pages_to_use, resource_basename, &reader);

	/* open pipes for interprocess communication */
	int pipe1[2];	int pipe2[2];
	/* error checking */
	if (pipe(pipe1) < 0) {
		/* 500 Internal Server Error */
		svc->error_occured_set_up_everything(500, true); return (svc);
	}
	if (pipe(pipe2) < 0)
	{
		close(pipe1[0]);
		close (pipe1[1]);
		/* 500 Internal Server Error */
		svc->error_occured_set_up_everything(500, true); return (svc);
	}

	/* set fds as non blocking */
	if (fcntl(pipe1[0], F_SETFL, O_NONBLOCK) < 0 \
			|| fcntl(pipe1[1], F_SETFL, O_NONBLOCK) < 0 \
			|| fcntl(pipe2[0], F_SETFL, O_NONBLOCK) \
			|| fcntl(pipe2[1], F_SETFL, O_NONBLOCK))
	{
		close(pipe1[0]);
		close (pipe1[1]);
		close(pipe2[0]);
		close (pipe2[1]);
		/* 500 Internal Server Error */
		svc->error_occured_set_up_everything(500, true); return (svc);
	}

	/* store fds in cgi_service object */
	svc->set_webserv_writes_on_me(pipe1[1]); svc->set_cgi_reads_on_me(pipe1[0]);
	svc->set_cgi_writes_on_me(pipe2[1]); svc->set_webserv_reads_on_me(pipe2[0]);

	/* set up the argvs for execve() */
	char **av = NULL;
	if (!(av = svc->malloc_and_build_cgi_argv(reader.get_cgi_bin_name(), resource_path)))
	{
		close(pipe1[0]);
		close (pipe1[1]);
		close(pipe2[0]);
		close (pipe2[1]);
		/* 500 Internal Server Error */
		svc->error_occured_set_up_everything(500, true);
		return (svc);
	}

	/* set up the cgi envp */
	char **ev = NULL;
	if (!(ev = svc->malloc_and_build_cgi_envp(resource_path)))
	{
		free(av[0]);
		free(av[1]);
		free(av);
		svc->set_argv(NULL);
		close(pipe1[0]);
		close (pipe1[1]);
		close(pipe2[0]);
		close (pipe2[1]);
		/* 500 Internal Server Error */
		svc->error_occured_set_up_everything(500, true);
		return (svc);
	}
	int env_size = 0; for (int i = 0; ev[i] != NULL; ++i) { ++env_size; }

	svc->set_argv(av); svc->set_envp(ev); svc->set_env_size(env_size);

	//DEBUG debug_envp_or_argv(svc->get_envp(), "envp");
	//DEBUG debug_envp_or_argv(svc->get_argv(), "argv");

	/* fork the cgi */
	svc->set_cgi_pid(fork()); //pid was -1 in constructor, reset no matter what.
	/* error checking */
	if (svc->get_cgi_pid() < 0)
	{
		free(svc->get_argv()[0]); free(svc->get_argv()[1]); free(svc->get_argv());
		svc->set_argv(NULL);
		for (int i = 0; i < env_size; ++i)
			{ free(svc->get_envp()[i]); }
		free(svc->get_envp()); svc->set_envp(NULL);
		close(pipe1[0]);
		close (pipe1[1]);
		close(pipe2[0]);
		close (pipe2[1]);
		svc->error_occured_set_up_everything(500, true);
		return (svc);
	}

	if (svc->get_cgi_pid() == 0) /* ========= inside child process ========= */
	{
		DEBUG std::cerr << "Inside forked process...\n";
		close(svc->get_webserv_writes_on_me());
		close(svc->get_webserv_reads_on_me());
		/* connect the pipe fds to stdin and stdout of the current processs */
		if (dup2(svc->get_cgi_reads_on_me(), 0) < 0)
			exit (EXIT_FAILURE); 
		if (dup2(svc->get_cgi_writes_on_me(), 1) < 0)
			exit (EXIT_FAILURE); 
		execve(svc->get_argv()[0], svc->get_argv(), svc->get_envp());
		DEBUG std::cerr << "==========EXCEVE FAILED==========...\n";
		exit (EXIT_FAILURE); 
	}
	else /* ========= inside parent process ========= */
	{
		//DEBUG_(DEBUG_CGI_SERVICE) debug_cgi_svc(svc, true);
		//DEBUG std::cout << "Inside parent process...\n";

//		/* close the cgi-side pipe fds */
		close(svc->get_cgi_reads_on_me());
		close(svc->get_cgi_writes_on_me());

		/* argv and envp are no longed relevanr to the parent process */
		free(av[0]); free(av[1]);  free(av); svc->set_argv(NULL);
		for (int i = 0; i < env_size; ++i) { free(ev[i]); }
		free(ev);
		svc->set_envp(NULL);

		try
		{
			//need to write to the cgi process
			svc->list_fd_write.push_back(svc->get_webserv_writes_on_me());
			//and need to read the answer from the cgi process
			svc->list_fd_read.push_back(svc->get_webserv_reads_on_me());
		} catch (std::exception &e)
		{ /* something bad happened... */
			DEBUG std::cout << "\tCOULD NOT PUSH BACK FD, KILLING CGI\n";
			svc->close_all_fds_except_for_fd_client();
			svc->remove_all_fds_in_list_fd_rw_except_for_fd_client();
			if (svc->get_cgi_pid() != -1)
				{ kill(svc->get_cgi_pid(), SIGTERM); svc->set_cgi_pid(-1); }
			close(svc->get_webserv_reads_on_me());
			close(svc->get_webserv_writes_on_me());
			delete svc; throw e; //rethrow exception as is.
		}
	}
	return (svc);
}

/* ************************************************************************* */
/* GET METHOD                                                                */
/* ************************************************************************* */

/*
** This function generates a get_service.
**
** 1) If the uri is considered a directory we will try to use the index
** directive in the configuration file. And if this is not possible, we either
** answer with the directory listing (autoindex is on in the configuration
** file and the directory can be opened), or we answer with an error.
**
** 2) if the uri is not considered to be a directory, we open a file, and add
** it fd to the list fd_read, and simply add the service* to the
** Server::list_services. Then later read_svc() will be called, and the
** reading_buffer will be filled. Then we answer back an http message to the
** client socket (through the write_svc method).
**
** PARAMETER:
** - just_head: if set to true, we build service normally, but the write_svc()
** function will shortcut the part when we write with the body.
*/
Service * Service_generator::generate_get_service(bool just_head_no_body)
{
	//DEBUG debug_entering_generate_xxx_service("generate_get_service()",\
	//		reader.get_client_fd(), reader.get_method(), reader.get_uri());
	//DEBUG_(DEBUG_GET_SERVICE) \
	//	debug_service_generator_preliminary_to_service_build(*this, \
	//	reader.get_client_fd(), reader.get_method(), reader.is_a_cgi_request());
	get_service *svc =  new get_service(reader.get_client_infos(), \
			error_pages_to_use, resource_basename, &reader, reader.get_uri());
	try {
		if (just_head_no_body) //set if we create an answer to HEAD method.
			svc->just_head_no_body = true;
		//note: if this function returns 0, the uri is neither a file or a dir.
		if (set_uri_is_a_directory() == 0)
		{
			svc->error_occured_set_up_everything(404, 1);
			return (svc);
		}
		if (uri_is_a_directory)
		{
			if (redirect_if_trailing_slash_missing(svc, reader.get_uri()))
				return (svc);
			if (generate_get_service_autoindex_case(svc) ==  1)
			{
	//			DEBUG_(DEBUG_GET_SERVICE) \
	//				debug_generate_get_service_more_key_data(*this, \
	//						reader.get_client_fd());
				return (svc);
			}
		}
	//	DEBUG_(DEBUG_GET_SERVICE)  \
	//		debug_generate_get_service_more_key_data(*this, \
	//				reader.get_client_fd());
		//if we get here, either the uri was not a directory, or the
		//index_to_use resolved in an existing file.
		return generate_get_service_assist(svc);
	}
	catch (std::exception & e)
	{
		svc->close_all_fds_except_for_fd_client();
		svc->remove_all_fds_in_list_fd_rw_except_for_fd_client();
		delete svc;
		throw e; //rethrow exception as is.
	}
}

/*
** We get in this piece of code when the stat function has been called, then
** we identified that the resource requested in the uri is a directory.
** This function checks if the uri ends or not with a '/'. If it is not the
** case, it will set up the answer for a 301 permanent redirect message.
** This is pure bonus. It allows to navigate between relative links in clients
** like Google chrome.
**
** Note: The empty URI still ends with forward slash, so it is never really
** empty.
**
** RETURN:	1: the setup is done for permanent redirect, just return service
** 			0: keep going in the process of generating a get_service*.
*/

int
Service_generator::redirect_if_trailing_slash_missing(get_service* svc, \
		const std::string &uri)
{
	if (uri[uri.length() - 1] != '/')
	{
		svc->set_redirect_to(uri + '/');
		svc->error_occured_set_up_everything(301, 1);
		return (1);
	}
	return (0);
}

/*
** This function is called by generate_get_service(). We get here if the uri
** was not condisedered a directory(URI not matching perfectly a
** location_block, or not ending by a '/'), or the resolution of the
** index_to_use file produced a path to an existing file.
**
** The purpose of this function is to open the file descriptor for the
** resource_path, and to set all the headers required into the get_service*.
**
** Note: if at any stage something failed, the status_code inside the object
** becomes something else than 200, and the function error_occured_set_up_
** everything() is called. The get_service* is still returned.
**
** RETURN: the get_service*.
*/
Service *
Service_generator::generate_get_service_assist(get_service * svc)
{
//	DEBUG_(DEBUG_GET_SERVICE) debug_print_fileStat(&file_stat, \
//			reader.get_client_fd());
	// open the ressource_fd and add it list_fd_read
	int ressource_fd = -1;
	if ((ressource_fd = open(resource_path.c_str(), O_RDONLY)) == -1)
	{
	//log = open() "f/ull/real/path" failed (13: Permission denied)
		svc->error_occured_set_up_everything(403, 1);
		return (svc);
	}
    if ((fcntl(ressource_fd, F_SETFL, O_NONBLOCK)) == -1)
	{
		//log ?
		close(ressource_fd);
		svc->error_occured_set_up_everything(500, 1);
		return (svc);
	}

	// add ressource_fd to the list of fds to add to select's fd_set
	try { svc->list_fd_read.push_back(ressource_fd); }
	catch (const std::exception &e)
	{
		close (ressource_fd);
		throw e;
	}
	svc->answer_set_up_headers(file_stat, get_content_type(resource_path));
	return (svc);
}

/*
** This function is called to assist the generate_get_service() function. It is
** called when we considered the URI is a directory(matches perfectly a
** location_block or is terminated by a '/'):
** - IF the index_to_use is gives an existing file, we return 0 and keep on
** seting up the answer like normal.
** - If this is not the case, we need to use the autoindex, we return 1 and the
** service is returned straight away as it is already set up(either an
** error_occured_set_up_everything() was called, or the
** set_up_service_when_autoindex_is_required() was called.
**
** Parameters:
** - svc: the get_service itself
**
** This function sets those member variables:
** - index_to_use.
** - autoindex_activated.
**
** RETURN:	0 keep to set_up_the service
** 			1 return the service already set up, just return it as is.
*/

int
Service_generator::generate_get_service_autoindex_case(get_service *svc)
{
	set_index_to_use(location_block_to_use);
	if (index_to_use.length())
	{
		//try new path to the index file.
		std::string tmp1 = concatenate_path(resource_path, index_to_use);
		struct stat tmp2;
		// Get info about the ressource file, try to see if it exists, and it is
		//a regular file
		if (-1 != stat(tmp1.c_str(), &tmp2) && S_ISREG(tmp2.st_mode))
		{
			resource_path = tmp1; //keep new path to index file.
			file_stat = tmp2; //keep its stat.
			return (0); //keep seting up the service normally with new path.
		}
	}
	//get here: file does not exists, we will try to reply with the autoindex.
	set_autoindex_activated(location_block_to_use);
	generate_get_service_autoindex_case_confirmed(svc);
	return (1);
}

/*
** This function will try to open the directory when the autoindex is required.
** We get here when The URI is considered a directory, and the index_to_use
** string is either empty, or the file doesnt exist. The purpose of this
** function is to act depending on the value of autoindex_avtivated boolean
** parameter:
** - If turned off we set status_code to 403 (log = directory index of
** "/full/real/path" is forbidden),
** - If turned on we will try to open the directory, and set up the answer's
**  headers.
*/

void
Service_generator::generate_get_service_autoindex_case_confirmed(get_service *svc)
{
	if (!autoindex_activated)
	{
		//log = directory index of "/full/real/path" is forbidden
		//note: error is 403 in nginx, but we set it to 404 to pass 42's tester
		svc->error_occured_set_up_everything(404, 1);
	}
	else if ((svc->autoindex.dir_handle = opendir(resource_path.c_str())) == NULL)
	{
		//log = "resource_path" failed (13: Permission denied)
		svc->error_occured_set_up_everything(403, 1);
	}
	else
	{
		//enter function with the autoindex.dir_handle non NULL, means
		//file_stat struct is not used.
		svc->answer_set_up_headers(file_stat, std::string("text/html"));
	}
}

/*
** This function is called when the boolean uri_is_directory is set, and the
** index_to_use string is empty or gives a non valid file. We then try to find
** if the autoindex is turned on in the location block selected/server block.
*/
void
Service_generator::set_autoindex_activated(const t_loc *location)
{
	if (!location)
		autoindex_activated = reader.get_server().autoindex;
	else
		autoindex_activated = location->autoindex;
}

/* ************************************************************************* */
/* END GET METHOD                                                             */
/* ************************************************************************* */

/* ************************************************************************* */
/* PUT METHOD                                                                */
/* ************************************************************************* */

/*
 * This object will attempt to open() a ressource fd at the specified location,
 * write() contents on that ressource (through the method write_svc), and
 * then write() back an http response to the client socket also through the
 * write_svc method (using a Statemachine to separate the different stages
 * of the work)
 */

Service * Service_generator::generate_put_service()
{
	//DEBUG debug_entering_generate_xxx_service("generate_put_service()",\
	//		reader.get_client_fd(), reader.get_method(), reader.get_uri());

	put_service *svc =  new put_service(reader.get_client_infos(), \
			error_pages_to_use, resource_basename, &reader);
	try {
		/* make sure the request URI matches an existing location block */
		if (location_block_to_use == NULL)
		{
			/* Otherwise set up a 404 Not Found */
			svc->error_occured_set_up_everything(404, true);
			return (svc);
		}
		// cf. rfc 7231, section 4.3.4 "... MUST send a 400 (Bad Request)
		//response to a PUT request that contains a Content-Range header field"
		if (svc->reader->get_headers().find("Content-Range: ") \
				!= svc->reader->get_headers().end())
		{
			svc->error_occured_set_up_everything(400, true); //Bad request
			return (svc);
		}

		/* check if the target file already exists using stat()'s return value */
		struct stat statbuf; int statret = stat(resource_path.c_str(), &statbuf);
		if (statret == 0)  { /* if file already exists */
			svc->set_status_code(204); /* 204 No Content */ }
		else
			{ svc->set_status_code(201); } /* 201 Created */

		/* try to open() on the server the target file corresponding to the request URI */ 
		int file_fd = open(resource_path.c_str(), \
				 						O_CREAT | O_RDWR | O_TRUNC, 0666);
		if (file_fd < 0) { /* error checking */
			if (errno == EISDIR) {
				/* 409 Conflict (when a dir of the same name exists where we try to open) */
				svc->error_occured_set_up_everything(409, true); return (svc); 
			}
			else if (errno == EACCES) { /* 500 internal server error (permissions) */
				svc->error_occured_set_up_everything(500, true); return (svc); 
			}
			else { /* 503 service unavailable (generic error) */
				svc->error_occured_set_up_everything(503, 1); return (svc);
			}
		}
		svc->set_file_fd(file_fd); /* save the target file_fd in the object */

		/* set the target file_fd as non-blocking */
		if (fcntl(file_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			//log ?
			close(file_fd);
			svc->error_occured_set_up_everything(500, 1);
			return (svc);
		}

		/* add the target file fd it to the service's list fd set write in
		 * order to write the http request's body to it */
		try { svc->list_fd_write.push_back(file_fd); }
		catch (const std::exception &e)
		{
			close (file_fd);
			throw e;
		}
		/* initialize the service objet's statemachine for incoming work */
		svc->writing_status = put_service::PUT_DEFAULT_FILE;
		svc->answer_set_up_headers(reader.get_server(), reader.get_uri());
	}
	catch (std::exception & e) { /* error checking */
		svc->close_all_fds_except_for_fd_client();
		svc->remove_all_fds_in_list_fd_rw_except_for_fd_client();
		delete svc; throw e; /* rethrow exception as is */
	}
	return (svc);
}
/* ************************************************************************* */
/* END PUT METHOD                                                            */
/* ************************************************************************* */

/* ************************************************************************* */
/* ERROR SVC                                                                 */
/* ************************************************************************* */

/*
** This function is called at an early stage in Servic_generator::generate().
** It is called when we detected an error in the parsing, or if the method is
** not allowd in the locatino block selected. Its purpose is to create an
** object of type service*. It sets inside it the error_status_code.  Then it
** doesnt call svc->error_occured_set_up_everything (as no need to close any
** filedescriptors). It directtly calls the fuction
** set_up_answer_error_page_required().
*/
Service * Service_generator::generate_error_service(int error_status_code)
		
{
	Error_service * svc;
	svc = new Error_service(reader.get_client_infos(), error_pages_to_use, \
				resource_basename, &reader, error_status_code);
	//DEBUG debug_entering_generate_xxx_service("generate_error_service()",\
	//		reader.get_client_fd(), reader.get_method(), reader.get_uri());
	//DEBUG_(DEBUG_ERROR_SERVICE) \
	//	debug_error_service_creation_context(&reader.get_server(), \
	//			error_status_code);
	try
	{
		if (reader.get_method() == HEAD)
			svc->just_head_no_body = true;
		svc->error_occured_set_up_everything(error_status_code, 1);
	}
	catch (std::exception & e)
	{
		delete svc;
		throw e; //rethrow exception as is.
	}
	return (svc);
}

/* ************************************************************************* */
/* END ERROR SVC                                                             */
/* ************************************************************************* */

/* ************************************************************************* */
/* DELETE SVC                                                                */
/* ************************************************************************* */

Service * Service_generator::generate_delete_service(void)
{
	//DEBUG debug_entering_generate_xxx_service("generate_delete_service()",\
	//		reader.get_client_fd(), reader.get_method(), reader.get_uri());
	//DEBUG_(DEBUG_DELETE_SERVICE) debug_print_reader_headers(reader.get_headers(), reader.get_client_fd());
	delete_service *svc = new delete_service(reader.get_client_infos(), \
			error_pages_to_use, resource_basename, &reader);
	try {
		/* check if the resource is a directory or a simple file */
		DIR * dir_ptr = opendir(resource_path.c_str());
		if (dir_ptr == NULL)
		{
			if (errno == ENOTDIR) /* if resource is not a dir... */
			{
				if (unlink(resource_path.c_str()) == -1) /* delete the file */
					svc->error_occured_set_up_everything(409, true);
				//if the file got deleted, the status code is 204
				else 
					svc->error_occured_set_up_everything(204, true);
				return (svc);
			}
			else /* if the resource is not a file, this means it does not exist */
			{
				// 404 Not Found
				svc->error_occured_set_up_everything(404, true); return (svc);
			}
		}
		else /* if the resource is indeed a directory and can be opened */
		{
			if (resource_path.back() != '/') /* if the resource path does not en with a '/' */
				{ svc->error_occured_set_up_everything(409, true); return (svc); }
			/* collect all the contents of the current directory */
			std::list<std::string> dir_entries;
			struct dirent * dir_entry = NULL;
			while ((dir_entry = readdir(dir_ptr)) != NULL)
			{
				std::string entry(dir_entry->d_name);
				if (dir_entry->d_type == DT_DIR) { entry += "/"; }
				dir_entries.push_back(entry);
			}
			closedir(dir_ptr);

			/* make sure the directory is empty */
			std::list<std::string>::iterator i = dir_entries.begin();
			while (i != dir_entries.end())
			{
				if (*i != "./" && *i != "../") { break ; } else { ++i; }
			}
			if (i != dir_entries.end()) /* if dir not empty */
			{
				svc->error_occured_set_up_everything(500, true); return (svc);
			}
			else /* if dir empty */
			{
				if (rmdir(resource_path.c_str()) == -1) /* remove dir */
					{ svc->error_occured_set_up_everything(500, true); return (svc); }
			}
		}
	}
	catch (std::exception & e)
	{
		svc->close_all_fds_except_for_fd_client();
		svc->remove_all_fds_in_list_fd_rw_except_for_fd_client();
		delete svc;
		throw e; //rethrow exception as is.
	}
	return (svc);
}

/* ************************************************************************* */
/* UTILS                                                                     */
/* ************************************************************************* */

/*
** This funciton will select the correct locaiton block to use. It is called
** each time we create a service. It is called no matter the method of the the
** request, because we still need to set the member variable error_page_to_use
** before a service is actually created and added to the to list of Services*
** Once found the member variable is set.
**
** PARAMETER:
** - uri: the uri that is yielded from the reader class.
**
** RETURN: t_loc *, NULL if not found.
*/
void
Service_generator::set_location_block_to_use(const std::string &uri)
{
	std::vector<t_loc>::const_iterator i;
	size_t match_specificity = 0;

	if (uri.length() == 0)
		return;
	for (i = reader.get_server().locations.begin();
			i != reader.get_server().locations.end(); ++i)
	{
		if (uri.find(i->path) == 0) //if match starts at begining of string.
		{
		//	DEBUG_(DEBUG_SERVICE_GENERATOR) std::cout << "\tit's a match !\n";
			// and if that match is more specific
			if (i->path.length() > match_specificity)
			{
				match_specificity = i->path.length();
				location_block_to_use = i.base();
			}
		}
	}
}

/*
** This funciton is in charge of fecthing the index_to_use member variable.
*/
void
Service_generator::set_index_to_use(const t_loc *location)
{
	//true means we chose the server block's directive.
	if (!location || !location->index.length())
		index_to_use = reader.get_server().index;
	else
		index_to_use = location->index;
}

/*
** This funciton will get the root to use from t_srv structure. It gets the
** most accurate root to use, depending on the location block selected.
** If previously no location block was selected, we just look into the
** server_block. If a location block was selected, we look first in the
** location block, and if there is no luck, we use the server bloc as a fall
** back.
**
** NOTE: If the alias directive is present (non empty string) in a location
** block, it prevails on the root directive.
** NOTE: the root is set to an empty string if the directive is not given.
**
** RETURN: std::string reference.
*/

void
Service_generator::set_root_to_use(const t_loc* location)
{
	//true means we chose the server block's directive. (fall back option).
	if (!location || (!location->root.length() && !location->alias.length()))
		root_to_use = reader.get_server().root;
	else
	{
		if (location->alias.length()) //priority to alias directive
		{
			use_alias = true;
			root_to_use = location->alias;
		}
		else //fall back onto the root directive.
			root_to_use = location->root;
	}
}

/*
** This function does just like get_roor_to_use. The only difference is that we
** get a structure. The error_page is a string that is tied to many integers.
*/

void
Service_generator::set_error_page_to_use(const t_loc* location)
{
 	//true means we chose the server block's directive.
	if (!location || !location->error_page.error_page.length())
		error_pages_to_use = reader.get_server().error_page;
	else
		error_pages_to_use = location->error_page;
}

/*
** This function will create the basename of the resource asked in the uri.
** It uses the member variable resource_path, and trims the end, keeping only
** the folder name, and places the result in resource_path_basename.
** note: If the location path finishes with a '/' there is nothing to trim
** note: This variable we are creating is really useful for those two cases:
** 	- we want to try to fetche the index page.
** 	- we want to try to fetche the error_page.
** 	In both scenarios we need to concatenate the resource_basename and another
** 	string.
*/

void
Service_generator::set_resource_basename(void)
{
	std::size_t found = resource_path.rfind("/");
	if (found != std::string::npos)
		resource_basename = resource_path.substr(0, found + 1);
	else
		resource_basename = resource_path;
}

/*
** This function is called when we first create our service_generator.
** It is called after the set_location_block_to_use(uri) function is
** called. It directly returns if the method is not allowed for the
** selected location block or the server block.
** PARAMETERS:
** - location: pointer to the location block selected after the uri has been
**	compared to a list of selection blocks (it can be NULL, in this case, refer
**	to the server block).
** - method: an int representing a method, yielded in the reader/parser.
**
** NOTE: the method allowed by default in the server block is GET (not even
** HEAD).
**
** RETURN:	0 method is allowed
** 			1 method is forbiden
*/

bool
Service_generator::is_method_not_allowed(const t_loc *location, int method)
{
	if (location && METHODS_ISSET(&location->methods, method))
		return (0); //allowed
	else if (!location && METHODS_ISSET(&reader.get_server().methods, method))
		return (0); //alowed
	return (1);
}

/*
** this function is called in service_generator each time we just created a
** service. It will set in the generator all the informations that will be
** required by the service:
** - location_block_to_use
** - root_to_use
** - resource_path
** - resource_basename
** - error_page
** - cgi_related variables inside the reader/parser like "is_cgi", "path_info",
**  "cgi_bin_name".
**
** PARAMETERS:
** - uri: the uri of the request.
** - method: the method yielded by the reader/parser.
**
** NOTE: as we go we might encounter some error, like method is forbiden.
**
** RETURN:	200: OK
**			else: an Error_service object is created with returned value as
**				status code.
*/
int
Service_generator::preliminary_set_up_to_service_build(const std::string &uri)
{
	set_location_block_to_use(uri);
	if (is_method_not_allowed(location_block_to_use, reader.get_method()))
	{
		//DEBUG_(DEBUG_SERVICE_GENERATOR) \
		//debug_service_generator_preliminary_to_service_build(*this, \
		//reader.get_client_fd(), reader.get_method(), reader.is_a_cgi_request());
		return (405); //method not allowed in request. 405
	}
	reader.set_up_related_to_cgi(location_block_to_use);
	set_root_to_use(location_block_to_use);
	set_resource_path(uri, reader.get_server().directory);
	set_resource_basename();
	set_error_page_to_use(location_block_to_use);
	//DEBUG_(DEBUG_SERVICE_GENERATOR) \
	//	debug_service_generator_preliminary_to_service_build(*this, \
	//	reader.get_client_fd(), reader.get_method(), reader.is_a_cgi_request());
	return (200); //means ok
}

/*
** This function simply checks that a uri points to an existing file, then it
** sets a booleean if the uri should be considered to be a
**
** URI considered to be a directory if:
** - an uri that is actually an existing directory.
**
** Note: If boolean is set to true, we will need to set the index_to_use
** variable. If it is empty srting, we will need to look for the
** autoindex_is_on variable.
**
** RETURN:	1 OK, keep processing.
** 			0 KO, the uri is neither a file or a directory.
*/
int
Service_generator::set_uri_is_a_directory(void)
{
	// get the info about the file, and make sure it does exist.
	if (-1 == stat(resource_path.c_str(), &file_stat))
	{
		//log = open() "/full/real/path" failed (2: No such file or directory)
		return (0);
	}
//	DEBUG_(DEBUG_GET_SERVICE) debug_print_fileStat(&file_stat, \
//			reader.get_client_fd());

	//note at this stage we know the uri is either valid file or directory.
	if (S_ISDIR(file_stat.st_mode))
		uri_is_a_directory = true;
	return (1);
}

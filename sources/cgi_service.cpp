#include <map>
#include <iostream>
#include "../includes/cgi_service.hpp"
#include "../includes/http_methods.hpp"
#include "../includes/utils.hpp"
#include "../includes/debug_functions.hpp"

/*
** the constructor of the service base class does add the fd_client to member
** variable fd_client, and also to list_fd_write.
*/
Cgi_service::Cgi_service(const t_client_info client_info, \
	t_error_page error_page, std::string basename, Request_reader *reader) :
	//abstract base class constructor
	Service(client_info, error_page, basename, reader),
	cgi_pid(-1),
	argc(0), argv(NULL),
	envp(NULL),
	encoding_of_content_from_cgi(KNOWN_LENGTH),
	received_from_cgi_buffer_struct(),
	total_body_size(0),
	content_length(0)
{
	answer_status = ANSWER_WAIT; //for the answer to fd_client status.
}

/*
** destructor.
** note: the desctuctor of the Service base class closes all the fds remaining
** in list_fd_read, list_fd_write but DOES NOT close the fd_client member
** attribute.
*/
Cgi_service::~Cgi_service(void)
{
	if (argv != NULL)
	{
		for (int i = 0; i < argc; i++)
			free(argv[i]);
		free(argv);
	}
	if (envp != NULL)
	{
		for (int i = 0; i < (int)(sizeof(envp) / sizeof(char *)); i++)
			free(envp[i]);
		free(envp);
	}
}

/*
 * recover the cgi binary file name, store it in a malloc'd string, and return.
 * RETURN VALUES :
 * - a pointer to the malloc'd string on success
 * - NULL otherwise
 */

char **
Cgi_service::malloc_and_build_cgi_argv(const std::string &cgi_bin_name, const std::string &resource_path)
{
	char	**av;
	char	*pname;
	char	*ressource;

	av = (char **)malloc(sizeof(char *) * 3);
	pname = ft_strdup(cgi_bin_name.c_str());
	ressource = ft_strdup(resource_path.c_str());
	if (av == NULL || pname == NULL || ressource == NULL)
	{
		free(av);
		free(pname);
		free(ressource);
		return (NULL);
	}
	set_argc(2);
	av[0] = pname;
	av[1] = ressource;
	av[2] = NULL; //needs to always be null terminated.
	return (av);
}

/*
 * recover the HTTP_META_VARIABLES and store them in a malloc'd pointer to
 * strings (char **).
 * NB : the number of meta variables must be known beforehand.
 */

char **
Cgi_service::malloc_and_build_cgi_envp(std::string const &resource_path)
{
	std::map<std::string, std::string> env_map;
	char	**env;
	int		envc;

	try
	{
		env_map["AUTH_TYPE"] = "Basic";
		//warning header map keys are case sensitive.
		if (reader->get_headers().find("content-length") != reader->get_headers().end())
		{
//			std::cout << "=========INSERTING CONTENT LENGTH TO CGI ENV===========\n";
			env_map["CONTENT_LENGTH"] = ft_ultos(reader->get_content_length());
		}
		env_map["CONTENT_TYPE"] = reader->get_mime_type();
		env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
		env_map["PATH_INFO"] = reader->get_uri();//it is the full URI.
		env_map["PATH_TRANSLATED"] = resource_path;
		env_map["QUERY_STRING"] = reader->get_query_string();
		env_map["REMOTE_ADDR"] = get_ip(*(const struct sockaddr_in *)(&reader->get_client_addr()));
		env_map["REMOTE_IDENT"] = ""; // Not set as we don't support authentification.
		env_map["REMOTE_USER"] = ""; // Not set as we don't support authentification.
		env_map["REQUEST_METHOD"] = method_to_str(reader->get_method());
		env_map["REQUEST_URI"] = reader->get_uri();
		env_map["SCRIPT_NAME"] = resource_path;
		env_map["SERVER_NAME"] = reader->get_server().server_name;
		env_map["SERVER_PORT"] = ft_ultos(reader->get_server().host_port);
		env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
		env_map["SERVER_SOFTWARE"] = "webserv/0.1.0";

		//to add otherwise php-cgi is crying blood
		env_map["REDIRECT_STATUS"] = std::string("200");
		
 //TODO(Jeu 03/06/2021 at 16:14:21) impove this shit in another life
		if (reader->get_header("x-secret-header-for-test").size() > 0)
		{
			env_map["HTTP_X_SECRET_HEADER_FOR_TEST"] = reader->get_header("x-secret-header-for-test");
		}
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return (NULL);
	}
	if (!(env = (char **)malloc(sizeof(char *) * (env_map.size() + 1))))
		return (NULL);
	envc = 0;
	for (std::map<std::string, std::string>::iterator it = env_map.begin(); it != env_map.end(); it++)
	{
		char *var;
		if (!(var = ft_strdup(std::string(it->first + "=" + it->second).c_str())))
		{
			for (int i = 0; i < envc; i++)
				free(env[i]);
			free(env);
			return (NULL);
		}
		env[envc++] = var;
	}
	env[envc] = NULL;
	return (env);
}
/* ************************************************************************** */
/* SETTERS AND GETTERS                                                        */
/* ************************************************************************** */


/* ************************************************************************** */
/* READ AND WRITE                                                             */
/* ************************************************************************** */

/*
** This function overwrites the abstract base class's virtual function.
** It is called each time a fd in which we want to write is picked up by select
** for our cgi_service object. It decides if we write to cgi process or if we
** write back our answer to client.
**
** NOTE: the returned value is only here to help us detect errors. If we are
** totally done with a fd, it is added to list_fd_done_writing, it is NOT
** removed from list_fd_write, and it is close (except if it is the fd_client).
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
Cgi_service::write_svc(int fd)
{
	try
	{
		if (fd == get_fd_client())
			return (write_to_client());
		else
			return (write_to_cgi());
	}
	catch (std::exception &e)
	{
		if (cgi_pid != -1)
		{
			kill(cgi_pid, SIGTERM);
			cgi_pid = -1;
		}
		throw e;
	}
}

/*
** This function overwrites the abstract base class's virtual function.
** It is called each time a fd in which we want to read is picked up by select
** for our cgi_service object. It decides if we read our futur answer from the
** cgi process or if we read from the fd_clent in order to feed data to the
** cgi process.
**
** NOTE: the returned value is only here to help us detect errors. If we are
** totally done with a fd, it is added to list_fd_done_reading, it is NOT
** removed from list_fd_read, and it is closed (except if it is the fd_client).
** NOTE: special mention for the read() from the cgi. It must not return -1 if
** read() call returned -1 while reading on the pipe (the pipe is set to non
** blocking, there is a reason for it). It it occasionally returning -1, no
** problem, return 0 instead. Eventually it will be noticed because of the
** timeout if it keeps doing that.
**
** RETURN:	-1 KO, something wrong happened while reading.
** 			0, no problem, but timeout_reference not updated.
** 			1, ok, timout_reference reset.
*/

int
Cgi_service::read_svc(int fd)
{
	int res = read_from_cgi(fd);
	//yes means the status code fetched from the cgi is an error code.
	if (get_status_code() >= 300)
		return (-1);
	//if the header section never ends: 500 status code error is raised
	if (answer_status == ANSWER_WAIT \
			&& header_for_client.length() > 4200)
		return (-1);
	if (res == 0 && encoding_of_content_from_cgi == KNOWN_LENGTH \
			&& received_from_cgi_buffer_struct.status != READING_DONE)
		return (-1); //->considered a problem.
	if (res == -1) //->considered ok, until timeout occurs.
		return (0); //this avoid having the service responding with an
	//error message just because we had the non blocking pipe returning -1
	return (res);
}

/*
** This function is in charge of writing our request to the cgi process, in its
** dedicated pipe.
** Note: the request must not be chunked.
**
** RETURN:	-1: KO with a write operation
** 			0: no problem,  but the timeout_reference wont be updated.
** 			1 or more: update the timeout_reference for the given service.
*/

int
Cgi_service::write_to_cgi()
{
	/* attempt to write something on the pipe fd that sends data to CGI */
	int wrtret;
	wrtret = write(webserv_writes_on_me, reader->body_from_client.c_str(),
					reader->body_from_client.length());
	if (wrtret < 0)
	{ // something bad happened //
		// if write() failed, we kill the CGI and return 0 to inform of an error //
		kill(cgi_pid, SIGTERM);
		cgi_pid = -1;
		return (wrtret);
	}
	/* if we wrote something, remove bytes from our buffer */
	if (wrtret > 0)
		reader->body_from_client.erase(0, size_t(wrtret));
	/* if the buffer is empty AND if the reader is done with the fd_client */
	if ((reader->body_from_client.length() == 0) \
			&& reader->get_request_state() == Request_reader::DONE)
	{
		//then add the pipe end webserv_writes_on_me to list_fd_done_writing.
		list_fd_done_writing.push_back(webserv_writes_on_me);
		//and close it.
		close(webserv_writes_on_me);
	}
	return (wrtret);
}

/*
** This function is called when we answer to fd_client (connection socket).
** 1) First of all we wait that the headers are fully ready for us.
** 2) We first write our headers to client.
** 3) When done writing the headers, we try to write the body.
** 4) When done with that we set the answer_status to ANSWER_DONE. the fd is
** added to list_fd_done_writing
**
** RETURN:	-1: KO with a write operation
** 			0: no problem,  but the timeout_reference wont be updated.
** 			1 or more: update the timeout_reference for the given service.
*/

int
Cgi_service::write_to_client()
{
	int res = 0;

	if (answer_status == ANSWER_WAIT)
		return (res); //avoid -1 so that it is not counted as an error.
	if (answer_status == ANSWER_HEADERS)
	{
		if ((res = answer_with_header(answer_status)) < 0)
			return (-1);
		return (res);
	}
	if (answer_status == ANSWER_BODY)
	{
		if ((res = answer_from_reading_buffer()) < 0)
			return (-1);

	}
	if (answer_status == ANSWER_DONE)
	{
		//note: never close the fd_client in the Service* scope.
		list_fd_done_writing.push_back(get_fd_client());
	}
	return (res);
}

/*
** This function is called each time we respond to client with body, directly
** from the buffer filled by the cgi.
**
** Note: If the encoding_of_content_from_cgi chosen was chunked,
** the chunking tags have been already inserted in place within the buffer for us.
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
Cgi_service::answer_from_reading_buffer(void)
{
	int res;
	//DEBUG_(DEBUG_CGI_SERVICE) debug_string("CGI to client: writing from buffer", \
	//		get_fd_client(), received_from_cgi_buffer_struct.buffer);
	res = write(get_fd_client(), received_from_cgi_buffer_struct.buffer, \
			received_from_cgi_buffer_struct.len);
	if (res == -1)
		return (res);
	//shift the content of the answer_buffer.buffer.
	memmove(received_from_cgi_buffer_struct.buffer, received_from_cgi_buffer_struct.buffer + res, \
			received_from_cgi_buffer_struct.len - res);
	received_from_cgi_buffer_struct.len -= res;
	//if totatlly done writing
	if (received_from_cgi_buffer_struct.status == READING_DONE \
			&& received_from_cgi_buffer_struct.len == 0) //fully done.
		answer_status = ANSWER_DONE;
	return (1);
}

/*
** This function will read from the pipe named "webserv_reads_on_me". It will
** read the output from the cgi process into the
** received_from_cgi_buffer_struct.buffer.
** 1) First of all we try to get completely the header section of the http
** message outputed. This is stored into Service::header_for_client.
** 2) Once this is done, we consider whatever is left into the buffer a piece
** of the body. it stays in the received_from_cgi_buffer_struct.buffer.
** 3) We parse the string header_for_client, looking for the http header
** "Content-Length: ...\r\n". If found we sent the member variable
** encoding_of_content_from_cgi to KNOWN_LENGTH, else to ALREADY_CHUNKED if we
** find "transfer-encoding: " header, or to NEED_CHUNK as a last resort.
** 4) Each time there is a new read into the buffer, we look for EOF (CHUNKING
** case). Or we wait for the total lenght of body matching the Content-Length.
** Until then we keep feeding the buffer...
**
** NOTE: when done reading, we close the fd, and add it to list_fd_done_reading
** 	and we set received_from_cgi_buffer_struct.status to DONE_READING, we also
**	kill the child process (the cgi script).
**
** RETURN:	0: EOF
** 			1 or more: OK, and the timeout reference is updated.
** 			-1: read failed. no update of timeout reference.
** 			-2: (no read attempt) not EOF, no update of timeout reference.
*/
int
Cgi_service::read_from_cgi(int fd)
{
	int previous_len = received_from_cgi_buffer_struct.len;
	int read_res, size_chunk;
	//read_into_received_from...() updates the buffer and its len
	if ((read_res = read_into_received_from_cgi_buffer_struct(fd)) < 0)
	{
		//if read_read < 0:
		//-1: means error,
		//-2: means just wait for buffer to be flush, or timeout occurs
		return (read_res);
	}
	size_chunk = read_res;
	if (answer_status == ANSWER_WAIT) //means we havent got the full headers yet
		size_chunk = update_header_for_client_and_find_double_crlf();
	else //usefull only for KNOWN_LENGTH case, anyway.
		total_body_size += read_res;
	if (this->get_status_code() >= 300)
		return (-1);
	if (answer_status != ANSWER_WAIT)
	{
		//sets the rcv_from_cgi_buffer_struct.status to READING_DONE if yes.
		if (is_reading_from_cgi_done(read_res))
		{
			if (cgi_pid != -1) // kill the process cgi_pid
			{
				kill(cgi_pid, SIGTERM);
				cgi_pid = -1;
			}
			//adding fd to list fd_done, and closing it (it is not fd_client).
			list_fd_done_reading.push_back(fd);
			close(fd); //closing webserv_reads_on_me, OK
		}
		//adds chunking if necessary.
		if (encoding_of_content_from_cgi == NEED_CHUNK)
			close_chunk(previous_len, size_chunk);
	}
	return (read_res);
}

/*
** This function reads into a buffer from the end pipe connected to the cgi
** process. If the length of the string contained into the buffer is non zero
** when entering the function, it means the data wasnt written fully or wasnt
** written at all by a potential previous call to write(). This data must not
** be overiden.
**
** Also if the reading of the headers is already done (the answer_status is no
** longer ANSWER_WAIT), we are now reading the body of the request. Depending
** on the encoding, we leave some space for the chunking tag in the reading
** buffer. the chunking tag will then be inserted in place, avoiding all sorts
** of memory allocation (much faster and resilient).
**
** PARAMETER:
** - fd: the fd in which we read, the variable "webserv_read_on_me".
**
** NOTE: in the case of the cgi, if the read returns 0 it doesnt mean we are
**	done.
**
** NOTE: the +5 in the code, refers to the size of the final closing chunk:
** 	"0\r\n\r\n"
**
** RETURN:	0: EOF, no update of timereference for the timeout.
** 			more than 0: ok, update of timeout reference.
** 			-1: read failed.
** 			-2: we skipp the reading, but we dont update timeout reference,
** 			nor we consider we reached EOF.
*/

int
Cgi_service::read_into_received_from_cgi_buffer_struct(int fd)
{
	size_t left_space = BUFFER_SIZE - received_from_cgi_buffer_struct.len;
	if (answer_status != ANSWER_WAIT && encoding_of_content_from_cgi == NEED_CHUNK)
	{
		//make more space, if we need to insert chunk tags
		//note: the + 5 represents the very last chunk tag: "0\r\n\r\n"
		left_space -= predict_chunk_tag_space(left_space) + 5;
	}
	if (left_space < 1)
	{
		//no need to read 0 bytes, or less..
		//returning -1 simulates that there is nothing to read, but not EOF!
		//it gives a chance to write_to_client() func to flush the buffer.
		return (-2);
	}

	int rdret = read(fd, received_from_cgi_buffer_struct.buffer + received_from_cgi_buffer_struct.len, \
			left_space);
	if (rdret < 0)
		return (rdret);
	received_from_cgi_buffer_struct.len += rdret;
	return (rdret);
}

/*
 * This function will look into the std::string s for a valid positive integer
 * number representation and return its value as an int.
*/
int		ft_stoi(std::string s)
{
	char const* digits = "0123456789";
	std::size_t const i = s.find_first_of(digits);
	std::size_t const j = s.find_first_not_of(digits, i);

	int ret = 0;
	for (size_t k = i; k < j; ++k)
	{
		ret *= 10;
		ret += (s[k] - '0');
	}
	return (ret);
}

/*
** This function will parse the std::string header_for_client, and make sure
** that if the header starting with "Status: " is present, it fetchs the status
** code that follows, and then it changes "Status: " into "HTTP/1.1 ".
**
** NOTE: the status code that is fetched is used to update the value of the
** object's status code.
*/
void
Cgi_service::update_header_containing_status_from_cgi_output()
{
	size_t	i;
	if ((i = header_for_client.find("Status:", 0)) != std::string::npos)
	{
		// make sure the header_for_client buffer contains the substring 'Status:'
		// AND the following status code
		if ((header_for_client.length() - i) > 11)
		{
			int status_code = ft_stoi(header_for_client.substr(i));
			//we set the status_code to the fetched value.
			set_status_code(status_code);
			//DEBUG std::cout << "\033[34m status_code fectched from cgi: " << status_code << "\033[m" << std::endl;
			//DEBUG std::cerr << "header_for_client before replace : |" << header_for_client << "|\n";
			header_for_client.erase(i, std::string("Status:").length());
			header_for_client.insert(i, std::string("HTTP/1.1"));
			//DEBUG std::cerr << "header_for_client after replace : |" << header_for_client << "|\n";
		}
	}
	else
	{
		if ((i = header_for_client.find("X-Powered")) != std::string::npos)
			header_for_client = "HTTP/1.1 200 OK\r\n" + header_for_client;
	}
}

/*
** This function will update the Service::header_for_client string (it
** represent the status_line + the headers up to the double CRLF CRLF).
** 1) It first looks into the buffer for a double CRLF. If it is found (most
** 	likely), we update the header_for_client string, and the buffer accordingly
** 2) If it is not found we append the content of the buffer, to the
** 	header_for_client string. WARNING: to avoid the case of a CRLF CRLF that
** 	got separated in two successive reads, we still scan the string
** 	header_for_client, looking for this "\r\n\r\n" needle.
** 3) if CRLF CRLF was found, we set the answer_status from ANSWER_WAIT to
** 	ANSWER_HEADERS, the function write_to_client() will be able to start doing
** 	its job. We also set up the encoding related variables(encoding and
** 	content_length).
**
** RETURN: this function returns the size of the body, after we took off the
** headers from the received_from_cgi_buffer_struct.buffer.
*/

int
Cgi_service::update_header_for_client_and_find_double_crlf(void)
{
	if (cgi_first_try_to_find_double_crlf_and_update() \
			|| cgi_second_try_to_find_double_crlf_and_update())
	{
		//getting here means the header_for_client ends with CRLF CRLF
		//get here if one of the two functions succeded.
		answer_status = ANSWER_HEADERS;
		cgi_set_encoding_variables();
		//add either close or keep-alive in the headers, if necessary
		cgi_add_connection_header();
		//updates the header from the cgi output if the "Status: " is present.
		//note: the status code can be updated!
		update_header_containing_status_from_cgi_output();
		//if the status code has been changed, and is now an error:
		if (this->get_status_code() >= 300)
			return (0);
		if (encoding_of_content_from_cgi == NEED_CHUNK)
		{
			//remove the last CRLF
			header_for_client.erase(header_for_client.size() - 2);
			header_for_client += "Transfer-encoding: chunked" CRLF;
			//readd the last CRLF, to close the body section.
			header_for_client += CRLF;
		}
	}
	return (received_from_cgi_buffer_struct.len);
}


/*
** add the header Connection: close/keep-alive, if necessary in the cgi output
*/
void
Cgi_service::cgi_add_connection_header()
{
	int  pos;
	if (service_thinks_reader_can_be_deleted)
	{
		if ((pos = ci_find_substr(header_for_client, "Connection: close")) == -1)
		{
			header_for_client.erase(header_for_client.size() - 2);
			header_for_client += "Connection: close" CRLF;
			header_for_client += CRLF;
		}
	}
	else if ((pos = ci_find_substr(header_for_client, "Connection: keep-alive")) == -1)
	{ 
		header_for_client.erase(header_for_client.size() - 2);
		header_for_client += "Connection: keep-alive" CRLF;
		header_for_client += CRLF;
	}
}


/*
** This function tries to find the pattern "\r\n\r\n" into the reading buffer.
** If it finds it:
** 1) it will separate the buffer in two: It appends to header_for_client the
** first part (including the "\r\n\r\n")
** 2) It updates the buffer accordingly (memmove the reaming data, and update
** its len).
** 3) it returns 1.
** Else it returns 0.
**
** RETURN:	1: found the "\r\n\r\n" into the buffer. we can change the
**				answer_status to ANSWER_HEADERS etc...
**			0: need further investigations.
*/

int
Cgi_service::cgi_first_try_to_find_double_crlf_and_update(void)
{
	char *ptr = ft_strnstr(received_from_cgi_buffer_struct.buffer, CRLF CRLF, \
			received_from_cgi_buffer_struct.len);
	if (ptr == NULL)
		return (0);
	//getting here means we have a macth directly into the buffer.
	ptr += 4; //place pointer past the needdle "\r\n\r\n" in haystack.
	size_t len_head = ptr - received_from_cgi_buffer_struct.buffer; //len of left part.
	header_for_client += std::string(received_from_cgi_buffer_struct.buffer, len_head);
	received_from_cgi_buffer_struct.len -= len_head; //update the buffer
	memmove(received_from_cgi_buffer_struct.buffer, ptr, received_from_cgi_buffer_struct.len);
	return (1);
}

/*
** This function is called if the buffer did not contain the needle "\r\n\r\n".
** It will first concatenate the potentially existing header_for_client (string
** representing the http status line and the headers, up to the CRLF CRLF), with
** the content of the buffer. Then as a safety net it rechecks into the string
** if the needle "\r\n\r\n" did not appear (it could have been split in two in
** two successive reads).
**
** RETURN:	1: found the "\r\n\r\n" into the buffer. we can change the
**				answer_status to ANSWER_HEADERS etc...
**			0: need further investigations.
*/
int
Cgi_service::cgi_second_try_to_find_double_crlf_and_update(void)
{
	size_t previous_header_len = header_for_client.length();
	header_for_client.append(received_from_cgi_buffer_struct.buffer, received_from_cgi_buffer_struct.len);
	std::size_t pos;
	if ((pos = header_for_client.find(CRLF CRLF)) == header_for_client.npos)
	{ //not found, we consider the whole content was header material.
		received_from_cgi_buffer_struct.len = 0;
		return (0);
	}
	//getting here means we found the pattern. It also means we need to
	//truncate whatever is beyond the match from our headers. And we will set
	//the buffer content appropriately(shift + update its length).
	header_for_client.erase(pos + 4); //skipping the two CRLF ans discarding.
	int size_shift = header_for_client.length() - previous_header_len;
	received_from_cgi_buffer_struct.len -= size_shift;
	memmove(received_from_cgi_buffer_struct.buffer, received_from_cgi_buffer_struct.buffer \
			+ size_shift, received_from_cgi_buffer_struct.len);
	return (1);
}

/*
** This function is in charge of seting up the encoding related variables:
** - encoding: either KNOWN_LENGTH, or NEED_CHUNK
** - content_length: set to the value found if the "Location: " header is there
** - total_body_size: set to the remaining len of read_buff_struct.buffer if
** 	encoding is KNOWN_LENGTH.
*/
void
Cgi_service::cgi_set_encoding_variables()
{
	int  pos;
	total_body_size = received_from_cgi_buffer_struct.len;
	if ((pos = ci_find_substr(header_for_client, "Content-Length: ")) != -1)
	{
		encoding_of_content_from_cgi = KNOWN_LENGTH;
		content_length = atoi(header_for_client.c_str() + pos + sizeof("Content-Length: ") - 1);
	}
	else if ((pos = ci_find_substr(header_for_client, "transfer-encoding: ")) != -1)
	{ //we decide the encoding is already done. we will just wait for EOF.
		encoding_of_content_from_cgi = ALREADY_CHUNKED;
	}
	else
	{
		encoding_of_content_from_cgi = NEED_CHUNK;
	}
}

/*
** This function tries to see if the reading from the cgi pipe is over or not.
** If this is the case, the received_from_cgi_buffer_struct.status is set to READING_DONE,
** and the fd is push onto the list list_fd_done_reading.
**
** NOTE: in the current implementation, we might encounter some trailer to the
**	body  message part, but we discard them.
**
** PARAMETER:
** - read_res: the returned value by read. If it is zero, it means EOF.
**
** RETURN:	1: done reading (we met EOF for NEED_CHUNK, or ALREADY_CHUNKED,
**				or reached content_length)
** 			0: not done reading yet
*/

int
Cgi_service::is_reading_from_cgi_done(int read_res)
{

	if (encoding_of_content_from_cgi == KNOWN_LENGTH)
	{
		//erasing trailers if present, fuck it.
		if (total_body_size >= content_length)
		{
			received_from_cgi_buffer_struct.len -= (total_body_size - content_length);
			received_from_cgi_buffer_struct.status = READING_DONE;
			//yes means that we already sent our response to client, and the
			//buffer is empty. Since its using content-length encoding, the
			//client must have
			if (received_from_cgi_buffer_struct.len == 0)
			{
				list_fd_done_writing.push_back(get_fd_client());
			}
			return (1);
		}
		return (0);
	}
	else
	{
		//getting here means the encoding is NEED_CHUNK or ALREADY_CHUNKED.
		if (read_res > 0)
			return (0);
		//reached EOF.
		received_from_cgi_buffer_struct.status = READING_DONE;
		return (1);
	}
}

/*
** This function will take care of adding the closing chunk tags to the buffer.
** In the function read_into_received_from_cgi_buffer_struct(), extra space was
** reserved for this. Plus some extra extra space was also reserved for the
** terminating chunk: "0\r\n\r\n".
*/

void
Cgi_service::close_chunk(const int previous_len , const int read_res)
{
	//make sure to not send the final chunk tag if the cgi process is
	//not done talking to us. (read could have return 0, mistaken!)
	if (read_res != 0)
	{
		insert_chunking_tag_in_reading_buffer(previous_len, read_res);
		received_from_cgi_buffer_struct.len += predict_chunk_tag_space(read_res);
	}
	//finally close the chunked message for good
	if (received_from_cgi_buffer_struct.status == READING_DONE)
	{
		//call the same function, with the actual length of the string, and the
		//size of zero. It appends at the end: "0\r\n\r\n".
		insert_chunking_tag_in_reading_buffer(received_from_cgi_buffer_struct.len, 0);
		received_from_cgi_buffer_struct.len += predict_chunk_tag_space(0);
	}
}

/*
** This funciton will insert the chunking tag into the reading buffer. It
** shifts the data contained in the buffer to make space for the first part of
** the chunking, then appends to the buffer's content the final "\r\n".
**
** PARAMETER:
** - previous_len: it is the lenght of string contained into the buffer
**	previously to the read() call.
**
** NOTE: special care is taken, to not shift the whole content of the buffer,
** only the newly read part is shifted. A previously read and tagged piece of
** data is left untouched.
*/
void
Cgi_service::insert_chunking_tag_in_reading_buffer(const int previous_len, \
		const int read_res)
{
	int size_shift  = predict_chunk_tag_space(read_res) - 2;
	memmove(received_from_cgi_buffer_struct.buffer + previous_len + size_shift, \
			received_from_cgi_buffer_struct.buffer + previous_len, read_res);

	//insert at previous_len the first part. fuck it we allocate a string.
	std::string str = get_close_chunk_string(read_res);

	for (size_t i = 0; i < str.length(); i++)
		received_from_cgi_buffer_struct.buffer[previous_len + i] = str.c_str()[i];

	//append trailing "\r\n"
	received_from_cgi_buffer_struct.buffer[previous_len + read_res + size_shift] = '\r';
	received_from_cgi_buffer_struct.buffer[previous_len + read_res + size_shift + 1] = '\n';
}


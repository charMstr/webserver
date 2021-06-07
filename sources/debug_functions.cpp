#include "../includes/debug_functions.hpp"
#include "../includes/server.hpp"
#include "../includes/exceptions.hpp"
#include "../includes/Service.hpp"
#include "../includes/cgi_service.hpp"

/*
** function to be called at the start of a debug function
** parameter: - str: should always be the macro __PRETTY_FINCTION__
*/
void debug_introduce_func(const char *str)
{
	std::cout << std::endl << "\033[33m";
	std::cout << "#############################################################" << std::endl;
	std::cout << " DEBUG on: \033[32m" << str << "\033[33m"<< std::endl;
	std::cout << "#############################################################" << "\033[0m" <<  std::endl;
}

/*
** funciton to be called when a test is done.
*/
void debug_end_func(void)
{
	std::cout << "\033[33m";
	std::cout << "###########################################################"\
		<< "##\033[m" << std::endl << std::endl;
}

/*
** loic's function to check wether parsing was completed properly.
*/

void debug_servers_print(std::vector<t_srv> *servers)
{
	debug_introduce_func(__PRETTY_FUNCTION__);
	for (std::vector<t_srv>::iterator i = servers->begin(); i != servers->end(); i++)
	{
		if (i != servers->begin())
			std::cout  << std::endl;
		debug_server_print(&(*i));
	}
	debug_end_func();
}

/*
** function that debugs one server block pointer.
** it is used in debug_servers_print()
*/

void  debug_server_print(const t_srv * const server)
{
	std::cout << "\033[38;5;9m ⦿ \033[mserver_name: " << server->server_name << std::endl;
	std::cout << "root directory: " << server->directory << std::endl;
	std::cout << "host_port: " << server->host_port << std::endl;
	std::cout << "root: " << server->root << std::endl;
	std::cout << "index: " << server->index << std::endl;
	if (!server->error_page.list_error_status.empty())
		debug_error_page_print(server->error_page);
	std::cout << "autoindex: " << server->autoindex << std::endl;
	std::cout << "cli_max_size: " << server->cli_max_size << std::endl;
	std::bitset<8> x(server->methods.bf);
	std::cout << "allowed methods: " << x << std::endl;
	if (!server->lst_cgi_pass.empty())
		debug_lst_cgi_pass_print(server->lst_cgi_pass);
	for (std::vector<t_loc>::const_iterator j = server->locations.begin(); j != server->locations.end(); j++)
	{
		std::cout << "location: " << std::endl;
		std::cout << "\tpath: " << j->path << std::endl;
		if (j->root != server->root)
			std::cout << "\troot: " << j->root << std::endl;
		if (j->index != server->index)
			std::cout << "\tindex: " << j->index << std::endl;
		if (j->autoindex != server->autoindex)
			std::cout << "\tautoindex: " << j->autoindex << std::endl;
		if (!(j->error_page.list_error_status.empty()) && \
				j->error_page.error_page != server->error_page.error_page)
		{
			std::cout << "\t";
			debug_error_page_print(j->error_page);
		}
		if (!j->lst_cgi_pass.empty())
		{
			std::cout << "\t";	
			debug_lst_cgi_pass_print(j->lst_cgi_pass);
		}
		if (j->cli_max_size != server->cli_max_size)
			std::cout << "\tcli_max_size: " << j->cli_max_size << std::endl;
		if (j->methods.bf != server->methods.bf)
		{
			std::bitset<8> y(j->methods.bf);
			std::cout << "\tallowed methods: " << y << std::endl;
		}
	}
}

/*
** prints content of a list of t_cgi_pass tupple, found in a server block, or
** wihtin a location block.
*/
void	debug_lst_cgi_pass_print(std::list<t_cgi_pass> const &lst_cgi_pass)
{	
	int i = 1;
	for (std::list<t_cgi_pass>::const_iterator it = lst_cgi_pass.begin();
			it != lst_cgi_pass.end(); it++)
	{
		std::cout << "cgi_pass " << i++ << ": ";
		std::cout << "bin = |" << it->cgi_bin;
		std::cout << "| extension = |" << it->extension;
		std::bitset<8> x(it->methods.bf);
		std::cout << "| METHODS = |" << x << "|" << std::endl;
	}
}


/*
** this function will specifically debug a error_page structure within a server
** block or its location block.
*/
void  debug_error_page_print(t_error_page const &error_page)
{
	std::cout << "error_page: " << error_page.error_page << "(";
	for (std::list<int>::const_iterator it = \
			error_page.list_error_status.begin(); \
			it != error_page.list_error_status.end(); it++)
	{
		if (it != error_page.list_error_status.begin())
			std::cout << ", ";
		std::cout << *it;
	}
	std::cout << ")" << std::endl;
}

void debug_enter_func(const char *s)
{
	std::cout << std::endl << "➡️  \033[32mENTERING \033[m" << s << " ..." << std::endl;
}

void debug_exit_func(const char *s)
{
	std::cout  << "➡️  \033[31mEXITING  \033[m" << s << " !" << std::endl;
}

/*
** DEBUG FUNCTION: displays each member of the vector pairs_fd_listen_t_srv:
** - listen_fd
** - t_srv structure corresponding to a virtual_server.
*/
void 	debug_vector_listen(const std::vector<t_pair_fd_listen_t_srv> &vector_listen, const t_fd_set_real &fd_set_real) //OK
{
	debug_introduce_func("vector containing pairs: fd_listen && virtual_servers");
	std::cout << "the number of virtual servers is: " <<  vector_listen.size() << std::endl;
	std::cout << "the number of recorded fds is: " << \
		fd_set_real.fd_mixed_list_write.size() + fd_set_real.fd_mixed_list_read.size()  << std::endl;
	for (std::vector<t_pair_fd_listen_t_srv>::const_iterator it = vector_listen.begin(); \
			it != vector_listen.end(); it++)
	{
		std::cout << "	- fd_listen = "<< it->fd_listen;
		std::cout << " and server_name is: " << it->v_server.server_name \
			<< std::endl;
	}
	int highest_fd = fd_set_real.fd_mixed_list_write.front();
	if (fd_set_real.fd_mixed_list_read.front() \
			> fd_set_real.fd_mixed_list_write.front())
		highest_fd = fd_set_real.fd_mixed_list_read.front();
	std::cout << "note: first fd of the mixed list + 1 = " << highest_fd + 1 << std::endl;
	std::cout << std::endl;
	debug_end_func();
}

/*
** prints out which fd_set was activated after a select call
*/
void	debug_print_which_fdsets_are_active(int fd,
			bool issetread, bool issetwrite, bool issetexcept)
{
	debug_introduce_func("Print which fd_sets are active:");
	std::cout << "Current fd being checked : " << fd << "\n";
	std::cout << "Is in fd_set_copy.fd_set_read : ";
	if (issetread) { std::cout << "yes\n"; } else { std::cout << "no\n"; }
	std::cout << "Is in fd_set_copy.fd_set_write : ";
	if (issetwrite) { std::cout << "yes\n"; } else { std::cout << "no\n"; }
	std::cout << "Is in fd_set_copy.fd_set_except : ";
	if (issetexcept) { std::cout << "yes\n"; } else { std::cout << "no\n"; }
	debug_end_func();
}


/*
** used when entering the function service_generator::generate_xxx_request()
*/
void	debug_entering_generate_xxx_service(const char *func_name, int fd, const int &method, const std::string &uri)
{
	std::string message;
	message = std::string("enterring ") + func_name;
	debug_introduce_func(message.c_str());
	std::cout << "fd_client: " << fd << std::endl;
	std::cout << "method: " << debug_return_method_string(method, 0) << std::endl;
	std::cout << "uri: " << uri << std::endl;
	debug_end_func();
}

/*
** used to display a message when a new client connects to a virtual server
*/
void debug_new_incoming_conexion(t_pair_fd_listen_t_srv &pair, int fd_client)
{
	debug_introduce_func("new client's conexion request");
	std::cout << "from server " << pair.v_server.server_name << " (" << \
		"fd_listen = " << pair.fd_listen << ")" << std::endl;
	std::cout << "new request_reader created (fd_client= " << fd_client << ")" \
		<< std::endl;
	debug_end_func();
}

/*
** This function will be called when exiting the function
** Request_reader::resume_processes_requests_from_client(). It displays the
** current request_readers objects in the list_requests.
*/

void debug_exiting_resume_processed_requests_from_clients(const std::list<Request_reader> &list_req)
{
	debug_introduce_func("list_request_readers");
	std::cout << "current request_readers objects being processed: " << \
		list_req.size() << std::endl;
	for (std::list<Request_reader>::const_iterator it = list_req.begin(); \
		it != list_req.end() ; it++)
	{
		std::cout << "	- fd_client = " << it->get_client_fd() << std::endl;
	}
	debug_end_func();
}

/*
** to peek at the headers parsed when creating a service in Service_generator
*/

void	debug_print_reader_headers(std::map<std::string, std::string> headers,\
		const int fd_client)
{
	debug_introduce_func("generate_get_service : parser.headers");
	std::cout << "fd_client: " << fd_client << std::endl;
	std::cout << "============" <<  std::endl;
	for (std::map<std::string, std::string>::const_iterator i = headers.begin();
			i != headers.end(); ++i)
	{
		std::cout << i->first << ": " << i->second << "\n";
	}
	debug_end_func();
}

/*
** this function is called inside the request_reader_object, it tells us on
** which fd we are listening, and it displays the "raw_request" so far.
*/

void debug_request_read_raw_request_so_far(const int &fd_client, \
		const std::string &raw_request, const int done_reading)
{
	debug_introduce_func("Request_reader: raw_request so far");
	std::cout << "fd_client = " << fd_client << ":" << std::endl;
	std::cout << "\e[38;5;121m" << raw_request << "\e[0m";
	std::cout << "DONE READING? ";
	if (done_reading)
		std::cout << "\033[32mYES";
	else
		std::cout << "\033[31mNO";
	std::cout << "\033[m" << std::endl;
	debug_end_func();
}

/*
** This function is called right after select and it will call 3 subfunctions
** that will loop over 3 different containers: vector_listen,
** list_request_readers, list_services. For each of thoses containers, it will
** tell us which fd is present, and if they were kept if the fd_set_copy or not
** after select().
*/

void debug_after_select_which_fd_is_set( \
		const std::vector<t_pair_fd_listen_t_srv> &vector_listen, \
		const std::list<Request_reader> &list_request_readers, \
		const std::list<Service*> &list_services, \
		const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except)
{
	debug_introduce_func("FILE DESCRIPTORS MONITORED");
	debug_after_select_which_fd_is_set_from_vector_listen(vector_listen, \
			fd_set_read, fd_set_write, fd_set_except);
	debug_after_select_which_fd_is_set_from_list_request_readers( \
			list_request_readers, fd_set_read, fd_set_write, fd_set_except);
	debug_after_select_which_fd_is_set_from_list_services(list_services, \
			fd_set_read, fd_set_write, fd_set_except);
	debug_end_func();
}

/*
** This function is called right after select and it tells us which fd are
** in vector_listen and if yes or no they are ready for non blocking I/O
** operations.
*/

void debug_after_select_which_fd_is_set_from_list_request_readers( \
	const std::list<Request_reader> &list_requests, \
	const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except)
{
	std::cout << "list_request_readers: ";
	if (list_requests.empty())
	{
		std::cout << "EMPTY!" << std::endl << std::endl;;
		return ;
	}
	std::cout << std::endl;
	std::cout << "\033[38;5;33m-------------------------------------------------------------" << std::endl;
	std::cout << "|\033[0m  fd number   \033[38;5;33m|\033[0m  need READ   \033[38;5;33m|\033[0m  need WRITE  \033[38;5;33m|\033[0m  need EXCEPT \033[38;5;33m|" << std::endl;
	std::cout << "-------------------------------------------------------------" << std::endl;
	for (std::list<Request_reader>::const_iterator it = \
			list_requests.begin(); it != list_requests.end(); it++)
	{
		std::string number;
  		std::stringstream ss;
  		ss << it->get_client_fd();
  		ss >> number;
		std::string spaces(8 - number.size(), ' ');
		std::cout <<  "|\033[m      " << number << spaces << "\033[38;5;33m|";
		if (FD_ISSET(it->get_client_fd(), &fd_set_read))
			std::cout << "      ✅      \033[38;5;33m|" ;
		else
			std::cout << "      ❌      \033[38;5;33m|" ;
		(void)(fd_set_write);
	//	if (FD_ISSET(it->get_client_fd(), &fd_set_write))
	//		std::cout << "      ✅      \033[38;5;33m|" ;
	//	else
		std::cout << "      ❌      \033[38;5;33m|" ;
		if (FD_ISSET(it->get_client_fd(), &fd_set_except))
			std::cout << "      ✅      \033[38;5;33m|" << std::endl;
		else
			std::cout << "      ❌      \033[38;5;33m|" << std::endl;
	}
	std::cout << "-------------------------------------------------------------\033[m" << std::endl << std::endl;;
}

/*
** This function is called right after select and it tells us which fd are
** in vector_listen and if yes or no they are ready for non blocking I/O
** operations.
*/

void debug_after_select_which_fd_is_set_from_vector_listen( \
	const std::vector<t_pair_fd_listen_t_srv> &vector_listen, \
	const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except)
{
	std::cout << "vector_listen: ";
	if (vector_listen.empty())
	{
		std::cout << "EMPTY!" << std::endl << std::endl;;
		return ;
	}
	std::cout << std::endl;
	std::cout << "\033[38;5;33m-------------------------------------------------------------" << std::endl;
	std::cout << "|\033[0m  fd number   \033[38;5;33m|\033[0m  need READ   \033[38;5;33m|\033[0m  need WRITE  \033[38;5;33m|\033[0m  need EXCEPT \033[38;5;33m|" << std::endl;
	std::cout << "-------------------------------------------------------------" << std::endl;
	for (std::vector<t_pair_fd_listen_t_srv>::const_iterator it = vector_listen.begin(); \
			it != vector_listen.end(); it++)
	{
		std::string number;
  		std::stringstream ss;
  		ss << it->fd_listen;
  		ss >> number;
		std::string spaces(8 - number.size(), ' ');
		std::cout <<  "|\033[m      " << number << spaces << "\033[38;5;33m|";
		if (FD_ISSET(it->fd_listen, &fd_set_read))
			std::cout << "      ✅      \033[38;5;33m|" ;
		else
			std::cout << "      ❌      \033[38;5;33m|" ;
		if (FD_ISSET(it->fd_listen, &fd_set_write))
			std::cout << "      ✅      \033[38;5;33m|" ;
		else
			std::cout << "      ❌      \033[38;5;33m|" ;
		if (FD_ISSET(it->fd_listen, &fd_set_except))
			std::cout << "      ✅      \033[38;5;33m|" << std::endl;
		else
			std::cout << "      ❌      \033[38;5;33m|" << std::endl;
	}
	std::cout << "-------------------------------------------------------------\033[m" << std::endl << std::endl;;
}

/*
** This function is called right after select and it tells us which fd are
** part of the list_services list, and if yes or no they are ready for non
** blocking I/O operations.
*/
void debug_after_select_which_fd_is_set_from_list_services( \
	const std::list<Service *> &list_services, \
	const fd_set &fd_set_read, const fd_set &fd_set_write, \
		const fd_set &fd_set_except)
{
	std::cout << "list_services: ";
	if (list_services.empty())
	{
		std::cout << "EMPTY!" << std::endl;
		return ;
	}
	std::cout << std::endl;
	std::cout << "\033[38;5;33m-------------------------------------------------------------" << std::endl;
	std::cout << "|\033[0m  fd number   \033[38;5;33m|\033[0m  need READ   \033[38;5;33m|\033[0m  need WRITE  \033[38;5;33m|\033[0m  need EXCEPT \033[38;5;33m|" << std::endl;
	std::cout << "-------------------------------------------------------------" << std::endl;
	for (std::list<Service*>::const_iterator it = list_services.begin(); \
			it != list_services.end(); it++)
	{
		for (std::list<int>::const_iterator it2 = (*it)->list_fd_read.begin();
				it2 != (*it)->list_fd_read.end(); it2++)
		{
			std::string number;
			std::stringstream ss;
			ss << *it2;
			ss >> number;
			std::string spaces(8 - number.size(), ' ');
			std::cout <<  "|\033[m      " << number << spaces << "\033[38;5;33m|";
			if (FD_ISSET(*it2, &fd_set_read))
				std::cout << "      ✅      \033[38;5;33m|" ;
			else
				std::cout << "      ❌      \033[38;5;33m|" ;
			/*
			if (FD_ISSET(*it2, &fd_set_write))
				std::cout << "      ✅      \033[38;5;33m|" ;
			else
				*/
			std::cout << "      ❌      \033[38;5;33m|" ;
			if (FD_ISSET(*it2, &fd_set_except))
				std::cout << "      ✅      \033[38;5;33m|" << std::endl;
			else
				std::cout << "      ❌      \033[38;5;33m|" << std::endl;
		}
		for (std::list<int>::const_iterator it2 = (*it)->list_fd_write.begin();
				it2 != (*it)->list_fd_write.end(); it2++)
		{
			std::string number;
			std::stringstream ss;
			ss << *it2;
			ss >> number;
			std::string spaces(8 - number.size(), ' ');
			std::cout <<  "|\033[m      " << number << spaces << "\033[38;5;33m|";
			/*
			if (FD_ISSET(*it2, &fd_set_read))
				std::cout << "      ✅      \033[38;5;33m|" ;
			else
				*/
			std::cout << "      ❌      \033[38;5;33m|" ;
			if (FD_ISSET(*it2, &fd_set_write))
				std::cout << "      ✅      \033[38;5;33m|" ;
			else
				std::cout << "      ❌      \033[38;5;33m|" ;
			if (FD_ISSET(*it2, &fd_set_except))
				std::cout << "      ✅      \033[38;5;33m|" << std::endl;
			else
				std::cout << "      ❌      \033[38;5;33m|" << std::endl;
		}
	}
	std::cout << "-------------------------------------------------------------\033[m" << std::endl;
}

/*
** this function will debug the object of type Request_parser, It this called
** when a request_reader is done reading, and before we create an object of
** type Service*.
*/
void	debug_request_parser(const Request_parser &parser, const std::map<std::string, std::string> &headers)
{
	debug_introduce_func("request parser");
	std::cout << "method: " << parser.get_method() << std::endl;
	std::cout << "uri: " << parser.get_uri() << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
	{
		std::cout << it->first << " --> \033[38;5;121m" ;
		std::cout << it->second << "\e[m" << std::endl;
	}
	std::cout << "request_buffer " << " --> \033[38;5;121m" ;
	std::cout << parser.get_request_buffer() << "\e[m" << std::endl;
	debug_end_func();
}

/*
** this function is called to display a message that easy to spot on the screen
** when we finally start our server after all the setup
*/
void	debug_start_server(void)
{
	std::cout << \
"  _      _________  ___________ _   __  _____________  ______" << std::endl << \
" | | /| / / __/ _ )/ __/ __/ _ \\ | / / / __/_  __/ _ \\/_  __/" << std::endl << \
" | |/ |/ / _// _  |\\ \\/ _// , _/ |/ / _\\ \\  / / / , _/ / /" << std::endl << \
" |__/|__/___/____/___/___/_/|_||___/ /___/ /_/ /_/|_| /_/" << std::endl << \
	std::endl;
}

void	debug_print_fileStat(const struct stat * fileStat, const int fd_client)
{
	debug_introduce_func("print filestat");
	std::cout << "fd_client: " << fd_client << std::endl;
printf("File Size: \t\t%lld bytes\n", fileStat->st_size);
printf("Number of Links: \t%d\n", fileStat->st_nlink);
printf("File Permissions: \t");
printf( (S_ISDIR(fileStat->st_mode)) ? "d" : "-");
printf( (fileStat->st_mode & S_IRUSR) ? "r" : "-");
printf( (fileStat->st_mode & S_IWUSR) ? "w" : "-");
	printf( (fileStat->st_mode & S_IXUSR) ? "x" : "-");
	printf( (fileStat->st_mode & S_IRGRP) ? "r" : "-");
	printf( (fileStat->st_mode & S_IWGRP) ? "w" : "-");
	printf( (fileStat->st_mode & S_IXGRP) ? "x" : "-");
	printf( (fileStat->st_mode & S_IROTH) ? "r" : "-");
	printf( (fileStat->st_mode & S_IWOTH) ? "w" : "-");
	printf( (fileStat->st_mode & S_IXOTH) ? "x" : "-");
	std::cout << std::endl;
	debug_end_func();
}

/*
** this funcition is called when we are done creating a Service* object.
** it displays the fd it contains in list_fd_read and list_fd_write. Those fd
** are added to Server::fd_set_real structure.
*/
void	debug_from_service_add_fd_to_real_set(const int fd_client, \
		const std::list<int> &list_write, const std::list<int> &list_read)
{
	debug_introduce_func("from new Service object, add fd to fd_real_set");
	for (std::list<int>::const_iterator it = list_read.begin(); \
			it != list_read.end(); it++)
	{
		std::cout << "added fd_read (fd = " << *it << ")" << std::endl;
	}
	for (std::list<int>::const_iterator it = list_write.begin(); \
			it != list_write.end(); it++)
	{
		if (*it == fd_client)
			std::cout << "added fd_client (fd = " << fd_client << ")" << std::endl;
		else
			std::cout << "added fd_write (fd = " << *it << ")" << std::endl;
	}
	debug_end_func();
}

/*
** parameter:
** -	title: the title of the test, displayed on terminal
** - 	fd: the fd which is concerned, that represents the object
** -	debug: the std::string we are debuging. it could be body, or payload...
*/

void	debug_string(const char *title, int fd, std::string const &debug)
{
	debug_introduce_func(title);
	std::cout << "fd_client: " << fd  << std::endl;
	std::cout << "============" <<  std::endl;
	std::cout << "|\033[38;5;121m" << debug  << "\033[0m|" << std::endl;
	debug_end_func();
}

/*
** this function will debug the t_builtin_msg structure that is member
** variable of the Service class. If holds a static constant string pointer,
** and the lenght of this string. Both selected after we used the current
** status_code(error code) as parameter. for body.
*/

void debug_t_hardcoded_str_body(const t_hardcoded_str *msg_handle)
{
	debug_introduce_func("builtin_body_to_use in Service::");
	std::cout << 
	"we are trying to answer with any error builtin page" << std::endl;

	std::cout << "body to use:" << std::endl;
	std::cout << "\033[38;5;121m" << msg_handle->str << "\033[m" << std::endl;
	std::cout << "size: " << msg_handle->len << std::endl;
	debug_end_func();
}

/*
** this function will debug the t_builtin_msg structure that is member
** variable of the Service class. If holds a static constant string pointer,
** and the lenght of this string. Both selected after we used the current
** status_code(error code) as parameter. for status_line
*/

void debug_t_hardcoded_str_status_line(const t_hardcoded_str *msg_handle)
{
	debug_introduce_func("builtin_status_line_to_use in Service::");
	std::cout << "status_line to use:" << std::endl;
	std::cout << "\t\033[38;5;121m" << msg_handle->str << "\033[m" << std::endl;
	std::cout << "size: " << msg_handle->len <<  std::endl;
	debug_end_func();
}

/*
** called when in constructor of error_service objet.
** note: it also give the location block in which we were parsing the uri?
*/

void debug_error_service_creation_context(const t_srv * const server, int status_code)
{
	debug_introduce_func("creation of Error_service object, context given:");
	std::cout << "status code: " << status_code << std::endl;
	std::cout << "server block used: " << std::endl;
	debug_server_print(server);
	debug_end_func();
}

/*
** returns a string depending on the method of the request
*/

std::string debug_return_method_string(int method, int is_cgi)
{
	if (is_cgi)
		return (std::string("CGI"));
	else if (method == OPTIONS)
		return (std::string("OPTIONS"));
	else if (method == GET)
		return (std::string("GET"));
	else if (method == HEAD)
		return (std::string("HEAD"));
	else if (method == POST)
		return (std::string("POST"));
	else if (method == PUT)
		return (std::string("PUT"));
	else if (method == DELETE)
		return (std::string("DELETE"));
	else if (method == TRACE)
		return (std::string("TRACE"));
	else if (method == CONNECT)
		return (std::string("CONNECT"));
	else
		return (std::string("\033[31mUNIDENTIFIED...\033[m"));
}

/*
** this function is called when we arrive in service generator, it helps
** debuging:
** - the uri after parsing,
** - the location block selected,
** - if the method is forbidden in the location_block
** - the root_to_use
** - the resource_path constructed,
** - the resource_path basename
** - the error_page reference chosen
*/

void	debug_service_generator_preliminary_to_service_build(Service_generator &svc_gen, int fd, int method, int is_cgi)
{
	std::string uri = svc_gen.reader.get_uri();
	const t_loc* location = svc_gen.location_block_to_use;
	std::string root = svc_gen.root_to_use;
	t_error_page  error_pages = svc_gen.error_pages_to_use;
	const std::string resource_path = svc_gen.resource_path;

	debug_introduce_func("Service_generator, key data");

	std::cout << "for fd: " << fd << std::endl;
	std::cout << "=============" << std::endl;
	debug_server_print(&(svc_gen.reader.get_server()));
	std::cout << "=============" << std::endl;
	std::cout << "uri: |\033[38;5;121m" << uri << "\033[m|" << std::endl;
	std::cout << "location_block_to_use: ";
	if (location)
		std::cout << "yes\nlocation->path = |\033[38;5;121m" << location->path << \
			"\033[m|" << std::endl;
	else
		std::cout << "\033[31mno\033[m" << std::endl;
	std::cout << "method: " << debug_return_method_string(method, is_cgi) \
		<< std::endl;
	if (svc_gen.is_method_not_allowed(location, method))
	{
		std::cout << "method forbiden: \033[31mYES\033[m" << std::endl;
			return ;
	}
	std::cout << "method forbiden: \033[32mNO\033[m" << std::endl;
	std::cout << "root_to_use: |\033[38;5;121m" << root << \
		"\033[m|" << std::endl;
	std::cout << "resource_path: |\033[38;5;121m" << resource_path << \
		"\033[m|" << std::endl;
	std::cout << "resource_basename: |\033[38;5;121m" << \
		svc_gen.resource_basename <<  "\033[m|" << std::endl;
	std::cout << "error_pages_to_use: ";
	if (error_pages.error_page.empty())
		std::cout << "\033[31mnone\033[m" << std::endl;
	else
	{
		std::cout << "\033[38;5;121m";
		debug_error_page_print(error_pages);
		std::cout << "\033[m";
	}
	debug_end_func();
}

/*
** This function is called when we are trying to generate a method that might
** use the autoindex system (like GET). It will display the value of the
** member variables:
** - uri_is_a_directory
** - index_to_use,
** - autoindex_acitvated
** - status_code when done.
** - answer_buffered_required?
*/

void	debug_generate_get_service_more_key_data(\
			Service_generator &svc_gen, int fd_client)
{
	std::string uri = svc_gen.reader.get_uri();
	const t_loc* location = svc_gen.location_block_to_use;

	debug_introduce_func("generating get service, more key data");
	std::cout << "fd_client: " << fd_client << std::endl;
	std::cout << "============" <<  std::endl;
	debug_server_print(&(svc_gen.reader.get_server()));
	std::cout << "=============" << std::endl;
	std::cout << "location_block_to_use: ";
	if (location)
		std::cout << "\033[32myes\033[m\nlocation->path = |\033[38;5;121m" \
			<< location->path << "\033[m|" << std::endl;
	else
		std::cout << "\033[31mno\033[m" << std::endl;
	std::cout << "uri: |\033[38;5;121m" << uri << "\033[m|" << std::endl;
	std::cout << "uri_is_a_directory: ";
	if (svc_gen.uri_is_a_directory)
		std::cout << "\033[32myes\033[m" << std::endl;
	else
	{
		std::cout << "\033[31mno\033[m" << std::endl;
		return ;
	}
	std::cout << "resource_basename: |\033[38;5;121m" << \
		svc_gen.resource_basename <<  "\033[m|" << std::endl;
	std::cout << "index_to_use: |\033[38;5;121m" << svc_gen.index_to_use \
		<<  "\033[m|" <<  std::endl;
	std::cout << "resource_path: |\033[38;5;121m" << svc_gen.resource_path << \
		"\033[m|" << std::endl;
	std::cout << "autoindex_activated: ";
	if (svc_gen.autoindex_activated)
		std::cout << "\033[32myes\033[m" << std::endl;
	else
		std::cout << "\033[31mno\033[m" << std::endl;
	debug_end_func();
}

/*
** function that displays a message just before we write our error_answer to
** client
*/
void 		debug_error_occured(int fd, int status_code)
{
	debug_introduce_func("ERROR OCCURED, about to write");
	std::cout << "fd_client: " << fd << std::endl;
	std::cout << "============" <<  std::endl;
	std::stringstream ss;
	ss << status_code;
	std::cout << "error_status_code = " << ss.str();
	if (status_code == 302)
		std::cout << "(REDIRECT)" << std::endl;
	else
		std::cout << "(HARDCODED BODY)" << std::endl;
	debug_end_func();
}

/*
** this function debugs the header_for_client member variable of any derivated
** object of service class.
*/

void debug_header_for_client(const std::string headers, const int fd)
{
	debug_introduce_func("HEADER_FOR_CLIENT after set_up");
	std::cout << "fd_client: " << fd << std::endl;
	std::cout << "============" <<  std::endl;
	std::cout << "|\033[38;5;121m" << headers << "\033[m|" << std::endl;
	debug_end_func();
}

/*
** this function will display the cgi service's key data
*/
void debug_cgi_svc(const Cgi_service *cgi, bool display_argv_envp)
{
	debug_introduce_func("Cgi_service: key variables");	

	std::cout << "script_name: |\033[38;5;121m" << cgi->reader->get_cgi_bin_name() << "\033[m|" << std::endl;
	std::cout << "answer_status: |\033[38;5;121m" << cgi->answer_status << "\033[m|" << std::endl;
	std::cout << "cgi_pid: |\033[38;5;121m" << cgi->get_cgi_pid() << "\033[m|" << std::endl;
	std::cout << "webserv_writes_on_me: |\033[38;5;121m" << cgi->get_webserv_writes_on_me() << "\033[m|" << std::endl;
	std::cout << "webserv_reads_on_me: |\033[38;5;121m" << cgi->get_webserv_reads_on_me() << "\033[m|" << std::endl;

	/* code below commented out because it segfaults */

	(void)(display_argv_envp);
//	std::cout << "argc: |\033[38;5;121m" << cgi->argc << "\033[m|" << std::endl;
//	for (int i = 0; display_argv_envp && i < cgi->argc; i++)
//		std::cout << "\t|\033[38;5;121m" << cgi->get_argv()[i] << "\033[m|" << std::endl;
//	std::cout << "env_size: |\033[38;5;121m" << cgi->env_size << "\033[m|" << std::endl;
//	for (int i = 0; display_argv_envp && i < cgi->env_size; i++)
//		std::cout << "\t|\033[38;5;121m" << cgi->get_envp()[i] << "\033[m|" << std::endl;
	debug_end_func();
}

//display a line when deleting something related to last param fd.
void 		debug_deletion(const char *what, const char *file, int line, int fd)
{
	std::cout << "  ❌ for fd = " << fd <<", discarding:"  << std::endl;
	std::cout << "    \033[31m" << what << "\033[m (file: " <<file<<\
		"; line: "<<line<< ") ❌" << std::endl << std::endl;
}

//display a line, when adding something related to last param fd.
void		debug_addition(const char *what, const char *file, int line, int fd)
{
	std::cout << " \033[42m＋\033[0m for fd = " << fd <<", adding:"  << std::endl;
	std::cout << "    \033[32m" << what << "\033[m (file: " <<file<<\
		"; line: "<<line<< ") \033[42m＋\033[0m" << std::endl << std::endl;
}

void	debug_request_reader(const Request_reader &reader)
{
	debug_introduce_func("Request_reader, just before deciding to generate service");	
	std::cout << "fd_client: " << reader.get_client_fd() << std::endl;
	std::cout << "============" <<  std::endl;
	std::string state;
	if (reader.get_request_state() == 0)
		state = "REQUEST_LINE";
	if (reader.get_request_state() == 1)
		state = "HEADERS";
	if (reader.get_request_state() == 2)
		state = "BODY";
	if (reader.get_request_state() == 3)
		state = "DONE";
	std::cout << "request_state: |\033[38;5;121m" << state << "\033[m|" << std::endl;
	std::cout << "status_code: |\033[38;5;121m" << reader.get_status() << "\033[m|" << std::endl;
	debug_end_func();
}

void	debug_timeout_reset(int fd)
{	
	std::string number;
	std::stringstream ss;
	ss << fd;
	ss >> number;
	std::string str = std::string("RESET TIMEOUT ⏰ for fd = ") + number;
	debug_introduce_func(str.c_str());
}

void	debug_timeout_occured(int fd)
{
	std::string number;
	std::stringstream ss;
	ss << fd;
	ss >> number;
	std::string str = std::string("⏰ TIMEOUT ⏰ OCCURED ⏰ for fd = ") + number;
	debug_introduce_func(str.c_str());
}

/*
** debugs the envp array for the CGI env variables
*/
void debug_envp_or_argv(char **envp, const char *name)
{
	debug_introduce_func(name);	
	int i;
	for (i = 0; envp[i] ; i++)
	{
		std::cout << "|\033[35m";
		std::cout << envp[i] << "\033[0m|";
 		std::cout << " (i = "<< i << ")" << std::endl;
	}
	std::cout << "index of NULL = " << i << std::endl;
	debug_end_func();
}

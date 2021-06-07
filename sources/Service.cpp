
#include "../includes/Service.hpp"
#include "../includes/Error_service.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/Services_hardcoded_body.hpp"
#include "../includes/Services_hardcoded_status_line.hpp"
#include "../includes/exceptions.hpp"
#include <unistd.h>

/*
** ABSTRACT class.
** See Service.hpp for details of the implementation, especially handling of
** file descriptors.
*/

/*
** constructor: Any object derivated from the service class will have its
** fd_client member variable set and the same fd is added to
** Service::list_fd_write.
**
** note: This fd_client is very specific (its the fd representing the conexion
** socket and the object of type Service* did not open it, so its not its
** responsability to close it). Plus we need it to address a basic answer to
** the client if a no_more_alloc occured for example.
*/

Service::Service(const t_client_info client_info, t_error_page error_page, \
		std::string basename, Request_reader *reader) :
	just_head_no_body(false),
//set to true if the service realise it will be the last exchange with client.
	service_thinks_reader_can_be_deleted(false),
	reader(reader),
	client_info(client_info),
	error_page_to_use(error_page),
	resource_basename(basename),
	status_code(200),
	error_occured(false),
	redirect_to(),
	answer_status(ANSWER_HEADERS),
	header_for_client(),
	answer_error_struct()
{
	does_service_thinks_reader_can_be_delete();
	list_fd_write.push_back(client_info.fd);
	return ;
}

/*
** destructor: this destructor will close the opened filedesctiptors that our
** object has strictly opened itself. It means all the fds in the fd_list_read,
** fd_list_write, but not fd_client.
**
** NOTE: Unless we are shutting down our server brutally, the lists
**	list_fd_read and lists_fd_write should be empty when we are done with the
**	object, as we removed the fds from them each time we were reading/writing.
**	the only fd remaining in Service::list_fd_write would be Service::fd_client
*/
Service::~Service(void)
{
	close_all_fds_except_for_fd_client();
}

/*
** function checks if the current service should be the last one for a given
** request_reader. if yes the bool service_thinks_reader_can_be_delete is set
** to true. And if this condition is met, the header "connection: close" should
** be added in the response to client.
*/

void
Service::does_service_thinks_reader_can_be_delete(void)
{
	//if the request was ill-formed (potentially malicious client)
	if (reader->ill_formed_request_so_no_more_reading)
		service_thinks_reader_can_be_deleted = true;

	//if timeout was detected in the loop managing the reader's life.
	if (reader->get_status() == 408)
		service_thinks_reader_can_be_deleted = true;

	//if max request_number on same socket is reached.
	reader->request_counter_for_keep_alive_minus_one();
	if (reader->get_request_counter_for_keep_alive_done())
		service_thinks_reader_can_be_deleted = true;

	//check if there is a "close" asked explicitely by client
	if (reader->get_header("connection") == "close")
		service_thinks_reader_can_be_deleted = true;
}

/*
** This function will search in the linked list list_fd_done_writing and try to
** find a given fd.
**
** note: fd are added to list_fd_done_writing by calls to write_svc(), when the
** end of the I/O are reached on those filedescriptors. But also if something
** fails in the Service object, and we decided to close all fd at once (except
** for a few exceptions like fd_client...).
**
** This function is called in Server:: scope, so that we know we can remove a
** fd from fd_set_real.fd_set_write
**
** RETURN: true, if we are done with a given fd.
** 			false, if we are not done with a given fd.
*/

bool
Service::is_done_writing_with_fd(int fd)
{
	for (std::list<int>::iterator it = list_fd_done_writing.begin(); \
			it != list_fd_done_writing.end(); it++)
	{
		if (fd == *it)
			return (true);
	}
	return (false);
}

/*
** This function will search in the linked list list_fd_done_reading and try to
** find a given fd. HOMOLOGOUS to is_done_writing_with_fd()
**
** note: fd are added to list_fd_done_reading by calls to read_svc(), when the
** end of the I/O are reached on those filedescriptors. But also if something
** fails in the Service object, and we decided to close all fd at once (except
** for a few exceptions like fd_client...).
**
** This function is called in Server:: scope, so that we know we can remove a
** fd from fd_set_real.fd_set_read
**
** RETURN: true, if we are done with a given fd.
** 			false, if we are not done with a given fd.
*/

bool
Service::is_done_reading_with_fd(int fd)
{
	for (std::list<int>::iterator it = list_fd_done_reading.begin(); \
			it != list_fd_done_reading.end(); it++)
	{
		if (fd == *it)
			return (true);
	}
	return (false);
}

/*
** This function is called when the Service object turns the reader into the
** no_more_alloc handler object. It is also called when the object is destroyed
** in its destructor. It will close all the fd, except for the fd_client which
** is the connection socket, required to send our 500 status code to the client
** later in the code.
*/

void
Service::close_all_fds_except_for_fd_client(void)
{
	for (std::list<int>::iterator fd_it = list_fd_read.begin(); \
		fd_it != list_fd_read.end(); fd_it++)
	{
		//cannot find the fd_client here
		close(*fd_it);
	}
	for (std::list<int>::iterator fd_it = list_fd_write.begin(); \
		fd_it != list_fd_write.end(); fd_it++)
	{
		//close all the fd except for the fd_client
		if (*fd_it != client_info.fd)
			close(*fd_it);
	}
}

/*
** This function will iterate through list_fd_read and add any fd to
** list_fd_done_reading (including fd_client).
** This function will also iterate through list_fd_write and add any fd to
** list_fd_done_writing (except for fd_client, that needs to remain monitored
** by select()).
** This function is called when we have an error in our object of type service,
** and we are outside service_generator. The service has already given all its
** opened fd to Server::fd_set_real.
** So If we get here it means something failed with the object while calling
** its function read_svc() or write_svc(). We cannot afford to
** close all the fd and then remove them from the Service::list_fd_read and
** Service::list_fd_write as we are currently iterating on those list and
** very deeply nested into function calls. We rather close the fd in a separate
** funciton and add them here to list_fd_done_xxx. They will then in the scope
** of Server:: class be remove from Server::fd_set_real.
*/
void
Service::add_to_list_done_all_fds_except_for_fd_client(void)
{
	for (std::list<int>::iterator i = list_fd_read.begin(); \
			i != list_fd_read.end(); ++i)
	{
		list_fd_done_reading.push_back(*i);
	}
	for (std::list<int>::iterator i = list_fd_write.begin(); \
			i != list_fd_write.end(); ++i)
	{
		// skip the fd_client
		if (*i == client_info.fd) { continue ; }
		list_fd_done_writing.push_back(*i);
	}
}

/*
** This function will tell us if there is no more I/O operations to be done
** within an service object.
**
** note: the list_fd_read and list_fd_write initially contain some fd on which
** 		we operate some read/write operations. Each time one is added to the
** 		list list_done_fd, in the server object, those fd are closed and
** 		removed from list_fd_write or list_fd_read.
**
** RETURN:	- true: if there is no more fd in the linked lists list_fd_read and
** 			list_fd_write.
** 			- false: if there is still some fd on which we are either writing
** 			or reading.
*/
bool
Service::is_done_reading_and_writing(void) const
{
	if (list_fd_read.empty() && list_fd_write.empty())
		return (true);
	return (false);
}

/*
** This method will be the interface with the Server object. From within the
** Server:: scope we get here when we know we can read on the fd.
**
** PARAMETERS:
** - fd in which we need to attempt to read
**
** NOTE: it all the subfunctions, the returned value must be handled like so:
** 	Basically always forward read()'s return, unchanged, except for a very
** 	special case. When reading on a pipe, since the fd is set to non blocking,
** 	the return of -1 should not be considered an error, (just a skip from
** 	select). Just update the return value to 0 in that case, to avoid
** 	considering it an error.
**
** NOTE: if we are done reading on that fd, we know it because fd is added to
** 			list_fd_done_reading.
**
** NOTE: when read_svc returned 0, we dont update the timeout_reference
** 		when the return > 0, we do update the timeout_reference.
** 		when the return is -1, we skip the result if it is the fd_client,
** 		otherwise it mease it was on a regular file --> error.
**
** NOTE: Too many returns of 0 or -1, will end up in a timeout for the given
** service.
*/

void
Service::read_svc_interface(int fd)
{
	int res = read_svc(fd);
	if (res > 0)
	{
		//DEBUG debug_timeout_reset(fd);
		if (!reset_timeout_reference())
		{
			error_occured_set_up_everything(500, 0);
			return ;
		}
	}
	//note: make sure a fail when reading on the cgi returns 0 and not -1
	if (res == -1)
	{
		if (get_status_code() >= 300)
			error_occured_set_up_everything(get_status_code(), 0);
		else
			error_occured_set_up_everything(500, 0);
	}
}

/*
** This function is called within the Server:: scope, once whe identified that
** we can actually write into this fd. It will select between 2 situations:
** - If error_occured is true, we know that we are writing to fd_client with
** the usual premade header and from a hardcoded body string.
** - else it means we are just going to call write_svc() for any object derived
** from service class (function is overiden)
**
** PARAMETERS:
** - fd in which we need to attempt to write
**
** NOTE: it all the subfunctions, the returned value must be handled like so:
** - call write, return its returned valued.
** - dont call error_occured_set_up_everything();
** - if done writing, push the fd onto list_fd_done_writing
**
** NOTE: if we are done writing, we know it because fd is added to
** list_fd_done_writing
*/

void
Service::write_svc_interface(int fd)
{
	int res;
	if (error_occured) 
	{ //note: we only get in here if the fd is fd_client
		//DEBUG debug_error_occured(fd, get_status_code());

 //TODO(Ven 04/06/2021 at 09:23:57) 
		// we should send an OOB message on the first write, to tell the client
		// to discard any previous message. (maybe we already sent many bytes).
		res = answer_to_fd_client_when_error_occured();
	}
	else
		res = write_svc(fd);
	if (res == -1)
	{
		if (fd == get_fd_client())
		{ //CLOSE THE CONNEXION TOTALLY, as we cant write no more on fd_client.
			close_all_fds_except_for_fd_client();
			//adding all the fd into list_done_fd... including fd_client.
			add_to_list_done_all_fds_except_for_fd_client();
			list_fd_done_writing.push_back(fd); //just added, not closed.
			//note: the fd_client is closed when the service is destroyed.
		}
		else
			error_occured_set_up_everything(500, 0);
	}
	//updating the time reference only when the write did not iddle to zero.
	else if (res > 0)
	{
		//DEBUG debug_timeout_reset(fd);
		if (!reset_timeout_reference())
			error_occured_set_up_everything(500, 0);
	}
}

/* ************************************************************************** */
/* GETTERS                                                                    */
/* ************************************************************************** */

/*
** This function will fectch for us the right body hardcoded string.
** It gets a structure t_hardcoded_str from a static array.
**
** note: the structure doesnt need to contain the srting's length anymore,
** whatever.
*/
t_hardcoded_str
Service::get_hardcoded_body_to_use()
{
	int status = get_status_code();
	t_hardcoded_str res;

	if (status == 200)
		res = webserv_hardcoded_bodies[status - \
			  WEBSERV_HARDCODED_BODY_OFFSET_200];
	else if ((status >= 301 && status <= 303) || status == 307 \
			|| status == 308)
		res = webserv_hardcoded_bodies[status - \
			  WEBSERV_HARDCODED_BODY_OFFSET_3XX];

	else if ((status >= 400 && status <= 406) || (status >= 408 \
			&& status <= 416) || status == 421 || status == 429)
		res = webserv_hardcoded_bodies[status - \
			  WEBSERV_HARDCODED_BODY_OFFSET_4XX_LOW];
	else if (status >= 494 && status <= 498)
		res = webserv_hardcoded_bodies[status - \
			  WEBSERV_HARDCODED_BODY_OFFSET_4XX_HIGH];
	else if ((status >= 500 && status <= 505) || status == 507)
		res = webserv_hardcoded_bodies[status - \
			  WEBSERV_HARDCODED_BODY_OFFSET_5XX];
	else
	{
		throw exception_webserver(strerror(errno), \
			"Service::get_hardcoded_body_to_use()", __FILE__, __LINE__, \
			"status_code out of range", WORKING);
	}
	//DEBUG_(DEBUG_ERROR_SERVICE) debug_t_hardcoded_str_body(&res);
	return (res);
}

/*
** This function will fectch for us the right status line hardcoded string.
** It gets a structure t_hardcoded_str from a static array.
**
** note: the structure doesnt need to contain the srting's length anymore,
** whatever.
*/

t_hardcoded_str
Service::get_hardcoded_status_line_to_use()
{
	int status = get_status_code();

	t_hardcoded_str res;
	if (status >= 100 && status <= 103)
		res  = webserv_hardcoded_status_lines[status - \
		WEBSERV_HARDCODED_STATUS_OFFSET_100];

	else if ((status >= 200 && status <= 208) || status == 226)
		res = webserv_hardcoded_status_lines[status - \
		WEBSERV_HARDCODED_STATUS_OFFSET_200];

	else if ((status >= 300 && status <= 305) || status == 307 || status == 308)
		res = webserv_hardcoded_status_lines[status - \
		WEBSERV_HARDCODED_STATUS_OFFSET_300];

	else if ((status >= 400 && status <= 418) || (status >= 422 \
		&& status <= 424) || status == 426 || status == 426 || status == 428 \
			|| status == 429 || status == 431 || status == 451)
		res = webserv_hardcoded_status_lines[status - \
		WEBSERV_HARDCODED_STATUS_OFFSET_400];

	else if ((status >= 500 && status <= 508) || status == 510 || status == 511)
		res = webserv_hardcoded_status_lines[status - \
		WEBSERV_HARDCODED_STATUS_OFFSET_500];

	else
	{
		throw exception_webserver(strerror(errno), \
			"Service::get_hardcoded_status_line_to_use()", __FILE__, \
			__LINE__, "status_code out of range", WORKING);
	}
	//DEBUG_(DEBUG_ERROR_SERVICE) debug_t_hardcoded_str_status_line(&res);
	return (res);
}

/*
** get_status_code is set to 200 at the creation of an object of type service.
**
** note: if we are creating an object of type error_service, it is directly
** set to another value (403). It can be set to any value between 300 and 599
** when a object derivated from base class service encounters an error.
*/
int
Service::get_status_code() const
{
	return (status_code);
}

int
Service::get_fd_client() const
{
	return (client_info.fd);
}

/*
** This function will return a string, the error page to use, depending on the
** status code.
**
** RETURN:	- empty string: meands there was no match
** 			- else: the name of the page to use.
*/

std::string
Service::get_error_page_string_from_status_code(void) const
{
	std::list<int>::const_iterator it;
	for (it = error_page_to_use.list_error_status.begin(); \
			it != error_page_to_use.list_error_status.end(); it++)
	{
		if (get_status_code() == *it)
			return (error_page_to_use.error_page);
	}
	return (std::string(""));
}

/*
** GETTER, get the answer_status of the service
*/
Service::e_answer_to_client_status
Service::get_answer_status(void) const
{
	return (answer_status);
}

/* ************************************************************************** */
/* SETTERS                                                                    */
/* ************************************************************************** */

/*
** this member variable will guide us in which constant builtin body we will
** chose when we want to either answer our builtin index page or any builtin
** error page.
** It is also usefull to build our status line in the header.
*/
void
Service::set_status_code(int status_code)
{
	this->status_code = status_code;
}

/* ************************************************************************** */
/* ERROR_HANDLING                                                             */
/* ************************************************************************** */

/*
** This is the main function that is called when an error occured. This can
** happen at two different stages and this is flaged thanks to the second
** parameter:
** a) when we are creating the service object (in class service_generator):
** - call Service::close_all_fds_except_for_fd_client(void);
** - DONT call Service::add_to_list_done_all_fds_except_for_fd_client(void),
** because those fd dont exist to the eyes of the Server:: object yet. It has
** not collected them into its fd_real_set structure...
** - instead call Service::remove_all_fds_in_list_fd_rw_except_for_fd_client();
** b) when the object is already created since a while and has already been
** added to list_services in Server:: scope. (it means the list_fd_read and
** list_fd_write have been iterated upon after service_generator was done, and
** all the fd have been added to Server::fd_set_real structure.)
** - call Service::close_all_fds_except_for_fd_client(void);
** - call Service::add_to_list_done_all_fds_except_for_fd_client(void);
** - DONT call Service::remove_all_fds_in_list_fd_rw_except_for_fd_client();
**
** c) it also called the function that resets the timeout reference, so that
** now our error answer has full time to be sent. And it makes sure the reader
** object pointer has its state set to DONE as it wont be used anymore.
**
** d) THEN it calls the function that will set up the way we are going to
** answer the error_page to client:
** - call function set_up_answer_to_client_when_error_page_required().
**
** Then all we can do is wait for another loop of select, until our fd gets
** noticed.
*/

void
Service::error_occured_set_up_everything(int error_status_code, bool at_generating_service_stage)
{
	//DEBUG_(DEBUG_ERROR_SERVICE) debug_timeout_reset(get_fd_client());
	if (!reset_timeout_reference()) //give a chance to answer in time.
		error_status_code = 500;

	answer_status = ANSWER_HEADERS; //reset status, just in case.
	set_status_code(error_status_code);
	error_occured = true;

	close_all_fds_except_for_fd_client();
	if (at_generating_service_stage == true)
		remove_all_fds_in_list_fd_rw_except_for_fd_client();
	else
		add_to_list_done_all_fds_except_for_fd_client();
	set_up_answer_to_client_when_error_page_required();
	//DEBUG_(DEBUG_ERROR_SERVICE) debug_header_for_client(header_for_client, \
	//		get_fd_client());
}

/*
** This function will build the header section of our answer when an error
** occured. There is two cases:
** - The error_page is set for the given status_code: we will change the status
**	code to 302 redirect, and add the location header.
** - The error_page is emtpy string: no location header, the status code is not
** changed.
**
** NOTE on the 302 redirect: in Nginx congiguration file, If the directive
** error_page has a location path that doesnt start with a forward slash, it
** does respond with a 302 redirect message (location header is set). If the
** location path starts with a '/' it does a sort of internal redirect, only
** replying with the correct status line, but sending the content of the file
** found at location path.
**
** NOTE: in both cases the body is yielded from our set of hardcoded answers.
*/

void
Service::set_up_answer_to_client_when_error_page_required(void)
{
	//we will redirect if an error page is set for the given status code and
	//the redirect_to string was not set yet.
	if (get_redirect_to().empty() && \
			get_error_page_string_from_status_code().length() != 0)
	{
		set_redirect_to(get_error_page_string_from_status_code());
		set_status_code(302);
	}

	//add status line
	try { header_for_client = get_hardcoded_status_line_to_use().str; }
	catch (const exception_webserver &e)
	{
		set_status_code(500); //fall back to 500 if code not hardcoded yet.
		header_for_client = get_hardcoded_status_line_to_use().str;
	}
	//set the body to use.
	try { answer_error_struct.known_body = get_hardcoded_body_to_use(); }
	catch (const exception_webserver &e)
	{
		//use a dummy struct, with length set to zero. no body used!
		answer_error_struct.known_body.str = NULL;
		answer_error_struct.known_body.len = 0;
	}

	header_for_client += "Content-Type: text/html" CRLF;

	//add the date
	header_for_client += "Date: " + get_the_time() + CRLF;

	//add the content-length
	std::stringstream ss;
	ss << answer_error_struct.known_body.len;
	header_for_client += "Content-Length: " + ss.str() + CRLF;

	if (service_thinks_reader_can_be_deleted)
		header_for_client += "Connection: close" CRLF;
	else
		header_for_client += "Connection: keep-alive" CRLF;


	//add location header;
	if (get_redirect_to().length())
		header_for_client += "Location: " + get_redirect_to() + CRLF;

	header_for_client += std::string("Server: Webserver42/1.0") + CRLF;
	//append extra CRLF
	header_for_client += CRLF;
}

/*
** This function is called when we had an error in our service, AND we are
** still in the service_generator function. This means we havent returned our
** service to Server:: scope yet, and all the opened fd have not been added
** to Server::fd_set_real yet. This funtion will clear list_fd_read from all
** fd. and it will also clear list_fd_write BUT LEAVING IN IT FD_CLIENT.
*/
void
Service::remove_all_fds_in_list_fd_rw_except_for_fd_client()
{
	// delete all fds in list_fd_read
	for (std::list<int>::iterator i = list_fd_read.begin(); i != list_fd_read.end(); ++i)
	{
		list_fd_read.erase(i);
	}
	// delete all fds in list_fd_write EXCEPT the fd_client
	for (std::list<int>::iterator i = list_fd_write.begin(); i != list_fd_write.end(); ++i)
	{
		if (*i != client_info.fd)
			{ list_fd_write.erase(i); }
	}
}

/*
** We get here after an error_occured, and the write_svc_interface() function
** notices we had the boolean error_occured raised to true. This function will
** try to answer to fd_client with a hardcoded message.
** First write the headers, which have been set up already. Then we write the
** body.
**
** NOTE: if we are fully done with writing to fd_client, we can add the
** fd_client to list_fd_done_writing (but not close it...).
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
Service::answer_to_fd_client_when_error_occured()
{
	int res = 0;

	if (answer_status == ANSWER_HEADERS)
	{
		if ((res = answer_with_header(answer_status)) == -1)
			return (-1);
		if (answer_status == ANSWER_BODY && just_head_no_body)
			answer_status = ANSWER_DONE;
		else
			return (res);
	}
	if (answer_status == ANSWER_BODY)
	{
		if ((res = answer_error_hardcoded_body()) == -1)
			return (-1);
	}
	if (answer_status == ANSWER_DONE)
		list_fd_done_writing.push_back(get_fd_client());
	return (res);
}

/*
** This function is in charge of writing the "header" (status_line + headers +
** CRLF) to the fd given as parameter
**
** NOTE: the header is already prepared in Service_generator class.
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
Service::answer_with_header(e_answer_to_client_status &status)
{
	int res = write(get_fd_client(), header_for_client.c_str(), \
			header_for_client.length());
	
	if (res < 0)
		return (res);
	header_for_client.erase(0, res);
	if (header_for_client.length() == 0)
		status = ANSWER_BODY;
	return (res);
}

/*
** We get here after function write_interface() notices the boolean
** error_occured was set to true, and the stage in answer is ANSWER_BODY.
** This boolean and a few other things have been set up for us by a previous
** call to error_occured_set_up_everything() and subcall to
** set_up_answer_to_client_when_error_page_required():
** - boolean error_occured is true.
** - stucture answer_error_struct is set and contains:
** 		- known_body;
** 		- position_in_known_body;
** 		note: known_body.str can be set to a NULL pointer.
**
** NOTE: when this function is fully done with writing the message, it sets the
** answer_status to ANSWER_DONE.
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
Service::answer_error_hardcoded_body(void)
{
	int res;

	if (answer_error_struct.known_body.str == NULL)
	{
		answer_status = ANSWER_DONE;
		return (0);
	}

	res = write(get_fd_client(), answer_error_struct.known_body.str + \
			answer_error_struct.position_in_known_body, \
			answer_error_struct.known_body.len - \
			answer_error_struct.position_in_known_body);
	if (res < 0)
		return (res);
	answer_error_struct.position_in_known_body += res;
	if (answer_error_struct.position_in_known_body == \
			answer_error_struct.known_body.len)
		answer_status = ANSWER_DONE;
	return (res);
}

/*
** this function returns the redirect_to string, which holds the value of the
** location header, if it needs to be set in the answer.
*/

const std::string &
Service::get_redirect_to(void) const
{
	return (redirect_to);
}

/*
** setter to redirect_to, the string that is normally empty, but filled if a
** "Location: " header needs to be set in answer
*/

void
Service::set_redirect_to(const std::string location)
{
	redirect_to = location;
}

/*
** Resets the timeout_reference. It is called each time we have had a successful
** call to read() or write() ( >= 0 ).
**
** NOTE: gettimeofday can fail. the status code would be set to 500 in that
** case and the function error_occured_set_up_everything will be called.
**
** RETURN:	true, no problem
** 			false, need error handling
*/

bool
Service::reset_timeout_reference(void)
{
	if (gettimeofday(&(reader->timeout_reference), NULL) == -1)
		return (false);
	return (true);
}

/*
** Useful for server class, when an object has timed out, and it makes sure
** that the error_occured wont stay up for ever.
*/
bool
Service::get_error_occured(void) const
{
	return (error_occured);
}

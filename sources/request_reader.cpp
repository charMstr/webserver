#include "../includes/request_reader.hpp"
#include "../includes/utils.hpp"
#include "../includes/debug_functions.hpp"
#include <iostream>
#include <string>

/* ************************************************************************** */
/*	Constructors and destructors                                              */
/* ************************************************************************** */

Request_reader::Request_reader(const t_client_info &cli, const t_srv &srv) :
	client(cli),
	server(srv),
	ill_formed_request_so_no_more_reading(false),
	service_already_generated(false),
	i_can_be_deleted(false), //default to keep alive
	ptr_service_generated(NULL),
	body_from_client(std::string("")),
	request_counter_for_keep_alive(MAX_REQUEST_NUMBER_FOR_KEEP_ALIVE),
	recv_calls(0),
	current_content_length(0),
	cli_max_size(0), // 0 means unlimited
	request_buffer(""),
	state(REQUEST_LINE),
	parser(Request_parser(request_buffer)),
	status(200),
	object_is_now_no_more_alloc_handler_bool(false),
	position_in_no_more_alloc_answer(0),
	timeout_reference()
{
	 if (gettimeofday(&timeout_reference, NULL) < 0)
	 {
		 status = 500;
	 }
}

//by copy
Request_reader::Request_reader(Request_reader const &src) :
	parser(Request_parser(request_buffer))
{
	*this = src;
	//need to make sure the reference is the right one!
}

Request_reader	&
Request_reader::operator=(Request_reader const &rhs)
{
	//std::cout << "\033[33m'=' operator called for Request_reader\033[0m" << std::endl;
	if (this != &rhs)
	{
		this->client = rhs.client;
		this->server = rhs.server;
		this->ill_formed_request_so_no_more_reading = rhs.ill_formed_request_so_no_more_reading;
		this->service_already_generated = rhs.service_already_generated;
		this->body_from_client = rhs.body_from_client;
		this->ptr_service_generated = rhs.ptr_service_generated;
		this->i_can_be_deleted = rhs.i_can_be_deleted;
		this->request_counter_for_keep_alive = rhs.request_counter_for_keep_alive;
		this->recv_calls = rhs.recv_calls;
		this->current_content_length = rhs.current_content_length;
		this->cli_max_size = rhs.cli_max_size;
		//copy of the string.
		this->request_buffer = rhs.request_buffer;
		this->state = rhs.state;
		this->parser = rhs.parser;
		//making sure the reference always points to the correct request_buffer
		this->parser.request_buffer = this->request_buffer;
		this->status = rhs.status;
		this->object_is_now_no_more_alloc_handler_bool = \
							rhs.object_is_now_no_more_alloc_handler_bool;
		this->position_in_no_more_alloc_answer = \
							rhs.position_in_no_more_alloc_answer;
		this->timeout_reference = rhs.timeout_reference;
	}
	return *this;
}

Request_reader::~Request_reader()
{

}

/* ************************************************************************** */
/*	 member functions                                                         */
/* ************************************************************************** */

/*
** this funciton is called when we want to keep alive the connection with the
** client (using the same fd_client, not closing it). We flush all the member
** variables to initial values, except for the request_counter_for_keep_alive,
** which should not ve touched.
*/

void
Request_reader::flush_reader_for_keep_alive(void)
{
	this->service_already_generated = false;
	this->ptr_service_generated = NULL;
	current_content_length = 0;
	recv_calls = 0;
	state = REQUEST_LINE;
	//create a new parser, but keep the same request_buffer that might already
	//contain the begining of the next request (keep alive).
	parser = Request_parser(request_buffer);
	status = 200;
	cli_max_size = 0;
	object_is_now_no_more_alloc_handler_bool = false;
	if (gettimeofday(&timeout_reference, NULL) < 0)
	{
		 status = 500;
	}
}

/*
** name: set_up_related_to_cgi()
**
** This function will first of all decide if the request is a CGI or not. If
** yes the uri might be modified (see details below) and a few variables are
** set.
**
** It first looks at the method: For a request to be cgi, the method must be
** either GET or POST.
** Then the uri must contain a file name that can trigger a cgi_script. This is
** decided by matching the extension of files in the URI, with some extensions
** found in the configuration file of our server. Those extensions are stored
** in the t_cgi_pass structures. We first try to match the extensions from the
** given location_block_to_use, then try to compare the extensions from the
** server block.
**
** NOTE: if the request is cgi, and the method is get, we will cut the end of
** the uri. Whatever is after the '?' character goes into the query_string
** variable.
**
** NOTE: PATH_INFO is the whole uri. (minus the "?key=value" of course).
**
** PARAMETERS:
** - location_block_to_use: a pointer (can be NULL) to the location block
**	selected. It allows us to find the correct list of t_cgi_pass structures.
**	Each element of those lists has then extension of a file, and the binary we
**	are going to launch for that type of extensions.
**
** ERRORS:
** 	If an error occurs, the status code is set to a value other than 200.
**	Exceptions can be thrown due to string manipulations.
*/

void
Request_reader::set_up_related_to_cgi(t_loc const *location_block_to_use)
{
	if (!(get_method() == POST || get_method() == GET))
		return;
	parser.set_up_related_to_cgi(location_block_to_use, server);
}

/*
**	This main function reads and parses a request, filling the class attributes.
**
**	RETURN:	0: in the case the read operation failed, or if read is done.
**				no update of timeout_reference
**			1 or more: the timeout_reference will be updated for service.
**			-1: error: the status code has been changed(parsing error etc).
**
** NOTE: on error, the appropriate response status code is set.
** 
** NOTE: For the keep-alive to be maintained, the request need to not be
** ill-formed. If its the case, we set ill_formed_request_so_no_more_reading
** to true, and return. We will then never read again on that socket, just
** answer and close that connection with this potentiatlly malicious client.
**
** If the request is rejected but not ill-formed, we have the duty to keep
** exchanging with the client. We will keep on reading on that client socket
** until the reader's state is set to DONE. This implies keeping on reading
** bodies until content-length is reached, or the last chunk is fully read.
** This case applies especially on the request that are more than cli_max_size.
*/

int	Request_reader::read_on_client_socket()
{
	int	read_bytes;
	int	ret = 1;
	//yes means we dont want to read anymore, until the reader is destroyed.
	if (ill_formed_request_so_no_more_reading)
		return (0); //no timeout update.

	if ((read_bytes = recv(client.fd, buffer, BUFFER_SIZE, 0)) < 0)
	{
		DEBUG std::cout << "\033[31mRequest_reader::read_on_client_socket() returns 0\033[m" << std::endl;
		return (0);
	}
	request_buffer.append(buffer, read_bytes);

	while (ret != 0)
	{
		switch (state)
		{
			case REQUEST_LINE: ret = request_line_handler();
				break;
			case HEADERS: ret = headers_handler();
				break;
			case BODY: ret = body_handler();
				break;
			case DONE:
				//if service created then deleted: reset the reader, make sure
				// to keep the end of the request_buffer
				if (service_already_generated && ptr_service_generated == NULL)
					flush_reader_for_keep_alive();
				return (read_bytes);
		}
		if (ret < 0)
		{
			if (status == 200)
				status = 400;
			ill_formed_request_so_no_more_reading = true;
			return (-1);
		}
		if (ret > 0)
		{
			if (state < BODY)
				parser.trash_procecced_bytes();
			if (state != DONE)
				state = static_cast<enum request_state>(1 + state);
			if (state == BODY)
				set_body_attributes();
		}
	}
	return (read_bytes);
}

/*
** return	0: keep going
** 			-1: fail
** 			1: ok we can move on further in the parser.
*/
int Request_reader::request_line_handler()
{
	size_t	end_pos;

	end_pos = request_buffer.find("\r\n", parser.get_offset());
	if (end_pos != std::string::npos)
	{
		if (end_pos - parser.get_offset() > 512)
		{
			status = 414; // URI too long
			return (-1);
		}
		if (parser.parse_start_line() < 0)
			return (-1);
		return (1);
	}
	if (request_buffer.size() - parser.get_offset() > 512)
	{
		status = 414; // URI too long
		return (-1);
	}
	return (0);
}

int Request_reader::headers_handler()
{
	size_t	length;
	int		ret;

	if ((length = request_buffer.find("\r\n\r\n", parser.get_offset())) != std::string::npos)
	{
		if (length > HEADERS_MAX_SIZE)
		{
			status = 431; // Request Header Fields Too Large
			return (-1);
		}
		while ((ret = parser.parse_header_field()) > 0)
		{
			continue;
		}
		if (ret < 0)
		{
			return (-1);
		}
		return (1);
	}
	return (0);
}

/*
** set the boolean ill_formed_request_so_no_more_reading to true if the request
** is ill-formed.
*/

int Request_reader::body_handler()
{
	int		ret;
	size_t	tmp = parser.get_offset();

	switch (parser.get_body_format())
	{
		case NO_BODY:
		{
			if (get_method() == POST || get_method() == PUT)
			{
				status = 411; // Length required
				return (-1);
			}
			return (1);
		}
		case CHUNKED_BODY:
		{
			while ((ret = parser.parse_chunked_body(current_content_length)) == 2)
				continue ;
			dump_request_buffer();
			if (cli_max_size > 0 && current_content_length > cli_max_size)
				status = 413; //413 Payload Too Large, dont return -1!!!
			return (ret);
		}
		case CONTENT_LENGTH_BODY:
		{
			parser.parse_body();
			current_content_length += parser.get_offset() - tmp;
			dump_request_buffer();
			if (current_content_length >= parser.get_content_length())
				return (1);
			return (0);
		}
		case INVALID_BODY:
		{
			std::cout << "\tIn Request_reader::body_handler, parser body format = INVALID_BODY\n";
			return (-1);
		}
	}
}

/*
**	This function is called when we need to write on a client socket but an
**	exception happened on server side.
**	We now has to avoid any type of string manipulation.
**	Returns	true if the fd_client has to be closed (write failed as well),
** 	false otherwise.
*/
bool Request_reader::write_no_more_alloc_answer(const char *msg, const size_t msg_size)
{
	int res = write(client.fd, msg + position_in_no_more_alloc_answer, msg_size - position_in_no_more_alloc_answer);
	if (res == -1)
		return (true);
	position_in_no_more_alloc_answer += res;
	if (position_in_no_more_alloc_answer == msg_size)
		return (true);
	return (false);
}

/*
**	Dumps the part of the body that has already been proccessed into the
**	ptr_service_generated->body_from_client string.
*/
void	Request_reader::dump_request_buffer(void)
{
	parser.dump_request_buffer(body_from_client);
}

/* ************************************************************************** */
/*	Getters                                                                   */
/* ************************************************************************** */

bool Request_reader::is_done_reading() const
{
	return (state == DONE);
}

bool Request_reader::can_be_deleted() const
{
	return(i_can_be_deleted);
}

const t_client_info &Request_reader::get_client_infos() const
{
	return (client);
}

int Request_reader::get_client_fd() const
{
	return (client.fd);
}

const struct sockaddr_storage &Request_reader::get_client_addr() const
{
	return (client.addr);
}

size_t Request_reader::get_client_addr_size() const
{
	return (client.addr_size);
}

size_t Request_reader::get_cli_max_size() const
{
	return (cli_max_size);
}

const t_srv &Request_reader::get_server() const
{
	return (server);
}

int Request_reader::get_status() const
{
	return (status);
}

int Request_reader::get_method(void) const
{
	return (parser.get_method());
}

const std::string &Request_reader::get_uri(void) const
{
	return (parser.get_uri());
}

const Headers &Request_reader::get_headers(void) const
{
	return (parser.get_headers());
}

const std::string Request_reader::get_header(std::string s) const
{
	return (parser.get_header(s));
}

bool Request_reader::is_a_cgi_request(void) const
{
	return (parser.is_a_cgi_request());
}

const std::string &Request_reader::get_query_string(void) const
{
	return (parser.get_query_string());
}

const std::string &Request_reader::get_mime_type() const
{
	return (parser.get_mime_type());
}

const std::string &Request_reader::get_cgi_bin_name() const
{
	return (parser.get_cgi_bin_name());
}

size_t Request_reader::get_content_length(void) const
{
	return (parser.get_content_length());
}

Request_reader::request_state Request_reader::get_request_state(void) const
{
	return (state);
}

bool Request_reader::does_request_timed_out(void)
{
	struct timeval newest;

	if (gettimeofday(&newest, NULL) == -1)
	{
		status = 500;
		return (true);
	}
	else if (newest.tv_sec - timeout_reference.tv_sec >= TIME_OUT_LIMIT_IN_SEC)
	{
		status = 408; // timeout
		return (true);
	}
	return (false);
}

//this boolean will tell us if the Service object has had to close all its fd
//except for the fd_client, so that we can generate an error 500 message.
bool
Request_reader::get_object_is_now_no_more_alloc_handler(void) const
{
	return (object_is_now_no_more_alloc_handler_bool);
}

/* ************************************************************************** */
/*	Setters                                                                   */
/* ************************************************************************** */

void Request_reader::reset_timeout_reference(void)
{
	if (gettimeofday(&timeout_reference, NULL) == -1)
		status = 500;
}

/*
** This function is called when we caught an exception, it avoids any type
** of string manipulation that might allocate on the heap. It set the boolean
** object_is_now_no_more_alloc_handler_bool to true. the message to write is
** a static variable.
*/

void
Request_reader::set_object_is_now_no_more_alloc_handler(void)
{
	object_is_now_no_more_alloc_handler_bool = true;
}

/*
** This function is called to set the state of the reader to DONE. It is used
** mainly in the method error_occured_set_up_everything() from abstract base
** class Service. It allows the reader contained into any Service to be set
** into the DONE state. Which make it deletable by the Server object, in a
** loop dedicated to Request_readers.
*/
void
Request_reader::set_state_to_done(void)
{
	state = DONE;
}

void
Request_reader::set_body_attributes(void)
{
	parser.set_body_format();
	set_cli_max_size();
}

void
Request_reader::set_cli_max_size()
{
	const t_loc	*location;

	location = get_location_block_to_use(parser.get_uri(), server);

	if (location != NULL)
		cli_max_size = location->cli_max_size;
	else
		cli_max_size = server.cli_max_size;
	if (cli_max_size != 0 && parser.get_body_format() == CONTENT_LENGTH_BODY && parser.get_content_length() > cli_max_size)
	{
		status = 413; //413 Payload Too Large
	}
}

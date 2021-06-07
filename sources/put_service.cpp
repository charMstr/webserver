#include "../includes/server.hpp"
#include "../includes/Service_generator.hpp"
#include "../includes/put_service.hpp"
#include "../includes/request_reader.hpp"
#include "../includes/request_parser.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/utils.hpp"
#include "../includes/Services_hardcoded_status_line.hpp"

put_service::put_service(const t_client_info client_info, \
		t_error_page error_page, std::string basename, Request_reader *reader) :
	//abstract base class constructor
	Service(client_info, error_page, basename, reader), 
	file_fd(-1),
	writing_status(PUT_DEFAULT_FILE)
{
	answer_status = ANSWER_WAIT; //for the answer to fd_client status.
}

/*
** destructor.
** note: the desctuctor of the Service base class closes all the fds remaining
** in list_fd_read, list_fd_write but DOES NOT close the fd_client member
** attribute.
*/
put_service::~put_service(void) { return ; }

/*
** Given a fd in Server's real fd set write, do some writing work.
** See response_service.hpp for the enum definitions.
** DEFAULT_HTTP : means we start writing an http message. Headers are needed.
** DEFAULT_FILE : means we are not writing an http message. Headers not needed.
** PROGRESSING_REGULAR : we simply write bytes to the fd
** PROGRESSING_CHUNKED : we are writing chunks to the fd
**
** NB : according to the Service abstract base class, always add the fd to the
** list "list_fd_done_writing" or "list_fd_done_reading" when done with the I/O
** operations with that fd, so that the Server class can be aware and remove
** that fd from its sort of "database" fd_set_real
** NB2: also those fd will have to be close (except for the fd_client).
**
** RETURN:	-1: KO, something wrong happened while writing.
** 			0: OK, but timeout_reference not updated.
** 			1 or more: OK, timout_reference reset.
*/

int
put_service::write_svc(int fd)
{
	if (fd == file_fd)
		return (write_to_target_file(fd));
	else
		return (write_back_to_client(fd));
}

/*
** Write from the put_service payload buffer (put_service::reader->body_from_client) to the target
** file fd.
**
** RETURN:	-1: KO, something wrong happened while writing.
** 			0: OK, but timeout_reference not updated.
** 			1 or more: OK, timout_reference reset.
*/

int
put_service::write_to_target_file(int fd)
{
	/* attempt to write something on target file fd */
	int wrtret = 0;
	wrtret = write(fd, reader->body_from_client.c_str(), reader->body_from_client.length());
	if (wrtret < 0)
	{ /* something bad happened */
		return (-1);
	}
	/* if we wrote something, remove bytes from our buffer */
	if (wrtret > 0)
		reader->body_from_client.erase(0, size_t(wrtret));

	/* if the buffer is empty AND if the reader is done with the fd_client */
	if ((reader->body_from_client.length() == 0) \
			&& reader->get_request_state() == Request_reader::DONE)
	{
		/* then add the target file fd to list_fd_done_writing and close it */
		list_fd_done_writing.push_back(fd); close(fd);

		/* advance the statemachine to PUT_DONE */
		writing_status = PUT_DONE;
		//DEBUG std::cout << "put_service: STATE PUT TO DONE" << std::endl;
		answer_status = ANSWER_HEADERS;
		return (1); //always update the timeout reference when sucess.
	}
	return (wrtret);
}

void		put_service::answer_set_up_headers(const t_srv & server, const std::string & uri)
{
	header_for_client = get_hardcoded_status_line_to_use().str;
	header_for_client += "Server: " SERVER_HEADER_VALUE CRLF;
	header_for_client += "Date: " + get_the_time() + CRLF;
	if (status_code == 201)
	{
		header_for_client += "Content-Length: 0" CRLF;
		header_for_client += "Location: http://" + server.server_name \
			+ ":" + std::to_string(server.host_port) + uri + CRLF;
	}

	if (service_thinks_reader_can_be_deleted)
		header_for_client += "Connection: close" CRLF;
	else
		header_for_client += "Connection: keep-alive" CRLF;
	//add extra CRLF to end headers section
	header_for_client += CRLF;
}

/*
** RETURN:	-1: KO, something wrong happened while writing.
** 			0: OK, but timeout_reference not updated.
** 			1 or more: OK, timout_reference reset.
*/

int
put_service::write_back_to_client(int fd)
{
	(void)(fd);
	/* if the put_service is not done writing to the target, do nothing */
	if (writing_status != PUT_DONE)
		return (0);

	int res = 0;
	if ((res = answer_with_header(answer_status)) == -1)
		return (-1);
	if (answer_status == ANSWER_BODY)
	{
		answer_status = ANSWER_DONE;
		//note: never close the fd_client in the Service* scope.
		list_fd_done_writing.push_back(get_fd_client());
		return (1); //always update the timeout_reference on the last success
	}
	return (res); 
}

/*
** implemenation of virtual function.
*/

int put_service::read_svc(int fd)
{
	(void)fd;
	return (0);
}

// setters
void put_service::set_file_fd(const int fd) { file_fd = fd; }

// getters

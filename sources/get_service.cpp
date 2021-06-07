#include <iostream>
#include "../includes/get_service.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/utils.hpp"

get_service::get_service(const t_client_info client_info, \
		t_error_page error_page, std::string basename, Request_reader *rdr, \
		const std::string uri) :
	//abstract base class constructor
	Service(client_info, error_page, basename, rdr),
	read_buffer_struct(),
	uri(uri),
	autoindex()
{
	//std::cout << "\033[33mconstructor called for request_service\033[0m" << std::endl;
	return ;
}

/*
** DESTRUCTOR:
** It always makes sure the dir_handle is closed (note: if it was closed
** previously, it would have been set to NULL already). The dir_handle is only
** used in the case we are answering to client with the directory listing
** (autoindex is on, and the uri was matching a directory.
**
** NOTE: the desctuctor of the Service base class closes all the fds remaining
** in list_fd_read, list_fd_write but DOES NOT close the fd_client member
** attribute.
*/
get_service::~get_service(void)
{
	if (autoindex.dir_handle)
		closedir(autoindex.dir_handle);
	return ;
}

/*
** This function reads into a buffer and updated the len of the string
** contained in the buffer. also the status is set to READING_DONE when done
** with reading.
**
** NB : according to the Service abstract base class, always add the fd to the
** list "list_fd_done_reading" when done with the reading operation,so that the
** Server class can be aware and remove that fd from its sort of "database"
** fd_set_real.
**
** NB2: also that fd will have to be closed (except for the fd_client).
**
** RETURN:	-1 KO, something wrong happened while reading.
** 			0, no problem, but timeout_reference not updated.
** 			1, ok, timout_reference reset.
*/
int get_service::read_svc(int fd)
{
	if (read_buffer_struct.len == BUFFER_SIZE)
		return (0); //buffer is already_full. zero so no update of timeout
	int rdret = read(fd, read_buffer_struct.buffer + read_buffer_struct.len, \
			BUFFER_SIZE - read_buffer_struct.len);
//	DEBUG_(DEBUG_GET_SERVICE) std::cout << "read res = " << rdret << std::endl;
	if (rdret < 0)
		return (-1);
	read_buffer_struct.len += rdret;
	if (rdret == 0)
	{
		read_buffer_struct.status = READING_DONE;
		list_fd_done_reading.push_back(fd);
		close(fd); //we know it is not the fd_client...
	}
	return (rdret);
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
** NOTE: the fd is always the fd_client as parameter.
**
** RETURN:	-1: KO, something wrong happened while writing.
** 			0: OK, but timeout_reference not updated.
** 			1 or more: OK, timout_reference reset.
*/

int
get_service::write_svc(int fd)
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
		if (autoindex.dir_handle != NULL)
		{
			if ((res = answer_chunk_from_directory_read()) == -1)
				return (-1);
		}
		else 	
		{
			if ((res = answer_from_reading_buffer()) == -1)
				return (-1);	
		}
	}
	if (answer_status == ANSWER_DONE)
	{
		//note: never close the fd_client in the Service* scope.
		list_fd_done_writing.push_back(fd);
		return (1); //always update the timeout_reference on the last success
	}
	return (res);
}

/*
** this function is called if the autoindex.dir_handle is non NULL, it means we
** are answering with the directory listing to fd_client. We are not sending the
** data from the reading_buffer, instead we are reading the directory's content
** and sending chunks after chunks.
**
** NOTE: the autoindex.dir_handle is closed in get_service destructor.
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
get_service::answer_chunk_from_directory_read(void)
{
	struct dirent	*entry;
	const size_t	saved_body_length = autoindex.body.length();

	if (autoindex.already_called_once == false) //append the start of our answer
		set_start_line_autoindex_body();
	while ((entry = readdir(autoindex.dir_handle)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		append_middle_line_autoindex_body(entry->d_name);
		if (autoindex.size_chunk >= CHUNK_SIZE)
		{
			close_chunk(saved_body_length, 0);
			//DEBUG_(DEBUG_GET_SERVICE) debug_string("get_service: body of" \
			//	"autoindex(\e[31mnot done\e[m)", client_info.fd, autoindex.body);
			return(answer_chunk());
		}
	}
	//getting here means this is the end of the directory list
	if (autoindex.reached_end_dir_list == false)
	{
		autoindex.reached_end_dir_list = true; //so we dont get here no more
		append_end_line_autoindex_body();
		close_chunk(saved_body_length, true); //adds final chunk recurcively.
	}
	//DEBUG_(DEBUG_GET_SERVICE) debug_string("get_service: body of " \
	//		"autoindex\e[m(\033[32mdone reading\033[m)", client_info.fd, autoindex.body);
	return(answer_chunk());
}

/*
** This function will try to write the chunk to the fd_client().
** If we know we reached the end of the directory list, and the body was fully
** sent, we declare the end of the writing operations with answer_status =
** ANSWER_DONE.
**
** NOTE: the dir_handle will be closed in the destructor.
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
get_service::answer_chunk(void)
{
	int res = write(get_fd_client(), autoindex.body.c_str(), \
			autoindex.body.length());
	if (res < 0)
		return (res);		
	autoindex.body.erase(0, res);
	if (autoindex.reached_end_dir_list && autoindex.body.length() == 0)
		answer_status = ANSWER_DONE;
	return (1);
}

/*
** This function will close a chunk of data. It will create the close_chunk
** string accordingly to the current size of the chunk. Then it will prepend
** the autoindex.body with the close_chunk string (using the offset
** saved_body_length allows us to add the chunk size in the right place, even
** if the previous write did not completely send the chunk). Then it appends
** the trailing CRLF, and reset the size_chunk to zero.
**
** If the second parameter is set to true, it means we are done with the data
** completely, we can recurce one time and set the final closing chunk with 0
** length
*/

void
get_service::close_chunk(const size_t  saved_body_length, bool last_chunk)
{
	autoindex.body.insert(saved_body_length, get_close_chunk_string(autoindex.size_chunk));
	autoindex.size_chunk = 0;
	autoindex.body += CRLF;
	if (last_chunk)
		close_chunk(autoindex.body.length(), 0);
}

/*
** this function appends to the autoindex.body the new entry. The insertion is
** made agremented with the html balise code. The size_chunk is updated
** accordingly.
*/

void
get_service::append_middle_line_autoindex_body(const char * entry)
{
	std::string new_line = std::string("<a href=\"") + entry + "\">" + entry +\
 		"</a>" CRLF;
	autoindex.size_chunk += new_line.length();
	autoindex.body += new_line;
}

/*
** This function appends to the autoindex.body the end of the body html code.
** it also updates the size_chunk variable. It is called when we reached the
** end of the directory read.
*/
void 
get_service::append_end_line_autoindex_body(void)
{
	std::string end_line = std::string("</pre><hr></body>" CRLF "</html>");
	autoindex.size_chunk += end_line.length();
	autoindex.body += end_line;
}

/*
** Sets autoindex.body to the first line to answer. Its is only called on the
** first run.
*/
void 
get_service::set_start_line_autoindex_body()
{
	autoindex.body = std::string("<html>" CRLF "<head><title>Index of " + uri \
			+ "</title></head>" CRLF "<body>" CRLF "<h1>Index of " + uri +\
			"</h1><hr><pre><a href=\"../\">../</a>" CRLF);
	autoindex.already_called_once = true;	
	autoindex.size_chunk += autoindex.body.length();
}

/*
** This function is called each we respond to a GET method, and the file is
** opened, and the autoindex is not needed.
** This situation means the Content-Length header is set (size of file known).
** We are going to just take the reading_buffer, and send its content to
** fd_client, until the reading operations are done and our buffer is empty.
**
** RETURN:	-1: KO with a write operation
** 			0 or more: OK (write did not fail)
*/

int
get_service::answer_from_reading_buffer(void)
{
	int res;

	//DEBUG_(DEBUG_GET_SERVICE) debug_string("GET SERVICE, wriring from buffer", \
	//		get_fd_client(), read_buffer_struct.buffer);
	res = write(get_fd_client(), read_buffer_struct.buffer, \
			read_buffer_struct.len);
	if (res == -1)
		return (-1);
	//shift the content of the answer_buffer.buffer.
	memmove(read_buffer_struct.buffer, read_buffer_struct.buffer + res, \
			BUFFER_SIZE - res);
	read_buffer_struct.len -= res;
	//if totatlly done writing
	if (read_buffer_struct.status == READING_DONE \
			&& read_buffer_struct.len == 0) //fully done.
		answer_status = ANSWER_DONE;
	return (1);
}

/*
** This function will set up all the headers for the get_service object.
** Depending on the value of directory_handle:
** - if non NULL, it means we opened a directory to read, we will set the
** 	encoding header to CHUNKING.
** - else (if NULL), we can set the header to Content-Length, and use the info
** 	contained int file_stat.
*/

void
get_service::answer_set_up_headers(const struct stat &file_stat, \
		const std::string &content_type)
{	
	//add status line
	header_for_client = get_hardcoded_status_line_to_use().str;
	//add the date
	header_for_client += "Date: " + get_the_time() + CRLF;
	//add the content type:
	header_for_client += "Content-Type: " + content_type + CRLF;

	// ENTITY HEADER : Last-Modified
	header_for_client += "Last-Modified: " + get_last_modified_time(&file_stat) + CRLF;
	header_for_client += "Server: " SERVER_HEADER_VALUE CRLF;

	if (service_thinks_reader_can_be_deleted)
		header_for_client += "Connection: close" CRLF;
	else
		header_for_client += "Connection: keep-alive" CRLF;

	//here chose between chunked ans content-length.
	if (autoindex.dir_handle)
		answer_set_up_encoding(CHUNKED, 0);
	else
		answer_set_up_encoding(KNOWN_LENGTH, file_stat.st_size);

	//append extra CRLF
	header_for_client += CRLF;
	//DEBUG_(DEBUG_GET_SERVICE) debug_header_for_client(header_for_client, get_fd_client());
}

/*
** This function is in charge of adding the correct encoding header to the
** header_for_client. Note: it must be called after the status_line has been
** added.
**
** PARAMETERS:
** - encoding: either KNOWN_LENGHT, or CHUNKED
** - size: only use if encoding is KNOWN_LENGTH
*/
void
get_service::answer_set_up_encoding(const e_transfer_encoding encoding, const size_t size)
{
	if (encoding == KNOWN_LENGTH)
	{
		this->encoding_answer_to_client = KNOWN_LENGTH;
		std::stringstream ss;
		ss << size;
		header_for_client += "Content-Length: " + ss.str() + CRLF;
	}
	else
	{
		this->encoding_answer_to_client = CHUNKED;
		header_for_client += "Transfer-Encoding: chunked" CRLF;
	}
}

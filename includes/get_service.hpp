#ifndef GET_SERVICE_HPP
# define GET_SERVICE_HPP

# include <string>
# include <map>
# include <sys/stat.h>
# include <string.h>
# include "../includes/Service.hpp"
# include "../includes/utils.hpp"

/*
** get_service is a concrete class, derived from the abstract class
** Service, and it is created when a client did a request, and we want to
** finally respond to it.
** We need to process the header +- body of the client's request.
**
** Parameters:
** 	- client_addr: the client'address is of type sockaddr_storage so it is
** 	family agnostic. It can hold either sockaddr_in (ipv4) or sockaddr_in6
** 	- size of the client_addr.
**
** It reimplements the read function mainly.
** It posses a buffer in which we are reading the client's request.
*/

enum ressource_reading_status { NONE, READING, DONE_READING };
enum response_writing_status {
	DEFAULT_HTTP, DEFAULT_FILE, PROGRESSING_REGULAR, PROGRESSING_CHUNKED
};

class get_service : public Service
{
	/* ********************************************************************** */
	/* DATA                                                                   */
	/* ********************************************************************** */
		typedef enum e_transfer_encoding
		{ KNOWN_LENGTH, CHUNKED} e_transfer_encoding;

		typedef enum e_status_reading_for_client
		{ STILL_READING, READING_DONE } e_status_reading;

		typedef struct 			s_reading_buffer
		{
			char				buffer[BUFFER_SIZE];
			int					len;
			e_status_reading	status;
		}						t_reading_buffer;

		t_reading_buffer			read_buffer_struct; //buffer in which we read.
		const std::string			uri;//used for the autoindex answer

		typedef struct	s_autoindex_utils
		{
			bool				already_called_once;
			DIR					*dir_handle;
			std::string			body;
			int					size_chunk;
			bool				reached_end_dir_list;
		}				t_autoindex_utils;

		e_transfer_encoding 		encoding_answer_to_client;
	public :
		t_autoindex_utils			autoindex;

	//DEBUG PURPOSE
		friend class get_service_test;

	/* ********************************************************************** */
	/* METHODS                                                                */
	/* ********************************************************************** */

	public :
		get_service(const t_client_info client_info, t_error_page error_page, \
				std::string basename, Request_reader *reader, \
				const std::string uri);
virtual	~get_service(void);

		int		read_svc(int fd);
		int		write_svc(int fd);

		void	answer_set_up_headers(const struct stat &file_stat, \
					const std::string &content_type);

	protected :
		void	answer_set_up_encoding(const e_transfer_encoding encoding, \
					const size_t size);
		int 	answer_from_reading_buffer(void);
		int		answer_chunk_from_directory_read(void);
		void	set_start_line_autoindex_body();
		void	append_middle_line_autoindex_body(const char *entry);
		void	append_end_line_autoindex_body(void);
		void	close_chunk(const size_t  saved_body_length, bool last_chunk);
		int		answer_chunk(void);

		// See: https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages

};

#endif // GET_SERVICE_HPP

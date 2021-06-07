#ifndef CGI_SERVICE_HPP
# define CGI_SERVICE_HPP

/*
** cgi_service is a concrete class, heriting from Service abstract base class.
** It is designed to answer to a request to is of cgi nature (telling the
** server that it should start a binary). See Common Gateway Interface on Google
**
** We create this class when the uri of the request contained in the path a
** file with a ".php" extension. Depending on the Method of the request, the
** process is slighlty different, but the main idea is quite the same, we
** identify which binary needs to be started, in which relative path, and if
** and only if all this is possible, we will start a new process. The process
** is created through a fork, exec and with a pipe arrangement.
** Then the object will wait for the reponse from the cgi process. And the
** response is transmitted back again to the client that did the request.
**
** note: just like any other objects derivated from Service base class:
** 	- object should add opened fd to its list_fd_read and list_fd_write
** 	- object's reimplementation of write_svc() and read_svc() should take care
** 	of closing those fd when they are done with them.
** 	- also when done, they should add the fd to list fd_done.
** 	- last thing, when done with an fd, the object itself should not remove the
** 	fd from the list_fd_write or list_fd_read, as the Server:: class is
** 	iterating on it.
*/

# include "../includes/utils.hpp"
# include "../includes/Service.hpp"
# include "../includes/request_reader.hpp"
# include "signal.h"

class Cgi_service : public Service
{
	public:
		Cgi_service(const t_client_info client_info, t_error_page error_page, \
				std::string basename, Request_reader *reader);
				

		virtual	~Cgi_service(void);

		//DEBUGING FRIENDSHIP
		friend class	Cgi_service_test;
		friend void		debug_cgi_svc(const Cgi_service *cgi, bool a);

		//if the fd is the fd_client, we dont close it when done.
		int		read_svc(int fd);

		//if the fd is the fd_client or the fd_cgi, we dont close it when done
		//int		write_svc(int fd);

		/* Getters/Setters */
		void    set_cgi_pid(pid_t pid) { cgi_pid = pid; }
        pid_t    get_cgi_pid(void) const { return (cgi_pid); }

		void	set_webserv_writes_on_me(int fd) { this->webserv_writes_on_me = fd; }
		int		get_webserv_writes_on_me(void) const { return (this->webserv_writes_on_me); }
		void	set_webserv_reads_on_me(int fd) { this->webserv_reads_on_me = fd; }
		int		get_webserv_reads_on_me(void) const { return (this->webserv_reads_on_me); }
		void	set_cgi_writes_on_me(int fd) { this->cgi_writes_on_me = fd; }
		int		get_cgi_writes_on_me(void) const { return (this->cgi_writes_on_me); }
		void	set_cgi_reads_on_me(int fd) { this->cgi_reads_on_me = fd; }
		int		get_cgi_reads_on_me(void) const { return (this->cgi_reads_on_me); }

		int		get_argc(void) const { return (argc); }
		void	set_argc(int n) { this->argc = n; }
		char ** get_argv(void) const { return (argv); }
		void	set_argv(char ** ptr) { this->argv = ptr; }
		void	set_envp(char ** ptr) { this->envp = ptr; }
		char **	get_envp(void) const { return (envp); }
		void	set_env_size(int size) { this->env_size = size; }
		int		get_env_size(void) const { return (env_size); }
		char **	malloc_and_build_cgi_argv(const std::string &cgi_bin_name,\
					std::string const &resource_pathdd);
		char **	malloc_and_build_cgi_envp(std::string const &resource_path);

	private:

		pid_t				cgi_pid;

		//add either close or keep-alive in the headers, if necessary
		void				cgi_add_connection_header(void);
		//to update the header_for_client, if "Status: " header is present
		void				update_header_containing_status_from_cgi_output();
		/* to store the fds of the pipes needed for cgi */
		int					webserv_writes_on_me;
		int					cgi_reads_on_me;
		int					cgi_writes_on_me;
		int					webserv_reads_on_me;

		/* needed for execve() */
		int					argc;
		char				**argv;
		char				**envp;			int		env_size;;

	private:

/* ************************************************************************** */
/* READ AND WRITE                                                             */
/* ************************************************************************** */
		typedef enum e_transfer_encoding
		{ KNOWN_LENGTH, NEED_CHUNK, ALREADY_CHUNKED} e_transfer_encoding;

		typedef enum e_status_reading_for_client
		{ STILL_READING, READING_DONE } e_status_reading;
		typedef struct 			s_reading_buffer
		{
			char				buffer[BUFFER_SIZE];
			int					len;
			e_status_reading	status;
		}						t_reading_buffer;

		e_transfer_encoding 	encoding_of_content_from_cgi;


		t_reading_buffer		received_from_cgi_buffer_struct;
		size_t					total_body_size;
		size_t					content_length;

		int					write_svc(int fd);
		int					write_to_client();
		int					write_to_cgi();
		int					answer_from_reading_buffer(void);

		int					read_from_cgi(int fd);
		int					read_into_received_from_cgi_buffer_struct(int fd);

		int 				update_header_for_client_and_find_double_crlf(void);
		int					cgi_first_try_to_find_double_crlf_and_update(void);
		int					cgi_second_try_to_find_double_crlf_and_update(void);
		void				cgi_set_encoding_variables();

		int					is_reading_from_cgi_done(int read_res);
		void				close_chunk(const int previous_len, \
									const int read_res);
		void				insert_chunking_tag_in_reading_buffer(\
								const int previous_len, const int read_res);
};

#endif

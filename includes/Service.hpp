#ifndef SERVICE_HPP
# define SERVICE_HPP

# include <cstddef>
# include <sys/socket.h>
# include <list>
# include <unistd.h>
# include <dirent.h> //for opendir()
# include "../includes/utils.hpp"
# include "../includes/webserver.hpp"
# include "../includes/Services_hardcoded_body.hpp"
# include "../includes/request_reader.hpp"

/*
** Structure that holds hardcoded constant strings and their length
** (used for status line and body)
** - str: a pointer to constant string.
** - len: the result of sizeof(msg) - 1.
*/
typedef struct s_hardcoded_str
{
	const char *str;
	size_t		len;
}				t_hardcoded_str;

class Server;

/*
** This is an abstract class that will allow us to do some polymorphism. Each
** time one of our virtual servers need to provide a response to a request, a
** derivated object will be created and added to a list of services.
** All derived class can be pointed to by the same pointer: Service*. Therefore
** a linked list of these pointers will allow us to keep track of the ongoing
** services being processed.
**
** Note: listening sockets are permanently present, they are treated separately
**
** Note: request_reader objects are not derivted from this class as they are
** slighly different:
** - for clarity we dont mix requests and services(responses).
** - they only have one fd in use (for reading on a conexion socket),
** - that fd should not be closed when done with the request_reader object, it
**	 must be passed to the Service(response) created.
**
** Note: The member variables list_fd... are there so that the object is
** directly associated with the filedescritpors it is using. It is in the form
** of a list so that an service object could for exemple read in multiple fd at
** the same time.
**
** Note: when creating a Service, some fd are created, they will then be closed
** by the service itself. This class does not open the fd_client, so it does
** not close it.
**
** Note: When inside write_svc() or read_svc() we are done reading or writing:
** 	- fd should be added to Service::list_fd_done.
** 	- fd should be close (except for the fd_client), or other exceptions, see
** 		implementation details of the functions read_svc() and write_svc()
** 	- fd should not be removed from the list_fd_write or list_fd_read as the
** 		list is being iterated uppon in the calling function.
*/

class Service
{
	/* ********************************************************************** */
	/* DATA                                                                   */
	/* ********************************************************************** */
	public:
		std::list<int>		list_fd_read;
		std::list<int>		list_fd_write;

		//used to store the fds we are done with, ready to be removed from the
		//fd_set_real.fd_set_read in the Server:: class.
		std::list<int>		list_fd_done_reading;
		//used to store the fds we are done with, ready to be removed from the
		//fd_set_real.fd_set_write in the Server:: class.
		std::list<int>		list_fd_done_writing;
		bool						just_head_no_body; //skip body?
		bool						service_thinks_reader_can_be_deleted;

		typedef enum	e_answer_to_client_status
		{
			ANSWER_HEADERS, ANSWER_BODY, ANSWER_DONE, ANSWER_WAIT
		}				e_answer_to_client_status;

		Request_reader				*reader;
	protected:

		typedef	struct	s_answer_error_to_client
		{
			t_hardcoded_str				known_body;
			size_t						position_in_known_body;
		}				t_answer_error_to_client;

		t_client_info				client_info;
		t_error_page				error_page_to_use;
		const std::string			resource_basename;
		int							status_code;//set it to 200 in constructor
		bool						error_occured;//set to false in constructor
		std::string					redirect_to;
		//the body from the client's request, that is ready to use (unchunked)

		//to keep track of where we are in the answering stage.	
		e_answer_to_client_status	answer_status;//for state machine

		//used to store the status_line + headers + CRLF.
		std::string					header_for_client;
	
		//stores the necessary to handle errors (exceptions or just error).
		t_answer_error_to_client	answer_error_struct;

		//the const array of t_builtin_hardcoded structures.
		const static t_hardcoded_str webserv_hardcoded_bodies[];
		const static t_hardcoded_str webserv_hardcoded_status_lines[];

		//for DEBUG
		friend class 				Error_service_test;
		friend class				Service_test; 
		friend class 				get_service_test;

	/* ********************************************************************** */
	/* METHODS                                                                */
	/* ********************************************************************** */
	public:
		Service(const t_client_info client_info, t_error_page error_page, \
				std::string basename, Request_reader *reader);

		virtual ~Service();

		//interface function: it is called in the Server:: scope while looping
		//on the list_fd_write
		void			write_svc_interface(int fd);

		//interface function: it is called in the Server:: scope while looping
		//on the list_fd_read
		void			read_svc_interface(int fd);

		//looks in the list_fd_done_reading to seek for a fd we are done with.
		virtual bool 	is_done_reading_with_fd(int fd);
		//looks in the list_fd_done_wrriting to seek for a fd we are done with.
		virtual bool 	is_done_writing_with_fd(int fd);
		//returns true if list_fd_read and list_fd_write are empty.
		virtual bool	is_done_reading_and_writing(void) const;

		void			close_all_fds_except_for_fd_client(void);
		void			add_to_list_done_all_fds_except_for_fd_client(void);
		void			remove_all_fds_in_list_fd_rw_except_for_fd_client(void);

	/* PUBLIC GETERS */
		int				get_status_code(void) const;
		int				get_fd_client() const;
		const std::string	&get_redirect_to(void) const;
		e_answer_to_client_status get_answer_status(void) const;
		bool			get_error_occured(void) const;

	/* PUBLIC SETTERS */
		void			set_status_code(int status_code);
		void			error_occured_set_up_everything(int error_status_code, \
						bool at_generating_service_stage);
		void			set_redirect_to(const std::string location);
		bool			reset_timeout_reference(void);

	protected:
		//will write on the fd, and close it when the object is done with it.
		//except for some exceptions.
		virtual int		write_svc(int fd) = 0;

		//will read on the fd, and close it when the object is done with it.
		//except for some exceptions.
		virtual int		read_svc(int fd) = 0;

		int				answer_with_header( \
				e_answer_to_client_status &status); //always to fd_client

		void			does_service_thinks_reader_can_be_delete(void);
		void			set_up_answer_to_client_when_error_page_required(void);
		int				answer_to_fd_client_when_error_occured(void);
		int				answer_error_hardcoded_body(void);

	/* PROTECTED GETERS */
		t_hardcoded_str		get_hardcoded_status_line_to_use(void);
		t_hardcoded_str		get_hardcoded_body_to_use(void);

		std::string			get_error_page_string_from_status_code(void) const;

	/* PROTECTED SETTERS */
//		int			read_error_page_from_fd_error_page(void); //reads from fd_error_page
//		int			write_error_page_to_fd_client(void); //writes to fd_client

};

#endif

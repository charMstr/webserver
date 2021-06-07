#ifndef PUT_SERVICE
# define PUT_SERVICE

# include <iostream>
# include <sstream>
# include <string>
# include <map>
# include <sys/stat.h>
# include <string.h>
# include "../includes/Service.hpp"
# include "../includes/utils.hpp"

/*
** put_service is a concrete class, derived from the abstract class
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

class put_service : public Service
{
	public:
		int										file_fd;
		/* state of the writing of the file to be PUT on the server */
		enum writing_status
			{ PUT_DEFAULT_FILE, PUT_PROGRESSING_FILE_REGULAR,
			PUT_PROGRESSING_FILE_CHUNKED, PUT_DONE, PUT_ERROR };
		writing_status							writing_status;

	public:
		//needs to be accessed in the service_generator.
		std::string								body;

	public :
		put_service(const t_client_info client_info, t_error_page error_page, \
				std::string basename, Request_reader *reader);

virtual	~put_service(void);

		int		read_svc(int fd);
		int		write_svc(int fd);
		int		write_to_target_file(int fd);
		int		write_back_to_client(int fd);
		void	answer_set_up_headers(const t_srv & server, const std::string & uri);

		// setters
		void	set_file_fd(const int fd);

		// getters
};

#endif // PUT_SERVICE

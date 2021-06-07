#ifndef DELETE_SERVICE
# define DELETE_SERVICE

# include <iostream>
# include <sstream>
# include <string>
# include <map>
# include <sys/stat.h>
# include <string.h>
# include "../includes/Service.hpp"
# include "../includes/utils.hpp"

/*
** The read function is never used (the request_reader is in charge of finishing
** any read operation with the client, in case there is a body to consume for
** example).
**
** The write function is never user. The base class Service provide the fuction
** answer_to_client_when_erro_occured and this is enough for us (replying 204
** when deletion was completed or 4xx or 5xx message).
*/


class delete_service : public Service
{
	public:
	public :
		delete_service(const t_client_info client_info, \
			 t_error_page error_page, std::string basename, \
			 Request_reader *reader);

virtual	~delete_service(void);

		int		read_svc(int fd);
		int		write_svc(int fd);
};

#endif // DELETE_SERVICE

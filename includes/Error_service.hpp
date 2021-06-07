#ifndef ERROR_SERVICE_HPP
# define ERROR_SERVICE_HPP

# include "../includes/Service.hpp"

/*
** This class inherits from the abstract base class Service. It will be
** instanciated by service_generator when we know there was an error straight
** away in the request(400 bad request). Its role is to answer to the client
** with the correct error message.
** From there it will look for a potential file to be opened ("error_page"
** mentioned in the server block configuration file), and try to use it to
** answer our client. If for some reason the file doesnt exist, cant be opened
** , etc, we will use a hard coded answer representing our error_page, its a
** constant string.
** 
** Note: This class is the most basic implementation of Abstract base class
** Service. Indeed its virtual methods read_svc() and write_svc() are
** implemented as empty body functions. It will only use the methods required
** to answer to client when an error occured. Therefor this class will be used
** in our unit tests to prove the capacity of all other classes to respond when
** an error occurs (setting up error to other values than 400, like 404 etc...
** ).
*/

class Error_service : public Service
{
	public:
		Error_service(const t_client_info client_inf, t_error_page err_page, \
				std::string basename, Request_reader *reader, int error_status);	

		//pure virtual in Service abstract base class: will never be called.
		int	write_svc(int fd) { (void)fd; return (0);}
		//pure virtual in Service abstract base class: will never be called.
		int	read_svc(int fd) { (void)fd; return (0);}

};

#endif

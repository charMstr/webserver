#include "../includes/server.hpp"
#include "../includes/Service_generator.hpp"
#include "../includes/Service.hpp"
#include "../includes/delete_service.hpp"
#include "../includes/request_reader.hpp"
#include "../includes/request_parser.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/utils.hpp"

delete_service::delete_service(const t_client_info client_info, \
		t_error_page error_page, std::string basename, Request_reader *rdr) :
	//abstract base class constructor
	Service(client_info, error_page, basename, rdr)
{
	return ;
}

/*
** destructor.
** note: the desctuctor of the Service base class closes all the fds remaining
** in list_fd_read, list_fd_write but DOES NOT close the fd_client member
** attribute.
*/ delete_service::~delete_service(void) { return ; }

/*
** dummy implementation of the abstract base class's func
*/

int
delete_service::write_svc(int fd)
{
	(void)fd;
	return (0);
}

/*
** dummy implementation of the abstract base class's func
*/
int delete_service::read_svc(int fd)
{
	(void)(fd);
	return (0);
}

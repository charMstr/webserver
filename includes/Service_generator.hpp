#ifndef SERVICE_GENERATOR_HPP
# define SERVICE_GENERATOR_HPP

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/uio.h>
# include <unistd.h>
# include <dirent.h>
# include "../includes/webserver.hpp"
# include "../includes/exceptions.hpp"
# include "../includes/request_parser.hpp"
# include "../includes/request_reader.hpp"
# include "../includes/get_service.hpp"
# include "../includes/put_service.hpp"

/*
** Service_generator is a class that will be used within the Server class. It
** is created so that we lighten the Server class.
**
** Service_generator class will try to create an object on the heap, the object
** is derived of the abstract base class Service. It will produce the correct
** object type according to the request given as input parameter, and it will
** open the required filedescriptors etc...
**
** The main public method that is the interface with the Server class is
** generate(). generate() will return the pointer to the created object, that
** pointer will be be added into Server's list "list_services".
**
** note: when some problem occurs. it is the responsability of this class to
** undo whats done. Talking about deleting memory it has allocated (Service
** allocated on the heap), and filedescriptors opened in the derivated classes.
*/

//forward declarations.
class Server;
class Service;
class Request_reader;
class Request_parser;

class Service_generator
{
	/* ********************************************************************** */
	/* DATA                                                                   */
	/* ********************************************************************** */
	private:
		Request_reader	&reader;

		const t_loc*			location_block_to_use;
		std::string				root_to_use;
		bool					use_alias;
		std::string				resource_path; //root_directory + root + uri
		std::string 			resource_basename; //full_path - file_name
		bool					uri_is_a_directory;
		std::string				index_to_use;
		t_error_page			error_pages_to_use;
		bool					autoindex_activated;;
		//used to check file can be opened.
		struct stat				file_stat;

	/* ********************************************************************** */
	/* METHODS                                                                */
	/* ********************************************************************** */
	public:
		Service_generator(Request_reader &request_reader);

virtual ~Service_generator();

		//note: if a bad_alloc exception occured, this func should always close
		//	the filedescritpors it has opened itself (so not the fd_client!),
		//	then rethrow.
		Service *generate();

	/* FRIEND FUNCTION */ //for debug
		friend void	debug_service_generator_preliminary_to_service_build( \
					Service_generator&, int fd, int method, int is_cgi);

		friend  void	debug_generate_get_service_more_key_data(\
			Service_generator &svc_gen, int fd_client);

	protected:
		//called before anything. it set a few variables like:
		//location_block_to_use, root_to_use, resource_path, resource_basename...
		int				preliminary_set_up_to_service_build( \
							const std::string &uri);
		void			set_up_service_when_autoindex_is_required(Service *svc);

		void			set_location_block_to_use(const std::string &uri);
		bool			is_method_not_allowed(const t_loc *location, int method);
		void			set_root_to_use(const t_loc*);
		void			set_error_page_to_use(const t_loc*);
		void			set_resource_path(std::string const &uri, \
							std::string const &directory);
		void 			set_resource_basename(void);

		int				set_uri_is_a_directory(void);
		int				redirect_if_trailing_slash_missing(get_service *svc, \
							const std::string &uri);
		void			set_autoindex_activated(const t_loc *location);
		void			set_index_to_use(const t_loc *location);


		std::string		concatenate_path(std::string const &left, \
							std::string const &right);

		const std::string	get_content_type(const std::string & uri) const;


		Service	*generate_error_service(int error_status_code);


		Service *generate_get_service(bool just_head);
		int		generate_get_service_autoindex_case(get_service *svc);
		void	generate_get_service_autoindex_case_confirmed(get_service *svc);
		Service *generate_get_service_assist(get_service * svc);

		Service *generate_post_service(void);
		Service *generate_put_service(void);
		Service *generate_cgi_service(void);
		Service *generate_delete_service(void);

		/* FRIEND FUNCTION FOR DEBUG */
		friend class Service_generator_test;
};

#endif

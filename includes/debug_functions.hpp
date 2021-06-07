#ifndef DEBUG_HPP
# define DEBUG_HPP

# include "../includes/webserver.hpp"
# include "../includes/Service.hpp"
# include "../includes/request_reader.hpp"
# include <sstream>
# include <sys/select.h>
# include <sys/stat.h>
# include "../includes/request_parser.hpp"
# include "../includes/Service_generator.hpp"
# include "../includes/cgi_service.hpp"

# include "../includes/debug_and_traces_macros.hpp"

class Server;

void	debug_introduce_func(const char *str);
void	debug_end_func(void);

void	debug_servers_print(std::vector<t_srv> *servers);
void 	debug_server_print(const t_srv * const server);
void	debug_error_page_print(t_error_page const &error_page);
void	debug_lst_cgi_pass_print(std::list<t_cgi_pass> const &lst_cgi_pass);
void 	debug_vector_listen(const std::vector<t_pair_fd_listen_t_srv> &vector_listen, const t_fd_set_real &fd_set_real); //OK
void	debug_print_which_fdsets_are_active(int fd,bool issetread, \
		bool issetwrite, bool issetexcept);
void	debug_enter_func(const char *s);
void	debug_exit_func(const char *s);


void	debug_entering_generate_xxx_service(const char *func_name, int fd, \
		const int &method, const std::string &uri);

void	debug_new_incoming_conexion(t_pair_fd_listen_t_srv &pair, \
		int fd_client);

void	debug_exiting_resume_processed_requests_from_clients( \
		const std::list<Request_reader> &list_requests);

void	debug_print_reader_headers(std::map<std::string, std::string> headers, \
		const int fd_client);

void	debug_after_select_which_fd_is_set( \
		const std::vector<t_pair_fd_listen_t_srv> &vector_listen, \
		const std::list<Request_reader> &list_request_readers, \
		const std::list<Service*> &list_services, \
		const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except);

void	debug_after_select_which_fd_is_set_from_vector_listen( \
		const std::vector<t_pair_fd_listen_t_srv> &vector_listen, \
		const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except);

void	debug_after_select_which_fd_is_set_from_list_request_readers( \
		const std::list<Request_reader> &list_requests, \
		const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except);

void	debug_after_select_which_fd_is_set_from_list_services( \
		const std::list<Service *> &list_services, \
		const fd_set &fd_set_read, const  fd_set &fd_set_write, \
		const fd_set &fd_set_except);

void	debug_print_fileStat(const struct stat * fileStat, const int fd_client);

void	debug_request_read_raw_request_so_far(const int &fd_client, \
		const std::string &raw_request, const int done_reading);

void	debug_request_parser(const Request_parser &parser, \
		const std::map<std::string, std::string> &headers);

void	debug_start_server(void);

void	debug_from_service_add_fd_to_real_set(const int fd_client, \
		const std::list<int> &list_write, const std::list<int> &list_read);

void	debug_string(const char *title, int fd, std::string const &debug);

void	debug_t_hardcoded_str_body(const t_hardcoded_str *msg_handle);

void	debug_t_hardcoded_str_status_line(const t_hardcoded_str *msg_handle);

void	debug_error_service_creation_context(const t_srv * const server, \
			int status_code);

void	debug_service_generator_preliminary_to_service_build(\
			Service_generator &svc_gen, int fd_client, int method, int is_cgi);

void	debug_generate_get_service_more_key_data(\
			Service_generator &svc_gen, int fd_client);

std::string debug_return_method_string(int method, int is_cgi);

void 		debug_error_occured(int fd, int status_code);
void		debug_header_for_client(const std::string headers, const int fd);
void		debug_cgi_svc(const Cgi_service *svc, bool display_argv_envp);
void 		debug_deletion(const char *what, const char *file, int line, int fd);
void 		debug_addition(const char *what, const char *file, int line, int fd);
void		debug_request_reader(const Request_reader &reader);

void		debug_timeout_reset(int fd);
void		debug_timeout_occured(int fd);

void		debug_envp_or_argv(char **envp, const char *name);

#endif

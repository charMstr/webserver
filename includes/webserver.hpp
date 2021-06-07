#ifndef WEBSERV_HPP
#define WEBSERV_HPP

# include <sys/select.h> //declaration of the "fd_set" type and select().
# include <stdlib.h>  //needed for "NULL" declaration.
# include <unistd.h> //needed by open().
# include <fcntl.h> //for fcntl, seting O_NONBLOCK flag.
# include <errno.h> //for fcntl, seting O_NONBLOCK flag.
# include <netinet/in.h> //for inet_addr()

# include <string>
# include <list>
# include <vector>

# include "../includes/http_methods.hpp"

# define DFL_CNF_PATH "./configuration_files/webserv.conf"
# define DFL_CLI_MAX_SIZE 0 // 0 means unlimited
# define DFL_CNF_MAX_SIZE 10000
# define BACKLOG 128 //max number of pending connexion on a listening socket.
# define BUFFER_SIZE 65535 // max number of bytes a socket can hold
//# define BUFFER_SIZE 1 // max number of bytes a socket can hold
# define CHUNK_SIZE 65000 // number of bytes sent in one chunk
# define SERVER_HEADER_VALUE "Webserv42/1.0" //the name of our server in headers.
//defines max number of request possible from client to server on same socket
# define MAX_REQUEST_NUMBER_FOR_KEEP_ALIVE 15001

// replace with a reasonable value, like 5...
# define TIME_OUT_LIMIT_IN_SEC 20

# if BUFFER_SIZE < 11
#  undef BUFFER_SIZE
# define BUFFER_SIZE 11
//done to make sure we cant segfault, especially in the code that handles the
//chunking.
# endif

/*
** usefull to contain in one place the fd_client, the client_addr, and
** client_address_size
** Note: the client'address is of type sockaddr_storage so it is family
** agnostic. It can hold either sockaddr_in (ipv4) or sockaddr_in6
*/
typedef struct	s_client_info
{
	int						fd;
	socklen_t				addr_size;
	struct sockaddr_storage	addr;

}				t_client_info;

/*
** usefull for the error_page structure. an error page is associated with at
** least one integer. A couple of integers can be coupled to a single
** error_page string. All integers need to be between 300 and 599 included.
** exemple of nginx conf line for more context to this explanation:
** error_page 500 502 503 /50x_error.html
*/
typedef struct s_error_page
{
	std::list<int>	list_error_status;
	std::string		error_page;
}				t_error_page;

/*
** This structure will hold a tripple:
** extension: a string representing a file extention.
** cgi_bin: a string representing the name of a binary.
**
** This associates the name of a binary to launch for a given file extension in
** the case the request from the client is a CGI request (mehod GET or POST).
*/
typedef struct s_cgi_pass
{
	std::string	extension;
	std::string cgi_bin;
	t_methods	methods; //can be GET or POST, maybe HEAD?
}				t_cgi_pass;

/*
**	describes a location context for a specific server
**	the path field contains the path we want to be routed by the server
**	when set, optional fields override the parent server's fields
**
** note: cgi_pass: see note below in struct s_srv.
*/
typedef struct				s_loc
{
	// mandatory
	std::string				path;

	// optional
	std::string				root;
	std::string				alias;
	std::string				index;
	t_error_page			error_page;
	bool					autoindex;
	unsigned int			cli_max_size;
	t_methods				methods;
	std::list<t_cgi_pass>	lst_cgi_pass;
}							t_loc;

/*
**	describes a virtual server, binded to a host:port
**	mandatory fields must be set at init time
**	optional fields take a default value if not set at init time
**
** note: cgi_pass: it contains a list of struct t_cgi_pass. each element
**	contains a file extension, and a given binary to launch for this given file
**	extension. This allows to have different binaries launched for different
**	files extension within the same server block. Same goes for location block.
*/
typedef struct				s_srv
{
	// mandatory
	unsigned int			host_port;
	std::string				root;

	// optional
	std::string				server_name;
	std::string				index;
	t_error_page			error_page;
	std::string				directory;
	bool					autoindex;
	unsigned int			cli_max_size;
	t_methods				methods;
	std::vector<t_loc>		locations;
	std::list<t_cgi_pass>	lst_cgi_pass;
}							t_srv;

int parse_config(const char *path, std::vector<t_srv> *servers);

/*
** This structure will contain a pair: fd_listen && virtual_server_ptr.
** A vector of this structures will be build when starting our server.
** Each virtual server block will be associated to a socket listening on the
** newtork.
** When select() returns, we will easyly identify that a client attempts to
** connect to one of our servers when going through that vector. Plus this
** structures will help us in keeping track of which virtual server block the
** client attempts to connect to and wether the connexion should or should not
** be accepted/processed.
**
** virtual_server_ptr:	a pointer to a server block and its informations.
** fd_listen:	the socket listening for that specific virtual server block.
*/
typedef struct	s_pair_fd_listen_t_srv
{
	int		fd_listen;
	t_srv	v_server;
}				t_pair_fd_listen_t_srv;

/*
** This structure will contain the fildescriptors to be monitored for
** non-blocking I/O operations. It is acting as a database.
**
** Note: Each time a new file descriptor is to be monitored by select, we will
** add it to the corresponding category (writable, readable or exceptions).
** When we no longer need to monitor a fd, we close it and remove it from this
** "database".
**
** Note: The list_all_fd member contains a mixed (read, write and except),
** ordered (first is the highest) linked-list of all the file descriptors. It
** will be used to know the highest fd (+ 1),requested as the first parameter
** to select() function.
*/
typedef struct s_fd_set_real
{
	fd_set 	fd_set_write;
	fd_set 	fd_set_read;
	fd_set 	fd_set_except;
	std::list<int>		fd_mixed_list_write;
	std::list<int>		fd_mixed_list_read;
}				t_fd_set_real;

/*
** This structure will contain the fd_sets that will be feed to select(). Those
** fd_sets are value-return variables. They will need to be re-updated after
** each call to select(). They are copied from the t_fd_set_real structure.
*/
typedef struct s_fd_set_copy
{
	fd_set 	fd_set_write;
	fd_set 	fd_set_read;
	fd_set 	fd_set_except;
}				t_fd_set_copy;

#endif // WEBSERV_HPP

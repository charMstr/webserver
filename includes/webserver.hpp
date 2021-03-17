/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/22 08:54:24 by lfalkau           #+#    #+#             */
/*   Updated: 2021/03/17 07:54:19 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

# include <netinet/in.h> //for inet_addr()
//# include <arpa/inet.h> //for inet_addr() as well ?
# include <sys/select.h> //declaration of the "fd_set" type and select().
# include <stdlib.h>  //needed for "NULL" declaration.
# include <unistd.h> //needed by open().
#include <fcntl.h> //for fcntl, seting O_NONBLOCK flag.

# include <exception>
# include <iostream>
# include <list>
# include <vector>

# include "http_methods.hpp"
# include "../includes/Service.hpp"
#include "../includes/Service.hpp"
#include "../includes/request_service.hpp"

# define DFL_CNF_PATH "./cnf/webserv.conf"
# define DFL_CNF_MAX_SIZE 4096
# define BACKLOG 128 //max number of pending connexion on a listening socket.

/*
**	describes a location context for a specific server
**	the path field contains the path we want to be routed by the server
**	when set, optional fields override the parent server's fields
*/
typedef struct s_loc
{
	// mandatory
	std::string			path;

	// optional
	std::string			root;
	std::string			index;
	std::string			error;
	unsigned int		cli_max_size;
	t_methods			methods;
} t_loc;

/*
**	describes a virtual server, binded to a host:port
**	mandatory fields must be set at init time
**	optional fields take a default value if not set at init time
*/
typedef struct s_srv
{
	// mandatory
	struct sockaddr_in	host;
	std::string			root;

	int					port;
	// optional
	std::string			server_name;
	std::string			index;
	std::string			error;
	unsigned int		cli_max_size;
	t_methods			methods;
	std::vector<t_loc>	locations;
} t_srv;

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
** This structure contains a pair: [ list of fd && pointer to abstract class ]
**  
**  fd_list: a linked lis of fildescriptors this structure is related to.
**  	we will compare the fildescriptors in this list to the
**  	fd_set_copy.fd_set_... with the FD_ISSET() maccro. If there is posotiv
**  	result, we will call the appropriate function from the Service pointer.
**  Service *: a pointer of type pointer to abstract class. Which will be used
**  	for polymorphism purpose. Each instanciation will have its own
**  	implementation of the read() and write() methods.
**
**	Note: a same Service concrete Object can have mutliple fd: exexmple of a
**		get_method which gives us a derived object that reads on one fd (open
**		file) and writes on another fd(connexion socket).
*/
typedef struct	s_pair_fd_service
{
	std::list<int> fd_list;
	Service *ptr;
}				t_pair_fd_service;

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
	std::list<int>		fd_mixed_list;
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

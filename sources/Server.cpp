/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 02:17:15 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 13:23:56 by lspiess          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"
#include "../includes/exceptions.hpp"
#include "../includes/server_utils.hpp"

/*
** Constructor for the server object. It will initialise all the fd_sets which
** will play the role of databases for the fildescriptors to be monitored by
** select().
**
** NOTE: the fd_sets are reset at the very start, then FD_SET() and FD_CLR()
**	should be used in the rest of the program.
*/

Server::Server(void)
{	
	FD_ZERO(&fd_set_real.fd_set_read);
	FD_ZERO(&fd_set_real.fd_set_write);
	FD_ZERO(&fd_set_real.fd_set_except);

	//DEBUG USAGE
	debug_counter = 0;
	return ;
}

/*
** destructor for the Server class. What needs to be cleaned:
** - vector_listen: in each cell a fd_listen which need to be closed. 	OK
** - list_services: contains a list of fd to be closed  				NON OK
** - list_services: contains a ptr to object allocated on the heap.		NON OK
**
** VALIDATE THE "NON OK" parts with some tests.
*/

Server::~Server(void)
{
	std::cout << "DEBUG: closing listening sockets:" << std::endl;
	for (std::vector<t_pair_fd_listen_t_srv>::iterator it \
			= vector_listen.begin(); it != vector_listen.end(); it++)
	{
		printf("DEBUG:		closing fd = %d\n", (*it).fd_listen);
		close((*it).fd_listen);	
	}
	std::cout << "DEBUG: closing all fd from list_services" << std::endl;
	for (std::list<t_pair_fd_service>::iterator it = list_services.begin(); \
			it != list_services.end(); it++)
	{
		for (std::list<int>::iterator nested_it = it->fd_list.begin(); \
			nested_it != it->fd_list.end(); nested_it++)
		{
			printf("DEBUG:		closing fd = %d\n", *nested_it);
			close(*nested_it);
		}
		delete(it->ptr);
	}
	return ;
}

/*
** THIS FUNCTION SHOULD BE MERGED WITH PRIVATE METHODE
** set_up_pair_fd_listen_t_srv()
**
** This public method will take care of setting up our server before we can
** actually start the main loop with the start_work() method.
**
** It will build a member variable, which is a vector containing pairs made of
** a virtual server and a listening socket created for it.
**
** PARAMETERS:
**	- virtual_servers: It is a vector of structures yielded after parsing the
** 	configuration file. Each structure represents a virtual server and its
** 	informations.
**
** RETURN: void
** WARNING: This method should throw an exception if
**	- the creation of sockets listening on the network fails.
**	- if the allocation of the vector fails.
*/

void 
Server::initialise(const std::vector<t_srv> &virtual_servers)
{
	(void )virtual_servers;

	//DUMMY FUNCTION that creates a fake vector of virtuals servers for now.
	std::vector<t_srv> dummy_servers;
	dummy_servers_creation(&dummy_servers);

	//should throw if an error occured.
	set_up_pair_fd_listen_t_srv(dummy_servers);
	debug_vector_listen();
}

/*
** This function should be merged with the public method initialise().
**
** This function populates a member variable named vector_listen. vector_listen
** contains in each cell a structure that is a pair. The pair is made of a
** virtual server from the parameter servers, and a listening socket created
** for it.
**
** PARAMETERS: servers, vector of virutal servers yielded from the parsing.
**
** NOTE: Each fd_listen created is added to fd_set_real.fd_set_read;
**
** RETURN: void
** WARNING: /!\ this function can throw exceptions if:
** 	- listening socket creation fails: exception_webserver thrown.
** 	- bad alloc happens with vector_listen, or fd_set_real.fd_mixed_list. Those
** 		exceptions are caught and rethrow in the form of a exception_webserver.
**
*/
int Server::set_up_pair_fd_listen_t_srv(std::vector<t_srv> &virtual_servers)
{
	t_pair_fd_listen_t_srv pair;
	for (std::vector<t_srv>::iterator it = virtual_servers.begin(); \
			it != virtual_servers.end(); it++)
	{
		pair.v_server = *it; 
		//this can throw an exception_webserver.
		pair.fd_listen = build_listening_socket(*it);
		try //this can throw with bad_alloc from std::list, or std::vector
		{
			add_fd_to_real_set(pair.fd_listen, fd_set_real.fd_set_read);
			vector_listen.push_back(pair);
		}
		catch (std::exception &e)
		{
			//closing the fd opened and placed in that pair. The pair was never
			//added to vector_listen
			close(pair.fd_listen);
			// and rethrow our custom exception type.
			throw exception_webserver(strerror(errno), INITIALISING);
		}
	}
	return (0);
}

/*
** This member function will build a listening socket for a given virtual
** server.
**
** PARAMETERS:
**	- server: a structure representing a virtual server.
**
** WARNING/!\: This function throws an exception and closes the listening
** 	socket in case of failure.
**
** RETURNS: the socket we created (its file descriptor).
*/
int Server::build_listening_socket(t_srv &server)
{
	int sock;

	/* Create the socket. */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw exception_webserver(strerror(errno), " socket()", INITIALISING);
	}
	/* Give the socket a name. */
	server.host.sin_family = AF_INET;
	server.host.sin_port = htons (server.port); //watch out to not do it twice...
	server.host.sin_addr.s_addr = htonl (INADDR_ANY); //same, dont do twice htonl
	if (bind (sock, (struct sockaddr *) &server.host, sizeof (server.host)) < 0)
	{
		close (sock);
		throw exception_webserver(strerror(errno), " bind()", INITIALISING);
	}
	if (listen(sock, BACKLOG) < 0)
	{
		close (sock);
		throw exception_webserver(strerror(errno), " listen()",INITIALISING);
	}
	return sock;
}

/*
** this function will set the O_NONBLOCK flag on already accepted sockets
*/

void Server::set_nonblock(int socket) {
    if ((fcntl(socket, F_SETFL, O_NONBLOCK)) == -1)
		throw exception_webserver(strerror(errno), " fcntl()", WORKING);
}

/*
** This function will add a fd to the fd_set given as parameter. The fd_set
** should be part of the fd_set_real structure and not its copy (fd_set_copy).
** It will also add the fd to the ordered list fd_mixed_list, containing all
** fd types to be monitored by select(). This list helps us in knowing the
** first argument to the select function (highest fd among the mixed_fd + 1).
**
** WARNING/!\: this function can throw a bad_alloc exception.
**
** See the function remove_fd_from_real_set(), which does the exact opposite.
*/
void Server::add_fd_to_real_set(int fd, fd_set& set)
{
	FD_SET(fd, &set);
	for (std::list<int>::iterator it = fd_set_real.fd_mixed_list.begin(); \
			it != fd_set_real.fd_mixed_list.end(); it++)
	{
		if (*it < fd)
		{
			fd_set_real.fd_mixed_list.insert(it, fd);
			return;
		}
	}
	fd_set_real.fd_mixed_list.push_back(fd);
}

/*
** This function will remove a fd to the fd_set given as parameter. The fd_set
** should be part of the fd_set_real structure and not its copy.
** It will also remove the fd from the ordered list containing all mixed fd.
**
** See the function add_fd_ro_real_set(), which does the exact opposite.
*/
void Server::remove_fd_from_real_set(int fd, fd_set& set)
{
	FD_CLR(fd, &set);
	for (std::list<int>::iterator it = fd_set_real.fd_mixed_list.begin(); \
			it != fd_set_real.fd_mixed_list.end(); it++)
	{
		if (*it == fd)
		{
			fd_set_real.fd_mixed_list.erase(it);
			return;
		}
	}
}

/*
** start_work: This function will be the entry point to the main work of the
** server, it is the core function that loops forever and calls select() until
** the server is being shut down manually or a problem occurs.
**
**	NOTE: Things already done when enterring this function:
** 	- parsing of configuration file of the webserver and creation of a
** 		structure for each virtual server blocks.
** 	- for each virtual server block, a fd_listen has been created, it is a
** 		socket listening on the network.
** 	- we have a link list of a structure representing a pair:
**		{ virtual_server_block_ptr && fd_listen }.
**
**	NOTE: To get out of the infinite loop:
** 	- signals received by our application
** 	- fatal error occured
**
**	WARNING: in this function exceptions can be thrown in call_select(),
**		check_incoming_connexions()...
**
**	RETURN: ... to be continued.
*/
int Server::start_work()
{
	int res_select;

	while (1) //infinite loop of the server.
	{
		std::cout << "Entering select()" << std::endl;
		res_select = call_select();
		std::cout << "Out of select()" << std::endl;

		//1) function that checks if we dont have an incoming connexion
		// (compares fd_set_copy.fd_set_read with the all the fd in the vector
		// vector_listen) CREATION OF A REQUEST
		check_incoming_connexions();

		//2) go over the linked list list_services, and in each structure,
		//check the fd against fd_sets with FD_ISSET() macros. If we have a
		//match for the fd_set_read, call the read function of the matching
		//object in the pair. same goes for a match with a fd_set_write.
		resume_processed_services();
		
	}
	//DO WE NEED TO HAVE DIFFERENT RETURNED VALUES?
	//SIGNAL MANAGMENTS ETC? LEAVE SOME WORK FOR THE shutdown() METHOD.
	return (0);
}

/*
** This function will go through the list of listening fd, and try to see if
** one of them is ready to accept an incomming connexion. If yes, we create a
** fd_client with accept(), add it to the set_fd_real in the read_set, and we
** also create on the heap an object "Request" derived from the abstract class
** Service. We finally add a pair made of this fd_client and this
** request_service to the list of processed services.
**
** NOTE: the fd_client is added to set_fd_read, but also set to be non-blocking
** so that upcoming writes or reads wont be blocking on this socket.
**
** NOTE: for now, the listening sockets are still blocking. to be changed if
** our server uses signals.
**
** WARNING: throwing custom exceptions
*/

void Server::check_incoming_connexions()
{
	socklen_t				client_addr_size;
	struct sockaddr_storage	client_addr; //can hold either ipv4 or ipv6
	int						fd_client;

	for (std::vector<t_pair_fd_listen_t_srv>::iterator it = vector_listen.begin(); \
			it != vector_listen.end(); it++)
	{
		//yes means we need to accept connexion and create a request_service
		if (FD_ISSET(it->fd_listen, &fd_set_copy.fd_set_read))
		{
			t_pair_fd_service pair;

			client_addr_size = sizeof(struct sockaddr_storage);
			//note: those fd_listen should be set to non blocking before?
			//maybe in the case our program will need to use signals, for cgi
			//impletmenation for example... but probably pipes will do.
			if ((fd_client = accept(it->fd_listen, \
					(struct sockaddr *)&client_addr, &client_addr_size)) < 0)
			{
				throw exception_webserver(strerror(errno), " accept()", \
						WORKING);	
			}
			set_nonblock(fd_client);	
			try
			{
				pair.ptr = new Request_service(client_addr, client_addr_size);
			}
			catch (std::exception &e)
			{
				close(fd_client); //was never added to the list_services.
				throw exception_webserver(strerror(errno), \
						" Request_service()", WORKING);
			}
			try
			{
				//add new fd to fd_set. can throw.
				add_fd_to_real_set(fd_client, fd_set_real.fd_set_read);	
				pair.fd_list.push_back(fd_client);
				list_services.push_back(pair);		
			}
			catch (std::exception &e)
			{
				delete(pair.ptr);
				close(fd_client); //was never added to list_services.
				throw exception_webserver(strerror(errno), \
						" check_incoming_connexions()", WORKING);
			}
		}
	}
}

/*
** This function will iterate through the linked list list_services (containing
** pairs [file descriptor && pointer to Service object]).
** For a single element of the list, we try to macth its corresponding file
** descriptors with a fd_set. If there is a match, we call the corresponding
** method in the service concrete object.
**
** exemple: for a given link, FD_ISSET() macro is used with the a fd and the 
** fd_set_copy->fd_set_read; If there is a match, it means we should call the
** read method from the Service object pointed to in this link.
*/
int Server::resume_processed_services()
{
	for (std::list<t_pair_fd_service>::iterator it = list_services.begin(); \
			it != list_services.end(); it++)
	{
		/* for a given link (*it), go through each fd in the fd_list, then if
		** there is is a match call either read() or write().
		*/
		for (std::list<int>::iterator i = (*it).fd_list.begin();
				i != (*it).fd_list.end(); ++i)
		{
			/* for every fd in a given link's fd_list, check if they have a
			** match in one of the three t_fd_set_copy
			*/
			int fd = *i;
			bool issetread, issetwrite, issetexcept;
			issetread = (FD_ISSET(fd, &fd_set_copy.fd_set_read) != 0 ? 1 : 0);
			issetwrite = (FD_ISSET(fd, &fd_set_copy.fd_set_write) != 0 ? 1 : 0);
			issetexcept = (FD_ISSET(fd, &fd_set_copy.fd_set_except) != 0 ? 1 : 0);

			std::cout << "Current fd being checked : " << fd << "\n";
			std::cout << "Is in fd_set_copy.fd_set_read : " << issetread << "\n";
			std::cout << "Is in fd_set_copy.fd_set_write : " << issetwrite << "\n";
			std::cout << "Is in fd_set_copy.fd_set_except : " << issetexcept << "\n";

			// Call the service's subroutine if one of its fds is "active"
			if (issetread)
				(*it).ptr->read(fd);
			if (issetwrite)
				(*it).ptr->write(fd);
		}
	}
	// sleep for reaability purposes when testing
	sleep(1);
	return (0);
}

/*
** This private method's main purpose is to call select(). This method is
** called in the main loop of start_work() method. It will place in some
** fd_sets (copy version) some file_descriptors, then call select.
**
** Note: In the fd_sets, only the file descriptors upon which we can operate
** some non blocking I/O operations will remain.
**
** WARNING: throwing custom exception if select() fails
**
** RETURN:	the number of fd still activated in the fd_set_copy's sets.
*/
int Server::call_select()
{
	int res;

	//recopies all the real fd_sets into the copy sets.
	FD_ZERO(&fd_set_copy.fd_set_write);
	FD_ZERO(&fd_set_copy.fd_set_read);
	FD_ZERO(&fd_set_copy.fd_set_except);
	FD_COPY(&fd_set_real.fd_set_write, &fd_set_copy.fd_set_write);
	FD_COPY(&fd_set_real.fd_set_read, &fd_set_copy.fd_set_read);
	FD_COPY(&fd_set_real.fd_set_except, &fd_set_copy.fd_set_except);

	// warning: select will be woken up if a signal occurs. use the self-pipe
	// trick here maybe? But for now we dont use signals...

	if ((res = select(fd_set_real.fd_mixed_list.front() + 1, 
				&fd_set_copy.fd_set_read, &fd_set_copy.fd_set_write, \
				&fd_set_copy.fd_set_except, NULL)) == -1)
		throw exception_webserver(strerror(errno), " select()", WORKING);
	return (res);
}

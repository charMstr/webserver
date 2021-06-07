#include "../includes/server.hpp"
#include "../includes/Service.hpp"
#include "../includes/exceptions.hpp"
#include "../includes/utils.hpp"
#include "../includes/request_reader.hpp"
#include "../includes/request_parser.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/Service_generator.hpp"
#include "../includes/get_service.hpp"

/*
** Constructor for the server object. It will initialise all the fd_sets which
** will play the role of databases for the fildescriptors to be monitored by
** select().
**
** NOTE: the fd_sets are reset at the very start, then FD_SET() and FD_CLR()
**	should be used in the rest of the program.
*/

const char *	Server::no_more_alloc_answer = \
			"HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

const size_t	Server::size_no_more_alloc_answer = \
		sizeof("HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\nContent-Length: 0\r\n\r\n") - 1;

Server::Server(void) :
	no_more_alloc_occured_in_server(0)
{
	FD_ZERO(&fd_set_real.fd_set_read);
	FD_ZERO(&fd_set_real.fd_set_write);
	FD_ZERO(&fd_set_real.fd_set_except);
	return ;
}

/*
** destructor for the Server class. What needs to be cleaned:
** - vector_listen: in each cell a fd_listen which needs to be closed. 	OK
** - list_request_readers: contains a fd_client, needs to be closed. 	OK
** - list_services: contains pointers allocated on the heap.			OK
**
** Note: list_services: each node contains a list_fd_write and a list_fd_read.
** those are closed in the Service class's destructor.
*/

Server::~Server(void)
{
//	DEBUG std::cout << "WebServer Shuting down." << std::endl;
//	DEBUG std::cout << "closing listening sockets:" << std::endl;
	for (std::vector<t_pair_fd_listen_t_srv>::iterator it \
			= vector_listen.begin(); it != vector_listen.end(); it++)
	{
//		DEBUG std::cout << "\tclosing fd = " <<  (*it).fd_listen << std::endl;
		close((*it).fd_listen);
	}
//	DEBUG std::cout << "closing fd client sockets:" << std::endl;
	for (std::list<Request_reader>::iterator it = list_request_readers.begin(); \
			it != list_request_readers.end(); it++)
	{
//		DEBUG std::cout << "\tclosing fd = " << it->get_client_fd() << std::endl;
		close(it->get_client_fd());
	}
//	DEBUG std::cout << "deleted Service* from list_services" << std::endl;
	for (std::list<Service *>::iterator it = list_services.begin(); \
			it != list_services.end(); it++)
		delete(*it); //delete the Service pointers we allocated on the heap.
	return ;
}

/*
** This function populates a member variable named vector_listen. vector_listen
** contains in each cell a structure that is a pair. The pair is made of a
** virtual server from the parameter virtual_servers, and a listening socket
** created for it.
**
** PARAMETERS:
**	- virtual_servers: It is a vector of structures yielded after parsing the
** 	configuration file. Each structure represents a virtual server and its
** 	informations.
**
** NOTE: Each fd_listen created is added to fd_set_real.fd_set_read;
**
** RETURN: void
** WARNING: /!\ this function can throw exceptions if:
** 	- listening socket creation fails: exception_webserver thrown.
** 	- bad alloc happens with vector_listen, or fd_set_real.fd_mixed_list. Those
** 		exceptions are caught and rethrow in the form of a exception_webserver.
*/

void
Server::initialise(std::vector<t_srv> &virtual_servers)
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
			add_fd_to_real_set_read(pair.fd_listen);
			vector_listen.push_back(pair);
		}
		catch (std::exception &e)
		{
			//closing the fd opened and placed in that pair. The pair was never
			//added to vector_listen
			close(pair.fd_listen);
			// and rethrow our custom exception type.
			throw exception_webserver(strerror(errno), \
					"Server::initialise()", __FILE__, __LINE__, INITIALISING);
		}
	}
	//DEBUG_(DEBUG_SERVER_CLASS) debug_vector_listen(vector_listen, fd_set_real);
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
	struct sockaddr_in host_addr;

	/* Create the socket. */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw exception_webserver(strerror(errno), \
				"Server::build_listaning_socket()", __FILE__, __LINE__, \
				"socket()", INITIALISING);
	}

	//make sure we can shutdown and restart server quickly
	set_socket_rebindable_for_quick_restart(sock);

	/* Give the socket a name. */
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons (server.host_port);
	host_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (sock, (struct sockaddr *) &host_addr, sizeof (host_addr)) < 0)
	{
		close (sock);
		throw exception_webserver(strerror(errno), \
				"Server::build_listening_socket()", __FILE__, __LINE__, \
				"bind()", INITIALISING);
	}
	if (listen(sock, BACKLOG) < 0)
	{
		close (sock);
		throw exception_webserver(strerror(errno), \
				"Server::build_listening_socket()", __FILE__, __LINE__, \
				"listen()", INITIALISING);
	}
	set_nonblock(sock);
	return sock;
}

/*
** this function will add a fd to the fd_set_real.fd_set_write. it also
** adds the fd to the list fd_mixed_list_write, containing all the fd
** in which select monitors for a possible write.
**
** NOTE: the addition of fd into fd_set_real is done after operaion that
** could throw, on purpose.
**
** WARNING/!\: this function can throw a bad_alloc exception.
**
** NOTE: See the function remove_fd_from_real_set_write(), which does the
** exact opposite. And the function add_fd_to_real_set_read...
*/
void Server::add_fd_to_real_set_write(int fd)
{
	for (std::list<int>::iterator it = fd_set_real.fd_mixed_list_write.begin(); \
			it != fd_set_real.fd_mixed_list_write.end(); it++)
	{
		if (*it < fd)
		{
			fd_set_real.fd_mixed_list_write.insert(it, fd);
			FD_SET(fd, &fd_set_real.fd_set_write);
			return;
		}
	}
	fd_set_real.fd_mixed_list_write.push_back(fd);
	FD_SET(fd, &fd_set_real.fd_set_write);
}

/*
** this function will add a fd to the fd_set_real.fd_set_read. it also
** adds the fd to the list fd_mixed_list_read, containing all the fd
** in which select monitors for a possible read.
**
** NOTE: the addition of fd into fd_set_real is done after operaion that
** could throw, on purpose.
**
** WARNING/!\: this function can throw a bad_alloc exception.
**
** NOTE: See the function remove_fd_from_real_set_read(), which does the
** exact opposite. And the function add_fd_to_real_set_read...
*/
void Server::add_fd_to_real_set_read(int fd)
{
	for (std::list<int>::iterator it = fd_set_real.fd_mixed_list_read.begin(); \
			it != fd_set_real.fd_mixed_list_read.end(); it++)
	{
		if (*it < fd)
		{
			fd_set_real.fd_mixed_list_read.insert(it, fd);
			FD_SET(fd, &fd_set_real.fd_set_read);
			return;
		}
	}
	fd_set_real.fd_mixed_list_read.push_back(fd);
	FD_SET(fd, &fd_set_real.fd_set_read);
}

/*
** This function will remove a fd to the fd_set_real.fd_set_write.  It will
** also remove the fd from the ordered list containing all mixed fd in which
** select wants to write.
**
** See the function add_fd_ro_real_set_write(), which does the exact opposite.
** See also remove_fd_from_real_set_read()
*/
void Server::remove_fd_from_real_set_write(int fd)
{
	FD_CLR(fd, &fd_set_real.fd_set_write);
	for (std::list<int>::iterator it = fd_set_real.fd_mixed_list_write.begin(); \
			it != fd_set_real.fd_mixed_list_write.end(); it++)
	{
		if (*it == fd)
		{
			fd_set_real.fd_mixed_list_write.erase(it);
			return;
		}
	}
}

/*
** This function will remove a fd to the fd_set_real.fd_set_read.  It will
** also remove the fd from the ordered list containing all mixed fd in which
** select wants to read.
**
** See the function add_fd_ro_real_set_read(), which does the exact opposite.
** See also remove_fd_from_real_set_read()
*/
void Server::remove_fd_from_real_set_read(int fd)
{
	FD_CLR(fd, &fd_set_real.fd_set_read);
	for (std::list<int>::iterator it = fd_set_real.fd_mixed_list_read.begin(); \
			it != fd_set_real.fd_mixed_list_read.end(); it++)
	{
		if (*it == fd)
		{
			fd_set_real.fd_mixed_list_read.erase(it);
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
** 	- we have a vector containing structures representing a pair:
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
		//DEBUG debug_enter_func("select()");
		res_select = call_select();
		//DEBUG debug_exit_func("select()");

		//1) function that checks if we dont have an incoming connexion
		// (compares fd_set_copy.fd_set_read with all the fd in the vector
		// vector_listen) CREATION OF A REQUEST_SERVICE
		//DEBUG debug_enter_func("check_incoming_connexions()");
		check_incoming_connexions(res_select);
		//DEBUG debug_exit_func("check_incoming_connexions()");

		//2) function that tries to complete colletcting the initial requests
		// from clients and then creates responses objects.
		//DEBUG debug_enter_func("resume_processed_requests_from_clients()");
		resume_processed_requests_from_clients(res_select);
		//DEBUG debug_exit_func("resume_processed_requests_from_clients()");

		//3) function that tries to resume the I/O operations for each servers'
		// responses objects currently being processed.
		//DEBUG debug_enter_func("resume_processed_services_to_clients()");
		resume_processed_services_to_clients(res_select);
		//DEBUG debug_exit_func("resume_processed_services_to_clients()");
	}
	//DO WE NEED TO HAVE DIFFERENT RETURNED VALUES?
	//SIGNAL MANAGMENTS ETC? LEAVE SOME WORK FOR THE shutdown() METHOD.
	return (0);
}

/*
** This function will loop over the linked list "list_request_readers", which
** contains the current clients' requests currently being processed.  Each node
** contains a special fd: "fd_client", the client socket on which we want to
** keep doing read()/recv() operations until we reach the request_readers'
** state of DONE. When we yielded enough informations (status line and headers,
** i.e. full body of the request), we can generate a Service. The reader might
** keep on reading along with the service doing other operations. The reader's
** lifespan is always longer than the service it generates. The reader is
** destroyed last. In the context of keep alive, the request_reader, might just
** reinitialise itself and keep reading other requests on the same network
** socket. If the last service generated realises the connexion should end, it
** closes the fd_client, and activates a boolean that allows the reader to know
** it should clear itself from the list.
**
** PARAMETERS: - res_select: the reference on the total number of fd to take
** care of on this loop with select. It will be decremented each time we match
** a fd in the set of fd we monitor. if it reaches zero we know we can shortcut
** and go back to select.
**
** NOTE: the fd_client is closed by the service, if the service thinks/decides
** that it was the last service fot this network socket.
**
** NOTE: If during all those sus-mentioned operations some exceptions are
** raised, our server cannot afford to just shutdown. It needs to be resilient
** and keep working. It will have to provide this specific client with an error
** message with status code from the 5xx family (problem on the server side).
** And it will have to accomplish all that without no further allocation on the
** heap(avoid std::string manipulations etc...).  Also keep in mind we cannot
** write directly in the client socket, we will have to wait for one more
** loop/select() call. We will turn our object into an error handler, raising
** the bool "object_is_now_no_more_alloc_handler".
**
** NOTE: the request_reader objects are treated separatedly as they are quite
**	different from the response_service objects:
**	- They only have one fd to monitor (always a fd on which we read).
**	- That fd should not be closed when we are done.
*/

void Server::resume_processed_requests_from_clients(int &res_select)
{
	for (std::list<Request_reader>::iterator it = \
			list_request_readers.begin(); \
			res_select > 0 && it != list_request_readers.end(); )
	{
		//yes means our object turned into a no_more_alloc_handler.
		if (it->get_object_is_now_no_more_alloc_handler())
		{
			if (FD_ISSET(it->get_client_fd(), &fd_set_copy.fd_set_write))
				no_more_alloc_handler_for_Request_reader(it);
		}
		else if (it->can_be_deleted())
			erase_processed_request_from_clients(it);
		//yes means the request timed out, generate an error service.
		else if (it->does_request_timed_out() && it->service_already_generated)
		{
			try {
				resume_processed_requests_from_clients_generate_svc(it);
				it++;
			}
			catch (const std::exception &e)
			{ //the current object becomes a no_more_alloc_handler().
				//DEBUG debug_deletion("fd_real_set.read (timeout)", \
				//	__FILE__, __LINE__, (*svc_it)->get_fd_client());
				remove_fd_from_real_set_read(it->get_client_fd());
				no_more_alloc_occured_in_server = 1;
				it->set_object_is_now_no_more_alloc_handler();
				it++;
			}
		}
		//yes means we can keep trying to read on the client socket.
		else if (FD_ISSET(it->get_client_fd(), &fd_set_copy.fd_set_read))
		{
			res_select--;
			try
			{
				//keep reading, even if the service has alread been created.
				int previous_status = it->get_status();
				int res;
				if ((res = it->read_on_client_socket()) > 0)
				{
					//DEBUG debug_timeout_reset(it->get_client_fd());
					it->reset_timeout_reference();
				}
				// if a first error occured while reading, we make sure the svc
				// gets ready to answer an error msg.
				else if (previous_status == 200 && it->get_status() != 200 \
						&& it->ptr_service_generated)
					it->ptr_service_generated->error_occured_set_up_everything(it->get_status(), false);
				//make sure a res < 0 triggers the fact that this is the last
				//answer on this client socket.
				if (res < 0 && it->ptr_service_generated)
					it->ptr_service_generated->service_thinks_reader_can_be_deleted = true;

				//make sure to flush the body_from_client as we go if not used
				//later
				if (it->ptr_service_generated && \
					(it->ptr_service_generated->get_error_occured() || \
					((it->get_method() != PUT) && (it->get_method() != POST))))
					it->body_from_client.erase();

				//DEBUG debug_request_reader(*it);
				// Generate a service if:
				// - A service has not been created yet.
				// - An error occured during the parsing
				// - The reader state is set to BODY and the cli_max_size is set to 0 (unlimited)
				// - The reader state is set to DONE
				if	(!it->service_already_generated \
					&& (it->get_status() != 200 || \
					(it->get_request_state() == Request_reader::BODY && !it->get_cli_max_size()) || \
					it->get_request_state() == Request_reader::DONE))
				{
					resume_processed_requests_from_clients_generate_svc(it);
				}
				it++;
			}
			catch (const std::exception &e)
			{ //the current object becomes a no_more_alloc_handler().
				//DEBUG debug_deletion("fd_real_set.read (timeout)", \
				//	__FILE__, __LINE__, (*svc_it)->get_fd_client());
				remove_fd_from_real_set_read(it->get_client_fd());
				no_more_alloc_occured_in_server = 1;
				it->set_object_is_now_no_more_alloc_handler();
				it++;
			}
		}
		else
			it++;
	}
//	DEBUG_(DEBUG_SERVER_CLASS) debug_exiting_resume_processed_requests_from_clients(list_request_readers);
}

/*
** This function is called in resume_processed_requests_from_clients() when the
** request_reader needs to be erased from the list.
** It mainly exists for readibility of the code. The fd_client has already been
** closed (when we answerer or tried to answer our error response), and the
** fd_client has already been removed from the fd_set_real.read (fds monitored
** by select).
*/

void
Server::erase_processed_request_from_clients(\
		std::list<Request_reader>::iterator &it)
{
	//DEBUG debug_deletion("Request_reader",__FILE__, __LINE__, \
	//		it->get_client_fd());
	it = list_request_readers.erase(it); //delete reader when done.
}

/*
** This function is called to reduce the size of function
** Server::resume_processed_requests_from_clients(). It is called while we were
** iterating on a Request_reader Object, and it considers we have enough
** informations to start generating the service.
**
** Its purpose is to create an object of type Service*.
** More precisely, it will:
** - remove the fd_client from the set fd_set_real, always!
** - create a Service object accordingly to parsed request.
** - re-add to fd_set_real all the fd used by the newly created service.
**
** note: any exception will be caught in calling function, turning the object
** itself into a no_more_alloc_handler (recycling the object to avoid further
** memory allocations).
**
** NOTE: When creating an object of type Service*, from a call to
**	service_generator.generate(), If some exception is raised, it is the
**	Service_generator's responsibility to make sure it:
**	1) catches the exception.
**	2) closes any fd it opened (so not the fd_client, obviously already opened).
**	3) rethrows the exact same exception.
*/

void Server::resume_processed_requests_from_clients_generate_svc(\
		std::list<Request_reader>::iterator it)
{
	//Create a service_generator object with the reader, and its internal
	//parser.
	Service_generator service_generator(*it);

	//note: exceptions are handled and rethrow in next func.
	Service *svc = service_generator.generate();

	//to not regenerate a service for the same request
	it->service_already_generated = true;
	//When we delete the service, we set its pointer to NULL
	it->ptr_service_generated = svc;
	try
	{
		from_service_add_fd_to_real_set(svc);
		//DEBUG_(DEBUG_SERVER_CLASS)	debug_addition("Service*", \
		//		__FILE__, __LINE__, it->get_client_fd());
		list_services.push_back(svc);
	}
	catch (const std::exception &e)
	{
		//DEBUG std::cout << e.what() << "file: "<< __FILE__ " and line: "<< __LINE__  << std::endl;
		from_service_remove_fd_from_real_set(svc);
		delete svc; //destructor closes all the opened_fd except fd_client
		throw e; //delete allocated memory and rethrow.
	}
}

/*
** This function is called after a Service object has been generated. It
** contains fds that are to be monitored in its fd_list_read and fd_list_write.
** This function will add all those fd to the fd_set_real structure acting as a
** database for select().
**
** Note: among the fd_list_write is the fd_client. It is also added to
** 	fd_set_real.
*/
void
Server::from_service_add_fd_to_real_set(const Service *svc)
{
	for (std::list<int>::const_iterator it = svc->list_fd_read.begin(); \
			it != svc->list_fd_read.end(); it++)
	{
		//DEBUG_(DEBUG_SERVER_CLASS) \
		//	debug_addition("fd_real_set.read", __FILE__, __LINE__, *it);
		add_fd_to_real_set_read(*it);
	}
	for (std::list<int>::const_iterator it = svc->list_fd_write.begin(); it != svc->list_fd_write.end(); it++)
	{
		//DEBUG_(DEBUG_SERVER_CLASS) \
		//	debug_addition("fd_real_set.write", __FILE__, __LINE__, *it);
		add_fd_to_real_set_write(*it);
	}
	//DEBUG_(DEBUG_SERVER_CLASS) debug_from_service_add_fd_to_real_set(svc->get_fd_client(), \
	//	svc->list_fd_write, svc->list_fd_read);
}

/*
** This function is called if an exception of type bad_alloc occured once a
** Service has successfully been created but could not be added properly to
** the list_services, or when a call to from_service_add_fd_to_real_set()
** failed at some stage.
**
** note: the fd_client is contained into the list_fd_write.
*/
void
Server::from_service_remove_fd_from_real_set(const Service *svc)
{
	for (std::list<int>::const_iterator it = svc->list_fd_read.begin(); \
			it != svc->list_fd_read.end(); it++)
		remove_fd_from_real_set_read(*it);
	for (std::list<int>::const_iterator it = svc->list_fd_write.begin(); \
			it != svc->list_fd_write.end(); it++)
		remove_fd_from_real_set_write(*it);
}

/*
** This function will go through the list of listening fd, and try to see if
** one of them is ready to accept an incomming connexion. If yes, we create a
** fd_client with accept(), add it to the fd_set_real.fd_read_set, and add it
** to an object of type request_reader which is added to list_request_readers.
**
** PARAMETERS:
** - res_select: the reference on the total number of fd to take care of on
** this loop with select. It will be decremented each time we match a fd in the
** set of fd we monitor. if it reaches zero we know we can shortcut and go back
** to select.
**
** NOTE: the fd_client is added to set_fd_read, but also set to be non-blocking
** so that upcoming writes or reads wont be blocking on this socket.
**
** NOTE: for now, the listening sockets are still blocking. to be changed if
** our server uses signals.
**
** WARNING: throwing custom exceptions only.
*/

void Server::check_incoming_connexions(int &res_select)
{
	t_client_info client;

	for (std::vector<t_pair_fd_listen_t_srv>::iterator it = vector_listen.begin(); \
			res_select > 0 && it != vector_listen.end(); it++)
	{
		//yes means we need to accept connexion and create a request_reader
		if (FD_ISSET(it->fd_listen, &fd_set_copy.fd_set_read))
		{
			res_select--;
			client.addr_size = sizeof(struct sockaddr_storage);
			if ((client.fd = accept(it->fd_listen, (struct sockaddr *)&client.addr, &client.addr_size)) < 0)
				continue; //log message?
			if ((fcntl(client.fd, F_SETFL, O_NONBLOCK)) == -1)
			{
				close(client.fd);
				continue; //log message?
			}
			Request_reader request(client, it->v_server);
			try
			{
				//DEBUG_(DEBUG_SERVER_CLASS) \
				//	debug_addition("Request_reader", __FILE__, __LINE__, client.fd);
				list_request_readers.push_back(request);
				//DEBUG_(DEBUG_SERVER_CLASS) \
				//	debug_addition("fd_real_set.read", __FILE__, __LINE__, client.fd);
				add_fd_to_real_set_read(client.fd);
			}
			catch (std::exception &e)
			{ close(client.fd); }
			//DEBUG_(DEBUG_SERVER_CLASS) debug_new_incoming_conexion(*it, client.fd);
		}
	}
}

/*
** This private method's main purpose is to call select(). This method is
** called in the main loop of start_work() method. It will place in some
** fd_sets (copy version) some file_descriptors, then call select.
**
** Note: In the fd_sets, only the file descriptors upon which we can operate
** some non blocking I/O operations will remain.
**
** RETURN:	the number of fd still activated in the fd_set_copy's sets.
*/

int Server::call_select()
{
	int res = 0;

	//if select returns 0 or -1 we just loop, fuck it.
	while (res <= 0)
	{
		fd_set_copy.fd_set_write = fd_set_real.fd_set_write;
		fd_set_copy.fd_set_read = fd_set_real.fd_set_read;
		fd_set_copy.fd_set_except = fd_set_real.fd_set_except;

		int highest_fd = fd_set_real.fd_mixed_list_write.front();
		if (fd_set_real.fd_mixed_list_read.front() \
				> fd_set_real.fd_mixed_list_write.front())
			highest_fd = fd_set_real.fd_mixed_list_read.front();

		if (no_more_alloc_occured_in_server)
		{
			highest_fd = add_fd_from_no_more_alloc_handlers_to_fd_set_copy(highest_fd);
			no_more_alloc_occured_in_server = false;
		}
		// warning: select will be woken up if a signal occurs. use the
		//self-pipe trick here maybe? But for now we dont use signals...
		res = select(highest_fd + 1, &fd_set_copy.fd_set_read, \
			&fd_set_copy.fd_set_write,  &fd_set_copy.fd_set_except, NULL);
	}
	//DEBUG debug_after_select_which_fd_is_set(vector_listen, \
	//		list_request_readers, list_services, fd_set_copy.fd_set_read, \
	//			fd_set_copy.fd_set_write, fd_set_copy.fd_set_except);
	return (res);
}

/*
** This function is called when the bool no_more_alloc_occured_in_server is true.
** This means that our server, while allocating some memory in objects (in a
** std::string manipulation for exemple) encountered a lack of space. The
** concerned object(s) have been transformed into a no_more_alloc_handler, so that
** we can give a 5xx error message to the client that made the request, without
** any further heap allocation.
**
** PARAMETERS:
** - highest_fd, it is the highest fd that will be given to select() as its
** 	first parameter, it is updated here with the objects being some bad_alloc
** 	handlers.
*/

int
Server::add_fd_from_no_more_alloc_handlers_to_fd_set_copy(int highest_fd)
{
	for (std::list<Request_reader>::const_iterator it = \
		list_request_readers.begin(); it != list_request_readers.end(); it++)
	{
		if (it->get_status() == -1)
		{
			FD_SET(it->get_client_fd(), &fd_set_copy.fd_set_write);
			if (it->get_client_fd()	> highest_fd)
				highest_fd = it->get_client_fd();
		}
	}
	return (highest_fd);
}

/*
** This function will assist resume_processed_services_to_clients(), it will
** loop over the list_fd_write for each node of the list_services.
** For each fd, we check if we can write, if yes we call the write_svc method.
** If we are done with the fd:
** 	- we remove it from the fd_set_real,
** 	- we remove it from within the object's list list_fd_write,
** 	- we close it unless it is the special fd_client (done at the very end of
** 		the service life).
**
** INPUT:
** 	- a given node of the list_services.
**	- res_select: the reference on the total number of fd to take care of on
**	this loop with select. It will be decremented each time we match a fd in
**	the set of fd we monitor. if it reaches zero we know we can shortcut and go
**	back to select.
**
** NOTE: if a bad_alloc exception is raised, it will be caught in the calling
** function. The object itself (*svc_it) will be turned into a bad_alloc
** handler.
*/

void Server::resume_processed_services_to_clients_WRITES_ONLY(\
		std::list<Service*>::iterator &svc_it, int &res_select)
{
	for (std::list<int>::iterator fd_it = (*svc_it)->list_fd_write.begin();
			res_select > 0 && fd_it != (*svc_it)->list_fd_write.end();)
	{
		if (FD_ISSET(*fd_it, &fd_set_copy.fd_set_write))
		{
			res_select--;
			//DEBUG_(DEBUG_BAD_ALLOC) throw std::bad_alloc(); //OK!
			(*svc_it)->write_svc_interface(*fd_it);
			if ((*svc_it)->is_done_writing_with_fd(*fd_it))
			{
				//updates fd_it internally
				clean_traces_of_fd_done(svc_it, fd_it);
			}
			else
				fd_it++;
		}
		else
			fd_it++;
	}
}

/*
** This function will assist resume_processed_services_to_clients(), it will
** loop over the list_fd_read for each node of the list_services.
** For each fd, we check if we can write, if yes we call the write_svc method.
** If we are done with the fd:
** 	- we remove it from the fd_set_real,
** 	- we remove it from within the object's list list_fd_write,
** 	- we close it unless it is the special fd_client (done at the very end of
** 		the service life).
**
** INPUT:
** 	- a given node of the list_services.
**	- res_select: the reference on the total number of fd to take care of on
**	this loop with select. It will be decremented each time we match a fd in
**	the set of fd we monitor. if it reaches zero we know we can shortcut and go
**	back to select.
**
** NOTE: if a bad_alloc exception is raised, it will be caught in the calling
** function. The object itself (*svc_it) will be turned into a bad_alloc
** handler.
*/

void Server::resume_processed_services_to_clients_READS_ONLY(\
		std::list<Service*>::iterator &svc_it, int &res_select)
{
	for (std::list<int>::iterator fd_it = (*svc_it)->list_fd_read.begin();
			res_select > 0 && fd_it != (*svc_it)->list_fd_read.end();)
	{
		if (FD_ISSET(*fd_it, &fd_set_copy.fd_set_read))
		{
			res_select--;
			//DEBUG_(DEBUG_BAD_ALLOC) throw std::bad_alloc(); //OK!
			(*svc_it)->read_svc_interface(*fd_it);
			if ((*svc_it)->is_done_reading_with_fd(*fd_it))
			{
				//updates fd_it internally
				clean_traces_of_fd_done(svc_it, fd_it);
			}
			else
				fd_it++;
		}
		else
			fd_it++;
	}
}

/*
**	This function is called when we noticed we are done with an fd. This
**	occurs either after a call to (svc*)->read_svc_interface() or to
**	(svc*)->write_svc_interface(). When in those function we are done with a
**	fd, it is closed (except if the fd is the fd_client for example), then it
**	is added to list_fd_done_reading or list_fd_done_writing. We do the same
**	for several fd at the same time if we encountered an error, in that case
**	we potentially closed many fd and added them all to list_fd_done...
**	The purpose of this function is to do the rest of the cleaning job:
**	- go through list_fd_done_reading and list_fd_done_writing,
**	- remove any fd met there from fd_set_real structure (so select doesnt
**		monitor them anymore).
**	- remove any fd met there from the list_fd_read and list_fd_write (pay
**	attention to update parameter fd_it as we are iterating on the same list in
**	calling function).
**	- clear list_fd_done_reading and list_fd_done_writing as we dont need the
**	 information it conveys anymore.
**
** PARAMETERS:
** - svc_it, the service we are iterating on, it is used to acces its lists:
** 	list_fd_read, list_fd_write, list_fd_done_reading and list_fd_done_writing
** - fd_it, the specific fd we were iterating on, it could come from
** list_fd_read, or list_fd_write. pay attention to update it if met.
**
** - except_for_fd_client: if set to true, we skip fd_client, it stays into
** lif_fd_write, and into fd_set_real.
*/
void
Server::clean_traces_of_fd_done(std::list<Service*>::iterator &svc_it, \
	std::list<int>::iterator &fd_it)
{
	//first loop on list_fd_done_reading
	for (std::list<int>::iterator it = (*svc_it)->list_fd_done_reading.begin() ;
			it != (*svc_it)->list_fd_done_reading.end(); it++)
	{
		//DEBUG debug_deletion("fd_set_read.read", __FILE__, __LINE__, *it);
		remove_fd_from_real_set_read(*it);
		//loop over list_fd_read and remove while updating the iterator fd_it
		for (std::list<int>::iterator it2 = (*svc_it)->list_fd_read.begin();
				it2 != (*svc_it)->list_fd_read.end();)
		{
			if (*it2 == *it)
			{
				//DEBUG debug_deletion("it->list_fd_read",__FILE__, __LINE__, \
				//		*it2);
				if (it2 == fd_it) //update fd_it in that case.
				{
					it2 = (*svc_it)->list_fd_read.erase(it2);
					fd_it = it2;
				}
				else
					it2 = (*svc_it)->list_fd_read.erase(it2);
			}
			else
				it2++;
		}
	}
	//then loop on list_fd_done_writing
	for (std::list<int>::iterator it = (*svc_it)->list_fd_done_writing.begin() ;
			it != (*svc_it)->list_fd_done_writing.end(); it++)
	{
		remove_fd_from_real_set_write(*it);
		//DEBUG debug_deletion("fd_set_real.write", __FILE__, __LINE__, *it);
		//loop over list_fd_write and remove while updating the iterator fd_it
		for (std::list<int>::iterator it2 = (*svc_it)->list_fd_write.begin();
				it2 != (*svc_it)->list_fd_write.end();)
		{
			if (*it2 == *it)
			{
				//DEBUG debug_deletion("it->list_fd_write",__FILE__, __LINE__, \
				//		*it2);
				if (it2 == fd_it)
				{
					it2 = (*svc_it)->list_fd_write.erase(it2);
					fd_it = it2;
				}
				else
					it2 = (*svc_it)->list_fd_write.erase(it2);
			}
			else
				it2++;
		}
	}
	//at the very end: clear both list_fd_done_reading/writing
	(*svc_it)->list_fd_done_reading.clear();
	(*svc_it)->list_fd_done_writing.clear();
}

/*
** This function will loop over the linked list "list_services" and try to
** resume the I/O operations for each object if possible. Each service contains
** a reference on the Request_reader that spawned it. If the service realises
** it should be the last one (too many requests on same socket, fatal error...)
** it will close the fd_client, and activate a boolean that lets know the
** request_reader it is done and should be deleted.
**
** PARAMETERS:
** - res_select: the reference on the total number of fd to take care of on
** this loop with select. It will be decremented each time we match a fd in the
** set of fd we monitor. if it reaches zero we know we can shortcut and go back
** to select.
**
** Note: Each object derivated from the abstract class Service contains two
** linked lists: list_fd_read, and list_fd_write. This is done just in case we
** need to operate on more than one fd of the same kind ("write" or "read") at
** the same time. Those two lists are handled in two separate subfunctions.
**
** Note: The filedescriptors are already opened for us when entering this
** function. We will have to close them and remove them from our "database"
** fd_set_real when we dont need them anymore.
**
** Note: If some bad_alloc exceptions happened at some stage, we turn, the 
** request_reader into a no_more_alloc_handler, and we deletd the service
** (closing any opened fd etc...).
**
** NOTE: at the end, it the service did not detect it should be last answer to
** the client on this connexion socket, and if the reader is fully done reading
** , we flush the reader. If the reader is not fully done reading, we can just
** delete the service and the reader will "reset/flush" itself when it has
** fully read the previous request.
*/

void Server::resume_processed_services_to_clients(int &res_select)
{
	for (std::list<Service*>::iterator svc_it = list_services.begin(); \
			res_select > 0 && svc_it != list_services.end();)
	{
		try
		{
			//trying to see if the timeout reference in the reader is too old.
			if ((*svc_it)->reader->does_request_timed_out())
			{
				(*svc_it)->set_status_code((*svc_it)->reader->get_status());
				//DEBUG debug_timeout_occured((*svc_it)->get_fd_client());
				//yes means the service got deleted, we continue in loop.
				if (resume_processed_services_to_clients_timeout_handling(\
							svc_it))
					continue;
			}
			resume_processed_services_to_clients_READS_ONLY(svc_it, res_select);
			resume_processed_services_to_clients_WRITES_ONLY(svc_it, res_select);
			if ((*svc_it)->is_done_reading_and_writing())
			{
				if ((*svc_it)->service_thinks_reader_can_be_deleted)
				{
					(*svc_it)->reader->i_can_be_deleted = true;	
					//DEBUG debug_deletion("fd_real_set.read (timeout)", \
					//	__FILE__, __LINE__, (*svc_it)->get_fd_client());
					remove_fd_from_real_set_read((*svc_it)->get_fd_client());
					//DEBUG debug_deletion("fd_client closed (done r & w)", \
					//	__FILE__, __LINE__, (*svc_it)->get_fd_client());
					close((*svc_it)->get_fd_client());
				}
				//only flush the reader if it is fully done reading
				else if ((*svc_it)->reader->get_request_state() == Request_reader::DONE)
					(*svc_it)->reader->flush_reader_for_keep_alive();
				//set the reader's ptr to NULL
				if ((*svc_it)->reader->ptr_service_generated)
					(*svc_it)->reader->ptr_service_generated = NULL;
				//DEBUG debug_deletion("Service* (done r & w)",__FILE__, \
				//	__LINE__, (*svc_it)->get_fd_client());

				delete(*svc_it);
				svc_it = list_services.erase(svc_it);
			}
			else
				svc_it++;
		}
		catch (std::exception &e)
		{
			no_more_alloc_occured_in_server = 1;
			//tell the reader it is now a no_more_allow_handler
			(*svc_it)->reader->set_object_is_now_no_more_alloc_handler();
			//remove all fd from the real set
			from_service_remove_fd_from_real_set(*svc_it);
			(*svc_it)->close_all_fds_except_for_fd_client();
			//set the reader's ptr to NULL
			if ((*svc_it)->reader->ptr_service_generated)
				(*svc_it)->reader->ptr_service_generated = NULL;
			//delete the service, remove it from the list, and update svc_it
			//DEBUG debug_deletion("Service* (done r & w)",__FILE__, \
			//	__LINE__, (*svc_it)->get_fd_client());
			delete(*svc_it);
			svc_it = list_services.erase(svc_it);
		}
	}
}

/*
** This function is called in func resume_processed_services_to_clients(),
** when the timeout happened for a request. Two scenarios:
** - It is the first timeout occurence for this service, it will take care of
** turning the service into an error responder.
** - Even when trying to answer with error message, the timeout reoccured,
** then the service is just deleted from the service_list.
**
** Note: when calling the function error_occured_set_up_everything() the
**	timeout reference is reset to zero, and also the reader pointer in the
**	service object - if existing - has its state set to DONE, and its value set
**	to NULL.
**
** PARAMETER:
** - svc_it, an iterator onto the list of Services pointers.
**
** RETURN:
** 		1: means the service got deleted, we erase the service from the list,
** 			and we incremente the iterator. (the main loop in the calling
** 			function will go to the next iteration).
** 		0: means the service just turned into an error responder. keep using
** 			this service in the calling function.
*/

int Server::resume_processed_services_to_clients_timeout_handling(std::list<Service*>::iterator  &svc_it)
{
	//if yes, it means we cant even write an error anymore,(pipe probably full,
	//or the connexion has been reset by client). It also means we already had
	//only one fd left in use: fd_client.
	if ((*svc_it)->get_error_occured() == 1)
	{
		//DEBUG debug_deletion("fd_real_set.write (timeout)", __FILE__, __LINE__, \
		//		(*svc_it)->get_fd_client());
		remove_fd_from_real_set_write((*svc_it)->get_fd_client());

		//make sure reader will be deleted and close connexion if we cannot
		//answer error msg on the connexion socket.
		(*svc_it)->reader->i_can_be_deleted = true;	

		//DEBUG debug_deletion("fd_real_set.read (timeout)", __FILE__, __LINE__, \
		//		(*svc_it)->get_fd_client());
		remove_fd_from_real_set_read((*svc_it)->get_fd_client());

		//DEBUG debug_deletion("fd_client closed (timeout)", \
		//		__FILE__, __LINE__, (*svc_it)->get_fd_client());
		close((*svc_it)->get_fd_client());

		//set the reader's ptr to NULL
		if ((*svc_it)->reader->ptr_service_generated)
			(*svc_it)->reader->ptr_service_generated = NULL;
		//DEBUG debug_deletion("Service* (timeout)",__FILE__, \
		//		__LINE__, (*svc_it)->get_fd_client());
		delete(*svc_it);
		svc_it = list_services.erase(svc_it);
		return (1);
	}
	//or just start handling an error.
 	//NOTE: false => filling lists fd_done_writing and list_fd_done_reading.
	(*svc_it)->error_occured_set_up_everything(\
			(*svc_it)->get_status_code(), false);

	//sinces we closed some fd (potentially), make sure they are removed from
	//fd_set_real.fd_set_write/read.
	std::list<int>::iterator dummy = (*svc_it)->list_fd_write.end();
	clean_traces_of_fd_done(svc_it, dummy);
	return (0);
}

/*
** Called from resume_processed_requests_from_clients().
** This function will be called when an object of type Request_reader got
** turned into a no_more_alloc_handler. It will print the 500 error message on its
** client's socket, then close the socket, and remove the iterator from the
** list on which we were looping when done.
**
** INPUT:
** 	- iterator from list of Request_reader objects.
**
** note: If the message could not be written fully in one I/O operation, then
** we keep the boolean no_more_alloc_occured_in_server set to true, and we just
** increment the iterator.
*/

void Server::no_more_alloc_handler_for_Request_reader( \
		std::list<Request_reader>::iterator &it)
{
	if (it->write_no_more_alloc_answer(no_more_alloc_answer, \
				size_no_more_alloc_answer))
	{
		//DEBUG debug_deletion("fd_real_set.write (timeout)", \
		//	__FILE__, __LINE__, (*svc_it)->get_fd_client());
		remove_fd_from_real_set_write(it->get_client_fd());

		//DEBUG debug_deletion("fd_client closed" \
		//		,__FILE__, __LINE__, it->get_client_fd());
		close(it->get_client_fd());

		//DEBUG debug_deletion("Request_reader" ,__FILE__, \
		//		__LINE__, it->get_client_fd());
		it = list_request_readers.erase(it); //remove from the list.
	}
	else //still need to handle a bad_alloc so keep the bool to true
	{
		no_more_alloc_occured_in_server = true;
		it++;
	}
}

#ifndef SERVER_HPP
# define SERVER_HPP

# include "../includes/webserver.hpp"

//forward declare
class Request_reader;
class Service;
class Service_generator;

class Server
{
	friend class Service_generator;
	public:
		Server(void);

virtual	~Server(void);

		//should throw an exception if failure occurs.
		//initialise everything before kicking off the start_work() method.
		void  initialise(std::vector<t_srv> &servers);

		//main function of our server, infinite loop in which we use select.
		int start_work();

	private:
		/* member functions*/

		//call select after setting up the fd_sets
		int call_select();

		//could be non method functino.
		int build_listening_socket(t_srv &server);

		//function that will try to see if new conexions are pending and then
		//call accept, and add a request object into the list of services.
		void check_incoming_connexions(int &res_select);

		//function that loops over "list_requests" and tries to keep yielding
		//initial requests. then parses it, and create response objects
		void resume_processed_requests_from_clients(int &res_select);
		//assist above function
		void resume_processed_requests_from_clients_generate_svc( \
			std::list<Request_reader>::iterator it);
		//erases a request_reader from the list, and updates the iterator.
		void erase_processed_request_from_clients(\
			std::list<Request_reader>::iterator &it);

		//function that loops over "list_responses" and tries to keep
		//processing the I/O operations.
		void resume_processed_services_to_clients(int &res_select);
		//assists the above function
		void resume_processed_services_to_clients_WRITES_ONLY( \
				std::list<Service*>::iterator &svc_it, int &res_select);
		//assists the above function
		void resume_processed_services_to_clients_READS_ONLY(\
			std::list<Service*>::iterator &svc_it, int &res_select);
		//called if a service timeout.
		int resume_processed_services_to_clients_timeout_handling(\
			std::list<Service*>::iterator &svc_it);

		//adds a fd to fd_set_real.fd_set_read and .fd_mixed_list_read
		void		add_fd_to_real_set_read(int fd);

		//adds a fd to fd_set_real.fd_set_write and .fd_mixed_list_write
		void		add_fd_to_real_set_write(int fd);

		//removes a file descriptor from a fd_set_real.fd_set_write and from fd_mixed_list_write.
		void		remove_fd_from_real_set_write(int fd);

		//removes a file descriptor from a fd_set_real.fd_set_read and from fd_mixed_list_read.
		void		remove_fd_from_real_set_read(int fd);

		void clean_traces_of_fd_done(std::list<Service*>::iterator &svc_it, \
				std::list<int>::iterator &fd_it);

		//add into the fd_set_real the lists of fd found in the Service pointer
		void from_service_add_fd_to_real_set(const Service *svc);
		//remove the fd found in the Service pointer, from the fd_set_real.
		void from_service_remove_fd_from_real_set(const Service *svc);

		//called if no_more_alloc_occured_in_server is true. it loops over the
		//list_services and list_request_readers and if the objects have been
		//turned into no_more_alloc_handlers, their fd_client is added
		//to_fd_set_copy
		int add_fd_from_no_more_alloc_handlers_to_fd_set_copy(int highest_fd);

		//responds with the 500 error message if no_more_alloc_occured.
		void no_more_alloc_handler_for_Request_reader( \
			std::list<Request_reader>::iterator &it);

		/* member variables*/

		t_fd_set_real						fd_set_real;
		t_fd_set_copy						fd_set_copy;	
		std::vector<t_pair_fd_listen_t_srv>	vector_listen;
		std::list<Request_reader>			list_request_readers;
		std::list<Service *>				list_services;

		bool							no_more_alloc_occured_in_server;
		static const char *				no_more_alloc_answer;
		static const size_t				size_no_more_alloc_answer;

//	public:
//	class	WriteToCgiPipeErrorException:: public 
//		(Cgi_service & svc, int pipe_fd)
};

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 02:17:15 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/15 13:30:30 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

//#include <iostream>
# include "../includes/webserver.hpp"

class Server
{
	public:
		//should throw an exception if failure occurs.
		Server(const std::vector<t_srv> &servers);

virtual	~Server(void);

		//set up the listening sockets 
		int set_up_pair_fd_listen_t_srv(std::vector<t_srv> &servers);

		//adds a file desctiptor a fd_set and into the list_all_fd.
		void add_fd_to_real_set(int fd, fd_set& set);
		//removes a file descriptor from a fd_set and from the list_all_fd.
		void remove_fd_from_real_set(int fd, fd_set& set);
		

		//main function of our server, infinite loop in which we use select.
		int start_work();

		//function that will try to see if new conexions are pending and then
		//call accept, and add a request object into the list of services.
		int check_incoming_connexions();

		/* function that will try to keep on processing any service object that
		** is part of the list of services currently being processed. */
		int resume_processed_services();

		//should clean the memory,close the remaining opened sockets and files.
		int shutdown();


		//DEBUG FUNCTIONS
		void	debug_vector_listen(void);

	protected:

	private:
		/* member functions*/
		int call_select();

		/* member variables*/
		t_fd_set_real			fd_set_real;
		t_fd_set_copy			fd_set_copy;	
		std::vector<t_pair_fd_listen_t_srv>	vector_listen;
		std::list<t_pair_fd_service>	list_services;
};

#endif

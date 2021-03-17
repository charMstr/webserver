/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug_functions.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/17 06:01:55 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 06:02:21 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"

/*
** DEBUG FUNCTION: displays each member of the vector pairs_fd_listen_t_srv
*/

void 	Server::debug_vector_listen(void) //OK
{
	std::cout << std::endl << "DEBUG FUNCTION: debug_vector_listen()" << std::endl;
	std::cout << "the number of virtual servers is: " <<  vector_listen.size() << std::endl;
	std::cout << "the number of recorded fds is: " <<  fd_set_real.fd_mixed_list.size() << std::endl;
	for (std::vector<t_pair_fd_listen_t_srv>::iterator it = vector_listen.begin(); \
			it != vector_listen.end(); it++)
	{
		std::cout << "fd_listen = "<< it->fd_listen;
		std::cout << " and server_name is: " << it->v_server.server_name \
			<< std::endl;
	}
	std::cout << "first fd of the mixed list + 1 = " << fd_set_real.fd_mixed_list.front() + 1 << std::endl;
	std::cout << std::endl;
}

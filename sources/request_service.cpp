/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_service.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 12:45:41 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 14:14:41 by lspiess          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../includes/request_service.hpp"

Request_service::Request_service(const sockaddr_storage client_addr, \
				const socklen_t addr_size) :
	addr(client_addr),
	addr_size(addr_size),
	buffer_size(1024)
{
	//std::cout << "\033[33mconstructor called for request_service\033[0m" << std::endl;
	return ;
}

Request_service::~Request_service(void)
{
	//std::cout << "\033[31m[~]\033[33mdestructor called for request_service\033[0m" << std::endl;
	return ;
}

int 
Request_service::read(int fd)
{
	memset(buffer, 0, buffer_size);

	// For now read byte by byte to check that select() behaves properly
	//nb_char_read = recv(fd, buffer, buffer_size, 0);
	nb_char_read = recv(fd, buffer, 1, 0);
	printf("Service recv() %d chars : |%s|\n", nb_char_read, buffer);
	request.append(buffer);
	std::cout << "request so far : " << request << "\n";

	//etc... check the result...
	return (0);
}

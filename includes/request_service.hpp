/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_service.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 12:45:41 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 07:34:05 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_SERVICE_HPP
# define REQUEST_SERVICE_HPP

//#include <iostream>
# include "../includes/Service.hpp"

/*
** Request_service is a concrete class, derived from the abstract class
** Service, and it is created when a new connexion is being started, and we
** need to process the header +- body of the client's request.
**
** Parameters:
** 	- client_addr: the client'address is of type sockaddr_storage so it is
** 	family agnostic. It can hold either sockaddr_in (ipv4) or sockaddr_in6
** 	- size of the client_addr.
**
** It reimplements the read function mainly.
** It posses a buffer in which we are reading the client's request.
*/

class Request_service : public Service
{
	public:
		Request_service(const sockaddr_storage client_addr, \
				const socklen_t addr_size);
virtual	~Request_service(void);

		int read(int fd);

		//write is useless in this derivated class
		int write(int fd) {(void)fd; return (0);}

	protected:		
		const struct sockaddr_storage	addr;
		const socklen_t					addr_size;
		char							buffer[1024];
		int								buffer_size;
		int 							nb_char_read;
};

#endif

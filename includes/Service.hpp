/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Service.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 10:20:55 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 07:06:18 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVICE_HPP
# define SERVICE_HPP

#include <cstddef>
#include <sys/socket.h>

/*
** This is an abstract class that will allow us to do some polymorphism. Each
** time one of our virtual servers need to provide a service, as listening to
** the initial client's request, or responding to the get
** method, an object will be created (a derived class).
** All derived class can be pointed to by the same pointer, therefore a linked 
** list of these pointers will allow us to keep track of the
** ongoing services taken care of.
**
** Note: listening sockets are permanently present, so they are not considered
** services. All other objects derived from this abstract base class will have
** a short lifespan and be deleted ASAP.
*/

class Service
{
	public:
		virtual ~Service() {}
		virtual int read(int fd) = 0;
		virtual int write(int fd) = 0;
};

#endif

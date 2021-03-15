/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_service.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 12:45:41 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/15 13:12:24 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_SERVICE_HPP
# define REQUEST_SERVICE_HPP

//#include <iostream>
# include "../includes/Service.hpp"

/*
** Request_service is a concrete class, detived from the abstract class
** Service, and is created when a new connexionis being started, and we need to
** process the header +- body of the client's request.
**
** It reimplements the read function mainly.
** It posses a buffer in which we are reading the client's request.
*/

class Request_service : public Service
{
	public:
		Request_service(void);
virtual	~Request_service(void);

		int read(int fd);

		//write is useless in this derivated class
		int write(int fd) {(void)fd; return (0);}

	protected:
		char	buffer[1024];
		int		buffer_size;
		int 	nb_char_read;

	private:		
};

#endif

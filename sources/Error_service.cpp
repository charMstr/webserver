/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error_service.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/23 14:44:16 by charmstr          #+#    #+#             */
/*   Updated: 2021/06/07 05:36:38 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Error_service.hpp"
#include "../includes/debug_functions.hpp"

/*
** This class inherits from the abstract base class Service. It will be
** instanciated by service_generator when we know there was an error straight
** away in the request(400 bad request). Its role is to answer to the client
** with the correct error message. Copying nginx behavior, when a bad request
** is detected, we will act as if any error occured, we look for an error_page
** directive in the chosen location_block, if not found, we look in the server_block 
** then we either respond with that page (open it...) or we dicide that we will respond
** from a hardcoded string.
**
** Note: This class is the most basic implementation of Abstract base class
** Service. Indeed its virtual methods read_svc() and write_svc() are
** implemented as empty body functions. It will only use the methods required
** to answer to client when an error occured.
*/

Error_service::Error_service(const t_client_info client_info, \
	t_error_page error_page, std::string basename, \
	Request_reader *reader, int error_status) :
	//abstract base class constructor
	Service(client_info, error_page, basename, reader)
{
	set_status_code(error_status);
}

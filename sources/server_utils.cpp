/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/17 06:05:00 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 07:59:16 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exceptions.hpp"
#include "../includes/server_utils.hpp"

/*
** delete this func when loic is done.
*/
void dummy_server_init(t_srv &server, const char *str, int port)
{
	server.root = "./";
	server.server_name = str;
	server.index = "index.html";
	server.error = "error.html";
	server.cli_max_size = 4096;
	server.methods.bf = GET;
	server.port = htons(port);
}

/*
** delete this function when loic is done.
*/
void 	dummy_servers_creation(std::vector<t_srv> *servers)
{
	t_srv virtual_server1;
	t_srv virtual_server2;
	t_srv virtual_server3;
	dummy_server_init(virtual_server1, "server1", 8080);
	dummy_server_init(virtual_server2, "server2", 8081);
	dummy_server_init(virtual_server3, "server3", 8082);
	virtual_server1.host.sin_addr.s_addr = htonl(INADDR_ANY);
	virtual_server2.host.sin_addr.s_addr = htonl(INADDR_ANY);
	virtual_server3.host.sin_addr.s_addr = htonl(INADDR_ANY);
	servers->push_back(virtual_server1);
	servers->push_back(virtual_server2);
	servers->push_back(virtual_server3);
}

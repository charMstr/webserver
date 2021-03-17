/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 02:26:17 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 07:58:57 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"
#include <exception>
#include "../includes/exceptions.hpp"

int	main(int argc __attribute__((unused)), char **argv __attribute__((unused)))	
{
	//const char			*path = argc > 1 ? argv[1] : DFL_CNF_PATH;
	std::vector<t_srv>	virtual_servers;

	/*
	if (parse_config(path, &servers) < 0)
	{
		std::cout << "PARSING ERROR" << std::endl;
		return (1);
	}
	*/

	Server server;
	try 
	{
		server.initialise(virtual_servers);
		server.start_work();
	}
	catch (exception_webserver &e)
	{
		std::cout << e.context() << e.what() << e.which() << e.when() << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

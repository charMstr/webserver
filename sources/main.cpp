/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 02:26:17 by charmstr          #+#    #+#             */
/*   Updated: 2021/06/07 19:55:37 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exceptions.hpp"
#include "../includes/webserver.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/server.hpp"

std::map<std::string, std::string> mime_types;

void initialize_mime_types(std::map<std::string, std::string> & mime_types);

int	main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	const char			*path = argc > 1 ? argv[1] : DFL_CNF_PATH;
	std::vector<t_srv>	virtual_servers;

	try
	{	
		//if a bad_alloc exception happened using C++ language...
		if (parse_config(path, &virtual_servers) < 0)
		{	//if a malloc error occured using raw C language...
			std::cout << "PARSING ERROR" << std::endl;
			return (EXIT_FAILURE);
		}
	}
	catch (const exception_webserver &e)
	{
		e.print_serialised();
		return (EXIT_FAILURE);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	DEBUG debug_servers_print(&virtual_servers);

	Server server;
	try
	{
		initialize_mime_types(mime_types);
		if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) //ignoring the broken pipe SIGNAL.
			throw exception_webserver(strerror(errno),"main()",__FILE__,__LINE__, "signal()", INITIALISING);
		server.initialise(virtual_servers);
		debug_start_server();
		server.start_work();
	}
	catch (exception_webserver &e)
	{
		e.print_serialised();
		return (EXIT_FAILURE);
	}
	catch (std::exception &e) //safety net.
	{
		std::cout << "(file:" << __FILE__ <<"; line = "<< __LINE__ << ") "<< e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	virtual_servers.clear();
	return (EXIT_SUCCESS);
}

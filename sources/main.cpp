/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/15 02:26:17 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/15 06:18:39 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"

int	main(int argc __attribute__((unused)), char **argv __attribute__((unused)))	
{
	//const char			*path = argc > 1 ? argv[1] : DFL_CNF_PATH;
	std::vector<t_srv>	servers;

	/*
	if (parse_config(path, &servers) < 0)
	{
		std::cout << "PARSING ERROR" << std::endl;
		return (1);
	}
	*/

	// should throw if an error occured while setting up the listening sockets.
	Server server(servers);

	//server.start_work();

	server.shutdown();
	return (0);
}

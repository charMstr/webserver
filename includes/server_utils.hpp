/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_utils.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/17 06:07:08 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 07:58:31 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_UTILS_HPP
# define SERVER_UTILS_HPP

# include "../includes/webserver.hpp"

void	dummy_server_init(t_srv &server, const char *str, int port);
void 	dummy_servers_creation(std::vector<t_srv> *servers);

#endif

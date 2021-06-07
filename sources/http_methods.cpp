/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_methods.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/30 14:27:11 by bccyv             #+#    #+#             */
/*   Updated: 2021/05/07 17:29:30 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/http_methods.hpp"

int str_to_method(const std::string &s)
{
	if (s == "OPTIONS") return (0b00000001);
	if (s == "GET") return (0b00000010);
	if (s == "HEAD") return (0b00000100);
	if (s == "POST") return (0b00001000);
	if (s == "PUT") return (0b00010000);
	if (s == "DELETE") return (0b00100000);
	if (s == "TRACE") return (0b01000000);
	if (s == "CONNECT") return (0b10000000);
	return (-1);
}

std::string method_to_str(int method)
{
	if (method == 0b00000001) return ("OPTIONS");
	if (method == 0b00000010) return ("GET");
	if (method == 0b00000100) return ("HEAD");
	if (method == 0b00001000) return ("POST");
	if (method == 0b00010000) return ("PUT");
	if (method == 0b00100000) return ("DELETE");
	if (method == 0b01000000) return ("TRACE");
	if (method == 0b10000000) return ("CONNECT");
	return ("");
}

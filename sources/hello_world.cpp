/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hello_world.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Remercille <Remercille@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/03 16:10:49 by Remercill         #+#    #+#             */
/*   Updated: 2021/06/03 18:16:01 by Remercill        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define BUFFER_SIZE 65535

#include <unistd.h>
int	main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	write(1, "HTTP/1.1 200 OK\r\n\r\nhello_world!", sizeof("HTTP/1.1 200 OK\r\n\r\nhello_world!") -1);
	write(2, "\033[31mHTTP/1.1 200 OK\r\n\r\nhello_world!\n\033[0m", sizeof("\033[31mHTTP/1.1 200 OK\r\n\r\nhello_world!\n\033[0m") -1);
	int res;
	char buffer[BUFFER_SIZE];
	while (1)
	{
		res = read(0, buffer, BUFFER_SIZE);
		if (res == 0)
			break;
			//exit(0); //done reading
		if (res == -1)
			continue; //skip, thanks to non_block flags
	}
	write(2, "\033[31mgoodbye from hello_world\n\033[0m", sizeof("\033[31mgoodbye from hello_world\n\033[0m") -1);
	return (0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/01 19:59:56 by bccyv             #+#    #+#             */
/*   Updated: 2021/03/03 11:00:08 by glafond-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <libregex.h>
#include <stdlib.h>
#include <regex.fa.h>

char	*readfile(const char *filename)
{
	FILE	*f;
	long	fsize;
	char	*string;

	f = fopen(filename, "rb");
	if (!f)
		return (NULL);
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	string = malloc(fsize + 1);
	string[fsize] = 0;
	if (!string)
	{
		fclose(f);
		return (NULL);
	}
	fread(string, 1, fsize, f);
	fclose(f);
	return (string);
}

int	main(int ac, char **av)
{
	t_regex		regex;
	int			ret;
	char		*string;
	char		*save;
	t_rematch	match;
	int			n;

	if (ac != 3)
	{
		printf("Missing arguments\n");
		printf("usage: %s regex file\n", av[0]);
		return (-1);
	}
	ret = re_compile(&regex, av[1]);
	if (ret < 0)
	{
		printf("Regex compilation error.\n");
		return (-1);
	}
	string = readfile(av[2]);
	if (!string)
	{
		re_free(&regex);
		return (-1);
	}
	n = 0;
	ret = re_nextmatch(&regex, string, &save, &match);
	while (!ret)
	{
		n++;
		write(1, "-> '", 4);
		write(1, match.start, (match.end + 2) - match.start);
		write(1, "'\n", 2);
		ret = re_nextmatch(&regex, NULL, &save, &match);
	}
	free(string);
	re_free(&regex);
	return (0);
}

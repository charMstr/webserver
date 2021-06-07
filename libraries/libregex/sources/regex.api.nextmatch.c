/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.api.nextmatch.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glafond- <glafond-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/02 22:09:37 by glafond-          #+#    #+#             */
/*   Updated: 2021/03/03 10:58:25 by glafond-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <libregex.h>
#include <regex.fa.h>

int	re_nextmatch(t_regex *regex, const char *str, char **saveptr, t_rematch *m)
{
	if (!str)
		str = *saveptr;
	while (*str)
	{
		*saveptr = re_match(regex->entrypoint, str);
		if (*saveptr && *saveptr != str)
		{
			m->start = (char *)str;
			m->end = *saveptr - 1;
			return (0);
		}
		str++;
	}
	return (-1);
}

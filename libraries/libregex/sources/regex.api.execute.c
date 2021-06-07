/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.api.execute.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glafond- <glafond-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/14 03:34:58 by glafond-          #+#    #+#             */
/*   Updated: 2021/03/03 10:57:35 by glafond-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <libregex.h>
#include <regex.fa.h>

int	re_count_matches(t_regex *regex, const char *str)
{
	char		*ret;
	int			nb_match;

	nb_match = 0;
	while (*str)
	{
		ret = re_match(regex->entrypoint, str);
		if (!ret || ret == str)
		{
			str++;
			continue ;
		}
		nb_match++;
		str = (const char *)ret;
	}
	return (nb_match);
}

int	re_full_match(t_regex *regex, const char *str)
{
	char		*ret;

	ret = re_match(regex->entrypoint, str);
	if (!ret || *ret)
		return (0);
	return (1);
}

char	*re_bmatch(t_regex *regex, const char *str)
{
	return (re_match(regex->entrypoint, str));
}

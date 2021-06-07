/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.api.compile.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glafond- <glafond-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/13 09:11:02 by glafond-          #+#    #+#             */
/*   Updated: 2021/02/22 18:47:52 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <libregex.h>
#include <regex.fa.h>
#include <stdlib.h>
#include <libft.h>

static char	*re_set_flags(t_regex *re)
{
	char	*tmp;
	size_t	i;

	re->flags = 0x0;
	tmp = re->re_string;
	if (*tmp == '^')
	{
		re->flags |= 0x1;
		tmp++;
	}
	i = 0;
	while (tmp[i] && tmp[i + 1])
		i++;
	if (tmp[i] == '$')
	{
		re->flags |= 0x2;
		tmp[i] = '\0';
	}
	return (tmp);
}

int	re_compile(t_regex *regex, const char *str)
{
	char	*tmp;

	regex->re_string = ft_strdup(str);
	if (!regex->re_string)
		return (-1);
	tmp = re_set_flags(regex);
	regex->entrypoint = dfa_generate(tmp);
	if (!regex->entrypoint)
	{
		free(regex->re_string);
		return (-1);
	}
	if (regex->flags & 0x2)
	{
		while (*tmp)
			tmp++;
		*tmp = '$';
	}
	return (0);
}

void	re_free(t_regex *re)
{
	free(re->re_string);
	dfa_free(re->entrypoint);
}

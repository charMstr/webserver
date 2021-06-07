/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.api.array.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/04 13:19:36 by lfalkau           #+#    #+#             */
/*   Updated: 2021/03/04 13:25:57 by lfalkau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <libregex.h>
#include <stdlib.h>

void	re_arr_free(t_regex *re, size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
		re_free(&re[i++]);
	free(re);
}

t_regex	*re_arr_create(const char *rexp[], size_t count)
{
	t_regex	*re;
	size_t	i;

	re = malloc(sizeof(t_regex) * count);
	if (!re)
		return (NULL);
	i = 0;
	while (i < count)
	{
		if (re_compile(&re[i], rexp[i]))
		{
			re_arr_free(re, i);
			return (NULL);
		}
		i++;
	}
	return (re);
}

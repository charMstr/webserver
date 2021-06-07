/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.pattern.add.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glafond- <glafond-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/17 08:58:37 by glafond-          #+#    #+#             */
/*   Updated: 2021/02/17 11:33:44 by lfalkau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <regex.fa.h>

int	pattern_add_range(t_pattern *pattern, int s, int e)
{
	if (s > e)
		return (-1);
	if (s / 8 > PATTERN_BYTES_LENGTH || e / 8 > PATTERN_BYTES_LENGTH)
		return (-1);
	while (s <= e)
	{
		(*pattern)[s / 8] |= (1 << (s % 8));
		s++;
	}
	return (0);
}

int	pattern_add_char(t_pattern *pattern, int c)
{
	int			div;

	div = c / 8;
	if (div > PATTERN_BYTES_LENGTH)
		return (-1);
	(*pattern)[div] |= (1 << (c % 8));
	return (0);
}

int	pattern_add_pattern(t_pattern *dest, char *src)
{
	int			index;

	index = 0;
	while (index < PATTERN_BYTES_LENGTH)
	{
		(*dest)[index] |= src[index];
		index++;
	}
	return (0);
}

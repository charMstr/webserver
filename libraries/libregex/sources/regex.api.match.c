/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.api.match.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glafond- <glafond-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/02 21:56:17 by glafond-          #+#    #+#             */
/*   Updated: 2021/03/03 10:57:11 by glafond-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <libregex.h>
#include <regex.fa.h>

char	*re_match(t_ds *state, const char *str)
{
	const char	*ptr;
	t_link_lst	*links;

	ptr = str;
	while (*ptr)
	{
		links = state->links;
		if (!links)
			break ;
		while (!pattern_match(links->link.pattern, *ptr))
		{
			links = links->next;
			if (!links)
			{
				if (state->is_final)
					return ((char *)ptr);
				return (NULL);
			}
		}
		state = (t_ds *)links->link.next;
		ptr++;
	}
	if (state->is_final)
		return ((char *)ptr);
	return (NULL);
}

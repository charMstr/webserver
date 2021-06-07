/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.dfa.utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 12:51:56 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/20 01:04:58 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <regex.fa.h>
#include <stdlib.h>

size_t	dfa_get_size(t_ds *st)
{
	t_link_lst	*links;
	size_t		size;

	if (!st || st->flag == 1)
		return (0);
	st->flag = 1;
	size = 1;
	links = st->links;
	while (links)
	{
		size += dfa_get_size(links->link.next);
		links = links->next;
	}
	return (size);
}

void	dfa_get_addresses(t_ds *st, t_vec *vec)
{
	t_link_lst	*links;

	if (!st || st->flag == 0)
		return ;
	st->flag = 0;
	vec->addr[vec->size++] = st;
	links = st->links;
	while (links)
	{
		dfa_get_addresses(links->link.next, vec);
		links = links->next;
	}
}

int	dfa_link_add(t_ds *dst, t_ds *src, t_pattern *p)
{
	t_link_lst	*new;
	t_link_lst	*tmp;

	tmp = src->links;
	while (tmp)
	{
		if (!pattern_cmp(tmp->link.pattern, *p))
			return (0);
		tmp = tmp->next;
	}
	new = malloc(sizeof(t_link_lst));
	if (!new)
		return (-1);
	pattern_copy(new->link.pattern, *p);
	new->link.next = dst;
	new->next = src->links;
	src->links = new;
	return (0);
}

static void	dfa_links_free(t_link_lst *links)
{
	if (links)
	{
		dfa_links_free(links->next);
		free(links);
	}
}

void	dfa_free(t_ds *entrypoint)
{
	t_vec	vec;
	size_t	i;

	vec.size = 0;
	vec.addr = malloc(sizeof(void *) * dfa_get_size(entrypoint));
	dfa_get_addresses(entrypoint, &vec);
	i = 0;
	while (i < vec.size)
	{
		dfa_links_free(((t_ds *)vec.addr[i])->links);
		free(vec.addr[i]);
		i++;
	}
	free(vec.addr);
}

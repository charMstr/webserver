/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.nfa.utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/03 08:40:00 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/19 22:53:20 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <regex.fa.h>
#include <stdlib.h>

size_t	nfa_get_size(t_ns *st)
{
	if (!st || st->flag == 1)
		return (0);
	st->flag = 1;
	return (nfa_get_size(st->left.next) + nfa_get_size(st->right.next) + 1);
}

void	nfa_get_addresses(t_ns *st, t_vec *vec)
{
	if (!st || st->flag == 0)
		return ;
	st->flag = 0;
	vec->addr[vec->size++] = st;
	nfa_get_addresses(st->left.next, vec);
	nfa_get_addresses(st->right.next, vec);
}

void	nfa_free(t_ns *entrypoint)
{
	t_vec	vec;
	size_t	i;

	vec.size = 0;
	vec.addr = malloc(sizeof(void *) * nfa_get_size(entrypoint));
	nfa_get_addresses(entrypoint, &vec);
	i = 0;
	while (i < vec.size)
	{
		free(vec.addr[i]);
		i++;
	}
	free(vec.addr);
}

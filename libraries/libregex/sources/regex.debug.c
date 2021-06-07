/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.debug.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/19 14:23:03 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/22 20:17:42 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <regex.fa.h>

static int	fa(t_ds *st, t_vec *v, int flag)
{
	size_t	i;

	i = 0;
	while (i < v->size)
	{
		if (flag == 0 && v->addr[i] == (t_ds *)st)
			return (i + 1);
		if (flag == 1 && v->addr[i] == (t_ns *)st)
			return (i + 1);
		i++;
	}
	return (0);
}

void	pattern_print(t_pattern pattern)
{
	int		i;
	char	b;
	char	c;

	i = 0;
	while (i < PATTERN_BYTES_LENGTH)
	{
		b = 1;
		c = pattern[i++];
		while (b)
		{
			printf("%c", c & b ? '1' : '0');
			b <<= 1;
		}
	}
}

void	dfa_print(t_ds *entrypoint)
{
	t_vec		vec;
	size_t		i;
	t_link_lst	*links;

	vec.size = 0;
	vec.addr = malloc(sizeof(void *) * dfa_get_size(entrypoint));
	dfa_get_addresses(entrypoint, &vec);
	i = 0;
	while (i < vec.size)
	{
		printf("%zu", i + 1);
		if (((t_ds *)vec.addr[i])->is_final)
			printf("f");
		printf(":\n");
		links = ((t_ds *)vec.addr[i])->links;
		while (links)
		{
			printf("\t%d ", fa(links->link.next, &vec, 1));
			pattern_print(links->link.pattern);
			printf("\n");
			links = links->next;
		}
		i++;
	}
	free(vec.addr);
}

void	nfa_print(t_ns *nfa)
{
	t_vec	vec;
	size_t	i;
	t_ns	*node;

	vec.size = 0;
	vec.addr = malloc(sizeof(void *) * nfa_get_size(nfa));
	nfa_get_addresses(nfa, &vec);
	i = 0;
	while (i < vec.size)
	{
		node = (t_ns *)(vec.addr)[i];
		printf("%zu: left -> %d ", i + 1, fa(node->left.next, &vec, 0));
		printf(" right -> %d ", fa(node->right.next, &vec, 0));
		printf("\n");
		i++;
	}
	free(vec.addr);
}

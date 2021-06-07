/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.nfa.link.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 11:34:55 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/20 02:04:40 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <regex.fa.h>
#include <libft.h>

void	nfa_link_init(t_link *ln)
{
	ft_memset(ln, 0, sizeof(t_link));
	ln->match = &pattern_match;
}

/*
**	NFA state can only have two links, that's why we only check for an
**	existing left link.
*/

void	nfa_link_add(t_ns *dst, t_ns *src, t_pattern p)
{
	if (!src->left.next)
	{
		src->left.next = dst;
		ft_memcpy(src->left.pattern, p, sizeof(t_pattern));
	}
	else
	{
		src->right.next = dst;
		ft_memcpy(src->right.pattern, p, sizeof(t_pattern));
	}
}

void	nfa_links_destroy(t_ns *st)
{
	nfa_link_init(&st->left);
	nfa_link_init(&st->right);
}

void	nfa_links_cpy(t_ns *dst, t_ns *src)
{
	dst->left = src->left;
	dst->right = src->right;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   regex.nfa.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/03 08:37:02 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/22 20:19:14 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <regex.fa.h>
#include <stdlib.h>
#include <libft.h>

static t_ns	*nfa_create(t_ns *e, const char **p, int n, t_alphabet **a);

static t_ns	*nfa_state_new(void)
{
	t_ns	*st;

	st = malloc(sizeof(t_ns));
	if (!st)
		return (NULL);
	st->is_final = 0;
	nfa_link_init(&st->left);
	nfa_link_init(&st->right);
	st->flag = 0;
	return (st);
}

static int	nfa_surround(t_ns *beg, t_ns *end, t_ns **new_beg, t_ns **new_end)
{
	t_pattern	p;

	if (!end)
		return (-1);
	*new_beg = nfa_state_new();
	if (!*new_beg)
		return (-1);
	*new_end = nfa_state_new();
	if (!*new_end)
	{
		free(*new_beg);
		return (-1);
	}
	nfa_links_cpy(*new_beg, beg);
	nfa_links_destroy(beg);
	pattern_epsilon(&p);
	nfa_link_add(*new_beg, beg, p);
	nfa_link_add(*new_end, end, p);
	return (0);
}

static t_ns	*nfa_or(t_ns *beg, t_ns *end, const char **ptr, int nested, t_alphabet **a)
{
	t_ns		*new_beg;
	t_ns		*new_end;
	t_ns		*right_beg;
	t_ns		*right_end;
	t_pattern	p;

	if (nfa_surround(beg, end, &new_beg, &new_end) < 0)
		return (NULL);
	right_beg = nfa_state_new();
	if (!right_beg)
		return (NULL);
	pattern_epsilon(&p);
	nfa_link_add(right_beg, beg, p);
	(*ptr)++;
	right_end = nfa_create(right_beg, ptr, nested, a);
	if (!right_end)
		return (NULL);
	nfa_link_add(new_end, right_end, p);
	return (new_end);
}

static t_ns	*nfa_quantifier(t_ns *beg, t_ns *end, const char **ptr)
{
	t_ns		*new_beg;
	t_ns		*new_end;
	t_pattern	p;

	if (nfa_surround(beg, end, &new_beg, &new_end) < 0)
		return (NULL);
	pattern_epsilon(&p);
	if (**ptr != '+')
		nfa_link_add(new_end, beg, p);
	if (**ptr != '?')
		nfa_link_add(new_beg, end, p);
	(*ptr)++;
	return (new_end);
}

static t_ns	*nfa_pattern(t_ns *beg, const char **ptr, t_alphabet **a)
{
	t_ns		*new_end;
	t_pattern	pattern;

	if (!beg)
		return (NULL);
	ft_memset(pattern, 0, sizeof(pattern));
	if (**ptr == '[' || **ptr == '\\' || **ptr == '.')
	{
		if (pattern_parse(&pattern, ptr))
			return (NULL);
	}
	else if (pattern_add_char(&pattern, *(*ptr)++))
		return (NULL);
	if (alphabet_add_pattern(a, pattern))
		return (NULL);
	new_end = nfa_state_new();
	if (!new_end)
		return (NULL);
	nfa_link_add(new_end, beg, pattern);
	return (new_end);
}

/*
**	Converts a string into a NFA, following the Thompson's construction method.
**	See ttps://tajseer.files.wordpress.com/2014/06/re-nfa-dfa.pdf.
*/

static t_ns	*nfa_create(t_ns *beg, const char **ptr, int nested, t_alphabet **a)
{
	t_ns		*end;
	t_ns		*tmp;

	if (!beg)
		return (NULL);
	end = beg;
	while (nested ? **ptr && **ptr != ')' : **ptr)
	{
		tmp = end;
		if (**ptr == '(' && (*ptr)++)
		{
			end = nfa_create(end, ptr, 1, a);
			if (!end || *(*ptr)++ != ')')
				return (NULL);
		}
		else
			end = nfa_pattern(end, ptr, a);
		if (ft_isinset("*+?", **ptr))
			end = nfa_quantifier(tmp, end, ptr);
		if (**ptr == '|')
			end = nfa_or(beg, end, ptr, nested, a);
		if (!end)
			return (NULL);
	}
	return (end);
}

t_ns	*nfa_generate(const char *str, t_alphabet **a)
{
	t_ns	*entrypoint;
	t_ns	*final_state;

	if (*str == '\0')
		return (NULL);
	entrypoint = nfa_state_new();
	if (!entrypoint)
		return (NULL);
	final_state = nfa_create(entrypoint, &str, 0, a);
	if (!final_state)
	{
		nfa_free(entrypoint);
		return (NULL);
	}
	final_state->is_final = 1;
	return (entrypoint);
}

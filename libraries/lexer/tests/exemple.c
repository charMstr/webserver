/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/18 21:46:02 by bccyv             #+#    #+#             */
/*   Updated: 2021/02/18 21:46:09 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include <stdlib.h>
// #include <stdio.h>
// #include <lexer.h>

static int	lex_push(t_lex **h, int tk, const char *beg, const char *end)
{
	t_lex	*lx;

	lx = malloc(sizeof(t_lex));
	if (!lx)
		return (-1);
	lx->token = tk;
	lx->beg = beg;
	lx->end = end;
	lx->next = *h;
	*h = lx;
	return (0);
}

static char	*tokenize(t_lex **lx, t_regex *re_arr, size_t cnt, const char *ptr)
{
	size_t	token;
	char	*max_ptr;
	char	*end_ptr;
	size_t	i;

	max_ptr = NULL;
	i = 0;
	while (i < cnt)
	{
		end_ptr = re_bmatch(&re_arr[i], ptr);
		if (end_ptr > max_ptr)
		{
			max_ptr = end_ptr;
			token = i;
		}
		i++;
	}
	if (max_ptr == NULL)
	{
		printf("Unexpected token: %.10s(..)\n", ptr);
		return (NULL);
	}
	if (lex_push(lx, token, ptr, max_ptr) < 0)
		return (NULL);
	return (max_ptr);
}

void	lex_print(t_lex *lx)
{
	if (lx)
	{
		lex_print(lx->next);
		printf("%d\t|%.*s|\n", lx->token, (int)(lx->end - lx->beg), lx->beg);
	}
}

void	lex_free(t_lex *lx)
{
	if (lx)
	{
		lex_free(lx->next);
		free(lx);
	}
}

t_lex	*lex_exe(t_regex *re_arr, size_t count, const char *src)
{
	t_lex	*lx;

	lx = NULL;
	while (*src)
	{
		src = tokenize(&lx, re_arr, count, src);
		if (!src)
			return (NULL);
		while ((*src >= 9 && *src <= 13) || *src == 32)
			src++;
	}
	return (lx);
}

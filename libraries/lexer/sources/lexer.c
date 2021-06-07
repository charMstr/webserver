/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/17 10:23:24 by lfalkau           #+#    #+#             */
/*   Updated: 2021/03/10 11:09:13 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <lexer.h>

/*
**	lex_push - Creates a new t_lex node and pushes it in front of the previous
**	one, modifying the head of the linked list.
**	@h: The t_lex linked list head.
**	@tk: The id of the token to be pushed
**	@beg: A pointer to the lexeme's begin.
**	@end: A pointer to the lexeme's end.
**	Return: 0 if successful, -1 on failure.
*/

static int	lex_push(t_lex **h, int tk, const char *beg, const char *end)
{
	t_lex	*lx;
	t_lex	*tmp;

	lx = malloc(sizeof(t_lex));
	if (!lx)
		return (-1);
	lx->token = tk;
	lx->beg = beg;
	lx->end = end;
	lx->next = NULL;
	if (!*h)
		*h = lx;
	else
	{
		tmp = *h;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = lx;
	}
	return (0);
}

/*
**	tokenize - Tries to get the longest match from ptr with all regular
**	expressions from re_arr, and create a t_lex node from it.
**	@lx: A pointer to a poiner to the head of the t_lex leading node.
**	@re_arr: The t_regex array.
**	@cnt: The length of the re_arr array.
**	@ptr: A pointer to the beginning of the expected next token.
**	@comment_tk: The index of the comment token, which willn't be pushed.
**	Return: A pointer past the recognized token,
**	NULL if no match is found.
*/

static char	*tokenize(t_lex **lx, t_regex *re_arr, size_t cnt, const char *ptr, int comment_tk)
{
	int		tk;
	char	*max_ptr;
	char	*end_ptr;
	size_t	i;

	max_ptr = NULL;
	i = 0;
	while (i < cnt)
	{
		end_ptr = re_bmatch(&re_arr[i], ptr);
		if (end_ptr && (end_ptr > max_ptr))
		{
			max_ptr = end_ptr;
			tk = i;
		}
		i++;
	}
	if (max_ptr == NULL || max_ptr == ptr)
		return (NULL);
	if (tk != comment_tk && lex_push(lx, tk, ptr, max_ptr) < 0)
		return (NULL);
	return (max_ptr);
}

/*
**	lex_print_error - Prints the line, the column of an unrecognized token, and
**	the token itself.
**	@beg: A pointer to the beginning of the source string.
**	@err: A pointer to the beginning of the unrecognized lexeme.
*/

static void	lex_print_error(const char *beg, const char *err)
{
	size_t	line;
	size_t	col;
	int		wlen;

	line = 1;
	col = 1;
	while (beg != err)
	{
		if (*beg == '\n' || *beg == '\r')
		{
			line++;
			col = 1;
		}
		col++;
		beg++;
	}
	printf("Token not recognized: line %zu, col %zu: ", line, col);
	wlen = 0;
	while (beg[wlen] > 32)
		wlen++;
	printf("\"%.*s\"\n", wlen, beg);
}

/* ************************************************************************** */
/*	API functions                                                             */
/* ************************************************************************** */

void	lex_print(t_lex *lx)
{
	while (lx)
	{
		printf("%d\t|%.*s|\n", lx->token, (int)(lx->end - lx->beg), lx->beg);
		lx = lx->next;
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

t_lex	*lex_exe(t_regex *re_arr, size_t count, int comment_tk, const char *src)
{
	t_lex		*lx;
	const char	*beg;
	const char	*ptr;

	lx = NULL;
	beg = src;
	while (*src)
	{
		ptr = src;
		src = tokenize(&lx, re_arr, count, src, comment_tk);
		if (!src)
		{
			lex_print_error(beg, ptr);
			return (NULL);
		}
		while ((*src >= 9 && *src <= 13) || *src == 32)
			src++;
	}
	return (lx);
}

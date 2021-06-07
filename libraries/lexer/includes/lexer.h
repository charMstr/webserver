/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/17 10:15:53 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/24 10:48:13 by lfalkau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

# include "libregex.h"

typedef struct s_lexer	t_lex;

/*
**	s_lexer structure is a linked list. Each node represents a lexeme.
**	Lexemes are stored backward (first node is the last token of the input).
**
**	token: Stores the token value, usually a define.
**	beg: A pointer to the first char of the lexeme.
**	end: A pointer past the last char of the lexeme.
**	next: A pointer to the next node.
*/
struct s_lexer
{
	unsigned int	token;
	const char		*beg;
	const char		*end;
	t_lex			*next;
};

/*
**	lex_exe try to lex a string.
**	On failure it prints an error calling lex_print_error.
**
**	re_arr: A t_regex structure array
**	count: The size of the regex array.
**	input: The string to lex.
**
**	return value: The linked list containing all lexemes and associated tokens.
*/
t_lex	*lex_exe(t_regex *re_arr, size_t count, int comment_tk, const char *src);

/*
**	lex_print prints a t_lex linked list.
**	Useful to debug.
**
**	lx: The linked list to be printed.
*/
void	lex_print(t_lex *lex);

/*
**	lex_free free a t_lex linked list.
**	It has to be called after each lex_exe call on its return value.
**
**	lx: The linked list to be freed.
*/
void	lex_free(t_lex *lex);

#endif

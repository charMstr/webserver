/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/17 14:03:48 by lfalkau           #+#    #+#             */
/*   Updated: 2021/03/04 13:27:59 by lfalkau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "inc/lexer.h"
#include <sys/stat.h>

#define LANGUAGE g_conf_lexemes //change if to the language you want to lex.
#define LANGUAGE_COMMENT 8

/* ************************************************************************** */
/*	Some languages definitions                                                */
/* ************************************************************************** */

const char	*g_c_lexemes[] = {
	[0] = "auto",												//auto
	[1] = "else",												//else
	[2] = "long",												//long
	[3] = "switch",												//switch
	[4] = "break",												//break
	[5] = "enum",												//enum
	[6] = "register",											//register
	[7] = "typedef",											//typedef
	[8] = "case",												//case
	[9] = "extern",												//extern
	[10] = "return",											//return
	[11] = "union",												//union
	[12] = "char",												//char
	[13] = "float",												//float
	[14] = "short",												//short
	[15] = "unsigned",											//unsigned
	[16] = "const",												//const
	[17] = "for",												//for
	[18] = "signed",											//signed
	[19] = "void",												//void
	[20] = "continue",											//continue
	[21] = "goto",												//goto
	[22] = "sizeof",											//sizeof
	[23] = "volatile",											//volatile
	[24] = "default",											//default
	[25] = "if",												//if
	[26] = "static",											//static
	[27] = "while",												//while
	[28] = "do",												//do
	[29] = "int",												//int
	[30] = "struct",											//struct
	[31] = "_Packed",											//_Packed
	[32] = "double",											//double
	[33] = "[\\w][\\w0-9]*",									//identifier
	[34] = "[\\(\\){};,]",										//separator
	[35] = "(&|\\||<<|>>|^|!|+|-|*|/|%|=|<|>)=?",				//operator
	[36] = "&&|\\|\\||?|:",										//logic operator
	[37] = "[0-9]+(\\.[0-9]+)?",								//number
	[38] = "\"([^\\\\\"]|\\\\[\\W\\w])*\"",						//string
	[39] = "\\[|\\]",											//array access
	[40] = "->|\\.",											//member access
	[41] = "(//[^\n]*)|(/\\*(*+[\\W\\w]|[\\W\\w]|/)*\\*+/)",	//comment
};

const char	*g_conf_lexemes[] = {
	[0] = "server",												//server
	[1] = "location",											//location
	[2] = "[{}]",												// block
	[3] = "[,;]",												// separator
	[4] = "[0-9]+",												//port
	[5] = "[a-z_]+",											//identifier
	[6] = "[A-Z]+",												//http method
	[7] = "(/|~)?(([a-z0-9]|\\.|\\.\\.)*/?)+",					//path
	[8] = "#[^\n]*",											//comment
};

const char	*g_math_lexemes[] = {
	[0] = "[\\w][\\w0-9]*",										//variable
	[1] = "[\\+-\\*%/]",										//operator
	[2] = "=",													//assignment
	[3] = "\\(",												//opening brace
	[4] = "\\)",												//closing brace
	[5] = "[0-9]+([.,][0-9]*)?",								//number
};

static void	*close_on_err(int fd)
{
	close(fd);
	return (NULL);
}

/*
**	Read a whole file and returns an allocated string which contains it.
**	return value: A string, or NULL on failure
*/

static char	*read_file(const char *path)
{
	int				fd;
	struct stat		st;
	char			*buff;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (NULL);
	if (fstat(fd, &st) < 0)
		close_on_err(fd);
	buff = malloc(sizeof(char) * (st.st_size + 1));
	buff[st.st_size] = 0;
	if (!buff)
		close_on_err(fd);
	buff[st.st_size] = 0;
	if (read(fd, buff, st.st_size) < 0)
		close_on_err(fd);
	close(fd);
	return (buff);
}

/*
**	Creates a t_lex linked list structure from a file and a given language
**	and prints it.
**
**	Usage: ./a.out <file_path>
*/

int	main(int ac, char **av)
{
	char		*buff;
	t_lex		*lex;
	t_regex		*re_arr;

	if (ac < 2)
		return (1);
	buff = read_file(av[1]);
	if (!buff)
		return (1);
	re_arr = re_arr_create(LANGUAGE, sizeof LANGUAGE / sizeof(char *));
	if (!re_arr)
		return (1);
	lex = lex_exe(re_arr, sizeof LANGUAGE / sizeof(char *), LANGUAGE_COMMENT, buff);
	if (!lex)
	{
		free(buff);
		re_arr_free(re_arr, sizeof LANGUAGE / sizeof(char *));
		return (1);
	}
	lex_print(lex);
	lex_free(lex);
	re_arr_free(re_arr, sizeof LANGUAGE / sizeof(char *));
	free(buff);
	return (0);
}

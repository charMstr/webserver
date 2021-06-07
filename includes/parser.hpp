/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/09 20:02:38 by bccyv             #+#    #+#             */
/*   Updated: 2021/05/27 07:00:29 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <iostream>
#include "../includes/webserver.hpp"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>

typedef enum e_tok {
	SERVER = 0,
	LOCATION,
	SERVER_NAME,
	LISTEN,
	ROOT,
	ALIAS,
	INDEX,
	ERROR,
	CLI_MAX_SIZE,
	ALLOW,
	AUTOINDEX,
	CGI_PASS,
	BLOCK_OP,
	BLOCK_CL,
	COMMA,
	SEPARATOR,
	PORT,
	METHOD,
	CGI_FILE_EXTENSION,
	DOMAIN_NAME,
	PATH,
	COMMENT,
} t_tok;

extern "C" {
	#include "../libraries/lexer/includes/lexer.h"
}

char	*err(int errcode, const char *path);
char	*read_config_file(const char *path);
static void server_init(t_srv &server, std::string &current_working_dir);
static void location_init(t_loc &location, t_srv &server);
static int	parse_domain_name(std::string &str, t_lex **ptr);
static int	parse_autoindex(bool &ai, t_lex **ptr);
static int	parse_path(std::string &str, t_lex **ptr);
static int	parse_error_page_line(t_error_page &err_page, t_lex **ptr);
static int	parse_listen(unsigned int &port, t_lex **ptr);
static int	parse_cli_max_size(unsigned int &cms, t_lex **ptr);
static int	parse_allow(t_methods &methods, t_lex **ptr);
static int	parse_cgi_pass(std::list<t_cgi_pass> &lst_cgi, t_lex **ptr);
static int	parse_location(t_srv &server, t_lex **ptr);
static int	parse_server(t_srv &server, t_lex **ptr, std::string &cwd);
int	set_current_working_dir(std::string &cwd);
int parse_config(const char *path, std::vector<t_srv> *servers);

#endif

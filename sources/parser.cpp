/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/26 10:28:17 by lfalkau           #+#    #+#             */
/*   Updated: 2021/06/07 19:55:21 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/parser.hpp"

const char	*g_lexemes[] = {

	//reserved keywords
	[SERVER] = "server",
	[LOCATION] = "location",
	[SERVER_NAME] = "server_name",
	[LISTEN] = "listen",
	[ROOT] = "root",
	[ALIAS] = "alias",
	[INDEX] = "index",
	[ERROR] = "error_page",
	[CLI_MAX_SIZE] = "cli_max_size",
	[ALLOW] = "allow",
	[AUTOINDEX] = "autoindex",
	[CGI_PASS] = "cgi_pass",

	//regex lexemes
	[BLOCK_OP] = "{",
	[BLOCK_CL] = "}",
	[COMMA] = ",",
	[SEPARATOR] = ";",
	[PORT] = "[0-9]+", //recycled for int in the "error_page 400 error.html" lines
	[METHOD] = "[A-Z]+",
	[CGI_FILE_EXTENSION] = "\\*|\\.[a-z]+",
	[DOMAIN_NAME] = "_|[a-z0-9A-Z-]+(\\.[a-z0-9A-Z-]+)*", //needs improvement
	[PATH] = "(/|~)?((-|_|[a-zA-Z0-9]|\\.|\\.\\.)*/?)+",
	[COMMENT] = "#[^\n]*",
};

char	*err(int errcode, const char *path)
{
	switch (errcode)
	{
		case 0:
			std::cout << "Webserv: " << path << " is not a valid file" << std::endl;
			break ;
		case 1:
			std::cout << "Webserv: " << path << " is too big" << std::endl;
			break ;
		case 2:
			std::cout << "Webserv: Error while opening " << path << std::endl;
			break ;
		case 3:
			std::cout << "Webserv: malloc error" << std::endl;
			break ;
		case 4:
			std::cout << "Webserv: Error while reading " << path << std::endl;
			break ;
		default:
			std::cout << "Webserv: undefined error while parsing " << path << std::endl;
			break ;
	}
	return (NULL);
}

char	*read_config_file(const char *path)
{
	int				file;
	char			*content;
	struct stat		sb;

	if (stat(path, &sb) < 0 || !S_ISREG(sb.st_mode))
		return (err(0, path));
	if (sb.st_size > DFL_CNF_MAX_SIZE)
		return (err(1, path));
	if ((file = open(path, O_RDONLY)) < 0)
		return (err(2, path));
	if (!(content = (char *)malloc(sizeof(char) * (sb.st_size + 1))))
	{
		close(file);
		return (err(3, NULL));
	}
	if (read(file, content, sb.st_size) < 0)
	{
		close(file);
		free(content);
		return (err(4, path));
	}
	content[sb.st_size] = '\0';
	close(file);
	return (content);
}

/*
** Initialising the values of the required field to default values. Then those
** can be overwritten as the parsing goes.
**
** NOTE: the structure error_page (containing the error_page name and the ints
** for which it is used) is not initialised. In nginx, if the error_page is not
** provided, an hard coded page is sent to the client's socket (no file needs
** to be opened, faster and probably safer?).
**
** parameters:
** 	- struct t_serv: we are filling it with standard values.
** 	- std::string current_working_director: it is always the same string for
** 		each server block, and it is just placed into server.directory
** 		note: In nginx it is hardcoded to be "/usr/local/etc/nginx/html/" on mac,
** 		and it is pre-pended to any root directive that are provided (or not),
** 		which in turn is prepended to the path_part_of_uri.
*/
static void server_init(t_srv &server, std::string &current_working_dir)
{
	server.directory = std::string(current_working_dir); //always the same

	server.root = "";
	server.host_port = 80;
	server.server_name = "_";
	server.index = "index.html";

	//note: if the error page is not defined, we respond with hardcoded page.
	//server.error_page.error_page = "error.html";

	server.autoindex = false;
	server.cli_max_size = DFL_CLI_MAX_SIZE;
	server.methods.bf = GET;
	server.locations.clear();
	server.lst_cgi_pass.clear(); //no tupple { extension, cgi_bin } by default
}

/*
** NOTE: the alias directive, just like in nginx: "The alias directive tells
** Nginx to replace what is defined in the location block with the path
** specified by the alias directive."
**
** NOTE: if the error page is not defined, we should just send a "page" that
** is hard coded in our server.
*/
static void location_init(t_loc &location, t_srv &server)
{
	location.path = "";
	location.index = server.index;
	location.root = server.root;
	location.alias = ""; //the alias will replace root if not the empty string.
	//location.error_page = server.error_page;
	location.autoindex = server.autoindex;
	location.cli_max_size = server.cli_max_size;
	location.methods.bf = server.methods.bf;
	location.lst_cgi_pass.clear(); //no tupple { extension, cgi_bin } by default
}

static int	parse_domain_name(std::string &str, t_lex **ptr)
{
	if ((*ptr)->token != DOMAIN_NAME)
		return (-1);
	str = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
	*ptr = (*ptr)->next;
	return (0);
}

static int	parse_autoindex(bool &ai, t_lex **ptr)
{
	if ((*ptr)->token != DOMAIN_NAME)
		return (-1);
	std::string str = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
	if (str == "on")
		ai = true;
	else if (str == "off")
		ai = (false);
	else
		return (-1);
	*ptr = (*ptr)->next;
	return (0);
}

static int	parse_path(std::string &str, t_lex **ptr)
{
	if ((*ptr)->token != PATH && (*ptr)->token != DOMAIN_NAME)
		return (-1);
	str = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
	*ptr = (*ptr)->next;
	return (0);
}

/*
** This function is called when the current token is of type ERROR (error_page
** keyword in configuration file).
** Few examples of valid nginx configuration lines will help understanding the
** goal of this function:
**
**  error_page 500 502 503 504 /custom_50x.html; VALID
**  error_page 400 /custom_50x.html; VALID
**  error_page 400 400 400 400  /custom_50x.html; VALID (even if same number)
**  error_page  /custom_50x.html; NOT VALID!
**  error_page 200 /custom_50x.html; NOT VALID!
**
** 	grammar: after the "error_page" reserved keyword, we need at least an int
** 	(PORT token), then a PATH token, and a SEPARATOR token.
**
** note: PORT token, should contain an int comprised between 300 and 599.
** exemple of nginx debuging trace:
** nginx: [emerg] value "400000" must be between 300 and 599 in
** /usr/local/etc/nginx/servers/nginx_mine.conf:14
**
** RETURN:	0 : ok
** 			-1 : failure.
*/

static int	parse_error_page_line(t_error_page &err_page, t_lex **ptr)
{
	if (err_page.list_error_status.empty() && (*ptr)->token != PORT)
	{
		std::cout << "\"error_page\" keyword  must be folowed by an integer"\
			<< std::endl;
		return (-1); //return because we need at least one number in list
	}
	if ((*ptr)->token == PORT)
	{
		int error_status = atoi((*ptr)->beg);
		if (error_status > 599 || error_status < 300)
		{
			std::cout << "integer folowing \"error_page\" keyword must be "\
				"between 300 and 599" << std::endl;
			return (-1);
		}
		std::list<int>::iterator it;
		it = find (err_page.list_error_status.begin(), \
				err_page.list_error_status.end(), error_status);
		if (it == err_page.list_error_status.end())
			//only add int if not present in list yet.
			err_page.list_error_status.push_back(error_status);
		*ptr = (*ptr)->next;
		//recurcion as long as we encounter more PORT tokens
		return (parse_error_page_line(err_page, ptr));
	}
	else if ((*ptr)->token == PATH || (*ptr)->token == DOMAIN_NAME)
	{
		err_page.error_page = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
		*ptr = (*ptr)->next;
		return (0);
	}
	std::cout << "keyword \"error_page\" + integer must be followed by a path"\
		<< std::endl;
	return (-1);
}

static int	parse_listen(unsigned int &port, t_lex **ptr)
{
	if ((*ptr)->token != PORT)
		return (-1);
	port = atoi((*ptr)->beg);
	*ptr = (*ptr)->next;
	return (0);
}

static int	parse_cli_max_size(unsigned int &cms, t_lex **ptr)
{
	if ((*ptr)->token != PORT)
		return (-1);
	cms = atoi((*ptr)->beg);
	*ptr = (*ptr)->next;
	return (0);
}

static int	parse_allow(t_methods &methods, t_lex **ptr)
{
	static std::string	methods_table[] = {"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"};
	unsigned int		i;
	unsigned int		method;

	while (*ptr && (*ptr)->token != SEPARATOR)
	{
		if ((*ptr)->token != METHOD)
			return (-1);
		i = 0;
		method = 0;
		while (i < 8)
		{
			if (methods_table[i].compare(0, std::string::npos, (*ptr)->beg, (*ptr)->end - (*ptr)->beg) == 0)
			{
				method = 1 << i;
				break ;
			}
			i++;
		}
		if (!method)
			return (-1);
		METHODS_SET(&methods, method);
		*ptr = (*ptr)->next;
		if (*ptr && (*ptr)->token == COMMA)
			*ptr = (*ptr)->next;
	}
	return (0);
}

/*
** NAME: parse_cgi_pass();
**
** This function has the role of parsing the cgi_pass directive. In our
** configuration file, the cgi_pass directive must have the form:
** cgi_pass + extention name + binary name + ;
** Its purpose is to associate the name of a binary to an extension.
** The cgi_pass element in structures server or location, is a list, so that we
** can have the choice to launch different cgi scripts, depending on the file
** extension.
**
** PARAMETERS:
** - lst_cgi: a reference to the filed named lst_cgi_pass, found either in the
**	t_srv struct(representing a server block) or the t_loc struct (representing
**	a location block within a server block).
** - ptr: a tripple ref pointer to a lexem.
**
** RETURNS: -1: an error occured.
** 			0: parsing went fine.
**
** ERRORS: an exceptoin can be thrown if push_back fails, or std::string fails.
*/

static int parse_cgi_pass(std::list<t_cgi_pass> &lst_cgi, t_lex **ptr)
{
	if ((*ptr)->token != CGI_FILE_EXTENSION)
		return (-1);
	t_cgi_pass tripple;
	tripple.extension = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
	*ptr = (*ptr)->next;
	if ((*ptr)->token != PATH && (*ptr)->token != DOMAIN_NAME)
		return (-1);
	tripple.cgi_bin = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
	*ptr = (*ptr)->next;
	if ((*ptr)->token != METHOD)
		return (-1);
	int res = parse_allow(tripple.methods, &(*ptr));
	lst_cgi.push_back(tripple);
	return (res);
}

static int	parse_location(t_srv &server, t_lex **ptr)
{
	t_loc	location;

	location_init(location, server);
	if (((*ptr)->token != PATH && (*ptr)->token != DOMAIN_NAME) || !(*ptr)->next || (*ptr)->next->token != BLOCK_OP)
		return (-1);
	location.path = std::string((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
	*ptr = (*ptr)->next->next;
	while (*ptr)
	{
		int token = (*ptr)->token;
		*ptr = (*ptr)->next;
		if (!*ptr)
			return (-1);
		int ret = 0;
		switch (token)
		{
			case BLOCK_CL:
				server.locations.push_back(location);
				return (0);
			case ROOT:
				ret = parse_path(location.root, ptr); break ;
			case ALIAS:
				ret = parse_path(location.alias, ptr); break ;
			case INDEX:
				ret = parse_path(location.index, ptr); break ;
			case ERROR:
				ret = parse_error_page_line(location.error_page, ptr); break ;
			case AUTOINDEX:
				ret = parse_autoindex(location.autoindex, ptr); break ;
			case CLI_MAX_SIZE:
				ret = parse_cli_max_size(location.cli_max_size, ptr); break ;
			case ALLOW:
				ret = parse_allow(location.methods, ptr); break ;
			case CGI_PASS:
				ret = parse_cgi_pass(location.lst_cgi_pass, ptr); break;
			default:
				return (-1);
		}
		if (*ptr && (*ptr)->token != SEPARATOR)
		{
			std::string str((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
			std::cout << "parser: error near token \"" << str \
				<< "\" in configuration file." << std::endl;
		}
		if (ret < 0 || !*ptr || (*ptr)->token != SEPARATOR)
			return (-1);
		*ptr = (*ptr)->next;
	}
	return (-1);
}

/*
** parameters:
** 	- t_stv struct: being filled by the parsing
** 	- t_lex **ptr.
** 	- std::string cwd: a string that contains the root_directory of server, it
** 		is place in all the t_srv blocks, and is always the same.
*/

static int	parse_server(t_srv &server, t_lex **ptr, std::string &cwd)
{
	if ((*ptr)->token != SERVER || !(*ptr)->next || (*ptr)->next->token != BLOCK_OP)
		return (-1);
	*ptr = (*ptr)->next->next;
	server_init(server, cwd);
	while (*ptr)
	{
		int token = (*ptr)->token;
		*ptr = (*ptr)->next;
		if (!*ptr && token != BLOCK_CL)
			return (-1);
		int ret = 0;
		switch (token)
		{
			case BLOCK_CL:
				return (0);
			case LOCATION:
				if (parse_location(server, ptr) < 0)
					return (-1);
				continue ;
			case SERVER_NAME:
				ret = parse_domain_name(server.server_name, ptr); break ;
			case LISTEN:
				ret = parse_listen(server.host_port, ptr); break ;
			case ROOT:
				ret = parse_path(server.root, ptr); break ;
			case INDEX:
				ret = parse_path(server.index, ptr); break ;
			case ERROR:
				ret = parse_error_page_line(server.error_page, ptr); break ;
			case AUTOINDEX:
				ret = parse_autoindex(server.autoindex, ptr); break ;
			case CLI_MAX_SIZE:
				ret = parse_cli_max_size(server.cli_max_size, ptr); break ;
			case ALLOW:
				ret = parse_allow(server.methods, ptr); break ;
			case CGI_PASS:
				ret = parse_cgi_pass(server.lst_cgi_pass, ptr); break;
			default:
				return (-1);
		}
		if (*ptr && (*ptr)->token != SEPARATOR)
		{
			std::string str((*ptr)->beg, (*ptr)->end - (*ptr)->beg);
			std::cout << "parser: error near token \"" << str \
				<< "\" in configuration file." << std::endl;
		}
		if (ret < 0 || !*ptr || (*ptr)->token != SEPARATOR)
			return (-1);
		*ptr = (*ptr)->next;
	}
	return (-1);
}

/*
** this function will take care of setting the current working directory into
** a reference to a std::string that already exists.
**
** RETURN:	0: ok
** 			-1: failure happened in the function getcwd().
*/

int	set_current_working_dir(std::string &cwd)
{
	char *malloced_cwd;

	if ((malloced_cwd = getcwd(NULL, 0)) == NULL)
		return (-1);
	try
	{
		cwd.append(malloced_cwd);
		free(malloced_cwd);
	}
	catch(std::exception const &e)
	{
		free(malloced_cwd);
		throw e;
	}
	return (0);
}

/*
** note: current_working_dir is created so that it holds the working directory
** when starting our webserv. This value will be placed into any of our t_srv
** struct in the field directory. It will be used and pre-pended to any "root"
** directive and any uri. In nginx it is hardcoded to be
** "/usr/local/etc/nginx/html/" on mac.
** The resulting real path of a request is always:
** server.directory + root + path_part_of_uri
*/

int parse_config(const char *path, std::vector<t_srv> *servers)
{
 	char		*raw_file;
	int			ret = -1;
	t_srv		server;
	t_lex		*lexer, *ptr;
	t_regex		*re_arr;
	std::string	current_working_dir;
	unsigned int size_g_lexemes;

	if (set_current_working_dir(current_working_dir))
		return (-1);
	if (!(raw_file = read_config_file(path)))
		return (-1);

	size_g_lexemes = sizeof(g_lexemes) / sizeof(char *);
	if (!(re_arr = re_arr_create(g_lexemes, size_g_lexemes)))
	{
		free(raw_file);
		return (-1);
	}
	ptr = lexer = lex_exe(re_arr, size_g_lexemes, COMMENT, raw_file);
	re_arr_free(re_arr, size_g_lexemes);
	if (!ptr)
	{
		free(raw_file);
		return (-1);
	}
	while (ptr && (ret = parse_server(server, &ptr, current_working_dir)) != -1)
		servers->push_back(server);
	lex_free(lexer);
	free(raw_file);
	return (ret);
}

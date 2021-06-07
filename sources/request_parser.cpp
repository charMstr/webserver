#include "../includes/request_parser.hpp"
#include "../includes/exceptions.hpp"
#include "../includes/string.hpp"
#include "../includes/http_methods.hpp"
#include "../includes/debug_functions.hpp"
#include "../includes/utils.hpp"

/* ************************************************************************** */
/*	Constructors and destructors                                              */
/* ************************************************************************** */

Request_parser::Request_parser(std::string &request_buffer) :
	request_buffer(request_buffer),
	offset(0),
	body_format(NO_BODY),
	method(0),
	is_cgi(false)
{
	//std::cout << "\033[33mconstructor called for request_parser\033[0m" << std::endl;
}

Request_parser::~Request_parser(void)
{
	//std::cout << "\033[31m[~]\033[33mdestructor called for request_parser\033[0m" << std::endl;
}

Request_parser::Request_parser(Request_parser const &src) :
	request_buffer(src.request_buffer)
{
	//std::cout << "\033[33m[COPY]constructor called for hey\033[0m" << std::endl;
	*this = src;
	return ;
}

Request_parser	&
Request_parser::operator=(Request_parser const &rhs)
{
	//std::cout << "\033[33m'=' operator called for Request_parser\033[0m" << std::endl;
	if (this != &rhs)
	{
		this->request_buffer = rhs.request_buffer;
		this->offset = rhs.offset;
		this->body_format = rhs.body_format;
		this->content_length = rhs.content_length;
		this->method = rhs.method;
		this->uri = rhs.uri;
		this->headers = rhs.headers;
		this->is_cgi = rhs.is_cgi;
		this->query_string = rhs.query_string;
		this->mime_type = rhs.mime_type;
		this->cgi_bin_name = rhs.cgi_bin_name;
	}
	return *this;
}

/* ************************************************************************** */
/*	 member functions                                                         */
/* ************************************************************************** */

/*
**	Sets the request method and URI. Updates the offset past the start line.
**	Returns 0 if succesful, -1 otherwise.
*/
int Request_parser::parse_start_line()
{
	if (parse_method() < 0)
		return (-1);
	if (parse_uri() < 0)
		return (-1);
    if (request_buffer.substr(offset, 10) != "HTTP/1.1\r\n")
        return (-1);
	offset += 10;
    return (0);
}

/*
**	Sets the request method URI. Updates the offset past the HTTP verb
**	Returns 0 if succesful, -1 otherwise.
*/
int	Request_parser::parse_method(void)
{
	if ((offset = request_buffer.find(" ")) == std::string::npos)
		return (-1);
	if ((method = str_to_method(request_buffer.substr(0, offset))) < 0)
		return (-1);
	offset++;
	return (0);
}

/*
**	Sets the request URI. Updates the offset past the URI.
**	Returns 0 if succesful, -1 otherwise.
*/
int	Request_parser::parse_uri(void)
{
	size_t  	tmp;

	tmp = offset;
	if ((offset = request_buffer.find(" ", offset)) == std::string::npos)
		return (-1);
	uri = request_buffer.substr(tmp, offset - tmp);
	offset++;
	return (0);
}

/*
** name: set_up_related_to_cgi()
** This function is called once we know we have a GET or a POST method. It is
** called in Service generator, once we decided which location block to use.
**
** PARAMETERS:
** - server: use if there is no match in the location_block_to_use's t_cgi_pass
**	structs.
** - location_block_to_use: a pointer to the selected location block, according
**	to the URI. It contains a list of t_cgi_pass structs. It it is NULL, or if
**	we dont find a match with it, we then use the server block. The structs
**	t_cgi_pass contain a field extension, it is compared with the end of the
**	URI.
**
**	RETURN: 200: status code still okay.
**			other: the new value of the status code, set in the reader.
*/
int
Request_parser::set_up_related_to_cgi(t_loc const *location_block_to_use, const t_srv &server)
{
	size_t tmp;
	std::string tmp_uri(uri);

	if ((tmp = uri.find("?")) != std::string::npos)
	{
		query_string = uri.substr(tmp + 1);
		tmp_uri.erase(tmp);
	}
	if (location_block_to_use)
		set_up_related_to_cgi_match_extension(\
				location_block_to_use->lst_cgi_pass, tmp_uri);
	if (!is_cgi) //if cgi was not set yet
		set_up_related_to_cgi_match_extension(\
				server.lst_cgi_pass, tmp_uri);
	if (is_cgi)
		uri = tmp_uri; //update uri with potentially resized uri.

 //TODO(Thu 27/05/2021 at 09:45:30)
	// here we could remove the escaped characters in uri...
	//ane potentially return something else than 200? check CGI/1.1 rfc... not
	//even sure its an error.
	return (200);
}

/*
** name: set_up_related_to_cgi_match_extension()
** This function will try to match at the end of the uri an extension found in
** one of the t_cgi_pass structs from the list.
**
** If a match is found, we set the is_cgi boolean to true, we set the values
** like PATH_INFO, and the script name is the cgi_bin field found in the
** t_cgi_pass struct.
**
** PARAMETERS:
** - cgi_tupples: a list of t_cgi_pass struct (containing std::string extension
** 	, the extension we are trying to match, and std::string cgi_bin, the
** 	cgi_script we are going to use if there is a match
** - uri: the string we are trying to match the extensions with.
**
** NOTE: for a match to be valid, we need to find the extension at the end of
** the uri, and there must be at least one valid filename charactere before the
** '.', example of fake match: uri = "foo/bar/.php"
**
** NOTE: if extension's value is just the "*" string its always a match,
** whatever the uri.
*/

void
Request_parser::set_up_related_to_cgi_match_extension(\
		std::list<t_cgi_pass> const &cgi_tupples, std::string const &uri)
{
	for (std::list<t_cgi_pass>::const_iterator it = cgi_tupples.begin();
			it != cgi_tupples.end(); it++)
	{
		size_t len_extension = it->extension.length();
		if (METHODS_ISSET(&it->methods, get_method()) \
			&& ((uri.length() > len_extension &&
			uri.compare(uri.size() - len_extension, std::string::npos, \
				it->extension) == 0) || (it->extension == "*")))
		{
			is_cgi = true;
			cgi_bin_name = it->cgi_bin;
			//std::string file_name = uri.substr(uri.rfind("/") + 1);
			mime_type = mime_types[it->extension];
		}
	}
}

static bool is_invalid_header(std::string const &key)
{
	if (key.length() >= 2 && key.compare(0, 2, "X-", 2) == 0)
		return (true);
	return (false);
}

/*
**	Parses a single header line.
**	If the parsed key already exists in the headers, then the parsed value is
**	appended to it, else, the key and value pair is inserted in the headers map.
**	Returns 0 if there is no more header to read, -1 on error, 1 otherwise.
*/
int	Request_parser::parse_header_field(void)
{
	std::string	key;
	std::string	value;
	size_t		i;

    if (request_buffer.compare(offset, 2, "\r\n", 2) == 0)
	{
		offset += 2;
		return (0);
	}
	if ((i = request_buffer.find(':', offset)) == std::string::npos)
		return (-1);
	key = request_buffer.substr(offset, i - offset);
	if (key.find(' ') != std::string::npos)
		return (-1);
	transform(key.begin(), key.end(), key.begin(), ::tolower);
	offset = i + 1;
	if ((i = request_buffer.find("\r\n", offset)) == std::string::npos)
		return (-1);
	value = request_buffer.substr(offset, i - offset);
	trim(value, std::isspace);
	offset = i + 2;
	if (get_header(key).size())
	{
		headers[key] += ", " + value;
	}
	else
	{
		if (!is_invalid_header(key))
			headers.insert(std::pair<std::string, std::string>(key, value));
	}
	return (1);
}

/*
**	This function simply moves the offset at the end as a content length type
**	body is already at its final state.
*/
void	Request_parser::parse_body(void)
{
	offset = request_buffer.size();
}

/*
**	This function parses a chunk of a chunked body.
**	It checks if a whole chunk is available between @offset and the end of
**	@request_buffer, if there is, it unchunks it and appends it to the body.
**	else it returns -2 (to wait for more bytes)
**	Returns -1 on error, 0 if the body is fully parsed, 1 if a whole chunk
**	has been parsed and 2 to wait for more bytes.
**	
**	PARAM:
**	- current_content_length: it is the body length of the request so far. it
**	is updated for the request_reader
**	- cli_max_size: to detect as early as possible a body too long.
**
**	Returns -1 on error, 0 to wait for more bytes, 1 if the final chunk is found
**	and 2 if a single chunk has been parsed.
*/
int Request_parser::parse_chunked_body(size_t  &current_content_length)
{
	size_t	chunk_pos;
	size_t	chunk_size;
	size_t	pos = offset;

	try
	{
		if ((chunk_pos = request_buffer.find("\r\n", offset)) == std::string::npos)
			return (0);
		chunk_size = ft_stoul(request_buffer, &pos, 16);

		if (pos != chunk_pos) //security. ok
			return (-1);
		//make sure we have the complete double CRLF after the 0.
		if (chunk_size == 0)
		{
			if (chunk_pos + 4 > request_buffer.length())
				return (0); //make sure the complete double CRLF is there.
			//need to unchunk here.... we do add + 4 to cover the double CRLF
			request_buffer.erase(offset, (chunk_pos + 4)  - offset);
			return (1);
		}
		chunk_pos += 2;
		//added the extra + 2 to be sure to capture the closing "\r\n"
		if (request_buffer.size() - chunk_pos < chunk_size + 2)
			return (0);
		request_buffer.erase(offset, chunk_pos - offset);
		offset += chunk_size;
		current_content_length += chunk_size;
		if (request_buffer.compare(offset, 2, "\r\n") == 0)
			request_buffer.erase(offset, 2);
		else
			return (-1);
		return (2);

	} catch (const std::exception &e)
	{
		//std::cerr << "\033[1m \033[35m" << e.what() << \
		//"In Request_parser::parse_chunked_body, RETURNING -1" << "\033[0m \033[21m" <<  std::endl;
		return (-1);
	}
}

void	Request_parser::trash_procecced_bytes(void)
{
	request_buffer.erase(0, offset);
	offset = 0;

}

void	Request_parser::dump_request_buffer(std::string &dst)
{
	if (offset != 0)
	{
		dst += request_buffer.substr(0, offset);
		trash_procecced_bytes();
	}
}


/* ************************************************************************** */
/*	Getters                                                                   */
/* ************************************************************************** */

size_t Request_parser::get_offset(void) const
{
	return (offset);
}

std::string &Request_parser::get_request_buffer(void) const
{
	return (request_buffer);
}

bool Request_parser::is_a_cgi_request(void) const
{
	return (is_cgi);
}

const std::string &Request_parser::get_query_string(void) const
{
	return (query_string);
}

const std::string &Request_parser::get_mime_type() const
{
	return (mime_type);
}

const std::string &Request_parser::get_cgi_bin_name() const
{
	return (cgi_bin_name);
}

int	Request_parser::get_method(void) const
{
	return (method);
}

const std::string &Request_parser::get_uri(void) const
{
	return (uri);
}

const std::string Request_parser::get_header(std::string key) const
{
	transform(key.begin(), key.end(), key.begin(), ::tolower);
	return (headers.find(key) == headers.end() ? std::string("") : headers.at(key));
}

const Headers &Request_parser::get_headers(void) const
{
	return (headers);
}

t_body_format Request_parser::get_body_format(void) const
{
	return (body_format);
}

size_t Request_parser::get_content_length(void) const
{
	return (content_length);
}

/* ************************************************************************** */
/*	Setters                                                                   */
/* ************************************************************************** */

static int	parse_content_length_header(std::string val, unsigned long &cl)
{
	size_t pos;
	std::vector<std::string> values;

	while ((pos = val.find(",")) != std::string::npos)
	{
        values.push_back(val.substr(0, pos));
		trim(values.back(), std::isspace);
        val.erase(0, pos + 1);
	}
	values.push_back(val.substr(0));
	trim(values.back(), std::isspace);
	for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++)
	{
		if (*it != values.front())
			return (-1);
	}
	pos = 0;
	cl = ft_stoul(values.front(), &pos);
	if (pos == std::string::npos)
		return (0);
	else
		return (-1);
}

/*
**	Sets the type of the request body and its related attributes.
*/
void Request_parser::set_body_format(void)
{
	std::string	field_value;

	// Request smuggling protection
	if (get_header("content-length").size() > 0 && get_header("transfer-encoding").size() > 0)
	{
		body_format = INVALID_BODY;
	}
	else if ((field_value = get_header("content-length")).size() > 0)
	{
		if (parse_content_length_header(field_value, content_length) < 0)
			body_format = INVALID_BODY;
		else
			body_format = CONTENT_LENGTH_BODY;
	}
	else if ((field_value = get_header("transfer-encoding")).size() > 0)
	{
		if (field_value.size() >= 7 && field_value.compare(field_value.size() - 7, std::string::npos, "chunked") == 0)
			body_format = CHUNKED_BODY;
		else
			body_format = INVALID_BODY;
	}
	else
		body_format = NO_BODY;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/28 06:52:58 by charmstr          #+#    #+#             */
/*   Updated: 2021/06/06 12:50:14 by Remercill        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserver.hpp"
#include "../includes/exceptions.hpp"
#include "../includes/utils.hpp"
#include <sys/stat.h>
#include <sys/time.h>

/*
** used to make it possible to shutdown our server then quickly restart it, and
** still be able to rebind a socket that the kernel has in a TIME_WAIT state.
**/
void set_socket_rebindable_for_quick_restart(int sock)
{
	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		throw exception_webserver(strerror(errno), \
		"setsockopt(SO_REUSEADDR) failed", __FILE__, __LINE__, INITIALISING);
}

/*
** this function will set the O_NONBLOCK flag on already accepted sockets
*/
void set_nonblock(int socket)
{
    if ((fcntl(socket, F_SETFL, O_NONBLOCK)) == -1)
		throw exception_webserver(strerror(errno), "set_nonblock()", \
				__FILE__, __LINE__, "fcntl()", WORKING);
}

/*
**	Inserts the root path at the beginning of the ressource path.
**	If root doesn't ends with a '/', one will be inserted between the two parts.
**	Returns 0 if successful, -1 otherwise.
*/
int	concatenate_path(const std::string &root, std::string &ressource)
{
	try
	{
		if (root.size() > 0 && root.at(root.size() - 1) != '/')
			ressource.insert(ressource.begin(), '/');
		ressource.insert(0, ressource);
	}
	catch (const std::exception& e)
	{
		return (-1);
	}
	return (0);
}

const t_loc	*get_location_block_to_use(const std::string &uri, t_srv &server)
{
	std::vector<t_loc>::const_iterator	i;
	size_t								match_specificity = 0;
	const t_loc							*best;

	best = NULL;
	if (uri.length() != 0)
	{
		for (i = server.locations.begin(); i != server.locations.end(); i++)
		{
			// if match starts at begining of string
			if (uri.find(i->path) == 0)
			{
				// and this match is more specific
				if (i->path.length() > match_specificity)
				{
					match_specificity = i->path.length();
					best = &(*i);
				}
			}
		}
	}
	return (best); // NULL if no location block matches the uri or the uri is empty
}

/*
**	Returns the path where to search for the ressource, based on the URI and
**	the server location blocks and the server default root.
*/
std::string locate_ressource(const std::vector<t_loc> &locations, const std::string &default_root, const std::string& path)
{
   const t_loc	*more_specific_loc = NULL;
   size_t		more_specific_loc_length = 0;

   for (std::vector<t_loc>::const_iterator i = locations.begin(); i != locations.end(); i++)
   {
	   if (path.find(i->path) == 0 && i->path.length() > more_specific_loc_length)
	   {
		   more_specific_loc = &(*i);
		   more_specific_loc_length = i->path.length();
	   }
   }
   return (more_specific_loc ? more_specific_loc->root : default_root);
}

/*
**	Returns true if the given path is an existing file, false otherwise.
*/
bool	do_file_exists(const std::string &path)
{
	struct stat	s;

	return (stat(path.c_str(), &s) >= 0);
}

/*
**	Converts @str into a base @base unsigned long, starting at pos @pos.
**	Returns the converted value.
**	The @pos pointer indicates how much characters have been converted. It is
**	set to std::string::npos if the whole string has been converted.
**
**	In case of failure, an exception is thrown.
*/
unsigned long	ft_stoul(const std::string& str, std::size_t* pos, int base)
{
	std::string		charset = "0123456789ABCDEF";
	unsigned long	value = 0, tmp;
	size_t			default_pos = 0;

	if (!pos)
		pos = &default_pos;
	if (base < 2 || base > 16 || *pos >= str.size())
		throw std::invalid_argument("Invalid arguments");
	charset.erase(base);
	while (isspace(str[*pos]))
		(*pos)++;
	for (; *pos < str.size(); (*pos)++)
	{
		tmp = charset.find(toupper(str[*pos]));
		if (tmp == std::string::npos)
			break ;
		tmp = value * base + tmp;
		if (tmp < value)
			throw std::out_of_range("An overflow occured");
		value = tmp;
	}
	if (*pos == str.size())
		*pos = std::string::npos;
	return (value);
}

std::string		ft_ultos(unsigned long n)
{
	std::string	s = "";

	try
	{
		while (n > 0)
		{
			s.insert(s.begin(), n % 10 + '0');
			n /= 10;
		}
		if (s.size() == 0)
			s = "0";
	}
	catch (const std::exception &e)
	{
		return ("");
	}
	return (s);
}

char *ft_strdup(const char *src)
{
	size_t	len = 0;
	char	*dst;

	if (!src)
		return (NULL);
	while (src[len])
		len++;
	if (!(dst = (char *)malloc(sizeof(char) * (len + 1))))
		return (NULL);
	// len + 1 will also copy the trailing '\0'
	for (size_t i = 0; i < len + 1; i++)
		dst[i] = src[i];
	return (dst);
}


std::string		get_the_time(void)
{
	struct tm time; struct timeval tv; char buf[64] = {'\0'};
	gettimeofday(&tv, NULL);

	strptime(std::to_string(tv.tv_sec).c_str(), "%s", &time);
	const char* fmt = "%a, %d %b %Y %X GMT";
	strftime(buf, sizeof(buf), fmt, &time);
	return (buf);
}

std::string		get_last_modified_time(const struct stat * fileStat)
{
	struct tm time; char buf[64] = {'\0'};
	const char * fmt = "%a, %d %b %Y %X GMT";

	strptime(std::to_string(fileStat->st_mtime).c_str(), "%s", &time);
	strftime(buf, sizeof(buf), fmt, &time);
	return (buf);
}

void	get_chunk(std::string & payload, std::string & data, size_t & already_encoded_data)
{
	(void)payload; (void)(data); (void)(already_encoded_data);
}

/*
** TAKEN FROM LIBFT
** /!\ original function isn't protected against NULL pointer inputs.
**
** RETURN:	address of the start of the string needle if found in haystack
**			else NULL
*/

char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
{
	size_t i;
	size_t j;

	i = 0;
	if (!needle[0])
	{
		return ((char *)haystack);
	}
	while ((i < len) && haystack[i])
	{
		j = 0;
		if (haystack[i] == needle[j])
		{
			while ((i + j < len) && haystack[i + j] && needle[j] \
					&& *(haystack + i + j) == *(needle + j))
				j++;
			if (*(needle + j) == '\0')
				return ((char *)(haystack + i));
		}
		i++;
	}
	return (NULL);
}

/*
** This fuction predicts the size of the chunk tag given the size of the string
** it returns the size the itoa_base(space_left_in_buffer, 16) would need,
** plus 4, for the double CRLF needed in each tag. (note: the trailing '\0' is
** taken into account in the result as we wont need it).
*/

int	predict_chunk_tag_space(size_t space_left_in_buffer)
{
	int i = 1;
	while (space_left_in_buffer >= 16)
	{
		space_left_in_buffer /= 16;
		i++;
	}
	return (i + 4); //4 represents the space taken by "\r\n\r\n"
}

/*
** This function returns the close_chunk string to use. It is based on the
** value of autoindex.size_chunk.
**
** NOTE: it only returs the begining of the chunk tag. we still need to append
** to the message the trailing CRLF.
*/

std::string
get_close_chunk_string(int size_chunk)
{
	std::stringstream ss;
	ss << std::hex << size_chunk;
	return (std::string(ss.str() + "\r\n"));
}

/*
** predicate, to check if two characters are equal (case insensitive).
** used: in ci_find_substr()
*/
struct my_equal {
    bool operator()(char ch1, char ch2) {
        return std::toupper(ch1) == std::toupper(ch2);
    }
};

// find substring (case insensitive)
int ci_find_substr(const std::string &str1, const std::string &str2)
{
    const std::string::const_iterator it = std::search( str1.begin(), \
		str1.end(), str2.begin(), str2.end(), my_equal());
    if ( it != str1.end() ) return it - str1.begin();
    else return -1; // not found
}

static void long_to_char_array(unsigned long n, unsigned char arr[4])
{
	for (int i = 0; i < 4; i++)
	{
		arr[i] = (n << (8 * i)) >> 24;
	}
}

std::string	get_ip(const struct sockaddr_in &addr)
{
	unsigned char	addr_bytes[4] = {0};
	std::string		literal_addr;

	long_to_char_array(ntohl(addr.sin_addr.s_addr), addr_bytes);
	literal_addr = ft_ultos(addr_bytes[0]) + "." + ft_ultos(addr_bytes[1]) + "." + ft_ultos(addr_bytes[2]) + "." + ft_ultos(addr_bytes[3]);

	return (literal_addr);
}

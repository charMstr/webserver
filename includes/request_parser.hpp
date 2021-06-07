/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_parser.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/14 12:55:12 by bccyv             #+#    #+#             */
/*   Updated: 2021/06/06 09:58:49 by Remercill        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP

# include <map>
# include <string>
# include <vector>
#include "../includes/webserver.hpp"

typedef enum e_body_format
{
	NO_BODY,
	CHUNKED_BODY,
	CONTENT_LENGTH_BODY,
	INVALID_BODY,
}	t_body_format;

extern std::map<std::string, std::string> mime_types;

/*
** This class will parse a request_buffer. A request_buffer is a std::string yielded
** by a request_reader object on a socket (fd_client).
**
** Note: In fine this class will be given as parameter to service_generator. A
** service_generator is an object that will allocate an object derived from the
** Service class. This created object will be added to the list of services
** being processed in the Server class.
**
** Note: When parsing a request_buffer, if some errors are found in it. We need to
** keep track about it, in member variables, so that we can later generate an
** appropriate service object. (ex: we need to answer an error message like 404
** or totally refuse a client's request.)
*/

typedef std::map<std::string, std::string>	Headers;

class Request_parser
{
	friend class Cgi_service;
	friend class request_reader;
	public:

		// Constructors and destructor
		Request_parser(std::string &request_buffer);
		~Request_parser();
		Request_parser(Request_parser const &src);
		Request_parser &operator=(Request_parser const &rhs);


		//friend, for debugging purpose
		friend class Cgi_service_test;

		// Member functions
		int							parse_start_line(void);
		int							parse_header_field(void);
		void						parse_body(void);
		int							parse_chunked_body(size_t &current_content_length);
		void						trash_procecced_bytes(void);
		void						dump_request_buffer(std::string &dst);
		int							set_up_related_to_cgi(\
				t_loc const *location_block_to_use, t_srv const &server);
		void						set_up_related_to_cgi_match_extension(\
				std::list<t_cgi_pass> const &cgi_tupples, std::string const &uri);

		// Getters
		size_t						get_offset(void) const;
		std::string					&get_request_buffer(void) const;
		int							get_method(void) const;
		const std::string			&get_uri(void) const;
		const std::string			get_header(std::string) const;
		const Headers				&get_headers(void) const;
		bool						is_a_cgi_request(void) const;
		const std::string			&get_query_string(void) const;
		const std::string			&get_mime_type() const;
		const std::string			&get_cgi_bin_name() const;
		t_body_format				get_body_format(void) const;
		size_t						get_content_length(void) const;

		// Setters
		void						set_body_format(void);

	private:

		// Member functions
		int							parse_method(void);
		int							parse_uri(void);

		// Attributes
	public:
		std::string					&request_buffer;
	private:
		size_t						offset;
		t_body_format				body_format;
		size_t						content_length;

		int							method;
		std::string					uri;
		Headers						headers;
		bool						is_cgi;
		std::string					query_string;
		std::string					mime_type;
		std::string					cgi_bin_name;
};


#endif

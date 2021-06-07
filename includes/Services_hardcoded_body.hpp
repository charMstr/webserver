/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Services_hardcoded_body.hpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/28 20:06:57 by charmstr          #+#    #+#             */
/*   Updated: 2021/06/07 19:56:14 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVICES_HARDCODED_BODY_HPP
# define SERVICES_HARDCODED_BODY_HPP

/*
** This file holds the builtin messages that we are going to use when answering
** to client directly, not from a filedescriptor. All those messages are stored
** into a static array of structures. The array is a static member variable of
** the class Service. Declaring all the static strings here is for the sake of
** clarity.
**
** note: In this array, each structure holds a message and its length.
**
** usage: std::cout << webserv_answer_builtin_bodies[307 - \
**			WEBSERV_HARDCODED_BODY_OFFSET_3XX].msg << std::endl;
*/

//defines that allow us to get to the correct offset in the const array of
//t_builtin_msg structure
# define WEBSERV_HARDCODED_BODY_OFFSET_200 200
# define WEBSERV_HARDCODED_BODY_OFFSET_3XX 299
# define WEBSERV_HARDCODED_BODY_OFFSET_4XX_LOW 390
# define WEBSERV_HARDCODED_BODY_OFFSET_4XX_HIGH 454
# define WEBSERV_HARDCODED_BODY_OFFSET_5XX 454

# include <string.h>
# define CRLF "\r\n"
# define webserv_set_t_hardcoded_str(str) {str, sizeof(str) - 1}
# define webserv_set_dummy_t_hardcoded_str() { NULL, 0 }

const char webserv_http_200_page[] =
"<html>" CRLF
"<head><title>42 WEBSERVER</title></head>" CRLF
"<body>" CRLF
"<center><h1>Welcome to our homemade webserver</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

const char webserv_http_error_301_page[] = 
"<html>" CRLF
"<head><title>301 Moved Permanently</title></head>" CRLF
"<body>" CRLF
"<center><h1>301 Moved Permanently</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_302_page[] =
"<html>" CRLF
"<head><title>302 Moved Temporarily</title></head>" CRLF
"<body>" CRLF
"<center><h1>302 Moved Temporarily</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_303_page[] =
"<html>" CRLF
"<head><title>303 See Other</title></head>" CRLF
"<body>" CRLF
"<center><h1>303 See Other</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_307_page[] =
"<html>" CRLF
"<head><title>307 Temporary Redirect</title></head>" CRLF
"<body>" CRLF
"<center><h1>307 Temporary Redirect</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_308_page[] =
"<html>" CRLF
"<head><title>308 Permanent Redirect</title></head>" CRLF
"<body>" CRLF
"<center><h1>308 Permanent Redirect</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_400_page[] =
"<html>" CRLF
"<head><title>400 Bad Request</title></head>" CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_401_page[] =
"<html>" CRLF
"<head><title>401 Authorization Required</title></head>" CRLF
"<body>" CRLF
"<center><h1>401 Authorization Required</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_402_page[] =
"<html>" CRLF
"<head><title>402 Payment Required</title></head>" CRLF
"<body>" CRLF
"<center><h1>402 Payment Required</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_403_page[] =
"<html>" CRLF
"<head><title>403 Forbidden</title></head>" CRLF
"<body>" CRLF
"<center><h1>403 Forbidden</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_404_page[] =
"<html>" CRLF
"<head><title>404 Not Found</title></head>" CRLF
"<body>" CRLF
"<center><h1>404 Not Found</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_405_page[] =
"<html>" CRLF
"<head><title>405 Not Allowed</title></head>" CRLF
"<body>" CRLF
"<center><h1>405 Not Allowed</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_406_page[] =
"<html>" CRLF
"<head><title>406 Not Acceptable</title></head>" CRLF
"<body>" CRLF
"<center><h1>406 Not Acceptable</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_408_page[] =
"<html>" CRLF
"<head><title>408 Request Time-out</title></head>" CRLF
"<body>" CRLF
"<center><h1>408 Request Time-out</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_409_page[] =
"<html>" CRLF
"<head><title>409 Conflict</title></head>" CRLF
"<body>" CRLF
"<center><h1>409 Conflict</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_410_page[] =
"<html>" CRLF
"<head><title>410 Gone</title></head>" CRLF
"<body>" CRLF
"<center><h1>410 Gone</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_411_page[] =
"<html>" CRLF
"<head><title>411 Length Required</title></head>" CRLF
"<body>" CRLF
"<center><h1>411 Length Required</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_412_page[] =
"<html>" CRLF
"<head><title>412 Precondition Failed</title></head>" CRLF
"<body>" CRLF
"<center><h1>412 Precondition Failed</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_413_page[] =
"<html>" CRLF
"<head><title>413 Request Entity Too Large</title></head>" CRLF
"<body>" CRLF
"<center><h1>413 Request Entity Too Large</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_414_page[] =
"<html>" CRLF
"<head><title>414 Request-URI Too Large</title></head>" CRLF
"<body>" CRLF
"<center><h1>414 Request-URI Too Large</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_415_page[] =
"<html>" CRLF
"<head><title>415 Unsupported Media Type</title></head>" CRLF
"<body>" CRLF
"<center><h1>415 Unsupported Media Type</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_416_page[] =
"<html>" CRLF
"<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
"<body>" CRLF
"<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_421_page[] =
"<html>" CRLF
"<head><title>421 Misdirected Request</title></head>" CRLF
"<body>" CRLF
"<center><h1>421 Misdirected Request</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_429_page[] =
"<html>" CRLF
"<head><title>429 Too Many Requests</title></head>" CRLF
"<body>" CRLF
"<center><h1>429 Too Many Requests</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_494_page[] =
"<html>" CRLF
"<head><title>400 Request Header Or Cookie Too Large</title></head>"
CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>Request Header Or Cookie Too Large</center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_495_page[] =
"<html>" CRLF
"<head><title>400 The SSL certificate error</title></head>"
CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>The SSL certificate error</center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_496_page[] =
"<html>" CRLF
"<head><title>400 No required SSL certificate was sent</title></head>"
CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>No required SSL certificate was sent</center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_497_page[] =
"<html>" CRLF
"<head><title>400 The plain HTTP request was sent to HTTPS port</title></head>"
CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>The plain HTTP request was sent to HTTPS port</center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_500_page[] =
"<html>" CRLF
"<head><title>500 Internal Server Error</title></head>" CRLF
"<body>" CRLF
"<center><h1>500 Internal Server Error</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_501_page[] =
"<html>" CRLF
"<head><title>501 Not Implemented</title></head>" CRLF
"<body>" CRLF
"<center><h1>501 Not Implemented</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_502_page[] =
"<html>" CRLF
"<head><title>502 Bad Gateway</title></head>" CRLF
"<body>" CRLF
"<center><h1>502 Bad Gateway</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_503_page[] =
"<html>" CRLF
"<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
"<body>" CRLF
"<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_504_page[] =
"<html>" CRLF
"<head><title>504 Gateway Time-out</title></head>" CRLF
"<body>" CRLF
"<center><h1>504 Gateway Time-out</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_505_page[] =
"<html>" CRLF
"<head><title>505 HTTP Version Not Supported</title></head>" CRLF
"<body>" CRLF
"<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

static const char webserv_http_error_507_page[] =
"<html>" CRLF
"<head><title>507 Insufficient Storage</title></head>" CRLF
"<body>" CRLF
"<center><h1>507 Insufficient Storage</h1></center>" CRLF
"<hr><center>42 webserv</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;

#endif

//# include <iostream>
//int	main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
//{
//	std::cout <<webserv_answer_builtin_bodies[200 - WEBSERV_HARDCODED_BODY_OFFSET_200].msg << std::endl;
//	std::cout <<webserv_answer_builtin_bodies[301 - WEBSERV_HARDCODED_BODY_OFFSET_3XX].msg << std::endl;
//	std::cout <<webserv_answer_builtin_bodies[302 - WEBSERV_HARDCODED_BODY_OFFSET_3XX].msg << std::endl;
//	std::cout <<webserv_answer_builtin_bodies[303 - WEBSERV_HARDCODED_BODY_OFFSET_3XX].msg << std::endl;
//	std::cout <<webserv_answer_builtin_bodies[307 - WEBSERV_HARDCODED_BODY_OFFSET_3XX].msg << std::endl;
//	std::cout <<webserv_answer_builtin_bodies[500 - WEBSERV_HARDCODED_BODY_OFFSET_5XX].msg<< std::endl;
//	std::cout <<webserv_answer_builtin_bodies[404 - WEBSERV_HARDCODED_BODY_OFFSET_4XX_FIRST_PART].msg<< std::endl;
//	std::cout <<webserv_answer_builtin_bodies[495 - WEBSERV_HARDCODED_BODY_OFFSET_4XX_LAST_PART].msg<< std::endl;
//	return (0);
//}

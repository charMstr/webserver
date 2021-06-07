/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Services_hardcoded_status_line.hpp                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 07:09:09 by charmstr          #+#    #+#             */
/*   Updated: 2021/06/07 19:49:16 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVICES_HARDCODED_STATUS_LINE_HPP
# define SERVICES_HARDCODED_STATUS_LINE_HPP

/*
** This file holds the builtin messages that we are going to use when answering
** to client, the very first list of the header. The right message is set along
** with its length from the value of the status_line member variable.
**
** All those messages are stored into a static array of structures. The array
** is a static member variable of the class Service. Declaring all the static
** strings here is for the sake of clarity.
**
** usage: std::cout << webserv_answer_builtin_status_line[307 - \
**			WEBSERV_HARDCODED_OFFSET_3XX].msg << std::endl;
*/

/*
** notes: the defines will just be substracted to the status_code, that will
** give us the index in the static array.
*/
# define WEBSERV_HARDCODED_STATUS_OFFSET_100 100
# define WEBSERV_HARDCODED_STATUS_OFFSET_200 196
# define WEBSERV_HARDCODED_STATUS_OFFSET_300 269
# define WEBSERV_HARDCODED_STATUS_OFFSET_400 360
# define WEBSERV_HARDCODED_STATUS_OFFSET_500 408


# include <string.h>
# define HTTP_VERSION "HTTP/1.1"
# define CRLF		"\r\n"
# define webserv_set_t_hardcoded_str(str) {str, sizeof(str) - 1}
# define webserv_set_dummy_t_hardcoded_str() { NULL, 0 }


/*####### 1xx - Informational #######*/
const char webserv_http_100_status[] = HTTP_VERSION " 100 " \
	"Continue" CRLF;

const char webserv_http_101_status_line[] = HTTP_VERSION " 101 " \
	"Switching Protocols" CRLF;

const char webserv_http_102_status_line[] = HTTP_VERSION " 102 " \
	"Processing" CRLF;

const char webserv_http_103_status_line[] = HTTP_VERSION " 103 " \
	"Early Hints" CRLF;


/*####### 2xx - Successful #######*/
const char	webserv_http_200_status_line[] = HTTP_VERSION " 200 " \
	"OK" CRLF;

const char	webserv_http_201_status_line[] = HTTP_VERSION " 201 " \
	"Created" CRLF;

const char	webserv_http_202_status_line[] = HTTP_VERSION " 202 " \
	"Accepted" CRLF;

const char	webserv_http_203_status_line[] = HTTP_VERSION " 203 " \
	"Non-Authoritative Information" CRLF;

const char	webserv_http_204_status_line[] = HTTP_VERSION " 204 " \
	"No Content" CRLF;

const char	webserv_http_205_status_line[] = HTTP_VERSION " 205 " \
	"Reset Content" CRLF;

const char	webserv_http_206_status_line[] = HTTP_VERSION " 206 " \
	"Partial Content" CRLF;

const char	webserv_http_207_status_line[] = HTTP_VERSION " 207 " \
	"Multi-Status" CRLF;

const char	webserv_http_208_status_line[] = HTTP_VERSION " 208 " \
	"Already Reported" CRLF;

const char	webserv_http_226_status_line[] = HTTP_VERSION " 226 " \
	"IM Used" CRLF;


/*####### 3xx - Redirection #######*/
const char	webserv_http_300_status_line[] = HTTP_VERSION " 300 " \
	"Multiple Choices" CRLF;

const char	webserv_http_301_status_line[] = HTTP_VERSION " 301 " \
	"Moved Permanently" CRLF;

const char	webserv_http_302_status_line[] = HTTP_VERSION " 302 " \
	"Moved Temporarily" CRLF;

const char	webserv_http_303_status_line[] = HTTP_VERSION " 303 " \
	"See Other" CRLF;

const char	webserv_http_304_status_line[] = HTTP_VERSION " 304 " \
	"Not Modified" CRLF;

const char	webserv_http_305_status_line[] = HTTP_VERSION " 305 " \
	"Use Proxy" CRLF;

const char	webserv_http_307_status_line[] = HTTP_VERSION " 307 " \
	"Temporary Redirect" CRLF;

const char	webserv_http_308_status_line[] = HTTP_VERSION " 308 " \
	"Permanent Redirect" CRLF;


/*####### 4xx - Client Error #######*/
const char	webserv_http_400_status_line[] = HTTP_VERSION " 400 " \
	"Bad Request" CRLF;

const char	webserv_http_401_status_line[] = HTTP_VERSION " 401 " \
	"Unauthorized" CRLF;

const char	webserv_http_402_status_line[] = HTTP_VERSION " 402 " \
	"Payment Required" CRLF;

const char	webserv_http_403_status_line[] = HTTP_VERSION " 403 " \
	"Forbidden" CRLF;

const char	webserv_http_404_status_line[] = HTTP_VERSION " 404 " \
	"Not Found" CRLF;

const char	webserv_http_405_status_line[] = HTTP_VERSION " 405 " \
	"Method Not Allowed" CRLF;

const char	webserv_http_406_status_line[] = HTTP_VERSION " 406 " \
	"Not Acceptable" CRLF;

const char	webserv_http_407_status_line[] = HTTP_VERSION " 407 " \
	"Proxy Authentication Required" CRLF;

const char	webserv_http_408_status_line[] = HTTP_VERSION " 408 " \
	"Request Timeout" CRLF;

const char	webserv_http_409_status_line[] = HTTP_VERSION " 409 " \
	"Conflict" CRLF;

const char	webserv_http_410_status_line[] = HTTP_VERSION " 410 " \
	"Gone" CRLF;

const char	webserv_http_411_status_line[] = HTTP_VERSION " 411 " \
	"Length Required" CRLF;

const char	webserv_http_412_status_line[] = HTTP_VERSION " 412 " \
	"Precondition Failed" CRLF;

const char	webserv_http_413_status_line[] = HTTP_VERSION " 413 " \
	"Payload Too Large" CRLF;

const char	webserv_http_414_status_line[] = HTTP_VERSION " 414 " \
	"URI Too Long" CRLF;

const char	webserv_http_415_status_line[] = HTTP_VERSION " 415 " \
	"Unsupported Media Type" CRLF;

const char	webserv_http_416_status_line[] = HTTP_VERSION " 416 " \
	"Range Not Satisfiable" CRLF;

const char	webserv_http_417_status_line[] = HTTP_VERSION " 417 " \
	"Expectation Failed" CRLF;

const char	webserv_http_418_status_line[] = HTTP_VERSION " 418 " \
	"I'm a teapot" CRLF;

const char	webserv_http_422_status_line[] = HTTP_VERSION " 422 " \
	"Unprocessable Entity" CRLF;

const char	webserv_http_423_status_line[] = HTTP_VERSION " 423 " \
	"Locked" CRLF;

const char	webserv_http_424_status_line[] = HTTP_VERSION " 424 " \
	"Failed Dependency" CRLF;

const char	webserv_http_426_status_line[] = HTTP_VERSION " 426 " \
	"Upgrade Required" CRLF;

const char	webserv_http_428_status_line[] = HTTP_VERSION " 428 " \
	"Precondition Required" CRLF;

const char	webserv_http_429_status_line[] = HTTP_VERSION " 429 " \
	"Too Many Requests" CRLF;

const char	webserv_http_431_status_line[] = HTTP_VERSION " 431 " \
	"Request Header Fields Too Large" CRLF;

const char	webserv_http_451_status_line[] = HTTP_VERSION " 451 " \
	"Unavailable For Legal Reasons" CRLF;


/*####### 5xx - Server Error #######*/
const char	webserv_http_500_status_line[] = HTTP_VERSION " 500 " \
	"Internal Server Error" CRLF;

const char	webserv_http_501_status_line[] = HTTP_VERSION " 501 " \
	"Not Implemented" CRLF;

const char	webserv_http_502_status_line[] = HTTP_VERSION " 502 " \
	"Bad Gateway" CRLF;

const char	webserv_http_503_status_line[] = HTTP_VERSION " 503 " \
	"Service Unavailable" CRLF;

const char	webserv_http_504_status_line[] = HTTP_VERSION " 504 " \
	"Gateway Time-out" CRLF;

const char	webserv_http_505_status_line[] = HTTP_VERSION " 505 " \
	"HTTP Version Not Supported" CRLF;

const char	webserv_http_506_status_line[] = HTTP_VERSION " 506 " \
	"Variant Also Negotiates" CRLF;

const char	webserv_http_507_status_line[] = HTTP_VERSION " 507 " \
	"Insufficient Storage" CRLF;

const char	webserv_http_508_status_line[] = HTTP_VERSION " 508 " \
	"Loop Detected" CRLF;

const char	webserv_http_510_status_line[] = HTTP_VERSION " 510 " \
	"Not Extended" CRLF;

const char	webserv_http_511_status_line[] = HTTP_VERSION " 511 " \
	"Network Authentication Required" CRLF;

#endif

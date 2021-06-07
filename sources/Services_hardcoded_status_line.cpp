/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Services_hardcoded_status_line.cpp                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 08:00:56 by charmstr          #+#    #+#             */
/*   Updated: 2021/04/30 06:23:45 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Services_hardcoded_status_line.hpp"
#include "../includes/Service.hpp"

/*
** This file exists so that we can initialise Service's static member variable
** previously declared in the Service.hpp (within the class):
** t_hardcoded_str webserv_answer_builtin_status_lines[];
*/

/*
** notes: the defines will just be substracted to the status_code, that will
** give us the index in the static array.
*/

const t_hardcoded_str
Service::webserv_hardcoded_status_lines[] =
{
	/*####### 1xx - Informational #######*/
//	#define WEBSERV_HARDCODED_STATUS_OFFSET_100 100
	webserv_set_t_hardcoded_str(webserv_http_100_status),
	webserv_set_t_hardcoded_str(webserv_http_101_status_line),
	webserv_set_t_hardcoded_str(webserv_http_102_status_line),
	webserv_set_t_hardcoded_str(webserv_http_103_status_line),

	/*####### 2xx - Successful #######*/
//	#define WEBSERV_HARDCODED_STATUS_OFFSET_200 196
	webserv_set_t_hardcoded_str(webserv_http_200_status_line),
	webserv_set_t_hardcoded_str(webserv_http_201_status_line),
	webserv_set_t_hardcoded_str(webserv_http_202_status_line),
	webserv_set_t_hardcoded_str(webserv_http_203_status_line),
	webserv_set_t_hardcoded_str(webserv_http_204_status_line),
	webserv_set_t_hardcoded_str(webserv_http_205_status_line),
	webserv_set_t_hardcoded_str(webserv_http_206_status_line),
	webserv_set_t_hardcoded_str(webserv_http_207_status_line),
	webserv_set_t_hardcoded_str(webserv_http_208_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 209 //
    webserv_set_dummy_t_hardcoded_str(),  // 210 //
    webserv_set_dummy_t_hardcoded_str(),  // 211 //
    webserv_set_dummy_t_hardcoded_str(),  // 212 //
    webserv_set_dummy_t_hardcoded_str(),  // 213 //
    webserv_set_dummy_t_hardcoded_str(),  // 214 //
    webserv_set_dummy_t_hardcoded_str(),  // 215 //
    webserv_set_dummy_t_hardcoded_str(),  // 216 //
    webserv_set_dummy_t_hardcoded_str(),  // 217 //
    webserv_set_dummy_t_hardcoded_str(),  // 218 //
    webserv_set_dummy_t_hardcoded_str(),  // 219 //
    webserv_set_dummy_t_hardcoded_str(),  // 220 //
    webserv_set_dummy_t_hardcoded_str(),  // 221 //
    webserv_set_dummy_t_hardcoded_str(),  // 222 //
    webserv_set_dummy_t_hardcoded_str(),  // 223 //
    webserv_set_dummy_t_hardcoded_str(),  // 224 //
    webserv_set_dummy_t_hardcoded_str(),  // 225 //
	webserv_set_t_hardcoded_str(webserv_http_226_status_line),

	/*####### 3xx - Redirection #######*/
//	#define WEBSERV_HARDCODED_STATUS_OFFSET_300 269
	webserv_set_t_hardcoded_str(webserv_http_300_status_line),
	webserv_set_t_hardcoded_str(webserv_http_301_status_line),
	webserv_set_t_hardcoded_str(webserv_http_302_status_line),
	webserv_set_t_hardcoded_str(webserv_http_303_status_line),
	webserv_set_t_hardcoded_str(webserv_http_304_status_line),
	webserv_set_t_hardcoded_str(webserv_http_305_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 306 //
	webserv_set_t_hardcoded_str(webserv_http_307_status_line),
	webserv_set_t_hardcoded_str(webserv_http_308_status_line),

	/*####### 4xx - Client Error #######*/
//	#define WEBSERV_HARDCODED_STATUS_OFFSET_400 360
	webserv_set_t_hardcoded_str(webserv_http_400_status_line),
	webserv_set_t_hardcoded_str(webserv_http_401_status_line),
	webserv_set_t_hardcoded_str(webserv_http_402_status_line),
	webserv_set_t_hardcoded_str(webserv_http_403_status_line),
	webserv_set_t_hardcoded_str(webserv_http_404_status_line),
	webserv_set_t_hardcoded_str(webserv_http_405_status_line),
	webserv_set_t_hardcoded_str(webserv_http_406_status_line),
	webserv_set_t_hardcoded_str(webserv_http_407_status_line),
	webserv_set_t_hardcoded_str(webserv_http_408_status_line),
	webserv_set_t_hardcoded_str(webserv_http_409_status_line),
	webserv_set_t_hardcoded_str(webserv_http_410_status_line),
	webserv_set_t_hardcoded_str(webserv_http_411_status_line),
	webserv_set_t_hardcoded_str(webserv_http_412_status_line),
	webserv_set_t_hardcoded_str(webserv_http_413_status_line),
	webserv_set_t_hardcoded_str(webserv_http_414_status_line),
	webserv_set_t_hardcoded_str(webserv_http_415_status_line),
	webserv_set_t_hardcoded_str(webserv_http_416_status_line),
	webserv_set_t_hardcoded_str(webserv_http_417_status_line),
	webserv_set_t_hardcoded_str(webserv_http_418_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 419 //
    webserv_set_dummy_t_hardcoded_str(),  // 420 //
    webserv_set_dummy_t_hardcoded_str(),  // 421 //
	webserv_set_t_hardcoded_str(webserv_http_422_status_line),
	webserv_set_t_hardcoded_str(webserv_http_423_status_line),
	webserv_set_t_hardcoded_str(webserv_http_424_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 425 //
	webserv_set_t_hardcoded_str(webserv_http_426_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 427 //
	webserv_set_t_hardcoded_str(webserv_http_428_status_line),
	webserv_set_t_hardcoded_str(webserv_http_429_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 430 //
	webserv_set_t_hardcoded_str(webserv_http_431_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 432 //
    webserv_set_dummy_t_hardcoded_str(),  // 433 //
    webserv_set_dummy_t_hardcoded_str(),  // 434 //
    webserv_set_dummy_t_hardcoded_str(),  // 435 //
    webserv_set_dummy_t_hardcoded_str(),  // 436 //
    webserv_set_dummy_t_hardcoded_str(),  // 437 //
    webserv_set_dummy_t_hardcoded_str(),  // 438 //
    webserv_set_dummy_t_hardcoded_str(),  // 439 //
    webserv_set_dummy_t_hardcoded_str(),  // 440 //
    webserv_set_dummy_t_hardcoded_str(),  // 441 //
    webserv_set_dummy_t_hardcoded_str(),  // 442 //
    webserv_set_dummy_t_hardcoded_str(),  // 443 //
    webserv_set_dummy_t_hardcoded_str(),  // 444 //
    webserv_set_dummy_t_hardcoded_str(),  // 445 //
    webserv_set_dummy_t_hardcoded_str(),  // 446 //
    webserv_set_dummy_t_hardcoded_str(),  // 447 //
    webserv_set_dummy_t_hardcoded_str(),  // 448 //
    webserv_set_dummy_t_hardcoded_str(),  // 449 //
    webserv_set_dummy_t_hardcoded_str(),  // 450 //
	webserv_set_t_hardcoded_str(webserv_http_451_status_line),

	/*####### 5xx - Server Error #######*/
//	#define WEBSERV_HARDCODED_STATUS_OFFSET_500 408
	webserv_set_t_hardcoded_str(webserv_http_500_status_line),
	webserv_set_t_hardcoded_str(webserv_http_501_status_line),
	webserv_set_t_hardcoded_str(webserv_http_502_status_line),
	webserv_set_t_hardcoded_str(webserv_http_503_status_line),
	webserv_set_t_hardcoded_str(webserv_http_504_status_line),
	webserv_set_t_hardcoded_str(webserv_http_505_status_line),
	webserv_set_t_hardcoded_str(webserv_http_506_status_line),
	webserv_set_t_hardcoded_str(webserv_http_507_status_line),
	webserv_set_t_hardcoded_str(webserv_http_508_status_line),
    webserv_set_dummy_t_hardcoded_str(),  // 509 //
	webserv_set_t_hardcoded_str(webserv_http_510_status_line),
	webserv_set_t_hardcoded_str(webserv_http_511_status_line)
};

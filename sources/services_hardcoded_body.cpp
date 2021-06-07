/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   services_hardcoded_body.cpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 02:16:19 by charmstr          #+#    #+#             */
/*   Updated: 2021/04/30 06:21:58 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Services_hardcoded_body.hpp"
#include "../includes/Service.hpp"

/*
** This file exists so that we can initialise Service's static member variable
** previously declared in the Service.hpp (within the class):
** t_hardcoded_str webserver_answer_bodies[];
*/

/*
** notes: the defines will just be substracted to the status_code, that will
** give us the index in the static array.
*/

const t_hardcoded_str
Service::webserv_hardcoded_bodies[] =
{
//	#define WEBSERV_HARDCODED_BODY_OFFSET_200 200
	webserv_set_t_hardcoded_str(webserv_http_200_page),

//	#define WEBSERV_HARDCODED_BODY_OFFSET_3XX 299
	webserv_set_dummy_t_hardcoded_str(),        // 300 //
	webserv_set_t_hardcoded_str(webserv_http_error_301_page),
    webserv_set_t_hardcoded_str(webserv_http_error_302_page),
    webserv_set_t_hardcoded_str(webserv_http_error_303_page),
    webserv_set_dummy_t_hardcoded_str(),        // 304 //
    webserv_set_dummy_t_hardcoded_str(),        // 305 //
    webserv_set_dummy_t_hardcoded_str(),        // 306 //
    webserv_set_t_hardcoded_str(webserv_http_error_307_page),
    webserv_set_t_hardcoded_str(webserv_http_error_308_page),

//	#define WEBSERV_HARDCODED_BODY_OFFSET_4XX_LOW 390
    webserv_set_t_hardcoded_str(webserv_http_error_400_page),
    webserv_set_t_hardcoded_str(webserv_http_error_401_page),
    webserv_set_t_hardcoded_str(webserv_http_error_402_page),
    webserv_set_t_hardcoded_str(webserv_http_error_403_page),
    webserv_set_t_hardcoded_str(webserv_http_error_404_page),
    webserv_set_t_hardcoded_str(webserv_http_error_405_page),
    webserv_set_t_hardcoded_str(webserv_http_error_406_page),
    webserv_set_dummy_t_hardcoded_str(),        // 407 //
    webserv_set_t_hardcoded_str(webserv_http_error_408_page),
    webserv_set_t_hardcoded_str(webserv_http_error_409_page),
    webserv_set_t_hardcoded_str(webserv_http_error_410_page),
    webserv_set_t_hardcoded_str(webserv_http_error_411_page),
    webserv_set_t_hardcoded_str(webserv_http_error_412_page),
    webserv_set_t_hardcoded_str(webserv_http_error_413_page),
    webserv_set_t_hardcoded_str(webserv_http_error_414_page),
    webserv_set_t_hardcoded_str(webserv_http_error_415_page),
    webserv_set_t_hardcoded_str(webserv_http_error_416_page),
    webserv_set_dummy_t_hardcoded_str(),        // 417 //
    webserv_set_dummy_t_hardcoded_str(),        // 418 //
    webserv_set_dummy_t_hardcoded_str(),        // 419 //
    webserv_set_dummy_t_hardcoded_str(),        // 420 //
    webserv_set_t_hardcoded_str(webserv_http_error_421_page),
    webserv_set_dummy_t_hardcoded_str(),        // 422 //
    webserv_set_dummy_t_hardcoded_str(),        // 423 //
    webserv_set_dummy_t_hardcoded_str(),        // 424 //
    webserv_set_dummy_t_hardcoded_str(),        // 425 //
    webserv_set_dummy_t_hardcoded_str(),        // 426 //
    webserv_set_dummy_t_hardcoded_str(),        // 427 //
    webserv_set_dummy_t_hardcoded_str(),        // 428 //
    webserv_set_t_hardcoded_str(webserv_http_error_429_page),

//	#define WEBSERV_HARDCODED_BODY_OFFSET_4XX_HIGH 454
    webserv_set_t_hardcoded_str(webserv_http_error_494_page), // 494, request header too large //
    webserv_set_t_hardcoded_str(webserv_http_error_495_page), // 495, https certificate error //
    webserv_set_t_hardcoded_str(webserv_http_error_496_page), // 496, https no certificate //
    webserv_set_t_hardcoded_str(webserv_http_error_497_page), // 497, http to https //
    webserv_set_t_hardcoded_str(webserv_http_error_404_page), // 498, canceled //
    webserv_set_dummy_t_hardcoded_str(),                     // 499, client has closed connection //

//	#define WEBSERV_HARDCODED_BODY_OFFSET_5XX 454
    webserv_set_t_hardcoded_str(webserv_http_error_500_page),
    webserv_set_t_hardcoded_str(webserv_http_error_501_page),
    webserv_set_t_hardcoded_str(webserv_http_error_502_page),
    webserv_set_t_hardcoded_str(webserv_http_error_503_page),
    webserv_set_t_hardcoded_str(webserv_http_error_504_page),
    webserv_set_t_hardcoded_str(webserv_http_error_505_page),
    webserv_set_dummy_t_hardcoded_str(),        // 506 //
    webserv_set_t_hardcoded_str(webserv_http_error_507_page),
};

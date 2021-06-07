/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug_and_traces_macros.hpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/22 00:33:03 by charmstr          #+#    #+#             */
/*   Updated: 2021/04/25 09:43:01 by Remercill        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEBUG_MACROS_HPP
# define DEBUG_MACROS_HPP

/*
** 	INTRODUCTION:
**
** 	Those preprocessor macros will allow us to keep the "debuging" or "trace"
**	functions in the code source.
**
**	MANUAL: 
**
** 	1) Start a line that needs to always be printed out on screen when
** 	debugging with "DEBUG ".
** 	exemple: DEBUG call_to_debuging_function();
** 	Note: This will be printed if DEBUG is defined and other than zero.
**
**	2) When a line needs to be printed out conditionnaly on screen. DEBUG
**	needs to be defined and other than zero, and another defined is used for
**	condition: TRACE_TARGET.
**	Start a line with "DEBUG_(number) ".
**	exemple: DEBUG_(number) call_to_debuging_function();
**
**	Note: For a message to be displayed, the number parameter should be exactly
**	equal to TRACE_TARGET. use the defined values in this file for the value of
**	number parameter.
**
**	Note: TRACE_TARGET will be defined to 0 if undefined, meaning all the 
**	"DEBUG_(x) func();" statements will be called.
**
**	IMPLEMENTATION: 
**
**	The use of for(;0;) instead of using a simple if () statement makes sure we
**	dont break some existing if or else hierarchy in the code.
*/

#define DEBUG_SERVER_CLASS 1
#define DEBUG_REQUEST_PARSER 2
#define DEBUG_REQUEST_READER 3
#define DEBUG_ERROR_SERVICE 4
#define DEBUG_PUT_SERVICE 5
#define DEBUG_GET_SERVICE 6
#define DEBUG_HEAD_SERVICE 7
#define DEBUG_DELETE_SERVICE 8
#define DEBUG_POST_SERVIVE 9
#define DEBUG_CGI_SERVICE 10
#define DEBUG_SERVICE_GENERATOR 11
#define DEBUG_BAD_ALLOC 12


//if TRACE_TARGET was not defined as a compiltion arg with -D, it is set to 0.
#	ifndef TRACE_TARGET
#		define TRACE_TARGET 0
#	endif 

#	ifndef DEBUG 
#		define DEBUG_(x) for(;0;) //line starting with DEBUG_(trace_level) won't be executed.
#		define DEBUG for(;0;)	//line starting with DEBUG won't be executed.
#	elif DEBUG == 0
#		define DEBUG_(x) for(;0;) //line starting with DEBUG_(trace_level) won't be executed.
#		undef DEBUG
#		define DEBUG for(;0;)	//line starting with DEBUG won't be executed.
#	else //DEBUG MODE IS ACTIVATED
#		undef DEBUG
#		define DEBUG  //lines starting with DEBUG will be executed.
#		if TRACE_TARGET == 0 //default mode if TRACE_TARGET was not defined
#			define DEBUG_(x) //all lines will be executed.
#		else //the parameter need to be exactly equal to TRACE_TARGET given as
			//compile parameter. This allow to target only a section of program
#			define DEBUG_(x) for (int debug = 0; debug < 1 && (TRACE_TARGET == x); debug++)
#		endif
#	endif

#endif

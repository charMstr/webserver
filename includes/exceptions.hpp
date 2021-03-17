/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/16 21:19:21 by charmstr          #+#    #+#             */
/*   Updated: 2021/03/17 03:18:09 by charmstr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

# include <exception>
# include <iostream>

/*
** this enum is used to potentially specify the context at which an exception
** is thrown.
*/
enum 		context_enum
{
	NON_GIVEN,
	PARSING = 1,
	INITIALISING,
	WORKING,
	SHUTTING_DOWN
};

/*
** Example of how to use this exception class:
** try
** {
** 	if (something happens)
**	  // only "what" is specified.
**	   --> throw exception_webserver(stderr(errno));
**	  	 or 
**	  // "which" (function) is specified, "when" is not specified
**	   --> throw exception_webserver(stderr(errno), "bind()");
**	     or
**	  // everything is specified including WORKING stage.
**	   --> throw exception_webserver(strerr(errno), "accept()", WORKING);
** }
** catch (std::exception &e)
** {
**	std::cout << e.context() << e.what() << e.which << e.when << std::endl;
** }
**
** NOTE: This exception class doesnt use a std::string as member variable.
** If yes, we could have implemented some method like append(), but we would
** have had to use the "noexcept" key word which is C++11.
*/

class exception_webserver : public std::exception
{
	private:
	const char	*what_msg;
	const char	*which_function;
	int			when_id;

	public:
	exception_webserver(const char *what_msg) :
		what_msg(what_msg),
		which_function(""),
		when_id(NON_GIVEN)
	{ }

	exception_webserver(const char *what_msg, const char *which_func) :
		what_msg(what_msg),
		which_function(which_func),
		when_id(NON_GIVEN)
	{ }

	exception_webserver(const char *what_msg, const context_enum  when_id) :
		what_msg(what_msg),
		which_function(""),
		when_id(when_id)
	{ }

	exception_webserver(const char *what_msg, const char *which_func, \
			const context_enum  when_id) :
		what_msg(what_msg),
		which_function(which_func),
		when_id(when_id)
	{ }


	//always displaying the same message
	const char *context() const
	{
		return ("Webserver exception happened: ");
	}

	//displays the name of the function that was called.
	const char *which() const
	{
		return (which_function);
	}

	/*
	** either displaying an empty string if "when_id" was not specified in the
	** constructor, or explaining at which stage we are in our program: either
	** PARSING, LEXING, INITIALISING, WORKING or SHUTING_DOWN
	*/
	const char *when() const
	{
		if (when_id == PARSING)
			return (" [PARSING stage]");	
		else if (when_id == INITIALISING)
			return (" [INITIALISING stage]");	
		else if (when_id == WORKING)
			return (" [WORKING stage]");	
		else if (when_id == SHUTTING_DOWN)
			return (" [SHUTTING_DOWN stage]");	
		else 
			return ("");
	}

	virtual const char* what() const throw() {
		return (what_msg);
	}	
};

#endif

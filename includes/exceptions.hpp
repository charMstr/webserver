/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: charmstr <charmstr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/16 21:19:21 by charmstr          #+#    #+#             */
/*   Updated: 2021/05/12 17:01:34 by lspiess          ###   ########.fr       */
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
enum 		which_prog_stage
{
	NON_GIVEN,
	PARSING = 1,
	INITIALISING,
	WORKING
};

/*
** This Class will be used as a base class for more specific exception. Its
** main purpose is to provide the print_merialise() public method, which is
** useful for debuging informations.
**
** Example of how to use/instanciate an object of this class:
** try
** {
** 	if (something happens)
**	 	1) only basic parameters are provided:
** 		- "what"
** 		- function name in which we throw the exception
** 		- __FILE__ macro. the name of the file in which we throw
** 		- __LINE__ macro
**		throw exception_webserver(strerror(errno), "main()", __FILE__, __LINE__);
**		2) the basic parameters plus one optional parameter:
**		- the reason why the exception is thrown.
**		throw exception_webserver(strerror(errno), "main()", __FILE__, __LINE__, "accept()");
**		3) the basic parameters plus one optional parameter:
**		- the stage in the program we are at.
**	throw exception_webserver(strerror(errno), "main()", __FILE__, __LINE__, WORKING);
**		4) the basic parameters plus the two opional parameters.
**		throw exception_webserver(stderror(errno), "main()", __FILE__, __LINE__, "bind()", INITIALISING);
** }
** catch (exception_webserver const &e)
** {
** 		if (DEBUG)
** 			e.print_serialied();
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
	const char	*caught_in_func_named;
	const char *file_name;
	int			line;
	const char	*name_triggering_reason;
	int			at_which_stage_id;
	bool 		known_triggering_func;

	public:
	//CONSTRUCTORS
	exception_webserver(const char *what_msg, const char *caught_in_func, \
		const char *__file__, int __line__) :
		what_msg(what_msg),
		caught_in_func_named(caught_in_func),
		file_name(__file__),
		line(__line__),
		name_triggering_reason(""),
		at_which_stage_id(NON_GIVEN),
		known_triggering_func(false)
	{ }

	
	exception_webserver(const char *what_msg, const char *caught_in_func, \
		const char *__file__, int __line__, \
			const char *name_triggering_reason) :
		what_msg(what_msg),
		caught_in_func_named(caught_in_func),
		file_name(__file__),
		line(__line__),
		name_triggering_reason(name_triggering_reason),
		at_which_stage_id(NON_GIVEN),
		known_triggering_func(true)
	{ }

	exception_webserver(const char *what_msg, const char *caught_in_func, \
		const char *__file__, int __line__, \
		const char *name_triggering_reason, const which_prog_stage enum_id) :
		what_msg(what_msg),
		caught_in_func_named(caught_in_func),
		file_name(__file__),
		line(__line__),
		name_triggering_reason(name_triggering_reason),
		at_which_stage_id(enum_id),
		known_triggering_func(true)
	{ }

	exception_webserver(const char *what_msg, const char *caught_in_func, \
		const char *__file__, int __line__, \
		const which_prog_stage enum_id) :
		what_msg(what_msg),
		caught_in_func_named(caught_in_func),
		file_name(__file__),
		line(__line__),
		name_triggering_reason(""),
		at_which_stage_id(enum_id),
		known_triggering_func(false)
	{ }

	/*
	** Just call this function to serialise the debug informations on screen
	** exemple of output:
	** "Webserver exception happened: Undefined error: 0 || caught in func:
	** main() [WORKING stage]"
	*/
	void print_serialised() const
	{
		std::cout << "[\033[31m" << introduce_exception() << " caught\033[m] ";
		std::cout << what();
		std::cout << " || caught in function: " << caught_in_func();
		if (known_triggering_func)
			std::cout << " || triggered by: " << triggering_reason();	
		std::cout << " || file: " << file_name << ", line: "<< line ;
		std::cout << at_which_stage();
		std::cout << std::endl;
	}

	// inherited function.
	virtual const char* what() const throw() {
		return (what_msg);
	}	

	protected:
	// always displaying the same message. can be overiden by child class
	virtual const char *introduce_exception() const
	{
		return ("Webserver exception");
	}

	private:
	// displays the name of the function in which we caught the signal
	const char *caught_in_func() const
	{
		return (caught_in_func_named);
	}

	// displays the name of the function that provoked the throw, if known
	// (optional in constructor).
	const char *triggering_reason() const
	{
		return (name_triggering_reason);
	}

	/*
	** Either displaying an empty string if "when_id" was not specified in the
	** constructor, or explaining at which stage we are in our program: either
	** PARSING, LEXING, INITIALISING, WORKING or SHUTING_DOWN
	** (optional in constructor).
	*/
	const char *at_which_stage() const
	{
		if (at_which_stage_id == PARSING)
			return (" [program stage: PARSING]");	
		else if (at_which_stage_id == INITIALISING)
			return (" [program stage: INITIALISING]");	
		else if (at_which_stage_id == WORKING)
			return (" [program stage: WORKING]");
		else 
			return (" [program stage: UNDEFINED]");
	}
};

#endif

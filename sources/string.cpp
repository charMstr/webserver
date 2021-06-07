/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/30 13:16:42 by bccyv             #+#    #+#             */
/*   Updated: 2021/03/30 13:16:51 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <algorithm>

/*
**	Returns true if str only contains non printable characters
*/
bool is_empty(std::string &s)
{
	return std::find_if(s.begin(), s.end(), isprint) == s.end();
}

/*
**	Trims characters of s matching f from start
*/
void ltrim(std::string &s, int (*f)(int))
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(f))));
}

/*
**	Trims characters of s matching f from end
*/
void rtrim(std::string &s, int (*f)(int))
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(f))).base(), s.end());
}

/*
**	Trims characters of s matching f from both ends
*/
void trim(std::string &s, int (*f)(int))
{
	rtrim(s, f);
    ltrim(s, f);
}

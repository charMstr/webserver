/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bccyv <bccyv@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/30 13:17:06 by bccyv             #+#    #+#             */
/*   Updated: 2021/03/30 13:17:16 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRING_HPP
#define STRING_HPP

bool is_empty(std::string &s);
void ltrim(std::string &s, int (*f)(int));
void rtrim(std::string &s, int (*f)(int));
void trim(std::string &s, int (*f)(int));

#endif

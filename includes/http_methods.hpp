/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_methods.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/26 11:21:36 by lfalkau           #+#    #+#             */
/*   Updated: 2021/01/26 14:53:47 by lfalkau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METHODS_HPP
#define METHODS_HPP

/*
//	This header provides a struct s_methods aliased to t_methods, which only
//	contains a bitfield, and that can be used to determine which http methods
//	are allowed.
//	__set_methods function sets options bits into the t_methods bitfield.
//	__isset_methods checks whether or not options bits are set in the t_method
//	bitfield and returns true if they does.
//
*/

#define OPTIONS 0b00000001
#define GET 0b00000010
#define HEAD 0b00000100
#define POST 0b00001000
#define PUT 0b00010000
#define DELETE 0b00100000
#define TRACE 0b01000000
#define CONNECT 0b10000000

typedef struct s_methods t_methods;

struct s_methods
{
	unsigned int bf : 8;
};

static inline void __methods_set(t_methods *m, unsigned int options)
{
	m->bf |= options;
}

static inline bool __methods_isset(t_methods *m, unsigned int options)
{
	return ((m->bf & options) == options);
}

static inline void __methods_clr(t_methods *m)
{
	m->bf = 0;
}

#define METHODS_SET(s, m) __methods_set((s), (m))
#define METHODS_ISSET(s, m) __methods_isset((s), (m))
#define METHODS_CLR(s) __methods_clr((s))

#endif // METHODS_HPP

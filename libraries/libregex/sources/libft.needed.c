/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.needed.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfalkau <lfalkau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/13 09:15:25 by lfalkau           #+#    #+#             */
/*   Updated: 2021/02/19 20:34:59 by bccyv            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <libft.h>
#include <stdlib.h>

/*
**	As we built the libregex from nothing, we decided to reimplement several
**	functions from the libc.
**	They should behave the same, so see the man of each related function
**	to get more informations about them.
*/

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char		*d;
	const unsigned char	*s;

	if (!dst && !src)
		return (NULL);
	d = (unsigned char *)dst;
	s = (const unsigned char *)src;
	while (n--)
		*d++ = *s++;
	return (dst);
}

int	ft_memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char	*s_1;
	const unsigned char	*s_2;

	s_1 = (const unsigned char *)s1;
	s_2 = (const unsigned char *)s2;
	while (n--)
	{
		if (*s_1 != *s_2)
			return (*s_1 - *s_2);
		s_1++;
		s_2++;
	}
	return (0);
}

void	*ft_memset(void *b, int c, size_t len)
{
	unsigned char	*tmp;
	unsigned char	d;

	tmp = (unsigned char *)b;
	d = (unsigned char)c;
	while (len--)
		*tmp++ = d;
	return (b);
}

char	*ft_strdup(const char *s1)
{
	size_t	len;
	char	*s2;

	len = 0;
	while (s1[len])
		len++;
	s2 = malloc(sizeof(char) * (len + 1));
	if (!s2)
		return (NULL);
	ft_memcpy(s2, s1, len + 1);
	return (s2);
}

int	ft_isinset(const char *charset, char c)
{
	while (*charset)
	{
		if (c == *charset)
			return (1);
		charset++;
	}
	return (0);
}

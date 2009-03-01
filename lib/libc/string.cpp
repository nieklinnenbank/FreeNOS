/*
 * Copyright (C) 2009 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "string.h"

int strcmp( char *dest, char *src )
{
    while ( *dest && *src && *dest == *src )
    {
	dest++;
	src++;
    }
    return (*dest - *src);
}

int strncmp( char *dest, char *src, Size count )
{
    Size num = 0;

    while ( num++ < count && *dest && *src && *dest == *src )
    {
	dest++;
	src++;
    }
    return (*dest - *src);
}

void * memset(void *dest, int ch, Size count)
{
    char *temp;

    for(temp = (char *) dest; count != 0; count--)
    {
	*temp++ = ch;
    }
    return (dest);
}

void * memcpy(void *dest, const void *src, Size count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    
    for(; count != 0; count--)
	*dp++ = *sp++;

    return (dest);
}

void * memmove(void *dest, const void *src, Size count)
{
    // TODO
    return (void *)(0);
}

int memcmp(const void *s1, const void *s2, Size count)
{
    // TODO
    return 0;
}

int strlen(char *str)
{
    char *s;

    for (s = str; *s; ++s);
    return (s - str);
}

int strcpy(char *dest, char *src)
{
    char *d = dest;
    char *s = src;
    int count = 0;

    while (*s)
    {
	*(d++) = *(s++);
	count++;
    }
    *(d++) = '\0';
    return (count);
}

int strncpy(char *dest, char *src, Size sz)
{
    char *d, *s;
    int val = 0;
    
    d = dest;
    s = src;

    while ( *s != '\0' && sz != 0 )
    {
	*d++ = *s++;
        sz--;
	val++;
    }
    *d++ = '\0';
    return val;
}

char * strcat(char *dest, char *src)
{
    char *s = dest;

    while (*s++)
	;

    strcpy(s, src);
    return dest;
}

Size strlcpy(char *dst, const char *src, Size siz)
{
	char *d = dst;
	const char *s = src;
	Size n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

/**
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

void * memset(void *dest, int ch, u32 count)
{
    char *temp;

    for(temp = (char *) dest; count != 0; count--)
    {
	*temp++ = ch;
    }
    return (dest);
}

void * memcpy(void *dest, const void *src, u32 count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    
    for(; count != 0; count--)
	*dp++ = *sp++;

    return (dest);
}

void * memmove(void *dest, const void *src, u32 count)
{
    // TODO
    return (void *)(0);
}

int memcmp(const void *s1, const void *s2, u32 count)
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
    int count;

    while (*s)
    {
	*(d++) = *(s++);
	count++;
    }
    *(d++) = '\0';
    return (count);
}

int strncpy(char *dest, char *src, u32 sz)
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

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

#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"

int vsnprintf(char *buffer, unsigned int size, const char *fmt, va_list args)
{
    char buf[20], *ptr;
    int ch;
    unsigned int written = 0;
    
    /* Loop formatted message. */
    while ((ch = *fmt++) && written < size)
    {
	if (ch != '%')
	{
	    *buffer++ = ch;
	    written++;
	}
	else
	{
	    switch (*fmt)
	    {
	    case 'd':
	    case 'u':
	    	itoa(buf, 10, va_arg(args, int));
		ptr = buf;
		goto string;
	
	    case 'l':
		itoa(buf, 10, va_arg(args, long));
		ptr = buf;
		goto string;
	
	    case 'x':
		itoa(buf, 16, va_arg(args, int));
		ptr = buf;
		goto string;
	
	    case 'c':
		buf[0] = va_arg(args, int);
		buf[1] = ZERO;
		ptr    = buf;
		goto string;
	
	    case 's':
		ptr = va_arg(args, char *);
	
	    string:
		while(*ptr && written++ < size)
		{
		    *buffer++ = *ptr++;
		}
		break;
	
	    default:
		*buffer++ = ch;
		written++;
		break;
	    }
	    fmt++;
	}
    }
    /* Null terminate. */
    if (written < size)
	*buffer = 0;
    return (written);
}

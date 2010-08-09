/*
 * Copyright (C) 1999  Free Software Foundation, Inc.
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

#include "stdlib.h"

void itoa(char *buffer, int divisor, int number)
{
    char *p = buffer, *p1, *p2, tmp;
    unsigned long ud = number;
    int remainder;
        
    /* Negative decimal. */
    if (divisor == 10 && number < 0)
    {
	*p++ = '-';
	buffer++;
	ud = -number;
    }
    /* Hexadecimal. */
    else if (divisor == 16)
    {
	*p++ = '0';
	*p++ = 'x';
	buffer += 2;
    }
    /* Divide ud by the divisor, until ud == 0. */
    do
    {
	remainder = ud % divisor;
	*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
    while (ud /= divisor);
    
    /* Terminate buffer. */
    *p = 0;
    
    /* Initialize pointers. */
    p1 = buffer;
    p2 = p - 1;
    
    /* Reverse buf. */
    while (p1 < p2)
    {
	tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
	p1++;
	p2--;
    }
}

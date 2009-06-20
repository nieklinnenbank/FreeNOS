/*
 * Copyright (C) 2009 Coen Bijlsma
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

#include <String.h>
#include <URI.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	/* Initialize terminal as stdio */
	for( int i = 0; i < 3; i++)
	{
		while(open("/dev/tty0", ZERO) < 0);
	}
	
        String u1("http://my.utrecht\%2d.nl");
        String u2("http://my.utrecht-.nl");
        
        URI uri1(u1);
        URI uri2(u2);
        
        printf("URI1 has scheme %s\n", *(uri1.getScheme()) );
        printf("URI1 equals URI2: %s\n", (uri1.equals(uri2) ? "true" : "false") );
        printf("URI1 normalized == %s\n", *(uri1.normalize()) );
	return 0;
}

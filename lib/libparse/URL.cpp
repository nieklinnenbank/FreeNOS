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

#include <stdio.h>
#include <string.h>
#include "URL.h"
#include "StringTokenizer.h"

URL::URL(char* uri) : URI(uri)
{
    if( _scheme == 0 )
    {
        // Temporary until exception support is added to FreeNOS
        printf("Empty scheme not allowed.\n");
        exit(1);
    } else {
        StringTokenizer st(KNOWN_SCHEMES, ' ');
        bool known = false;
        
        while( st.hasNext() )
        {
            if( strcasecmp( st.next(), _scheme) == 0 )
            {
                known = true;
                break;
            }
        }
        
        if( !known )
        {
            printf("Unknown scheme: %s.\n", _uri);
            // Temporary until exception support is added to FreeNOS
            exit(1);
        }
    }

}

URL::~URL()
{
}

char** URL::split()
{
    return (char**)0;
}
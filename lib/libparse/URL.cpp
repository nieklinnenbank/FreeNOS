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

URL* URL::create(char* scheme, char* url)
{
    if( scheme == (char*)0 )
    {
        return (URL*)0;
    }
    
    StringTokenizer st(KNOWN_SCHEMES, ' ');
    bool known = false;
    
    while( st.hasNext() )
    {
        if( strcasecmp( st.next(), scheme) == 0 )
        {
            known = true;
            break;
        }
    }
    
    if( !known )
    {
        printf("Unknown scheme: %s.\n", scheme);
        return (URL*)0;
    }
    
    if( strcasecmp("file", scheme) == 0 )
    {
        return new URL(url);
    }
    
    return (URL*)0;
}

URL::URL(char* uri) : URI(uri)
{
}

URL::~URL()
{
}

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

#include <stdlib.h> // EXIT_SUCCESS & EXIT_FAILURE
#include <stdio.h>
#include <string.h>
#include "StringTokenizer.h"
#include "FileURL.h"

FileURL::FileURL(char* url) : URL(url)
{
    if( strcasecmp( "file", _scheme ) != 0 )
    {
        printf("Not a FileURL: %s.\n", _uri);
        exit(EXIT_FAILURE);
    }
    
    _splitted = (Vector<String>*)0;
}

FileURL::~FileURL()
{
    if( _splitted )
    {
        for( Size size = 0; size < _splitted->count(); size++ )
        {
            delete _splitted->get(size);
        }
        
        delete _splitted;
    }
}

Vector<String>* FileURL::split(char sep)
{

    // Are we already splitted?
    if( !_splitted )
    {
        // Example: file:///var/log/apache2/access.log
    
        String hier(_hierarchical);
    
        if( hier.startsWith("//") )
        {
            hier = hier.substring(2);
        }
        
        StringTokenizer st(hier, sep);

        Vector<String>* ret = new Vector<String>();

        while( st.hasNext() )
        {
            String* s  = new String(st.next());
            String* trim = s->trim();
            
            if( trim != (String*)0 && trim->size() > 0)
            {
                ret->insert( trim );
            }
            delete s;
        }

        _splitted = ret;
    }
    
    return _splitted;
}

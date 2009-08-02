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

#include <Assert.h>
#include <stdlib.h>
#include "StringTokenizer.h"
#include "URI.h"

#include <stdio.h>

URI::URI(char* u) : uri(u)
{
    if( uri == (char*)0 )
    {
        /* TODO: Throw exception if support is added to FreeNOS. */
        printf("ERROR: Illegal URI.\n");
        exit(EXIT_FAILURE);
    }
    
    scheme = 0;
    hierarchical = 0;
    query = 0;
    fragment = 0;
    normalized = 0;
    
    StringTokenizer st(uri, ':');
    unsigned int count = st.count();
    
    if( count > 1 )
    {
        scheme = strdup(st.next());
    } else {
        printf("Illegal URI: %s\n", uri);
        exit(EXIT_FAILURE);
    }
    
    StringTokenizer querySt(st.next(), '?');
    
    if( querySt.count() > 0 )
    {
        hierarchical = strdup(querySt.next());
        
        if( querySt.count() > 1 )
        {
    
            StringTokenizer fragmentSt(querySt.next(), '#');
            query = strdup(fragmentSt.next());
            
            if( fragmentSt.count() > 1 )
            {
                fragment = strdup(fragmentSt.next());
            }
        }
    }
    
}

URI::~URI()
{
    if( scheme != 0 )
    {
        free( scheme );
    }
    
    if( hierarchical != 0 )
    {
        free( hierarchical );
    }
    
    if( query != 0 )
    {
        free( query );
    }
    
    if( fragment != 0 )
    {
        free( fragment );
    }
    
    if( normalized != 0 )
    {
        free( normalized );
    }
}

char* URI::getScheme() const
{
    return scheme;
}

char* URI::getHierarchical() const
{
    return hierarchical;
}

char* URI::getQuery() const
{
    return query;
}

char* URI::getFragment() const
{
    return fragment;
}

char* URI::getRawURI() const
{
    return uri;
}

bool URI::equals(URI& uri)
{
    char* own = this->normalize();
    char* other = uri.normalize();
    unsigned int length = strlen(own);
    bool inEncoding = false;
    
    if( strlen(own) != strlen(other) )
    {
        return false;
    } else {
        for( unsigned int i = 0; i < length; i++ )
        {
            if( ! inEncoding )
            {
                if( own[i] != other[i] )
                {
                    return false;
                }
                
                if( own[i] == '%' )
                {
                    inEncoding = true;
                }
            } else {
                if( own[i] != other[i] )
                {
                    /* Compare them in a case insensitive way. */
                    int result = (int)(own[i] - other[i]);
                    if( result != 32 && result != -32 )
                    {
                        return false;
                    }
                }
                
                /* See if we are at the end of the encoded string */
                if( i > 0 && ( own[i - 1] != '%' ) )
                {
                    inEncoding = false;
                }
            }
        }
    }
    
    return true;
}

char* URI::normalize()
{
    Size size = strlen(uri);
    
    if( normalized == (char*)NULL )
    {
        char* original = uri;
        char* copy = (char*)malloc( size + 1);
        memset(copy, 0, size + 1);
        unsigned int copyPointer = 0;
        bool inEncoding = false;
        char* encoded = (char*)malloc(3);
        memset(encoded, 0, 3);
        
        for( Size i = 0; i < size; i++ )
        {
            if( original[i] == '%' )
            {
                inEncoding = true;
                continue;
            }
            
            if( ! inEncoding )
            {
                copy[copyPointer++] = original[i];
            } else {
                if( encoded[0] == 0 )
                {
                    encoded[0] = original[i];
                } else {
                    encoded[1] = original[i];
                    char decoded = decode(encoded);
                    
                    if( strchr(UNRESERVED_CHARS, (int)decoded) != NULL ){
                        copy[copyPointer++] = decoded;
                    } else {
                        copy[copyPointer++] = '%';
                        copy[copyPointer++] = encoded[0];
                        copy[copyPointer++] = encoded[1];
                    }
                    
                    encoded[0] = 0;
                    encoded[1] = 0;
                    inEncoding = false;
                }
            }
        }
        
        free( encoded );
        normalized = copy;
    }
    
    return normalized;
}

char URI::decode(char* encoded)
{
    long int dec = strtol(encoded, (char**)NULL, 16);
    return (char)dec;
}

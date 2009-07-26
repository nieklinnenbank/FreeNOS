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

URI::URI(char* uri) : _uri(uri)
{
    if( _uri == (char*)0 )
    {
        // TODO: Throw exception if support is added to FreeNOS.
        printf("ERROR: Illegal URI.\n");
        exit(1);
    }
    
    _scheme = 0;
    _hierarchical = 0;
    _query = 0;
    _fragment = 0;
    _normalized = 0;
    
    StringTokenizer st(_uri, ':');
    
    if( st.count() > 0 )
    {
        _scheme = strdup(st.next());
    }
    
    StringTokenizer querySt(st.next(), '?');
    
    if( querySt.count() > 0 )
    {
        _hierarchical = strdup(querySt.next());
        
        if( querySt.count() > 1 )
        {
    
            StringTokenizer fragmentSt(querySt.next(), '#');
            _query = strdup(fragmentSt.next());
            
            if( fragmentSt.count() > 1 )
            {
                _fragment = strdup(fragmentSt.next());
            }
        }
    }
    
}

URI::~URI()
{
    if( _scheme != 0 )
    {
        free( _scheme );
        _scheme = 0;
    }
    
    if( _hierarchical != 0 )
    {
        free( _hierarchical );
        _hierarchical = 0;
    }
    
    if( _query != 0 )
    {
        free( _query );
        _query = 0;
    }
    
    if( _fragment != 0 )
    {
        free( _fragment );
        _fragment = 0;
    }
    
    if( _normalized != 0 )
    {
        free( _normalized );
        _normalized = 0;
    }
}

char* URI::getScheme() const
{
    return _scheme;
}

char* URI::getHierarchical() const
{
    return _hierarchical;
}

char* URI::getQuery() const
{
    return _query;
}

char* URI::getFragment() const
{
    return _fragment;
}

char* URI::getRawURI() const
{
    return _uri;
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
                    // Compare them in a case insensitive way
                    int result = (int)(own[i] - other[i]);
                    if( result != 32 && result != -32 )
                    {
                        return false;
                    }
                }
                
                // See if we are at the end of the encoded string
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
    Size size = strlen(_uri);
    
    if( _normalized == (char*)NULL )
    {
        char* original = _uri;
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
                    char decoded = _decode(encoded);
                    
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
        
        free(encoded);
        _normalized = copy;
    }
    
    return _normalized;
}

char URI::_decode(char* encoded)
{
    long int dec = strtol(encoded, (char**)NULL, 16);
    return (char)dec;
}

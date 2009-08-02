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
#include <stdlib.h>
#include <string.h>

#include <StringTokenizer.h>

StringTokenizer::StringTokenizer(String& seq, char delim)
{
    this->init(*seq, delim);
}

StringTokenizer::StringTokenizer(char* seq, char delim)
{
    this->init(seq, delim);
}

StringTokenizer::~StringTokenizer()
{
    free( currentToken );
    free( nextToken );
    free( sequence );
}

bool StringTokenizer::hasNext()
{
    return !( nextToken == 0 );
}

unsigned int StringTokenizer::count()
{
    if( cnt == 0 )
    {
        cnt = 1;
        
        for( unsigned int i = 0; i < strlen( sequence ); i++)
        {
            if( delimiter == sequence[i] )
            {
                cnt++;
            }
        }
        
    }
    
    return cnt;
}

char* StringTokenizer::next()
{
    char* ptr = 0;
    char* ptr2 = 0;
    
    if( nextToken == 0 )
    {
        return (char*) NULL;
    }
    
    if( currentToken != 0 )
    {
        free( currentToken );
    }
    
    /* Copy nextToken into currentToken. */
    currentToken = strdup(nextToken);
    
    /* Shift until next delimiter */
    for( ptr = sequence + currentLocation; 
        ptr[0] != '\0' && ptr[0] != delimiter; ptr++){};
    
    /* (re)fill nextToken */
    free( nextToken );
    if( ptr[0] == '\0' )
    {
        nextToken = 0;
    } else {
        ptr++;
        ptr2 = strchr(ptr, delimiter);
        
        if( ptr2 == 0 )
        {
            /* This is the last token */
            nextToken = strdup(ptr);
        } else {
            nextToken = (char*)malloc(ptr2 - ptr + 1);
            memset(nextToken, 0, ptr2 - ptr + 1);
            strncpy(nextToken, ptr, ptr2 - ptr);
        }
        
        currentLocation += strlen(nextToken) +1;
        }
        
        return currentToken;
}

void StringTokenizer::init(char* seq, char delim)
{
    char* ptr;
    char* value = seq;
    
    assert( value != 0 && strlen(value) > 0);
    sequence = strdup(value);
    
    if( delim == '\0' )
    {
        delim = ' ';
    }
    
    delimiter = delim;
    
    /* Initialize the other variables */
    currentLocation = 0;
    currentToken = 0;
    cnt = 0;
    
    if( (ptr = strchr( sequence, delim )) == 0)
    {
        nextToken = strdup(sequence);
    } else {
        nextToken = (char*)malloc(ptr - sequence + 1);
        memset(nextToken, 0, ptr - sequence + 1);
        strncpy(nextToken, sequence, ptr - sequence);
        currentLocation = ptr - sequence;
    }
    
}

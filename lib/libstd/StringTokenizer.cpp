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

#include "StringTokenizer.h"
#include "MemoryBlock.h"
#include "String.h"

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
    delete currentToken;
    delete nextToken;
    delete sequence;
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
        
        for( unsigned int i = 0; i < String::strlen( sequence ); i++)
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
        delete currentToken;
    }
    
    /* Copy nextToken into currentToken. */
    currentToken = String::strdup(nextToken);
    
    /* Shift until next delimiter */
    for( ptr = sequence + currentLocation; 
        ptr[0] != '\0' && ptr[0] != delimiter; ptr++){};
    
    /* (re)fill nextToken */
    delete nextToken;
    if( ptr[0] == '\0' )
    {
        nextToken = 0;
    } else {
        ptr++;
        ptr2 = String::strchr(ptr, delimiter);
        
        if( ptr2 == 0 )
        {
            /* This is the last token */
            nextToken = String::strdup(ptr);
        } else {
            nextToken = new char[ptr2 - ptr + 1];
            MemoryBlock::set(nextToken, 0, ptr2 - ptr + 1);
            String::strlcpy(nextToken, ptr, (ptr2 - ptr)+1);
        }
        
        currentLocation += String::strlen(nextToken) +1;
    }
        
        return currentToken;
}

void StringTokenizer::init(char* seq, char delim)
{
    char* ptr;
    char* value = seq;
    
    /*
     * If the sequence starts with a (number of)
     * delimiters, ignore them.
     */
    while(value && value[0] == delim)
    {
        value++;
    }
    
    assert( value != 0 && String::strlen(value) > 0);
    sequence = String::strdup(value);
    
    if( delim == '\0' )
    {
        delim = ' ';
    }
    
    delimiter = delim;
    
    /* Initialize the other variables */
    currentLocation = 0;
    currentToken = 0;
    cnt = 0;
    
    if( (ptr = String::strchr( sequence, delim )) == 0)
    {
        nextToken = String::strdup(sequence);
    } else {
        nextToken = new char[ptr - sequence + 1];
        MemoryBlock::set(nextToken, 0, ptr - sequence + 1);
        String::strlcpy(nextToken, sequence, (ptr - sequence)+1);
        currentLocation = ptr - sequence;
    }
    
}

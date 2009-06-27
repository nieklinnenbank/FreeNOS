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
	this->_init(*seq, delim);
}

StringTokenizer::StringTokenizer(char* seq, char delim)
{
	this->_init(seq, delim);
}

StringTokenizer::~StringTokenizer()
{
	free( _currentToken );
	free( _nextToken );
	free( _sequence );
}

bool StringTokenizer::hasNext()
{
	return !( _nextToken == 0 );
}

unsigned int StringTokenizer::count()
{
	if( _count == 0 )
	{
		_count = 1;
		
		for( unsigned int i = 0; i < strlen( _sequence ); i++)
		{
		        if( _delimiter == _sequence[i] )
		        {
		                _count++;
		        }
		}
		
	}
	
	return _count;
}

char* StringTokenizer::next()
{
	char* ptr = 0;
	char* ptr2 = 0;
	
	if( _nextToken == 0 )
	{
		return (char*) NULL;
	}
	
	if( _currentToken != 0 )
	{
		free( _currentToken );
	}
	
	// Copy _nextToken into _currentToken
	_currentToken = strdup(_nextToken);
	
	// Shift until next _delimiter
        for( ptr = _sequence + _currentLocation; 
          ptr[0] != '\0' && ptr[0] != _delimiter; ptr++){};
        
	// (re)fill _nextToken
	free( _nextToken );
	if( ptr[0] == '\0' )
	{
		_nextToken = 0;
	} else {
	        ptr++;
	        ptr2 = strchr(ptr, _delimiter);
	        
	        if( ptr2 == 0 )
	        {
	            // This is the last token
	            _nextToken = strdup(ptr);
	        } else {
	            _nextToken = (char*)malloc(ptr2 - ptr + 1);
	            memset(_nextToken, 0, ptr2 - ptr + 1);
	            strncpy(_nextToken, ptr, ptr2 - ptr);
	        }
	        
	        _currentLocation += strlen(_nextToken) +1;
	}
	
	return _currentToken;
}

void StringTokenizer::_init(char* seq, char delim)
{
	char* ptr;
	char* value = seq;
	
	assert( value != 0 && strlen(value) > 0);
	_sequence = strdup(value);
	
	if( delim == '\0' )
	{
	    delim = ' ';
	}
        
        _delimiter = delim;
	
	// Initialize the other variables
	_currentLocation = 0;
	_currentToken = 0;
	_count = 0;
        
        if( (ptr = strchr( _sequence, delim )) == 0)
        {
            _nextToken = strdup(_sequence);
        } else {
            _nextToken = (char*)malloc(ptr - _sequence + 1);
            memset(_nextToken, 0, ptr - _sequence + 1);
            strncpy(_nextToken, _sequence, ptr - _sequence);
            _currentLocation = ptr - _sequence;
        }
        
}

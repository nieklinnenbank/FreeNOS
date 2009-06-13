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
	char dl[] = {delim, '\0'};
	this->_init(*seq, dl);
}

StringTokenizer::StringTokenizer(String& seq, String& delim)
{
	this->_init(*seq, *delim);
}

StringTokenizer::StringTokenizer(String& seq, char* delim)
{
	this->_init(*seq, delim);
}

StringTokenizer::StringTokenizer(char* seq, char delim)
{
	char dl[] = {delim, '\0'};
	this->_init(seq, dl);
}

StringTokenizer::StringTokenizer(char* seq, char* delim)
{
	this->_init(seq, delim);
}

StringTokenizer::~StringTokenizer()
{
	free( _currentToken );
	free( _nextToken );
	free( _sequence );
	free( _delimeters );
}

bool StringTokenizer::hasNext()
{
	return ( _nextToken != 0 );
}

int StringTokenizer::count()
{
	if( _count == 0 )
	{
		_count = 1;
		
		for( unsigned int i = 0; i < strlen( _sequence ); i++)
		{
			for( unsigned int n = 0; n < strlen( _delimeters ); n++)
			{
				if( _sequence[i] == _delimeters[n])
				{
					_count++;
				}
			}
		}
		
	}
	
	return _count;
}

String StringTokenizer::next()
{
	char* ptr = 0;
	char* ptr2 = 0;
	char* retval = 0;
	
	if( _nextToken == 0 )
	{
		return String();
	}
	
	if( _currentToken != 0 )
	{
		free( _currentToken );
	}
	
	int nextTokenLength = strlen(_nextToken);
	
	// Copy _nextToken into currentToken
	_currentToken = (char*)malloc( nextTokenLength + 1 );
	memset( _currentToken, 0, nextTokenLength + 1 );
	strcpy( _currentToken, _nextToken );
	
	// Shift until next _delimeter
	ptr = _sequence + _currentLocation;
	while( ptr[0] != 0 )
	{
		bool delimeterFound = false;
		for( unsigned int i = 0; i < strlen( _delimeters ); i++ )
		{
			if( ptr[0] == _delimeters[i] )
			{
				delimeterFound = true;
			}
			
		}
		
		if( ! delimeterFound )
		{
			ptr++;
		} else {
			break;
		}
	}
	
	// (re)fill _nextToken
	free( _nextToken );
	if( ptr[0] == '\0' )
	{
		_nextToken = 0;
	} else {
		ptr++;
		
		for( unsigned int i = 0; i < strlen( _delimeters ); i++ )
		{
			ptr2 = strchr( ptr, _delimeters[i] );
			if( ptr2 > 0 )
			{
				// A delimeter is found
				break;
			}
		}
		
		if( ptr2 == 0 )
		{
			// If this is the last token
			int ptrLength = strlen(ptr);
			_nextToken = (char*)malloc(ptrLength + 1 );
			memset( _nextToken, 0, ptrLength + 1 );
			strcpy( _nextToken, ptr );
		} else {
			_nextToken = (char*)malloc(ptr2 - ptr + 1 );
			memset( _nextToken, 0, ptr2 - ptr + 1 );
			strncpy( _nextToken, ptr, ptr2 - ptr );
		}
		
		_currentLocation += strlen( _nextToken ) + 1;
	}
	
	int currentTokenLength = strlen(_currentToken);
	retval = (char*)malloc( currentTokenLength  + 1 );
	memset( retval, 0, currentTokenLength + 1 );
	strcpy( retval, _currentToken );
	
	return String( retval, true );
}

void StringTokenizer::_init(char* seq, char* delim)
{
	char* ptr;
	char* value = seq;
	
	if( value == 0 )
	{
		printf("The _sequence to be checked cannot be NULL.\n"); // TODO: change to  fprintf()
		return;
	} else {
		int length = strlen(value);
		char* tmp = (char*)malloc( length + 1);
		memset(tmp, 0, length + 1);
		
		strcpy(tmp, value);
		_sequence = tmp;
	}
	
	if( delim == 0 || strlen( delim ) == 0 )
	{
	    delim = " ";
	}
	
	// Create a copy of delim and set _delimeters to it.
	_delimeters = (char*)malloc(strlen(delim) + 1);
	memset(_delimeters, 0, strlen(delim) + 1);
	strcpy(_delimeters, delim);
	
	// Initialize the other variables
	_currentLocation = 0;
	_currentToken = 0;
	_count = 0;
	
	for( unsigned int i = 0; i < strlen( _delimeters ); i++ )
	{
		ptr = strchr( _sequence, _delimeters[i] );
		if( ptr > 0 )
		{
		    break;
		}
	}
	
	if( ptr  == 0)
	{
		int length = strlen(_sequence);
		_nextToken = (char*)malloc( length + 1 );
		memset( _nextToken, 0, length + 1 );
		strcpy( _nextToken, _sequence );
	} else {
		_nextToken = (char*)malloc( ptr - _sequence + 1 );
		memset( _nextToken, 0, ptr - _sequence + 1 );
		strncpy( _nextToken, _sequence, ptr - _sequence);
		_currentLocation = ptr - _sequence;
	}
}
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

#ifndef __LIBPARSE_STRINGTOKENIZER_H
#define __LIBPARSE_STRINGTOKENIZER_H

#include <String.h>

/**
 * @brief Class for splitting Strings into tokens
 * @since 2009-06-11
 * @changed $Date: $
 * @version: $Id $
 *
 * This class takes a String and chops it up into pieces.
 */
class StringTokenizer
{
    public:

	/**
	 * Constructor.
	 * Initializes the tokenizer
	 */
	StringTokenizer(String& seq, char delim);
	
	/**
	 * Constructor.
	 * Initializes the tokenizer
	 */
	StringTokenizer(String& seq, String& delim);
	
	/**
	 * Constructor
	 * Initializes the tokenizer
	 */
	StringTokenizer(String&, char* delim);
	
	/**
	 * Constructor.
	 * Initializes the tokenizer
	 */
	StringTokenizer(char* seq, char delim);
	
	/**
	 * Constructor.
	 * Initializes the tokenizer
	 */
	StringTokenizer(char* seq, char* delim);
	
	/**
	 * Destructor
	 */
	virtual ~StringTokenizer();
	
	/**
	 * Returns whether there are any tokens left
	 * after the current position.
	 */
	bool hasNext();
	
	/**
	 * Returns the amount of tokens there are
	 * in the String
	 */
	int count();
	
	/**
	 * Returns the next token
	 */
	String next();

    private:

	char*	_sequence;
	char*	_delimeters;
	int	_currentLocation;
	int	_count;
	char*	_currentToken;
	char*	_nextToken;
	
	void _init(char* seq, char* delim);
};

#endif /* __LIBPARSE_STRINGTOKENIZER_H */

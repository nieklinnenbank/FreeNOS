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

#ifndef __LIBREGEXP_REGEXP_H
#define __LIBREGEXP_REGEXP_H

#include <String.h>
#include <string.h>

/**
 * This class privides regexp functionality.
 * This implementation currently only supports []
 */
class Regexp
{
    public:
    
	/**
	 * Constructor that takes a String that contains the
	 * pattern to match other strings to.
	 */
	Regexp(String& pattern);
	
	/**
	 * Constructor that takes a char* that contains the
	 * pattern to match other strings to.
	 */
	Regexp(const char* pattern);
	
	/**
	 * Matches the given subject to the pattern
	 * and returns whether that succeeded.
	 */
	bool match(String& subject);
	
	/**
	 * Matches the given subject to the pattern
	 * and returns whether that succeeded.
	 */
	bool match(char* subject);
	
	/**
	 * Returns the pattern this Regex matches 
	 * subjects to.
	 */
	const char* getPattern();
	
    private:

	const char* _pattern;
};

#endif /* __LIBREGEXP_REGEXP_H */

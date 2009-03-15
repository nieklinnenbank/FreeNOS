/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __STRING_H
#define __STRING_H

#include <string.h>
#include "Comparable.h"
#include "Types.h"
#include "Assert.h"

/** Default maximum length of a String's value. */
#define STRING_DEFAULT_MAX 128

/**
 * Abstraction of strings.
 */
class String : public Comparable<String>
{
    public:

	/**
	 * Empty constructor.
	 */
	String()
	{
	    value = ZERO;
	}

	/**
	 * Constructor.
	 * @param s Initial value of the String.
	 * @param dv Delete value in the destructor.
	 */
	String(char *s, bool dv = true) : deleteValue(dv)
	{
	    init(s, STRING_DEFAULT_MAX);
	}

	/**
	 * Copy constructor.
	 * @param s String instance pointer.
	 * @param dv Delete value in the destructor.
	 */
	String(String *s, bool dv = true) : deleteValue(dv)
	{
	    init(s->value, s->size());
	}
    
	/**
	 * Constant constructor.
	 * @param s Initial value of the String.
	 * @param dv Delete value in the destructor.
	 */
	String(const char *s, bool dv = false) : deleteValue(dv)
	{
	    init((char *) s, STRING_DEFAULT_MAX);
	}
    
	/**
	 * Constrant constructor including the maximum length.
	 * @param s Initial value of the String.
	 * @param max Maximum length of the String.
	 * @param dv Delete value in the destructor.
	 */
	String(const char *s, Size max, bool dv = false)
	    : deleteValue(dv)
	{
	    init((char *) s, max);
	}

	/** 
	 * Destructor. 
	 */
	~String()
	{
	    if (deleteValue && value) delete value;
	}

	/**
	 * Calculates the length of the String.
	 * @return Length of the current String value.
	 */
	Size size() const
	{
	    return strlen(value);
	}

	/**
	 * Read the string byte-wise.
	 * @param index Index of the character inside the String to read.
	 * @return Character value of the given index.
	 */
	u8 valueAt(Size index) const
	{
	    assert(index < size());
	    return value[index];
	}

	/**
	 * Index operator.
	 * @param index Index of the character inside the String to read.
	 * @return Character value at the given index.
	 */
	char operator [] (Size index) const
	{
	    return (char) valueAt(index);
	}

	/**
	 * Assignment operator.
	 * @param s String instance.
	 */
	void operator = (const String & s)
	{
	    this->value = s.value;
	}

	/**
	 * Assignment operator.
	 * @param s String instance.
	 */
	void operator = (String *s)
	{
	    assertRead(s);
	    assertRead(s->value);
	    this->value = s->value;
	}
	
	/**
	 * Assignment operator.
	 * @param s Constant string.
	 */
	void operator = (const char *s)
	{
	    assertRead(s);
	    this->value = (char *) s;
	}

	/**
	 * Compare two Strings.
	 * @param s String instance.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (String *s)
	{
	    assertRead(s);
	    return strcmp(value, s->value) == 0;
	}
	
	/**
	 * Compare a String with a character array.
	 * @param ch Character array.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (char *ch)
	{
	    assertRead(ch);
	    return strcmp(value, ch) == 0;
	}
	
	/**
	 * Compare a String with a character array.
	 * @param ch Character array.
	 * @return True if equal, false otherwise.
	 */	
	bool equals(String *s)
	{
	    assertRead(s->value);
	    return strcmp(value, s->value) == 0;
	}

	/**
	 * Compare a String with a character array.
	 * @param ch Character array.
	 * @return True if equal, false otherwise.
	 */	
	bool equals(const String & s)
	{
	    assertRead(s.value);
	    return strcmp(value, s.value) == 0;
	}

	
	/**
	 * Dereference operator.
	 * @return Pointer to the String value.
	 */
	char * operator * ()
	{
	    return value;
	}

    private:

	/**
	 * Initialize the String.
	 * @param s Initial value of the String.
	 * @param max Maximum length of the String.
	 */
	void init(char *s, Size max)
	{
	    assertRead(s);
	    assert(max > 0);

	    Size sz = strlen(s);	// TODO: let's simply enforce the maximum
	    if (max < sz)
		sz = max;

	    value = new char[sz + 1];
	    strncpy(value, s, sz);
	    value[sz] = 0;
	}

	/** Current value of the String. */    
	char *value;
	
	/** Do we need to delete() to value in destructor? */
	bool deleteValue;
};

#endif /* __STRING_H */

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
#include <ctype.h>
#include <stdlib.h>
#include "Comparable.h"
#include "Types.h"
#include "Macros.h"
#include "Assert.h"

/**
 * Test for a wildcard character.
 * @param ch Input character.
 * @return True if wildcard, false otherwise.
 */
#define WILDCARD(c) ((c) == '*')

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
	 */
	String(char *s)
	{
	    value = strdup(s);
	}

	/**
	 * Copy constructor.
	 * @param s String instance pointer.
	 */
	String(String *s)
	{
	    value = strdup(s->value);
	}
    
	/**
	 * Constant constructor.
	 * @param s Initial value of the String.
	 */
	String(const char *s)
	{
	    value = strdup(s);
	}
    
	/**
	 * Constrant constructor including the maximum length.
	 * @param s Initial value of the String.
	 * @param max Maximum length of the String.
	 */
	String(const char *s, Size max)
	{
	    size_t len = strlen(s);
	    if (len > max)
		len = max;

	    value = (char *) malloc(len + 1);
	    memcpy(value, s, len + 1);
	    value[len] = ZERO;
	}

	/** 
	 * Destructor. 
	 */
	~String()
	{
	    if (value)
	    {
		free(value);
		value = 0;
	    }
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
         * Returns a copy of this String, converted to lowercase.
         * @return The lowercase variant of this String.
         */
        String toLowerCase()
        {
            Size length = this->size();
            char* nv = (char*)malloc(length + 1);
            memset(nv, 0, length + 1);
            
            for( Size index = 0; index < length; index++ )
            {
                nv[index] = tolower(value[index]);
            }
            
            return String(nv);
        }
        
        /**
         * Returns a copy of this String, converted to uppercase.
         * @return The uppercase variant of this String.
         */
        String toUpperCase()
        {
            Size length = this->size();
            char* nv = (char*)malloc(length + 1);
            memset(nv, 0, length + 1);
            
            for( Size index = 0; index < length; index++ )
            {
                nv[index] = toupper(value[index]);
            }
            
            return String(nv);
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
	 * Returns true if and only if this String contains the specified String.
	 * @param sequence The String searched for in this String.
	 * @return bool Whether the specified String is contained in this String.
	 */
        bool contains(String& sequence)
        {
            return contains(*sequence);
        }
        
        /**
         * Returns true if and only if this String contains the specified char*.
         * @param sequence The char* searched for in this String.
         * @return bool Whether the specified char* is contained in this String.
         */
        bool contains(char* sequence)
        {
            if(sequence == (char*) NULL ||
	       strlen(sequence) > strlen(value))
            {
                return false;
            }
	    
            /* See if the first character of sequence occurs in value. */
            char* pt = strchr(value, sequence[0]);
            if( pt == (char*) NULL )
            {
                return false;
            }
	    
            while(*pt)
            {
        	if(strncmp( sequence, pt, strlen( sequence ) == 0))
                {
                    return true;
                }
                pt++;
            }
            return false;
        }

        /**
         * Tests if this String starts with the specified prefix.
         * @param prefix The prefix.
         * @return true If the character sequence represented by the 
         * argument is a prefix of the character sequence represented by
         * this String; false otherwise.
         */
        bool startsWith(String& prefix)
        {
            return startsWith(*prefix);
        }
        
        /**
         * Tests if this String starts with the specified prefix.
         * @param prefix The prefix.
         * @return true If the character sequence represented by the 
         * argument is a prefix of the character sequence represented by
         * this String; false otherwise.
         */
        bool startsWith(char* prefix)
        {
            if( ! prefix )
            {
                return false;
            }
            
            Size pLength = strlen(prefix);
            Size vLength = strlen(value);
            
            if( pLength > vLength )
            {
                return false;
            }
            
            for( Size pos = 0; pos < pLength; pos++ )
            {
                if( value[pos] != prefix[pos] )
                {
                    return false;
                }
            }
            
            return true;
        }
        
        /**
         * Tests if this String ends with the specified suffix.
         * @param suffix The String to compare with the end of this String.
         * @return bool Whether this String ends with the given suffix.
         */
        bool endsWith(String& suffix)
        {
            return endsWith(*suffix);
        }
        
        /**
         * Tests if this String ends with the specified suffix.
         * @param suffix The String to compare with the end of this String.
         * @return bool Whether this String ends with the given suffix.
         */
        bool endsWith(char* suffix)
        {
            Size sLength = strlen(suffix);
            Size vLength = strlen(value);
                
            if( sLength > vLength )
            {
                return false;
            }
                
            char* pt = (value + (vLength - sLength) );
            return( strcmp(pt, suffix) == 0);
        }
        
        /**
         * Returns a new String that contains a copy of this String
         * starting from the given index (inclusive).
         *
         * @param index The begin index to create the substring of.
         * @return String* a pointer to the newly created substring.
         */
        String substring(unsigned int index)
        {
            return substring(index, strlen(value) - index);
        }
        
        /**
         * Returns a new String that contains a copy of this String,
         * starting from index (inclusive), and copies at most
         * size characters.
         *
         * @param index The begin index to create the substring of.
         * @param size The maximum size of the substring.
         * @return String* a pointer to the newly created substring.
         */
        String substring(unsigned int index, unsigned int size)
        {
            size_t length = strlen(value);
            if( index >= length )
            {
                index = length - 1;
            }

            char *copy;
            copy = (char *)malloc(size);
            memcpy(copy, value+index, size);

            String s(copy);
            free(copy);
            return s;
        }
        
        /**
         * Returns an exact copy of this String.
         * The memory for the cloned String is allocated with the 'new'
         * operator, and has to be cleaned up with 'delete'.
         * @return The cloned String.
         */
        String* clone()
        {
            // The constructor of String creates a copy
            // of the specified char*
            String* clone = new String(value);
            return clone;
        }
        
        /**
         * Returns a copy of this String, with leading and trailing
         * whitespace omitted
         */
        String* trim()
        {
            Size from = 0;
            Size to = 0;
            Size length = strlen(value);
            
            for( Size pos = 0; pos < length; pos++ )
            {
                if( ! isWhitespace(value[pos]) )
                {
                    from = pos;
                    break;
                }
            }
            
            for( Size pos = length - 1; true ; pos-- )
            {
                if( pos <= from || ! isWhitespace(value[pos]) )
                {
                    to = pos;
                    break;
                }
            }
            
            if( from == to )
            {
                return (String*)0;
            }

            char* trimmed;
            trimmed = (char *)malloc(to - from + 1);
            memset(trimmed, 0, (to - from + 1));
            memcpy(trimmed, value + from, to - from + 1);

            String* t = new String(trimmed);
            free(trimmed);
            return t;
        }
        
	/**
	 * Matches the given string against a mask.
	 * @param string Input string.
	 * @param mask Pattern to match against.
	 * @return True if match, false otherwise.
	 */
	bool match(char *string, char *mask)
	{
	    /* Loop until the end of the mask, */
	    while (*mask)
	    {
		/* See if the current character is a wildcard or not. */
		if (!WILDCARD(*mask))
		{
		    /*
		     * If it's not a wildcard, the string and mask
		     * must match exactly to be a match.
		     */
		    if (!*string || *mask != *string)
		    {
		        return false;
		    }
		    mask++, string++;
		}
		else
		{
		    /* If we have a wildcard, look for the next character. */
		    while (WILDCARD(*mask))
		    {
			mask++;
		    }
		    /*
		     * There is more coming after the wildcard, to which the
		     * string must match.
		     */
		    if (*mask)
		    {
			/*
			 * Loop until the char in string matches the char
			 * after the wildcard.
			 */
			while (true)
			{
			    if (!*string)
				return false;
			
			    if (*mask == *string)
				break;

			    string++;
			}
		    }
		    /* Mask ends with a wildcard, which means the string matches. */
		    else
			return true;
		}
	    }
	    /* If there remains more data in the string, it's not a match. */
	    return *string ? false : true;
	}

	/**
	 * Matches the String against a mask.
	 * @param mask Pattern to match against.
	 * @return True if match, false otherwise.
	 */	
	bool match(char *mask)
	{
	    return value ? match(value, mask) : false;
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
	 * Compares this String to the given String. 
	 * @param s The String to compare us to.
	 * @return int < 0, 0, > 0 if we are greater than, equal to
	 * or less then the given String.
	 */
	int compareTo(const String & s)
	{
	    return strcmp(value, s.value);
	}
	
	/**
	 * Compares this String to the given String, ignoring
	 * case considerations.
	 * @param s The String to compare to this String.
	 * @return bool Whether the given String is equal to this String.
	 */
        bool equalsIgnoreCase(String& s)
        {
            return equalsIgnoreCase(*s);
        }
        
        /**
         * Compares this String to the given char*, ignoring
         * case considerations.
         * @param s The char* to compare to this String.
         * @return Whether the given char* equals this String.
         */
        bool equalsIgnoreCase(char* s)
        {
            return strcasecmp(value, s);
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
	 * Dereference operator.
	 * @return Pointer to the String value.
	 */
	char * operator * ()
	{
	    return value;
	}
	
	static bool isWhitespace(char c)
	{
	    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
	}

    private:

	/** Current value of the String. */    
	char *value;
};

#endif /* __STRING_H */

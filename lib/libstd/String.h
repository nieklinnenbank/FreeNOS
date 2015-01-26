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
 * Converts the letter c to lowercase.
 * @param c The letter to convert.
 * @return The converted letter, or c if conversion was not possible.
 */
#define tolower(c) \
    (c >= 'A' && c <= 'Z') ? (c + 32) : (c)

/**
 * Converts the letter c to uppercase.
 * @param c The letter to convert.
 * @return The converted letter, or c if conversion was not possible.
 */
#define toupper(c) \
    (c >= 'a' && c <= 'z') ? (c - 32) : (c)

/**
 * Abstraction of strings.
 */
class String : public Comparable<String>
{
    public:

	/**
	 * Empty constructor.
	 */
	String();

	/**
	 * Constructor.
	 * @param s Initial value of the String.
	 */
	String(char *s);

	/**
	 * Copy constructor.
	 * @param s String instance pointer.
	 */
	String(String *s);
    
	/**
	 * Constant constructor.
	 * @param s Initial value of the String.
	 */
	String(const char *s);
    
	/**
	 * Constrant constructor including the maximum length.
	 * @param s Initial value of the String.
	 * @param max Maximum length of the String.
	 */
	String(const char *s, Size max);

	/** 
	 * Destructor. 
	 */
	~String();

	/**
	 * Calculates the length of the String.
	 * @return Length of the current String value.
	 */
	Size size() const;
	
	/**
         * Returns a copy of this String, converted to lowercase.
         * @return The lowercase variant of this String.
         */
        String toLowerCase();
        
        /**
         * Returns a copy of this String, converted to uppercase.
         * @return The uppercase variant of this String.
         */
        String toUpperCase();

	/**
	 * Read the string byte-wise.
	 * @param index Index of the character inside the String to read.
	 * @return Character value of the given index.
	 */
	u8 valueAt(Size index) const;
	
	/**
	 * Returns true if and only if this String contains the specified String.
	 * @param sequence The String searched for in this String.
	 * @return bool Whether the specified String is contained in this String.
	 */
        bool contains(String& sequence);
        
        /**
         * Returns true if and only if this String contains the specified char*.
         * @param sequence The char* searched for in this String.
         * @return bool Whether the specified char* is contained in this String.
         */
        bool contains(char* sequence);
        
        /**
         * Returns true if and only if this String contains the specified char.
         * @param c The char to search for.
         * @return bool Whether the specified char is contained in this String.
         */
        bool contains(char c);

        /**
         * Tests if this String starts with the specified prefix.
         * @param prefix The prefix.
         * @return true If the character sequence represented by the 
         * argument is a prefix of the character sequence represented by
         * this String; false otherwise.
         */
        bool startsWith(String& prefix);
        
        /**
         * Tests if this String starts with the specified prefix.
         * @param prefix The prefix.
         * @return true If the character sequence represented by the 
         * argument is a prefix of the character sequence represented by
         * this String; false otherwise.
         */
        bool startsWith(char* prefix);
        
        /**
         * Tests if this String ends with the specified suffix.
         * @param suffix The String to compare with the end of this String.
         * @return bool Whether this String ends with the given suffix.
         */
        bool endsWith(String& suffix);
        
        /**
         * Tests if this String ends with the specified suffix.
         * @param suffix The String to compare with the end of this String.
         * @return bool Whether this String ends with the given suffix.
         */
        bool endsWith(char* suffix);
        
        /**
         * Returns a new String that contains a copy of this String
         * starting from the given index (inclusive).
         *
         * @param index The begin index to create the substring of.
         * @return String* a pointer to the newly created substring.
         */
        String substring(unsigned int index);
        
        /**
         * Returns a new String that contains a copy of this String,
         * starting from index (inclusive), and copies at most
         * size characters.
         *
         * @param index The begin index to create the substring of.
         * @param size The maximum size of the substring.
         * @return String* a pointer to the newly created substring.
         */
        String substring(unsigned int index, unsigned int size);
        
        /**
         * Returns an exact copy of this String.
         * The memory for the cloned String is allocated with the 'new'
         * operator, and has to be cleaned up with 'delete'.
         * @return The cloned String.
         */
        String* clone();
        
        /**
         * Returns a copy of this String, with leading and trailing
         * whitespace omitted
         */
        String* trim();
        
	/**
	 * Matches the given string against a mask.
	 * @param string Input string.
	 * @param mask Pattern to match against.
	 * @return True if match, false otherwise.
	 */
	bool match(char *string, char *mask);

	/**
	 * Matches the String against a mask.
	 * @param mask Pattern to match against.
	 * @return True if match, false otherwise.
	 */	
	bool match(char *mask);

	/**
	 * Compare a String with a character array.
	 * @param ch Character array.
	 * @return True if equal, false otherwise.
	 */	
	bool equals(String *s);

	/**
	 * Compare a String with a character array.
	 * @param ch Character array.
	 * @return True if equal, false otherwise.
	 */	
	bool equals(const String & s);
	
	/**
	 * Compares this String to the given String. 
	 * @param s The String to compare us to.
	 * @return int < 0, 0, > 0 if we are greater than, equal to
	 * or less then the given String.
	 */
	int compareTo(const String & s);
	
	/**
	 * Compares this String to the given String, ignoring
	 * case considerations.
	 * @param s The String to compare to this String.
	 * @return bool Whether the given String is equal to this String.
	 */
        bool equalsIgnoreCase(String& s);
        
        /**
         * Compares this String to the given char*, ignoring
         * case considerations.
         * @param s The char* to compare to this String.
         * @return Whether the given char* equals this String.
         */
        bool equalsIgnoreCase(char* s);
	
	/**
	 * Index operator.
	 * @param index Index of the character inside the String to read.
	 * @return Character value at the given index.
	 */
	char operator [] (Size index) const;

	/**
	 * Assignment operator.
	 * @param s String instance.
	 */
	void operator = (const String & s);

	/**
	 * Assignment operator.
	 * @param s String instance.
	 */
	void operator = (String *s);
	
	/**
	 * Assignment operator.
	 * @param s Constant string.
	 */
	void operator = (const char *s);

	/**
	 * Compare two Strings.
	 * @param s String instance.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (String *s);
	
	/**
	 * Compare a String with a character array.
	 * @param ch Character array.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (char *ch);
	
	/**
	 * Dereference operator.
	 * @return Pointer to the String value.
	 */
	char * operator * ();
	
	static bool isWhitespace(char c);

        static unsigned strlen(const char *str);

        static int strcmp(const char * dest, const char *src);

        static char * strdup(char *str);

        static int strcasecmp(const char *dest, const char *src );

        static char * strchr(const char *s, int c);

        static int strncmp( const char *dest, const char *src, unsigned count );

        static unsigned strlcpy(char *dst, const char *src, unsigned siz);

    private:

	/** Current value of the String. */    
	char *value;
};

#endif /* __STRING_H */

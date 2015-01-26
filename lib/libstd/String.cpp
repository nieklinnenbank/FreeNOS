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

#include "String.h"
#include "MemoryBlock.h"

String::String()
{
    value = ZERO;
}

String::String(char *s)
{
    value = strdup(s);
}

String::String(String *s)
{
    value = strdup(s->value);
}

String::String(const char *s)
{
    value = strdup((char *)s);
}

String::String(const char *s, Size max)
{
    unsigned len = strlen(s);
    if (len > max)
        len = max;

    value = new char [len + 1];
    MemoryBlock::copy(value, s, len + 1);
    value[len] = ZERO;
}

String::~String()
{
    if (value)
    {
        delete value;
        value = 0;
    }
}

Size String::size() const
{
    return strlen(value);
}

String String::toLowerCase()
{
    Size length = this->size();
    char* nv = new char[length + 1];
    MemoryBlock::set(nv, 0, length + 1);

    for( Size index = 0; index < length; index++ )
    {
        nv[index] = tolower(value[index]);
    }
    return String(nv);
}

String String::toUpperCase()
{
    Size length = this->size();
    char* nv = new char[length + 1];
    MemoryBlock::set(nv, 0, length + 1);

    for( Size index = 0; index < length; index++ )
    {
        nv[index] = toupper(value[index]);
    }

    return String(nv);
}

u8 String::valueAt(Size index) const
{
    assert(index < size());
    return value[index];
}

bool String::contains(String& sequence)
{
    return contains(*sequence);
}

bool String::contains(char* sequence)
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

bool String::contains(char c)
{
    return (strchr(value, c) != NULL);
}

bool String::startsWith(String& prefix)
{
    return startsWith(*prefix);
}

bool String::startsWith(char* prefix)
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

bool String::endsWith(String& suffix)
{
    return endsWith(*suffix);
}

bool String::endsWith(char* suffix)
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

String String::substring(unsigned int index)
{
    return substring(index, strlen(value) - index);
}

String String::substring(unsigned int index, unsigned int size)
{
    unsigned length = strlen(value);
    if( index >= length )
    {
        index = length - 1;
    }

    char *copy;
    copy = new char[size];
    MemoryBlock::copy(copy, value+index, size);

    String s(copy);
    delete copy;
    return s;
}

String* String::clone()
{
    // The constructor of String creates a copy
    // of the specified char*
    String* clone = new String(value);
    return clone;
}

String* String::trim()
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
    trimmed = new char[to - from + 1];
    MemoryBlock::set(trimmed, 0, (to - from + 1));
    MemoryBlock::copy(trimmed, value + from, to - from + 1);

    String* t = new String(trimmed);
    delete trimmed;
    return t;
}

bool String::match(char *string, char *mask)
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

bool String::match(char *mask)
{
    return value ? match(value, mask) : false;
}

bool String::equals(String *s)
{
    assertRead(s->value);
    return strcmp(value, s->value) == 0;
}

bool String::equals(const String & s)
{
    assertRead(s.value);
    return strcmp(value, s.value) == 0;
}

int String::compareTo(const String & s)
{
    return strcmp(value, s.value);
}

bool String::equalsIgnoreCase(String& s)
{
    return equalsIgnoreCase(*s);
}

bool String::equalsIgnoreCase(char* s)
{
    return strcasecmp(value, s);
}

char String::operator [] (Size index) const
{
    return (char) valueAt(index);
}

void String::operator = (const String & s)
{
    this->value = s.value;
}

void String::operator = (String *s)
{
    assertRead(s);
    assertRead(s->value);
    this->value = s->value;
}

void String::operator = (const char *s)
{
    assertRead(s);
    this->value = (char *) s;
}

bool String::operator == (String *s)
{
    assertRead(s);
    return strcmp(value, s->value) == 0;
}

bool String::operator == (char *ch)
{
    assertRead(ch);
    return strcmp(value, ch) == 0;
}

char * String::operator * ()
{
    return value;
}

bool String::isWhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

unsigned String::strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s);
    return (s - str);
}

int String::strcmp( const char *dest, const char *src )
{
    while ( *dest && *src && *dest == *src )
    {
        dest++;
        src++;
    }
    return (*dest - *src);
}

char * String::strdup(char *str)
{
    unsigned sz;
    char *copy;

    sz = strlen(str) + 1;

    if ((copy = new char[sz]) == NULL)
        return (char *) (NULL);

    MemoryBlock::copy(copy, str, sz);
    return(copy);
}

int String::strcasecmp(const char *dest, const char *src )
{
    
    while ( *dest && *src && (tolower((int)*dest) == tolower((int)*src)) )
    {
	dest++;
	src++;
    }
    return (tolower((int)*dest) - tolower((int)*src));
}

char * String::strchr(const char *s, int c)
{
    while (*s && *s != c)
    {
	s++;
    }
    return (char *) (*s ? s : NULL);
}

int String::strncmp( const char *dest, const char *src, unsigned count )
{
    while (*dest && *src && *dest == *src && count)
    {
	if (--count)
	{
	    dest++, src++;
	}
    }
    return (*dest - *src);
}

unsigned String::strlcpy(char *dst, const char *src, unsigned siz)
{
	char *d = dst;
	const char *s = src;
	unsigned n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

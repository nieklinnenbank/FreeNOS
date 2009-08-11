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

#ifndef __STRINGBUFFER_H
#define __STRINGBUFFER_H

#include <Vector.h>
#include <String.h>
#include <string.h>
#include <stdio.h>

class StringBuffer
{
    private:
        Vector<char>* _chars;
        
    public:
        StringBuffer()
        {
            _chars = new Vector<char>();
        }
        
        ~StringBuffer()
        {
            for( Size s = 0; s < _chars->count(); s++ )
            {
                delete _chars->get(s);
            }
            
            delete _chars;
        }
        
        /**
         * Appends a char to the buffer.
         * @param c The char to add to the buffer.
         */
        void append(char c)
        {
            if( c != 0 )
            {
                char* d = new char(c);
                _chars->insert(&d[0]);
            }
        }
        
        /**
         * Convenience method to add an char* to the buffer.
         * @param c The char* to add to the buffer.
         */
        void append(char* c)
        {
            for(Size pos = 0; pos < strlen(c); pos++)
            {
                append(c[pos]);
            }
        }
        
        /**
         * Reads the buffer and creates a new String from the current 
         * contents.
         * @return The String* representing the current contents of the buffer.
         */
        String* toString()
        {
            Size count = _chars->count();
            
            char* c = (char*)malloc(count + 1);
            memset(c, 0, count + 1);
            
            for( Size pos = 0; pos < count; pos++ )
            {
                c[pos] = _chars->get(pos)[0];
            }
            
            String* s = new String(c);
            free(c);
            return s;
        }
        
};

#endif /* __STRINGBUFFER_H */

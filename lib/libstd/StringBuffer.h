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

#include "Vector.h"
#include "String.h"

class StringBuffer
{
    private:
        Vector<char>* _chars;

    public:
        StringBuffer();

        ~StringBuffer();

        /**
         * Appends a char to the buffer.
         * @param c The char to add to the buffer.
         */
        void append(char c);

        /**
         * Convenience method to add an char* to the buffer.
         * @param c The char* to add to the buffer.
         */
        void append(char* c);

        /**
         * Reads the buffer and creates a new String from the current 
         * contents.
         * @return The String* representing the current contents of the buffer.
         */
        String* toString();
};

#endif /* __STRINGBUFFER_H */

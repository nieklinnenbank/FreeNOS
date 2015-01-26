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

#include "StringBuffer.h"
#include "MemoryBlock.h"

StringBuffer::StringBuffer()
{
    _chars = new Vector<char>();
}

StringBuffer::~StringBuffer()
{
    for( Size s = 0; s < _chars->count(); s++ )
    {
        delete _chars->get(s);
    }

    delete _chars;
}

void StringBuffer::append(char c)
{
    if( c != 0 )
    {
        char* d = new char(c);
        _chars->insert(&d[0]);
    }
}

void StringBuffer::append(char* c)
{
    for(Size pos = 0; pos < String::strlen(c); pos++)
    {
        append(c[pos]);
    }
}

String* StringBuffer::toString()
{
    Size count = _chars->count();

    char* c = new char[count + 1];
    MemoryBlock::set(c, 0, count + 1);

    for( Size pos = 0; pos < count; pos++ )
    {
        c[pos] = _chars->get(pos)[0];
    }

    String* s = new String(c);
    delete c;
    return s;
}

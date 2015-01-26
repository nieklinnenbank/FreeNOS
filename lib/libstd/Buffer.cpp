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

#include "Buffer.h"
#include "MemoryBlock.h"

Buffer::Buffer(Size sz)
{
    buffer = new u8[sz];
    size   = sz;
    filled = ZERO;
}

Buffer::~Buffer()
{
    delete buffer;
}

void Buffer::bytesIn(void *buffer, Size *num)
{
    /* Make prior assumptions. */
    assertRead(buffer);
    assertRead(num);
    assert(filled <= size);

    /*
     * Calculate the number of bytes to move in, at max.
     */
    Size bytes = (size - filled) < *num ?
                 (size - filled) : *num;

    /* Do we have space remaining? */
    if (bytes)
    {
        MemoryBlock::copy(this->buffer + filled, buffer, bytes);
        this->filled += bytes;
    }
}

void * Buffer::bytesOut(Size *num)
{
    /* Make prior assumptions. */
    assertRead(num);
    assertWrite(buffer);

    /*
     * Calculate the number of bytes to move out.
     */
    Size bytes = filled < *num ?
                 filled : *num;

    /* Update state. */
    filled -= bytes;
    *num = bytes;

    /* Return a pointer to the internal buffer. */
    return (void *)(buffer + filled);
}

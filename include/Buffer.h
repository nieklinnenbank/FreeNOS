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

#ifndef __BUFFER_H
#define __BUFFER_H

#include <Types.h>
#include <Macros.h>
#include <Assert.h>
#include <string.h>

/**
 * @brief Represents an abstract buffering datatype.
 *
 * This class manages an internal buffer from which bytes
 * may be taken (moved out) or added (moved in).
 *
 */
class Buffer
{
    public:
    
	/**
	 * @brief Constructor function.
	 *
	 * Responsible for initializing the class members and
	 * allocating a memory buffer.
	 *
	 * @param sz Size of the internal buffer.
	 * 
	 * @see buffer
	 */
	Buffer(Size sz = 8192)
        {
	    buffer = new u8[sz];
	    size   = sz;
	    filled = ZERO;
        }
	
	/**
	 * @brief Destructor function.
	 *
	 * Releases the internal memory buffer.
	 *
	 * @see buffer
	 */
	~Buffer()
	{
	    delete buffer;
	}

	/**
	 * @brief Move bytes into the buffer.
	 *
	 * This function allows moving bytes into the internal
	 * buffer. It will verify whether or not enough remaining
	 * space is available.
	 *
	 * @param buffer Pointer to bytes to insert.
	 * @param num On input, the number of bytes to insert. On
	 *            output the number of bytes actually moved in.
	 *
	 * @see buffer
	 */
	void bytesIn(void *buffer, Size *num)
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
		memcpy(this->buffer + filled, buffer, bytes);
		this->filled += bytes;
	    }
	}

	/**
	 * @brief Move bytes out of the buffer.
	 *
	 * This function allows removing bytes from the internal
	 * buffer. It will verify whether or not there are any
	 * bytes available.
	 *
	 * @param num On input, the number of bytes to remove. On
	 *            output the number of bytes actually moved out.
	 *
	 * @see buffer
	 */	
        void * bytesOut(Size *num)
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
    
    private:
    
	/** Internal memory buffer managed by this class. */
	u8 *buffer;
	
	/** Size of the buffer in bytes. */
	Size size;
	
	/** Number of bytes which are occupied. */
	Size filled;
};

#endif /* __BUFFER_H */

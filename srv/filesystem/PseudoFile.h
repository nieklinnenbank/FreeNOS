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

#ifndef __FILESYSTEM_PSEUDO_FILE_H
#define __FILESYSTEM_PSEUDO_FILE_H

#include <Types.h>
#include <Error.h>
#include "File.h"
#include "FileMode.h"
#include "Directory.h"
#include "IOBuffer.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief Pseudo files only exist in memory.
 */
class PseudoFile : public File
{
    public:

	/**
	 * @brief Empty constructor.
	 */
	PseudoFile()
	{
	    size   = ZERO;
	    buffer = ZERO;
	    access = OwnerRW;
	}

	/**
	 * @brief Constructor function.
	 * @param str Input string.
	 */
	PseudoFile(char *str)
	{
	    access = OwnerRW;
	    size   = strlen(str);
    	    buffer = new char[size + 1];
            strlcpy(buffer, str, size + 1);
	}
	
	/**
	 * @brief Constructor with formatted input.
	 * @param format Format string.
	 * @param ... Argument list.
	 */
	PseudoFile(const char *format, ...)
	{
	    va_list args;
	    
	    /* Allocate buffer. */
	    buffer = new char[512];
	    
	    /* Format the input. */
	    va_start(args, format);
	    size = vsnprintf(buffer, 512, format, args);
	    va_end(args);
	    
	    /* Set permissions. */
	    access = OwnerRW;
	}

	/**
	 * @brief Destructor function, which releases the buffer.
	 */
	~PseudoFile()
	{
	    delete buffer;
	}

	/** 
         * @brief Read bytes from the file. 
	 *
         * @param buffer Output buffer. 
         * @param size Number of bytes to read, at maximum. 
         * @param offset Offset inside the file to start reading. 
         * @return Number of bytes read on success, Error on failure. 
	 *
	 * @see IOBuffer
         */
        Error read(IOBuffer *buffer, Size size, Size offset)
	{
	    /* Bounds checking. */
	    if (offset >= this->size)
	    {
		return 0;
            }
	    else
	    {
		/* How much bytes to copy? */
		Size bytes = this->size - offset > size ?
		    				   size : this->size - offset;
    
	        /* Copy the buffers. */
		return buffer->write(this->buffer + offset, bytes);
	    }
	}

    protected:
    
	/** Buffer from which we read. */
	char *buffer;
};

/**
 * @}
 */

#endif /* __FILESYSTEM_PSEUDO_FILE_H */

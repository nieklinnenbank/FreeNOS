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

#ifndef __FILESYSTEM_IOBUFFER_H
#define __FILESYSTEM_IOBUFFER_H

#include <API/VMCopy.h>
#include <Types.h>
#include <Error.h>
#include "FileSystemMessage.h"

/**
 * @brief Abstract Input/Output buffer.
 */
class IOBuffer
{
    public:
    
	/**
	 * @brief Constructor function.
	 *
	 * @param msg Describes the request being processed.
	 */
	IOBuffer(FileSystemMessage *msg) : message(msg)
	{
	}
    
	/**
	 * @brief Read bytes from the I/O buffer.
	 *
	 * @param buffer Copy bytes from the I/O buffer to this memory address.
	 * @param size Number of bytes to copy.
	 * @param offset The offset inside the I/O buffer to start reading.
	 * @return Number of bytes read on success, and error code on failure.
	 */
	Error read(void *buffer, Size size, Size offset = ZERO)
	{
	    return VMCopy(message->from, Read,
                         (Address) buffer,
                         (Address) message->buffer + offset, size);
	}
	
	
	/**
	 * @brief Write bytes to the I/O buffer.
	 *
	 * @param buffer Contains the bytes to write.
	 * @param size Number of bytes to write.
	 * @param offset The offset inside the I/O buffer to start writing.
	 * @return Number of bytes written on success, and error code on failure.
	 */
	Error write(void *buffer, Size size, Size offset = ZERO)
	{
	    return VMCopy(message->from, Write,
                         (Address) buffer,
                         (Address) message->buffer + offset, size);
	}
    
    private:
    
	/**
	 * @brief Current request being processed.
	 *
	 * Read() and write() will use fields from the current request
	 * to fill in arguments for VMCopy().
	 *
	 * @see VMCopy
	 * @see IOBuffer::read
	 * @see IOBuffer::write
	 */
	FileSystemMessage *message;
};

#endif /* FILESYSTEM_IOBUFFER_H */

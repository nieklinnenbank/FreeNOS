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

#ifndef __TERMINAL_TERMINAL_H
#define __TERMINAL_TERMINAL_H

#include <api/IPCMessage.h>
#include <Macros.h>
#include <Types.h>
#include <Error.h>

/** Size of the I/O buffers. */
#define BUFFERSIZE 1024

/**
 * Buffered I/O Terminal.
 */
class Terminal
{
    public:

	/**
	 * Class constructor.
	 */
	Terminal();

	/**
	 * Class destructor.
	 */
	virtual ~Terminal();

	/**
	 * Attemps to flush all buffers.
	 */
	void flush();

	/**
	 * Read bytes from the readBuffer.
	 * @param buffer Output buffer.
	 * @param size Maximum bytes to copy.
	 * @return Number of bytes read.
	 */
	int bufferedRead(s8 *buffer, Size size);
	
	/**
	 * Write bytes to the writeBuffer.
	 * @param buffer Input buffer.
	 * @param size Maximum number of bytes to copy.
	 * @return Number of bytes written.
	 */
	int bufferedWrite(s8 *buffer, Size size);

	/**
	 * Read bytes from the underlying transient.
	 * @param buffer Buffer to store bytes to read.
	 * @param size Number of bytes to read.
	 * @return Number of bytes on success and ZERO on failure.
	 */
	virtual int read(s8 *buffer, Size size)
	{
	    return ENOSUPPORT;
	}

	/**
	 * Write bytes to the underlying recipient.
	 * @param buffer Buffer containing bytes to write.
	 * @param size Number of bytes to write.
	 * @return Number of bytes on success and ZERO on failure.
	 */	
	virtual int write(s8 *buffer, Size size)
	{
	    return ENOSUPPORT;
	}

    protected:

	/** Buffers all read operations. */
	s8 readBuffer[BUFFERSIZE];
	
	/** Buffers all write operations. */
	s8 writeBuffer[BUFFERSIZE];

	/** Amount of bytes in the readBuffer. */
	Size readBytes;
	
	/** Amount of bytes in the writeBuffer. */
	Size writeBytes;
};

#endif /* __TERMINAL_TERMINAL_H */

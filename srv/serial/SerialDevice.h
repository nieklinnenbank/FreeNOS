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

#ifndef __SERIAL_SERIALDEVICE_H
#define __SERIAL_SERIALDEVICE_H

#include <api/IPCMessage.h>
#include <FileSystemMessage.h>
#include <Macros.h>
#include <Types.h>
#include <Error.h>

/** Size of the I/O buffers. */
#define BUFFERSIZE 1024

/**
 * Buffered serial I/O device.
 */
class SerialDevice
{
    public:

        /** 
         * Constructor function. 
         * @param base I/O base port. 
         */
	SerialDevice(u16 base, u16 irq);

	/**
	 * Class destructor.
	 */
	virtual ~SerialDevice();

        /** 
         * Retrieve the base I/O port. 
         * @return Base I/O port. 
         */
        u16 getBase()
        {
            return base;
        }
	
	/**
         * Returns our IRQ number.
         * @return IRQ number.
         * @return Zero on success and non-zero on failure.
         */
        u16 getIRQ()
        {
            return irq;
        }

	/**
	 * Attemps to flush all buffers.
	 * @return Number of bytes flushed.
	 */
	Size flush();

	/**
	 * Are we currently processing a request?
	 * @return True if request pending, false otherwise.
	 */
	bool isRequestPending()
	{
	    return requestPending;
	}

	/**
	 * Read bytes from the readBuffer.
	 * @param buffer Output buffer.
	 * @param size Maximum bytes to copy.
	 * @return Number of bytes read.
	 */
	Size bufferedRead(FileSystemMessage *msg);
	
	/**
	 * Write bytes to the writeBuffer.
	 * @param buffer Input buffer.
	 * @param size Maximum number of bytes to copy.
	 * @return Number of bytes written.
	 */
	Size bufferedWrite(FileSystemMessage *msg);

	/**
	 * Read bytes from the underlying transient.
	 * @param buffer Buffer to store bytes to read.
	 * @param size Number of bytes to read.
	 * @return Number of bytes on success and ZERO on failure.
	 */
	virtual Size read(s8 *buffer, Size size)
	{
	    return ENOTSUP;
	}

	/**
	 * Write bytes to the underlying recipient.
	 * @param buffer Buffer containing bytes to write.
	 * @param size Number of bytes to write.
	 * @return Number of bytes on success and ZERO on failure.
	 */	
	virtual Size write(s8 *buffer, Size size)
	{
	    return ENOTSUP;
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
	
        /** Base I/O port. */
        u16 base;

        /** IRQ vector number. */
        u16 irq;
	
	/** Pending I/O request. */
	FileSystemMessage request;
	
	/** Is there a request pending? */
	bool requestPending;
};

#endif /* __SERIAL_SERIALDEVICE_H */

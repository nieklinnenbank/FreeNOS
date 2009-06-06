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

#include <API/VMCopy.h>
#include <FileSystemMessage.h>
#include <Types.h>
#include <string.h>
#include "SerialDevice.h"

SerialDevice::SerialDevice(u16 b, u16 q)
    : readBytes(ZERO), writeBytes(ZERO),
      base(b), irq(q)
{
    /* Clear buffers. */
    memset(&readBuffer,  0, BUFFERSIZE);
    memset(&writeBuffer, 0, BUFFERSIZE);
}

SerialDevice::~SerialDevice()
{
}

Size SerialDevice::flush()
{
    FileSystemMessage fs;
    int bytes;

    /* Read as many bytes as possible. */
    if ((bytes = read(readBuffer + readBytes, BUFFERSIZE - readBytes)) > 0)
    {
        /* Increment count. */
	readBytes += bytes;
    }
    /* Write as many bytes as possible. */
    else if ((bytes = write(writeBuffer, writeBytes)) > 0)
    {
	/* Move the written bytes from the buffer. */
	memcpy(writeBuffer, writeBuffer + bytes, writeBytes - bytes);
    
        /* Decrement count. */
	writeBytes -= bytes;
    }
    /* Process pending I/O request, if any. */
    if (requestPending)
    {
	switch (request.action)
	{
	    /* Read out data. */
	    case ReadFile:
		bytes = bufferedRead(ZERO);
		break;
	
	    /* Write out data. */
	    case WriteFile:
	    default:
		bytes = bufferedWrite(ZERO);
		break;
	}
	/* Inform filesystem if the operation completed. */
	if (bytes > 0)
	{
	    fs        = request;
	    fs.action = IODone;
	    fs.size   = bytes;
	    fs.result = ESUCCESS;
	    fs.ipc(request.from, Send, sizeof(fs));
	}
    }
    return bytes;
}

Size SerialDevice::bufferedRead(FileSystemMessage *req)
{
    /* Set request. */
    if (req)
	request = req;

    /* Calculate how many bytes to read. */
    Size num = readBytes < request.size ? readBytes : request.size;

    /* Read bytes. */
    if (num)
    {
	if ((num = VMCopy(request.procID, Write, (Address) readBuffer,
			 (Address) request.buffer, num)) > 0)
	{
	    readBytes -= num;
	}
	requestPending = false;
    }
    /* No bytes available yet. */
    else
	requestPending = true;
	
    return num;
}

Size SerialDevice::bufferedWrite(FileSystemMessage *req)
{
    /* Set request. */
    if (req)
	request = req;

    /* Calculate number of bytes to write. */
    Size num = request.size < (BUFFERSIZE - writeBytes) ?
	       request.size : (BUFFERSIZE - writeBytes);

    /* Write bytes. */
    if (num)
    {
	if ((num = VMCopy(request.procID, Read, (Address) writeBuffer + writeBytes,
	                 (Address) request.buffer, num)) > 0)
	{
	    writeBytes += num;
	}
	requestPending = false;
    }
    /* Could not write all bytes yet. */
    else
	requestPending = true;

    return num;
}

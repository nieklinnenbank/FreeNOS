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

#include <Types.h>
#include <string.h>
#include "Terminal.h"

Terminal::Terminal() : readBytes(ZERO), writeBytes(ZERO)
{
    /* Clear buffers. */
    memset(&readBuffer,  0, BUFFERSIZE);
    memset(&writeBuffer, 0, BUFFERSIZE);
}

Terminal::~Terminal()
{
}

void Terminal::flush()
{
    int bytes;

    /* Read as many bytes as possible. */
    if ((bytes = read(readBuffer + readBytes, BUFFERSIZE - readBytes)) > 0)
    {
        /* Increment count. */
	readBytes += bytes;
    }
    /* Write as many bytes as possible. */
    if ((bytes = write(writeBuffer, writeBytes)) > 0)
    {
	/* Move the written bytes from the buffer. */
	memcpy(writeBuffer, writeBuffer + bytes, writeBytes - bytes);
    
        /* Decrement count. */
	writeBytes -= bytes;
    }
}

int Terminal::bufferedRead(s8 *buffer, Size size)
{
    Size num = readBytes < size ? readBytes : size;

    memcpy(buffer, readBuffer, num);
    readBytes -= num;
    return num;
}

int Terminal::bufferedWrite(s8 *buffer, Size size)
{
    Size num = size < (BUFFERSIZE - writeBytes) ?
	       size : (BUFFERSIZE - writeBytes);

    memcpy(writeBuffer + writeBytes, buffer, num);
    writeBytes += num;
    return num;
}

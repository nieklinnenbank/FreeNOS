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

#include <File.h>
#include <FileMode.h>
#include "PCIServer.h"
#include "PCIConfig.h"

PCIConfig::PCIConfig(u16 bus, u16 slot, u16 func)
{
    this->access = OwnerRW;
    this->bus    = bus;
    this->slot   = slot;
    this->func   = func;
}

Error PCIConfig::read(IOBuffer *buffer, Size size, Size offset)
{
    Size bytes;
    ulong value = 0;

    /* Bounds checking. */
    if (offset >= 256)
    {
	return 0;
    }
    else
    {
	/* Read out the register. */
	switch (size)
	{
	    case 1:
		bytes = 1;
		value = PCI_READ_BYTE(bus, slot, func, offset);
		break;
			
	    case 2:
		bytes = 2;
		value = PCI_READ_WORD(bus, slot, func, offset);
		break;
			
	    default:
		bytes = 4;
		value = PCI_READ_LONG(bus, slot, func, offset);
		break;
	}
        /* Copy the buffers. */
	return buffer->write(&value, bytes);
    }
}

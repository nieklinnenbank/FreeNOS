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
#include "PCIFile.h"

PCIFile::PCIFile(u16 bus, u16 slot, u16 func, u16 reg, Size size)
{
    this->access = OwnerRW;
    this->bus  = bus;
    this->slot = slot;
    this->func = func;
    this->reg  = reg;
    this->size = size;
}

Error PCIFile::read(IOBuffer *buffer, Size size, Size offset)
{
    char buf[4];
	
    /* Bounds checking. */
    if (offset >= this->size)
    {
	return 0;
    }
    else
    {
	/* Read out the register. */
	switch (this->size)
	{
	    case 1:
		buf[0] = PCI_READ_BYTE(bus, slot, func, reg);
		break;
			
	    case 2:
		*(u16 *)(&buf) = PCI_READ_WORD(bus, slot, func, reg);
		break;
			
	    default:
		*(u32 *)(&buf) = PCI_READ_LONG(bus, slot, func, reg);
		break;
	}
	/* How much bytes to copy? */
	Size bytes = this->size - offset > size ?
	    				   size : this->size - offset;
        
        /* Copy the buffers. */
	return buffer->write(buf + offset, bytes);
    }
}

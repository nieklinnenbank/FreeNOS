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

#include <FreeNOS/API.h>
#include <CoreMessage.h>
#include <Types.h>
#include <errno.h>
#include "VGA.h"

VGA::VGA(Size w, Size h) : width(w), height(h)
{
}

Error VGA::initialize()
{
    CoreMessage mem;

    /* Request VGA memory. */
    mem.action    = CreatePrivate;
    mem.size      = PAGESIZE;
    mem.virt      = ZERO;
    mem.phys      = VGA_PADDR;
    mem.access    = VirtualMemory::Present |
                    VirtualMemory::User |
                    VirtualMemory::Readable |
                    VirtualMemory::Writable |
                    VirtualMemory::Pinned;
    mem.type      = IPCType;
    IPCMessage(CORESRV_PID, API::SendReceive, &mem, sizeof(mem));

    /* Point to the VGA mapping. */
    vga = (u16 *) mem.virt;

    /* Clear screen. */
    for (uint i = 0; i < width * height; i++)
    {                                                  
        vga[i] = VGA_CHAR(' ', LIGHTGREY, BLACK);
    }
    
    /* Disable hardware cursor. */
    WriteByte(VGA_IOADDR, 0x0a);
    WriteByte(VGA_IODATA, 1 << 5);
    
    /* Successfull. */
    return ESUCCESS;
}

Error VGA::read(s8 *buffer, Size size, Size offset)
{
    if (offset + size > width * height * sizeof(u16))
    {
	return EFAULT;
    }
    memcpy(buffer, vga + (offset / sizeof(u16)), size);
    return size;
}

Error VGA::write(s8 *buffer, Size size, Size offset)
{
    if (offset + size > width * height * sizeof(u16))
    {
	return EFAULT;
    }
    memcpy(vga + (offset / sizeof(u16)), buffer, size);    
    return size;
}

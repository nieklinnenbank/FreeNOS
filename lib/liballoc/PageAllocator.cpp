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

#include <API/IPCMessage.h>
#include <API/ProcessCtl.h>
#include <MemoryMessage.h>
#include <Config.h>
#include "PageAllocator.h"

PageAllocator::PageAllocator(Size size)
    : start(ZERO), allocated(ZERO)
{
    MemoryMessage mem;

    /* First reserve ~128MB virtual memory. */
    mem.action = ReservePrivate;
    mem.bytes  = 1024 * 1024 * 128;
    mem.virtualAddress = 1024 * 1024 * 16;
    mem.ipc(MEMSRV_PID, SendReceive, sizeof(mem));

    /* Set heap pointer. */
    start = mem.virtualAddress;

    /* Allocate the given bytes. */
    allocate(&size);
}

PageAllocator::PageAllocator(PageAllocator *p)
    : start(p->start), allocated(p->allocated)
{
}

Address PageAllocator::allocate(Size *size)
{
    MemoryMessage msg;
    Address ret = start + allocated;
    Size bytes  = *size > PAGEALLOC_MINIMUM ?
		  *size : PAGEALLOC_MINIMUM;

    /* Fill in the message. */
    msg.action = CreatePrivate;
    msg.bytes  = bytes;
    msg.protection      = PAGE_RW | PAGE_RESERVED;
    msg.virtualAddress  = (1024 * 1024 * 16) + allocated;
    msg.physicalAddress = ZERO;
    msg.ipc(MEMSRV_PID, SendReceive, sizeof(msg));

    /* Update count. */
    allocated += msg.bytes;

    /* Success. */
    *size = msg.bytes;
    return ret;
}

void PageAllocator::release(Address addr)
{
    // TODO
}

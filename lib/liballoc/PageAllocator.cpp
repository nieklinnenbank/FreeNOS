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

#include <api/IPCMessage.h>
#include <api/ProcessCtl.h>
#include <MemoryServer.h>
#include "PageAllocator.h"

PageAllocator::PageAllocator()
    : heapStart(ZERO), heapEnd(ZERO), allocated(ZERO)
{
}

PageAllocator::PageAllocator(Size size)
    : heapStart(ZERO), heapEnd(ZERO), allocated(ZERO)
{
    allocate(&size);
}

PageAllocator::PageAllocator(PageAllocator *p)
    : heapStart(p->heapStart), heapEnd(p->heapEnd),
      allocated(p->allocated)
{
}

Address PageAllocator::allocate(Size *size)
{
    MemoryMessage msg;
    Address ret  = heapStart + allocated;
    Size bytes = *size > PAGEALLOC_MINIMUM ? *size : PAGEALLOC_MINIMUM;

    /* Do we need to allocate more pages? */
#if 0
    if (heapEnd - (heapStart + allocated) < *size)
    {
#endif
	/* Fill in the message. */
	msg.action =  HeapGrow;
        msg.bytes  =  bytes;

	/* Grow heap. */
        IPCMessage(MEMSRV_PID, SendReceive, &msg);

	/* Update heap pointers. */
	heapStart  = msg.startAddr;
	heapEnd    = msg.endAddr;
#if 0
    }
#endif
    /* Update count. */
    allocated += bytes;

    /* Success. */
    return ret;
}

void PageAllocator::release(Address addr)
{
    MemoryMessage msg;

    /* Release heap pages. */
    for (Address i = heapStart + allocated; i > addr; i += PAGESIZE)
    {
	if (i - addr >= PAGESIZE)
	{
	    /* Fill in the message. */
	    msg.action =  HeapShrink;
	    msg.bytes  =  PAGESIZE;
	    
	    /* Shrink heap. */
	    IPCMessage(MEMSRV_PID, SendReceive, &msg);
	}
    }
    /* Update counter. */
    allocated = addr - heapStart;
}

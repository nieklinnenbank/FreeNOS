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
#include "PageAllocator.h"

PageAllocator::PageAllocator(Size size)
    : start(ZERO), allocated(ZERO)
{
    // Set heap pointer
    start = USER_HEAP;

    // Allocate the given bytes
    allocate(&size);
}

PageAllocator::PageAllocator(PageAllocator *p)
    : start(p->start), allocated(p->allocated)
{
}

Address PageAllocator::allocate(Size *size)
{
    CoreMessage msg;
    Address ret = start + allocated;

    // TODO: #warning TODO: perhaps align to page size???
    Size bytes  = *size > PAGEALLOC_MINIMUM ?
                  *size : PAGEALLOC_MINIMUM;

    // Fill in the message. */
    msg.action = CreatePrivate;
    msg.size   = bytes;
    msg.access = VirtualMemory::Present  |
                 VirtualMemory::User     |
                 VirtualMemory::Readable |
                 VirtualMemory::Writable |
                 VirtualMemory::Reserved;
    msg.virt   = USER_HEAP + allocated;
    msg.phys   = ZERO;
    msg.type   = IPCType;
    msg.from   = SELF;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // Update count
    allocated += msg.size;

    // Success
    *size = msg.size;
    return ret;
}

void PageAllocator::release(Address addr)
{
    // TODO
}

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

PageAllocator::PageAllocator(Address base, Size size)
{
    m_base      = base;
    m_size      = size;
    m_allocated = 0;
}

PageAllocator::PageAllocator(PageAllocator *p)
{
    m_base = p->m_base;
    m_size = p->m_size;
    m_allocated = p->m_allocated;
}

Address PageAllocator::base()
{
    return m_base;
}

Size PageAllocator::size()
{
    return m_size;
}

Size PageAllocator::available()
{
    return m_size - m_allocated;
}

Allocator::Result PageAllocator::allocate(Size *size, Address *addr, Size align)
{
    CoreMessage msg;
    
    // Set return address
    *addr = m_base + m_allocated;

    // TODO: sanity checks
    // TODO: #warning TODO: perhaps align to page size???
    Size bytes  = *size > PAGEALLOC_MINIMUM ?
                  *size : PAGEALLOC_MINIMUM;

    bytes = aligned(bytes);

    // Fill in the message. */
    msg.action = CreatePrivate;
    msg.size   = bytes;
    msg.access = Memory::Present  |
                 Memory::User     |
                 Memory::Readable |
                 Memory::Writable;
    msg.virt   = m_base + m_allocated;
#warning do we need to pass the region here too?
    msg.phys   = ZERO;
    msg.type   = IPCType;
    msg.from   = SELF;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // Clear the pages
    MemoryBlock::set((void *) msg.virt, 0, msg.size);

    // Update count
    m_allocated += msg.size;

    // Success
    *size = msg.size;
    return Success;
}

Allocator::Result PageAllocator::release(Address addr)
{
    // TODO: let the heap shrink if possible
    return InvalidAddress;
}

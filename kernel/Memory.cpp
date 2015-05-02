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

#include <Log.h>
#include <BubbleAllocator.h>
#include <PoolAllocator.h>
#include <System/Constant.h>
#include "Memory.h"

Memory::Memory(Size memorySize)
    : m_physicalMemory(memorySize / PAGESIZE)
{
    NOTICE("memorySize =" << memorySize);
}

Error Memory::initialize(Address heap)
{
    NOTICE("heap =" << heap);

    Size meta = sizeof(BubbleAllocator) + sizeof(PoolAllocator);
    Allocator *bubble, *pool;

    /* Setup the dynamic memory heap. */
    bubble = new (heap) BubbleAllocator();
    pool   = new (heap + sizeof(BubbleAllocator)) PoolAllocator();
    pool->setParent(bubble);

    // TODO: pass the heap size as argument instead of assuming 1MB.

    /* Setup the heap region (1MB). */
    bubble->region(heap + meta, (1024 * 1024) - meta);

    /* Set default allocator. */
    Allocator::setDefault(pool);
    return 0;
}

Size Memory::getTotalMemory()
{
    return m_physicalMemory.size() * PAGESIZE;           
}

Size Memory::getAvailableMemory()
{
    return m_physicalMemory.count(false) * PAGESIZE;
}

Address Memory::allocatePhysical(Size size)
{
    Size num = size / PAGESIZE;

    if ((size % PAGESIZE))
        num++;

    return m_physicalMemory.setNext(num) * PAGESIZE;
}

Address Memory::allocatePhysicalAddress(Address addr)
{
    if (isAllocated(addr))
    {
        FATAL("physical address already allocated: " << addr);
        for (;;);
    }
    m_physicalMemory.set(addr / PAGESIZE);
    return addr;
}

bool Memory::isAllocated(Address page)
{
    return m_physicalMemory.isSet(page / PAGESIZE);
}

Error Memory::releasePhysical(Address page)
{
    m_physicalMemory.unset(page / PAGESIZE);
    return 0;
}

Address Memory::allocate(Address vaddr, MemoryAccess flags)
{
    // Allocate a new physical page
    Address newPage = allocatePhysical(PAGESIZE);

    // Map it to the requested virtual address
    return map(newPage, vaddr, flags);
}

Address Memory::allocate(Process *p, Address vaddr, MemoryAccess flags)
{
    /* Allocate new physical page. */
    Address newPage = allocatePhysical(PAGESIZE);

    /* Map it into the target process. */
    return map(p, newPage, vaddr, flags);
}

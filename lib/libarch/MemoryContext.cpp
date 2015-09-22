/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <FreeNOS/System.h>
#include <SplitAllocator.h>
#include "MemoryContext.h"

MemoryContext * MemoryContext::m_current = 0;

MemoryContext::MemoryContext(MemoryMap *map, SplitAllocator *alloc)
{
    m_alloc = alloc;
    m_map   = map;
}

MemoryContext::~MemoryContext()
{
}

MemoryContext * MemoryContext::getCurrent()
{
    return m_current;
}

MemoryContext::Result MemoryContext::mapRange(Memory::Range *range)
{
    Result r = Success;

#warning should make the allocation responsibility by the caller?
    // Allocate physical pages, if needed.
    if (!range->phys)
        m_alloc->allocate(&range->size, &range->phys);

    // Insert virtual page(s)
    for (Size i = 0; i < range->size; i += PAGESIZE)
    {
        if ((r = map(range->virt + i,
                     range->phys + i,
                     range->access)) != Success)
            break;
    }

    if (r != Success)
    {
        // TODO: unset the physical pages...
    }
    return r;
}

MemoryContext::Result MemoryContext::mapRegion(MemoryMap::Region region,
                                               Size size,
                                               Memory::Access access)
{
#warning Unused function!
    Memory::Range range;
    Result r;

    if ((r = findFree(size, region, &range.virt)) != Success)
        return r;

    range.phys   = ZERO;
    range.size   = size;
    range.access = access;

    return mapRange(&range);
}

MemoryContext::Result MemoryContext::unmapRange(Memory::Range *range)
{
    Result r = Success;

    for (Size i = 0; i < range->size; i += PAGESIZE)
        if ((r = unmap(range->virt + i)) != Success)
            break;

    return r;
}

MemoryContext::Result MemoryContext::release(Address virt)
{
#warning implement release
    return Success;
}

MemoryContext::Result MemoryContext::releaseRegion(MemoryMap::Region region)
{
#warning implement region release
    return Success;
}

MemoryContext::Result MemoryContext::releaseRange(Memory::Range *range)
{
    Address phys;

    for (Size i = 0; i < range->size; i += PAGESIZE)
    {
        if (lookup(range->virt + i, &phys) == Success)
            m_alloc->release(phys);
    }
    return Success;
}

MemoryContext::Result MemoryContext::findFree(Size size, MemoryMap::Region region, Address *virt)
{
    Memory::Range r = m_map->range(region);
    Size currentSize = 0;
    Address addr = r.virt, currentAddr = r.virt, tmp;
    
    while (addr < r.virt+r.size && currentSize < size)
    {
        // TODO: check for success instead. error codes may change.
        if (lookup(addr, &tmp) == InvalidAddress)
        {
            currentSize += PAGESIZE;
        }
        else
        {
            currentSize = 0; 
            currentAddr = addr + PAGESIZE;
        }
        addr += PAGESIZE;
    }

    if (currentSize >= size)
    {
        *virt = currentAddr;
        return Success;
    }
    else
        return OutOfMemory;
}

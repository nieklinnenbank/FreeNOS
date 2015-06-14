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

#include <FreeNOS/API.h>
#include <FreeNOS/Kernel.h>
#include "Memory.h"

Memory::Memory(Address pageDirectory, BitArray *memoryMap)
{
    m_memoryMap = memoryMap;
}

Memory::~Memory()
{
}

Address Memory::mapRange(Range *range)
{
    Address addr = 0;

    // Allocate physical pages, if needed.
    if (!range->phys)
    {
        Size num = range->size / PAGESIZE;
    
        if (range->size % PAGESIZE)
            num++;

        range->phys = m_memoryMap->setNext(num) * PAGESIZE;
    }
    // Find free virtual pages, if not set.
    if (!range->virt)
        range->virt = findFree(range->size);
    
    // Insert virtual page(s)
    for (Size i = 0; i < range->size; i += PAGESIZE)
    {
        Address r = map(range->phys + i,
                        range->virt + i,
                        range->access);

        if (!addr)
            addr = r;
    }
    return addr;
}

void Memory::unmapRange(Range *range)
{
    for (Size i = 0; i < range->size; i += PAGESIZE)
        unmap(range->virt + i);
}

void Memory::releaseRange(Range *range)
{
    for (Size i = 0; i < range->size; i += PAGESIZE)
    {
        // Don't release pinned pages. */
        if (access(range->virt + i, PAGESIZE, Pinned))
            continue;

        m_memoryMap->unset(lookup(range->virt + i) / PAGESIZE);
    }
}

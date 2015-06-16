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
#include <BitAllocator.h>
#include "Memory.h"

Memory::Memory(Address pageDirectory, BitAllocator *phys)
{
    m_phys = phys;

    // Ask the kernel for the physical memory allocator, if needed
    if (!isKernel && !m_phys)
    {
        SystemInformation info;
        m_phys = info.memoryAllocator;
    }
}

Memory::~Memory()
{
}

Address Memory::mapRange(Range *range)
{
    Address addr = 0;

    // Allocate physical pages, if needed.
    if (!range->phys)
        m_phys->allocate(&range->size, &range->phys);

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

        m_phys->release(lookup(range->virt + i));
    }
}

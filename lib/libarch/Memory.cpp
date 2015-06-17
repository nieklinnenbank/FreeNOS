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
#include <Log.h>
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
    // Must have virtual address
    if (!range->virt)
    {
        FATAL("invalid ZERO virtual address");
        return 0;
    }

    // Allocate physical pages, if needed.
    if (!range->phys)
        m_phys->allocate(&range->size, &range->phys);

    // Insert virtual page(s)
    for (Size i = 0; i < range->size; i += PAGESIZE)
    {
        Address r = map(range->phys + i,
                        range->virt + i,
                        range->access);
    }
    return range->virt;
}

Address Memory::mapRegion(Memory::Region region, Size size, Access access)
{
    Memory::Range range;

    range.virt   = findFree(size, region);
    range.phys   = ZERO;
    range.size   = size;
    range.access = access;

    return mapRange(&range);
}

void Memory::unmapRange(Range *range)
{
    for (Size i = 0; i < range->size; i += PAGESIZE)
        unmap(range->virt + i);
}

void Memory::releaseRange(Range *range)
{
    for (Size i = 0; i < range->size; i += PAGESIZE)
        m_phys->release(lookup(range->virt + i));
}

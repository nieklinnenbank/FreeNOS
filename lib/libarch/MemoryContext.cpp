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

#pragma clang optimize off
#pragma GCC push_options
#pragma GCC optimize ("O0")

#include <FreeNOS/System.h>
#include <SplitAllocator.h>
#include "MemoryContext.h"

MemoryContext * MemoryContext::m_current = 0;

MemoryContext::MemoryContext(MemoryMap *map, SplitAllocator *alloc)
    : m_alloc(alloc)
    , m_map(map)
    , m_mapRangeSparseCallback(this, &MemoryContext::mapRangeSparseCallback)
    , m_savedRange(ZERO)
    , m_numSparsePages(ZERO)
{
}

MemoryContext::~MemoryContext()
{
}

MemoryContext * MemoryContext::getCurrent()
{
    return m_current;
}

MemoryContext::Result MemoryContext::mapRangeContiguous(Memory::Range *range)
{
    Result r = Success;

    // Allocate a block of contiguous physical pages, if needed.
    if (!range->phys)
    {
        Allocator::Range alloc_args;
        alloc_args.address = 0;
        alloc_args.size = range->size;
        alloc_args.alignment = PAGESIZE;

        if (m_alloc->allocate(alloc_args) != Allocator::Success)
            return OutOfMemory;

        range->phys = alloc_args.address;
    }

    // Insert virtual page(s)
    for (Size i = 0; i < range->size; i += PAGESIZE)
    {
        if ((r = map(range->virt + i,
                     range->phys + i,
                     range->access)) != Success)
            break;
    }

    return r;
}

MemoryContext::Result MemoryContext::mapRangeSparse(Memory::Range *range)
{
    Allocator::Range alloc_args;

    // Allocate a set of physical pages (non-contiguous)
    m_savedRange = range;
    m_numSparsePages = 0;

    alloc_args.address = 0;
    alloc_args.size = range->size;
    alloc_args.alignment = 0;

    // This invokes our callback for each new page that is allocated
    if (m_alloc->allocateSparse(alloc_args, &m_mapRangeSparseCallback) != Allocator::Success)
        return OutOfMemory;

    return Success;
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
    Address phys;
    Result result = lookup(virt, &phys);

    if (result == Success)
        m_alloc->release(phys);

    return result;
}

MemoryContext::Result MemoryContext::findFree(Size size, MemoryMap::Region region, Address *virt) const
{
    Memory::Range r = m_map->range(region);
    Size currentSize = 0;
    Address addr = r.virt, currentAddr = r.virt, tmp;

    while (addr < r.virt+r.size && currentSize < size)
    {
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

void MemoryContext::mapRangeSparseCallback(Address *phys)
{
    Result r = Success;

    for (Size i = 0; i < 8U * PAGESIZE; i += PAGESIZE)
    {
        r = map(m_savedRange->virt + m_numSparsePages, (*phys) + i, m_savedRange->access);
        if (r != Success)
            break;

        m_numSparsePages += PAGESIZE;
    }

    assert(r == Success);
}

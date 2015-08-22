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

#include <SplitAllocator.h>
#include <MemoryBlock.h>
#include "IntelCore.h"
#include "IntelPaging.h"

//
// SplitAllocator: physical allocator which defines two regions: low and high.
//                 it can allocate from either low or high memory, returning physical addresses, aligned e.g. on PAGESIZE.
//
// MemoryMap:      defines the virtual memory layout, defines which regions belong to the kernel,
//                 which belong to the user.
//

IntelPaging::IntelPaging(MemoryMap *map, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
{
    IntelCore core;

    // Allocate page directory from low physical memory.
    if (alloc->allocateLow(sizeof(IntelPageDirectory),
                          &m_pageDirectoryAddr) == Allocator::Success)
    {
        m_pageDirectory = (IntelPageDirectory *) alloc->toVirtual(m_pageDirectoryAddr);

        // Initialize the page directory
        MemoryBlock::set(m_pageDirectory, 0, sizeof(IntelPageDirectory));

        // Lookup the currently active page directory
        IntelPageDirectory *currentDirectory =
            (IntelPageDirectory *) alloc->toVirtual(core.readCR3());

        // Inherit kernel mappings. The kernel has permanently mapped 1GB of
        // physical memory (i.e. the "low memory" in SplitAllocator). The low
        // memory starts at its physical base address offset (varies per core).
        Memory::Range kdata = m_map->range(MemoryMap::KernelData);
        m_pageDirectory->copy(currentDirectory,
                              kdata.virt,
                              kdata.virt + kdata.size);

        // Also inherit kernel private mappings, such as APIC mappings.
        kdata = m_map->range(MemoryMap::KernelPrivate);
        m_pageDirectory->copy(currentDirectory,
                              kdata.virt,
                              kdata.virt + kdata.size);
    }
}

IntelPaging::IntelPaging(MemoryMap *map, Address pageDirectory, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
{
    m_pageDirectory     = (IntelPageDirectory *) alloc->toVirtual(pageDirectory);
    m_pageDirectoryAddr = pageDirectory;
}

IntelPaging::~IntelPaging()
{
    // TODO: release physical memory for this context?
}

MemoryContext::Result IntelPaging::activate()
{
    // TODO: perhaps activate paging here, and perform kernel mapping (move it from IntelBoot.S)
    // That way it is also more similar to the ARM implementation.
    IntelCore core;
    core.writeCR3(m_pageDirectoryAddr);
    m_current = this;
    return Success;
}

MemoryContext::Result IntelPaging::map(Address virt, Address phys, Memory::Access acc)
{
    MemoryContext::Result r = m_pageDirectory->map(virt, phys, acc, m_alloc);

    // TODO: make TLB flushing conditional? (only if the context is currently activated)
    if (r == Success)
        tlb_flush(virt);

    return r;
}

MemoryContext::Result IntelPaging::unmap(Address virt)
{
    MemoryContext::Result r = m_pageDirectory->unmap(virt, m_alloc);

    // TODO: make TLB flushing conditional? (only if the context is currently activated)
    if (r == Success)
        tlb_flush(virt);

    return r;
}

MemoryContext::Result IntelPaging::lookup(Address virt, Address *phys)
{
    return m_pageDirectory->translate(virt, phys, m_alloc);
}

MemoryContext::Result IntelPaging::access(Address virt, Memory::Access *access)
{
    return m_pageDirectory->access(virt, access, m_alloc);
}

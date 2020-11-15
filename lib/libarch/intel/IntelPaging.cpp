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

IntelPaging::IntelPaging(MemoryMap *map, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
    , m_pageDirectory(0)
    , m_pageDirectoryAddr(0)
    , m_pageDirectoryAllocated(false)
{
}

IntelPaging::IntelPaging(MemoryMap *map, Address pageDirectory, SplitAllocator *alloc)
    : MemoryContext(map, alloc)
    , m_pageDirectory((IntelPageDirectory *) alloc->toVirtual(pageDirectory))
    , m_pageDirectoryAddr(pageDirectory)
    , m_pageDirectoryAllocated(false)
{
}

IntelPaging::~IntelPaging()
{
    if (m_pageDirectoryAllocated)
    {
        m_alloc->release(m_pageDirectoryAddr);
    }
}

MemoryContext::Result IntelPaging::initialize()
{
    if (m_pageDirectoryAddr != 0)
    {
        return MemoryContext::Success;
    }

    IntelCore core;
    Allocator::Range phys, virt;

    phys.address = 0;
    phys.size = sizeof(IntelPageDirectory);
    phys.alignment = sizeof(IntelPageDirectory);

    // Allocate page directory from low physical memory.
    if (m_alloc->allocate(phys, virt) != Allocator::Success)
    {
        return MemoryContext::OutOfMemory;
    }

    m_pageDirectoryAllocated = true;
    m_pageDirectoryAddr = phys.address;
    m_pageDirectory = (IntelPageDirectory *) virt.address;

    // Initialize the page directory
    MemoryBlock::set(m_pageDirectory, 0, sizeof(IntelPageDirectory));

    // Lookup the currently active page directory
    IntelPageDirectory *currentDirectory =
        (IntelPageDirectory *) m_alloc->toVirtual(core.readCR3());

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

    return MemoryContext::Success;
}

MemoryContext::Result IntelPaging::activate(bool initializeMMU)
{
    IntelCore core;
    core.writeCR3(m_pageDirectoryAddr);
    m_current = this;
    return Success;
}

MemoryContext::Result IntelPaging::map(Address virt, Address phys, Memory::Access acc)
{
    MemoryContext::Result r = m_pageDirectory->map(virt, phys, acc, m_alloc);

    // Flush TLB entry
    if (r == Success && m_current == this)
        tlb_flush(virt);

    return r;
}

MemoryContext::Result IntelPaging::unmap(Address virt)
{
    MemoryContext::Result r = m_pageDirectory->unmap(virt, m_alloc);

    // Flush TLB entry
    if (r == Success && m_current == this)
        tlb_flush(virt);

    return r;
}

MemoryContext::Result IntelPaging::lookup(Address virt, Address *phys) const
{
    return m_pageDirectory->translate(virt, phys, m_alloc);
}

MemoryContext::Result IntelPaging::access(Address virt, Memory::Access *access) const
{
    return m_pageDirectory->access(virt, access, m_alloc);
}

MemoryContext::Result IntelPaging::releaseSection(const Memory::Range & range,
                                                  const bool tablesOnly)
{
    return m_pageDirectory->releaseSection(range, m_alloc, tablesOnly);
}

MemoryContext::Result IntelPaging::releaseRange(Memory::Range *range)
{
    return m_pageDirectory->releaseRange(*range, m_alloc);
}

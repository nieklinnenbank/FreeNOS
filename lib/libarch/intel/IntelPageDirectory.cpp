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
#include "IntelConstant.h"
#include "IntelPageDirectory.h"

#define PAGE_NONE       0
#define PAGE_PRESENT    1
#define PAGE_READ       0
#define PAGE_EXEC       0
#define PAGE_WRITE      2
#define PAGE_USER       4
#define PAGE_SECTION    (1 << 7)

/**
 * Entry inside the page directory of a given virtual address.
 *
 * @param vaddr Virtual Address.
 *
 * @return Index of the corresponding page directory entry.
 */
#define DIRENTRY(vaddr) \
    ((vaddr) >> DIRSHIFT)

IntelPageTable * IntelPageDirectory::getPageTable(Address virt, SplitAllocator *alloc) const
{
    u32 entry = m_tables[ DIRENTRY(virt) ];

    // Check if the page table is present.
    if (!(entry & PAGE_PRESENT))
        return ZERO;
    else
        return (IntelPageTable *) alloc->toVirtual(entry & PAGEMASK);
}

MemoryContext::Result IntelPageDirectory::copy(IntelPageDirectory *directory,
                                               Address from,
                                               Address to)
{
    while (from < to)
    {
        m_tables[ DIRENTRY(from) ] = directory->m_tables[ DIRENTRY(from) ];
        from += MegaByte(4);
    }
    return MemoryContext::Success;
}

MemoryContext::Result IntelPageDirectory::map(Address virt,
                                              Address phys,
                                              Memory::Access access,
                                              SplitAllocator *alloc)
{
    IntelPageTable *table = getPageTable(virt, alloc);
    Allocator::Range allocPhys, allocVirt;

    // Check if the page table is present.
    if (!table)
    {
        allocPhys.address = 0;
        allocPhys.size = sizeof(IntelPageTable);
        allocPhys.alignment = PAGESIZE;

        // Allocate a new page table
        if (alloc->allocate(allocPhys, allocVirt) != Allocator::Success)
            return MemoryContext::OutOfMemory;

        MemoryBlock::set((void *)allocVirt.address, 0, sizeof(IntelPageTable));

        // Assign to the page directory
        m_tables[ DIRENTRY(virt) ] = allocPhys.address | PAGE_PRESENT | PAGE_WRITE | flags(access);
        table = getPageTable(virt, alloc);
    }
    return table->map(virt, phys, access);
}

MemoryContext::Result IntelPageDirectory::unmap(Address virt, SplitAllocator *alloc)
{
    IntelPageTable *table = getPageTable(virt, alloc);
    if (!table)
        return MemoryContext::InvalidAddress;
    else
        return table->unmap(virt);
}

MemoryContext::Result IntelPageDirectory::translate(Address virt,
                                                    Address *phys,
                                                    SplitAllocator *alloc) const
{
    IntelPageTable *table = getPageTable(virt, alloc);
    if (!table)
    {
        if (m_tables[DIRENTRY(virt)] & PAGE_SECTION)
        {
            const Address offsetInSection = virt % MegaByte(4);

            *phys = (m_tables[DIRENTRY(virt)] & SECTIONMASK) +
                    ((offsetInSection / PAGESIZE) * PAGESIZE);
            return MemoryContext::Success;
        }
        return MemoryContext::InvalidAddress;
    }
    else
        return table->translate(virt, phys);
}

MemoryContext::Result IntelPageDirectory::access(Address virt,
                                                 Memory::Access *access,
                                                 SplitAllocator *alloc) const
{
    IntelPageTable *table = getPageTable(virt, alloc);
    if (!table)
        return MemoryContext::InvalidAddress;
    else
        return table->access(virt, access);
}

u32 IntelPageDirectory::flags(Memory::Access access) const
{
    u32 f = 0;

    if (access & Memory::Writable) f |= PAGE_WRITE;
    if (access & Memory::User)     f |= PAGE_USER;

    return f;
}

inline void IntelPageDirectory::releasePhysical(SplitAllocator *alloc,
                                                const Address phys)
{
    // Some pages that are part of the boot core's memory region
    // are mapped on secondary cores. They can't be released there.
    const Address allocBase = alloc->base();
    const Size allocSize = alloc->size();
    if (phys < allocBase || phys > allocBase + allocSize)
    {
        return;
    }

    // Note that some pages may have double mappings.
    // Avoid attempting to release the same page twice or more.
    if (alloc->isAllocated(phys))
    {
        alloc->release(phys);
    }
}

MemoryContext::Result IntelPageDirectory::releaseRange(const Memory::Range range,
                                                       SplitAllocator *alloc)
{
    Address phys;

    // Walk the full range of memory specified
    for (Size addr = range.virt; addr < range.virt + range.size; addr += PAGESIZE)
    {
        IntelPageTable *table = getPageTable(addr, alloc);
        if (table == ZERO)
        {
            return MemoryContext::InvalidAddress;
        }

        if (table->translate(addr, &phys) != MemoryContext::Success)
        {
            return MemoryContext::InvalidAddress;
        }

        releasePhysical(alloc, phys);
        table->unmap(addr);
    }

    return MemoryContext::Success;
}

MemoryContext::Result IntelPageDirectory::releaseSection(const Memory::Range range,
                                                         SplitAllocator *alloc,
                                                         const bool tablesOnly)
{
    Address phys;

    // Input must be aligned to section address
    if (range.virt & ~SECTIONMASK)
    {
        return MemoryContext::InvalidAddress;
    }

    // Walk the page directory
    for (Size addr = range.virt; addr < range.virt + range.size; addr += MegaByte(4))
    {
        IntelPageTable *table = getPageTable(addr, alloc);
        if (!table)
        {
            continue;
        }

        // Release mapped pages, if requested
        if (!tablesOnly)
        {
            for (Size i = 0; i < MegaByte(4); i += PAGESIZE)
            {
                if (table->translate(i, &phys) == MemoryContext::Success)
                {
                    releasePhysical(alloc, phys);
                }
            }
        }
        // Release page table
        alloc->release(m_tables[ DIRENTRY(addr) ] & PAGEMASK);
        m_tables[ DIRENTRY(addr) ] = 0;
    }

    return MemoryContext::Success;
}

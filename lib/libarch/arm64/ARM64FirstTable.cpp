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
#include <MemoryBlock.h>
#include "ARM64Constant.h"
#include "ARM64FirstTable.h"


extern C void uart_hex_u64(unsigned long d);
extern C void uart_hex(unsigned int d);
extern C void uart_puts(char *s);

void ARM64FirstTable::initialize(ARM64FirstTable *firstTable)
{
    for (Address virt = 0; virt < GigaByte(4UL); virt += L1_BLOCK_SIZE) {
        unsigned int l1_idx = L1_IDX(virt);
        u64 tbl = (u64) &(firstTable->m_tables_l2[l1_idx][0]);
        firstTable->m_tables_l1[l1_idx] = tbl | PT_PAGE;
    }
}

u64 ARM64FirstTable::get_l2_entry(Address virt, SplitAllocator *alloc, u64 **tbl_l2, unsigned int *l2_idx) const
{
    unsigned int l1_idx = L1_IDX(virt);

    assert(l1_idx < 4); // virtual address must be lower than 4GB
    u64 entry_l1 = m_tables_l1[l1_idx];

    // already mapped as table while initialization
    //if (!IS_PT_PAGE_TBL(entry_l1))
    //    return ZERO;

    *tbl_l2 = (u64 *)alloc->toVirtual(entry_l1 & PAGEMASK);
    *l2_idx = L2_IDX(virt);

    return (*tbl_l2)[*l2_idx];
}

/**
 * @}
 */

/**
 * Entry inside the page directory of a given virtual address.
 *
 * @param vaddr Virtual Address.
 *
 * @return Index of the corresponding page directory entry.
 */
ARM64SecondTable * ARM64FirstTable::getSecondTable(Address virt, SplitAllocator *alloc) const
{

    u64 *tbl_l2;
    unsigned int l2_idx;

    u64 entry_l2 = get_l2_entry(virt, alloc, &tbl_l2, &l2_idx);

    // Check if the page table is present.
    if (!IS_PT_PAGE_TBL(entry_l2))
        return ZERO;
    else
        return (ARM64SecondTable *) alloc->toVirtual(entry_l2 & PAGEMASK);
}

MemoryContext::Result ARM64FirstTable::map(Address virt,
                                         Address phys,
                                         Memory::Access access,
                                         SplitAllocator *alloc)
{
    ARM64SecondTable *table = getSecondTable(virt, alloc);
    Arch::Cache cache;
    Allocator::Range allocPhys, allocVirt;

    // Check if the page table is present.
    if (!table)
    {
        u64 *tbl_l2;
        unsigned int l2_idx;

        u64 entry_l2 = get_l2_entry(virt, alloc, &tbl_l2, &l2_idx);

        // Reject if already mapped as a (super)section
        if (IS_PT_BLOCK(entry_l2))
            return MemoryContext::AlreadyExists;

        // Allocate a new page table
        allocPhys.address = 0;
        allocPhys.size = sizeof(ARM64SecondTable);
        allocPhys.alignment = PAGESIZE;

        if (alloc->allocate(allocPhys, allocVirt) != Allocator::Success)
            return MemoryContext::OutOfMemory;

        MemoryBlock::set((void *)allocVirt.address, 0, PAGESIZE);

        // Assign to the page directory. Do not assign permission flags (only for direct sections).
        tbl_l2[l2_idx] = allocPhys.address | PT_PAGE;
        //cache.cleanData(&m_tables[DIRENTRY(virt)]);
        table = getSecondTable(virt, alloc);
    }
    return table->map(virt, phys, access);
}

MemoryContext::Result ARM64FirstTable::mapLarge(Memory::Range range,
                                              SplitAllocator *alloc)
{
    if (range.size & L2_BLOCK_RANGE)
        return MemoryContext::InvalidSize;

    if ((range.phys & ~L2_BLOCK_MASK) || (range.virt & ~L2_BLOCK_MASK))
        return MemoryContext::InvalidAddress;

    for (Address i = 0; i < range.size; i += L2_BLOCK_SIZE)
    {
        u64 *tbl_l2;
        unsigned int l2_idx;

        u64 entry_l2 = get_l2_entry(range.virt + i, alloc, &tbl_l2, &l2_idx);

        if (IS_PT_BLOCK(entry_l2))
            return MemoryContext::AlreadyExists;

        u64 val  = (range.phys + i) | PT_BLOCK | flags(range.access);
        tbl_l2[l2_idx] = val;
    }
    return MemoryContext::Success;
}

MemoryContext::Result ARM64FirstTable::unmap(Address virt, SplitAllocator *alloc)
{
    ARM64SecondTable *table = getSecondTable(virt, alloc);

    if (!table)
    {
        u64 *tbl_l2;
        unsigned int l2_idx;

        u64 entry_l2 = get_l2_entry(virt, alloc, &tbl_l2, &l2_idx);
        if (IS_PT_BLOCK(entry_l2))
        {
            tbl_l2[l2_idx] = PT_NONE;
            return MemoryContext::Success;
        }
        else
            return MemoryContext::InvalidAddress;
    }
    else
        return table->unmap(virt);
}

MemoryContext::Result ARM64FirstTable::translate(Address virt,
                                               Address *phys,
                                               SplitAllocator *alloc) const
{
    ARM64SecondTable *table = getSecondTable(virt, alloc);
    if (!table)
    {
        u64 *tbl_l2;
        unsigned int l2_idx;

        u64 entry_l2 = get_l2_entry(virt, alloc, &tbl_l2, &l2_idx);
        if (IS_PT_BLOCK(entry_l2))
        {
            const Address offsetInSection = virt % L2_BLOCK_SIZE;

            *phys = (entry_l2 & L2_BLOCK_RANGE) +
                    ((offsetInSection / PAGESIZE) * PAGESIZE);
            return MemoryContext::Success;
        }
        return MemoryContext::InvalidAddress;
    }
    else
        return table->translate(virt, phys);
}

MemoryContext::Result ARM64FirstTable::access(Address virt,
                                            Memory::Access *access,
                                            SplitAllocator *alloc) const
{
    ARM64SecondTable *table = getSecondTable(virt, alloc);
    if (!table)
        return MemoryContext::InvalidAddress;
    else
        return table->access(virt, access);
}

u32 ARM64FirstTable::flags(Memory::Access access) const
{
    u64 f = PT_KERNEL | PT_AF;

    // Permissions
    if (!(access & Memory::Executable)) f |= PT_NX;
    if ((access & Memory::User))        f |= PT_USER;
    if (!(access & Memory::Writable))   f |= PT_RO;

    // Cache
    if (access & Memory::Device)
        f |= PT_OSH | PT_DEV;
    else
        f |= PT_ISH | PT_NC; /* FIXME: Is PT_OSH appropriate? */
    return f;
}

inline void ARM64FirstTable::releasePhysical(SplitAllocator *alloc,
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

MemoryContext::Result ARM64FirstTable::releaseRange(const Memory::Range range,
                                                  SplitAllocator *alloc)
{
    Address phys;

    // Walk the full range of memory specified
    for (Address addr = range.virt; addr < range.virt + range.size; addr += PAGESIZE)
    {
        ARM64SecondTable *table = getSecondTable(addr, alloc);
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

MemoryContext::Result ARM64FirstTable::releaseSection(const Memory::Range range,
                                                    SplitAllocator *alloc,
                                                    const bool tablesOnly)
{
    Address phys;

    // Input must be aligned to section address
    if (range.virt & ~L2_BLOCK_MASK)
    {
        return MemoryContext::InvalidAddress;
    }

    // Walk the page directory
    for (Address addr = range.virt; addr < range.virt + range.size; addr += L2_BLOCK_SIZE)
    {
        ARM64SecondTable *table = getSecondTable(addr, alloc);
        if (!table)
        {
            continue;
        }
        //else, what about large 2MB block?

        // Release mapped pages, if requested
        if (!tablesOnly)
        {
            for (Address i = 0; i < L2_BLOCK_SIZE; i += PAGESIZE)
            {
                if (table->translate(i, &phys) == MemoryContext::Success)
                {
                    releasePhysical(alloc, phys);
                }
            }
        }
        // Release page table
        u64 *tbl_l2;
        unsigned int l2_idx;

        u64 entry_l2 = get_l2_entry(addr, alloc, &tbl_l2, &l2_idx);

        alloc->release(entry_l2 & PAGEMASK);
        tbl_l2[l2_idx] = 0;
    }

    return MemoryContext::Success;
}

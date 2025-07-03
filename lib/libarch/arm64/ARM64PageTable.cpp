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
#if 0
#include "ARM64Core.h"
#include "ARM64Constant.h"
#endif
#include "ARM64PageTable.h"

// descriptor types
#define PT_NONE         0
#define PT_PAGE         0b11        //page/table descriptor
#define PT_BLOCK        0b01        //block descriptor
#define PT_TYPE_MASK    0b11
#define GET_PT_TYPE(entry)          ((entry) & PT_TYPE_MASK)
#define IS_PT_PAGE_TBL(entry)       (GET_PT_TYPE(entry) == PT_PAGE)
#define IS_PT_BLOCK(entry)          (GET_PT_TYPE(entry) == PT_BLOCK)

// accessibility
#define PT_KERNEL       (0<<6)      // privileged, supervisor EL1 access only (default)
#define PT_USER         (1<<6)      // unprivileged, EL0 access allowed
#define PT_RW           (0<<7)      // read-write (default)
#define PT_RO           (1<<7)      // read-only
#define PT_AF           (1<<10)     // access flag
#define PT_NX           (1UL<<54)   // no execute

// shareability
#define PT_OSH          (2<<8)      // outter shareable
#define PT_ISH          (3<<8)      // inner shareable

// define in MAIR register
#define PT_MEM          (0<<2)      // normal memory
#define PT_DEV          (1<<2)      // device MMIO
#define PT_NC           (2<<2)      // non-cachable

#define TTBR_CNP        1

extern C void uart_hex_u64(unsigned long d);
extern C void uart_hex(unsigned int d);
extern C void uart_puts(char *s);

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
static inline u32 get_entry_idx(Address vaddr, u8 level)
{
    switch (level) {
        case 1:
            return ((vaddr) >> L1_DIRSHIFT) & DIR_MASK;
        case 2:
            return ((vaddr) >> L2_DIRSHIFT) & DIR_MASK;
        case 3:
            return ((vaddr) >> L3_DIRSHIFT) & DIR_MASK;
    }
    // never reach
    return 0;
}

static inline Address get_blk_addr_from_entry(u64 entry, u8 level)
{
    switch (level) {
        case 1:
            return entry & L1_BLOCK_MASK;
        case 2:
            return entry & L2_BLOCK_MASK;
    }
    // never reach
    return 0;
}

#define get_page_tbl_addr_from_entry(entry) \
    ((entry) & PAGEMASK)

static inline u64 get_block_range(u8 level)
{
    switch (level) {
        case 1:
            return L1_BLOCK_RANGE;
        case 2:
            return L2_BLOCK_RANGE;
    }
    // never reach
    return 0;
}

static inline u64 get_block_size(u8 level)
{
    switch (level) {
        case 1:
            return L1_BLOCK_SIZE;
        case 2:
            return L2_BLOCK_SIZE;
    }
    // never reach
    return 0;
}

static void flags2Access(u64 entry);

ARM64PageTable * ARM64PageTable::getNextTable(Address virt, SplitAllocator *alloc) const
{
    u64 entry = m_tables[get_entry_idx(virt, m_level)];

    // Check if the page table is present.
    if (m_level == 3 || !IS_PT_PAGE_TBL(entry))
        return ZERO;
    else if (alloc == ZERO)
        return (ARM64PageTable *) get_page_tbl_addr_from_entry(entry);
    else
        return (ARM64PageTable *) alloc->toVirtual(get_page_tbl_addr_from_entry(entry));
}

void ARM64PageTable::setNextTable(Address virt, Address tbl, SplitAllocator *alloc)
{
    m_tables[get_entry_idx(virt, m_level)] = tbl | PT_PAGE;
}

/* TODO: check again */
MemoryContext::Result ARM64PageTable::map(Address virt,
                                         Address phys,
                                         Memory::Access access,
                                         SplitAllocator *alloc)
{
#if 0
    Arch::Cache cache;
#endif
    if (m_level == 3) {
        u32 idx = get_entry_idx(virt, m_level);

        u64 entry = m_tables[idx];
        if (IS_PT_PAGE_TBL(entry))
            return MemoryContext::AlreadyExists;

        u64 val  = phys | PT_PAGE | flags(access);

        m_tables[idx] = val;
        //TODO: clean cache
        //cache.cleanData(&m_pages[TABENTRY(virt)]);
        return MemoryContext::Success;
    }

    ARM64PageTable *table = getNextTable(virt, alloc);
    Allocator::Range allocPhys, allocVirt;

    // Check if the page table is present.
    if (!table)
    {
        // Reject if already mapped as a (super)section
        u64 entry = m_tables[get_entry_idx(virt, m_level)];
        if (IS_PT_BLOCK(entry))
            return MemoryContext::AlreadyExists;

        // Allocate a new page table
        allocPhys.address = 0;
        allocPhys.size = sizeof(ARM64PageTable);
        allocPhys.alignment = PAGESIZE;

        if (alloc->allocate(allocPhys, allocVirt) != Allocator::Success)
            return MemoryContext::OutOfMemory;

        MemoryBlock::set((void *)allocVirt.address, 0, PAGESIZE);

        // Assign to the page directory. Do not assign permission flags (only for direct sections).
        m_tables[get_entry_idx(virt, m_level)] = allocPhys.address | PT_PAGE;
#if 0
        cache.cleanData(&m_tables[get_entry_idx(virt)]);
#endif
        table = getNextTable(virt, alloc);
        table->m_level = m_level + 1;
    }
    return table->map(virt, phys, access, alloc);
}

MemoryContext::Result ARM64PageTable::mapBlock2(Memory::Range range,
                                              SplitAllocator *alloc)
{
#if 0
    Arch::Cache cache;
#endif
    if (m_level != 1)
        return MemoryContext::InvaildArgs;

#if MMU_DEBUG
    uart_puts("Table base: [");
    uart_hex_u64((u64)&m_tables[0]);
    uart_puts("]\nVirt: ");
    uart_hex_u64(range.virt);
    uart_puts("\nPhys: ");
    uart_hex_u64(range.phys);
    uart_puts("\nLevel: ");
    uart_hex(m_level);
    uart_puts("\nSize: ");
    uart_hex_u64(range.size);
    uart_puts("\n");
#endif
    Size blk_size = get_block_size(m_level+1);
    for (Size i = 0; i < range.size; i += blk_size)
    {

        ARM64PageTable *tbl = getNextTable(range.virt + i, alloc);

        if (!tbl) {
            //FIXME: for alloc table for second level page table
        }

        Memory::Range blk = {
            range.virt + i,
            range.phys + i,
            blk_size,
            range.access,
        };
        tbl->mapBlock(blk, alloc);
#if MMU_DEBUG
        uart_puts("m_tables[0x");
        uart_hex(idx);
        uart_puts("] = 0x");
        uart_hex_u64(val);
        uart_puts("\n");
#endif
#if 0
        cache.cleanData(&m_tables[get_entry_idx(range.virt + i)]);
#endif
    }
    return MemoryContext::Success;
}
MemoryContext::Result ARM64PageTable::mapBlock(Memory::Range range,
                                              SplitAllocator *alloc)
{
#if 0
    Arch::Cache cache;
#endif

    if (range.size & get_block_range(m_level))
        return MemoryContext::InvalidSize;

    if ((range.phys & 0xfff) || (range.virt & 0xfff))
        return MemoryContext::InvalidAddress;
#if MMU_DEBUG
    uart_puts("Table base: [");
    uart_hex_u64((u64)&m_tables[0]);
    uart_puts("]\nVirt: ");
    uart_hex_u64(range.virt);
    uart_puts("\nPhys: ");
    uart_hex_u64(range.phys);
    uart_puts("\nLevel: ");
    uart_hex(m_level);
    uart_puts("\nSize: ");
    uart_hex_u64(range.size);
    uart_puts("\n");
#endif
    for (Size i = 0; i < range.size; i += get_block_size(m_level))
    {

        u32 idx = get_entry_idx(range.virt + i, m_level);

        u64 entry = m_tables[idx];
        if (IS_PT_BLOCK(entry))
            return MemoryContext::AlreadyExists;

        u64 val  = (range.phys + i) | PT_BLOCK | flags(range.access);

        m_tables[idx] = val;
#if MMU_DEBUG
        uart_puts("m_tables[0x");
        uart_hex(idx);
        uart_puts("] = 0x");
        uart_hex_u64(val);
        uart_puts("\n");
#endif
#if 0
        cache.cleanData(&m_tables[get_entry_idx(range.virt + i)]);
#endif
    }
    return MemoryContext::Success;
}

/* TODO: check again */
MemoryContext::Result ARM64PageTable::unmap(Address virt, SplitAllocator *alloc)
{
#if 0
    Arch::Cache cache;
#endif
    if (m_level == 3) {
        u32 idx = get_entry_idx(virt, m_level);
        //u64 entry = m_tables[idx];
        m_tables[idx] = PT_NONE;
        //TODO: clean cache
        //cache.cleanData(&m_tables[idx]);
        return MemoryContext::Success;
    }

    ARM64PageTable *table = getNextTable(virt, alloc);

    if (!table)
    {
        u64 entry = m_tables[get_entry_idx(virt, m_level)];
        if (IS_PT_BLOCK(entry))
        {
            m_tables[get_entry_idx(virt, m_level)] = PT_NONE;
#if 0
            cache.cleanData(&m_tables[get_entry_idx(virt)]);
#endif
            return MemoryContext::Success;
        }
        else
            return MemoryContext::InvalidAddress;
    }
    else
        return table->unmap(virt, alloc);
}

/* TODO: Check again */
MemoryContext::Result ARM64PageTable::translate(Address virt,
                                               Address *phys,
                                               SplitAllocator *alloc) const
{
    if (m_level == 3) {
        u32 idx = get_entry_idx(virt, m_level);

        u64 entry = m_tables[idx];
        if (!IS_PT_PAGE_TBL(entry))
            return MemoryContext::InvalidAddress;

        *phys = get_page_tbl_addr_from_entry(entry);
        return MemoryContext::Success;
    }

    ARM64PageTable *table = getNextTable(virt, alloc);
    if (!table)
    {
        u32 idx = get_entry_idx(virt, m_level);

        u64 entry = m_tables[idx];
        if (IS_PT_BLOCK(entry))
        {
            const Address offsetInBlock = virt % get_block_size(m_level);

            //FIXME: why round down? 
            *phys = (entry & get_block_range(m_level)) +
                    ((offsetInBlock / PAGESIZE) * PAGESIZE);
            return MemoryContext::Success;
        }
        return MemoryContext::InvalidAddress;
    }
    else
        return table->translate(virt, phys, alloc);
}

MemoryContext::Result ARM64PageTable::access(Address virt,
                                            Memory::Access *access,
                                            SplitAllocator *alloc) const
{
    if (m_level == 3) {
        u32 idx = get_entry_idx(virt, m_level);

        u64 entry = m_tables[idx];
        if (!IS_PT_PAGE_TBL(entry))
            return MemoryContext::InvalidAddress;
        // Permissions
        *access = Memory::Readable;

        if (contain_flags(entry, PT_USER))
            *access |= Memory::User;

        if (!(contain_flags(entry, PT_RO)))
            *access |= Memory::Writable;

        // Caching
        if (contain_flags(entry, (PT_OSH | PT_DEV)))
            *access |= Memory::Device;
        else if (contain_flags(entry, PT_OSH | PT_NC))
            *access |= Memory::Uncached;
        else
            *access |= Memory::InnerCached;

        return MemoryContext::Success;
    }

    ARM64PageTable *table = getNextTable(virt, alloc);
    if (!table)
        return MemoryContext::InvalidAddress;
    else
        return table->access(virt, access, alloc);
}

u32 ARM64PageTable::flags(Memory::Access access) const
{
    u64 f = PT_KERNEL | PT_AF;

    // Permissions
    if (!(access & Memory::Executable)) f |= PT_NX;
    if ((access & Memory::User))        f |= PT_USER;
    if (!(access & Memory::Writable))   f |= PT_RO;

    // Cache
    if (access & Memory::Device)
        f |= PT_OSH | PT_DEV;
    else if (access & Memory::Uncached)
        f |= PT_OSH | PT_NC; /* FIXME: Is PT_OSH appropriate? */
    else
        f |= PT_ISH | PT_MEM;

    return f;
}

static void flags2Access(u64 entry)
{
}

/* TODO: Check again */
inline void ARM64PageTable::releasePhysical(SplitAllocator *alloc,
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

/* TODO: Check again */
MemoryContext::Result ARM64PageTable::releaseRange(const Memory::Range range,
                                                  SplitAllocator *alloc)
{
    Address phys;

    // Walk the full range of memory specified
    for (Size addr = range.virt; addr < range.virt + range.size; addr += PAGESIZE)
    {
        ARM64PageTable *table = getNextTable(addr, alloc);
        if (table == ZERO)
        {
            return MemoryContext::InvalidAddress;
        }

        if (table->translate(addr, &phys, alloc) != MemoryContext::Success)
        {
            return MemoryContext::InvalidAddress;
        }

        releasePhysical(alloc, phys);
        table->unmap(addr, alloc);
    }

    return MemoryContext::Success;
}

/* TODO: Check again */
MemoryContext::Result ARM64PageTable::releaseBlock(const Memory::Range range,
                                                    SplitAllocator *alloc,
                                                    const bool tablesOnly)
{
    Address phys;

    if (m_level == 3)
        return MemoryContext::InvalidAddress;

    u64 mask = m_level == 1 ? L1_BLOCK_MASK : L2_BLOCK_MASK;
    u64 block_range = get_block_range(m_level);

    // Input must be aligned to section address
    if (range.virt & ~mask)
    {
        return MemoryContext::InvalidAddress;
    }

    // Walk the page directory
    for (Size addr = range.virt; addr < range.virt + range.size; addr += block_range)
    {
        ARM64PageTable *table = getNextTable(addr, alloc);
        if (!table)
        {
            continue;
        }

        // Release mapped pages, if requested
        if (!tablesOnly)
        {
            for (Size i = 0; i < block_range; i += PAGESIZE)
            {
                if (table->translate(i, &phys, alloc) == MemoryContext::Success)
                {
                    releasePhysical(alloc, phys);
                }
            }
        }
        // Release page table
        alloc->release(m_tables[ get_entry_idx(addr, m_level) ] & PAGEMASK);
        m_tables[ get_entry_idx(addr, m_level) ] = PT_NONE;
    }

    return MemoryContext::Success;
}

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
#include <MemoryBlock.h>
#include <Types.h>
#include <BitAllocator.h>
#include <Log.h>
#include "IntelCore.h"
#include "IntelMemory.h"

IntelMemory::IntelMemory(Address pageDirectory, BitAllocator *phys)
    : Memory(pageDirectory, phys)
{
    // Default to the local page directory
    m_pageTableBase = PAGEDIR_LOCAL;
    m_pageDirectory = ((Address *) m_pageTableBase) + (PAGEDIR_LOCAL >> PAGESHIFT);

    if (pageDirectory)
    {
        // Find a free range for the page table mappings
        for (Size i = 0; i < PAGEDIR_MAX; i++)
        {
            if (!(m_pageDirectory[i] & PAGE_PRESENT))
            {
                m_pageTableBase = i * PAGETAB_MAX * PAGESIZE;
                m_pageDirectory = ((Address *) m_pageTableBase) + (PAGEDIR_LOCAL >> PAGESHIFT);
                break;
            }
        }
        // TODO: this function is a hack. Later, the kernel should not do any (virtual)memory anymore.
        // the coreserver should have full access to the virtual memory, without kernel help. Perhaps
        // it will need to run in ring0 for that on intel, which is acceptable.

        // Modify the local page directory to insert the mapping
        Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
        localDirectory[ DIRENTRY(m_pageTableBase) ] =
            (Address) pageDirectory | PAGE_WRITE | PAGE_PRESENT;
        tlb_flush_all();
    }
}

IntelMemory::~IntelMemory()
{
    if (m_pageTableBase != PAGEDIR_LOCAL)
    {
        Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
        localDirectory[ DIRENTRY(m_pageTableBase) ] = 0;
        tlb_flush_all();
    }
}

Memory::Range IntelMemory::range(Memory::Region region)
{
    Memory::Range r;

    // Initialize unused fields
    r.phys   = 0;
    r.access = None;

    // Fill region virtual range
    switch (region)
    {
        case KernelData:    r.virt = 0;          r.size = MegaByte(3);   break;
        case KernelHeap:    r.virt = 0x00300000; r.size = MegaByte(1);   break;
        case KernelStack:   r.virt = 0x08400000; r.size = KiloByte(4);   break;
        case PageTables:    r.virt = 0x00400000; r.size = MegaByte(64);  break;
        case KernelPrivate: r.virt = 0x04400000; r.size = MegaByte(64);  break;
        case UserData:      r.virt = 0x80000000; r.size = MegaByte(256); break;
        case UserHeap:      r.virt = 0xb0000000; r.size = MegaByte(256); break;
        case UserStack:     r.virt = 0xc0000000; r.size = KiloByte(4);   break;
        case UserPrivate:   r.virt = 0xa0000000; r.size = MegaByte(256); break;
        case UserShared:    r.virt = 0xd0000000; r.size = MegaByte(256); break;
    }
    return r;
}

Address * IntelMemory::getPageTable(Address virt)
{
    if (!(m_pageDirectory[ DIRENTRY(virt) ] & PAGE_PRESENT))
        return (Address *) ZERO;
    else
        return ((Address *) m_pageTableBase) + (((virt & PAGEMASK) >> DIRSHIFT) * PAGETAB_MAX);
}

Address IntelMemory::map(Address phys, Address virt, Access flags)
{
    Size size = PAGESIZE;

    // Must have a virtual address
    if (!virt)
    {
        FATAL("invalid ZERO virtual address");
        return 0;
    }

    // find unused physical page if not specified
    if (!phys)
        m_phys->allocate(&size, &phys);

    Address *pageTable = getPageTable(virt);

    // Does the page table exist?
    if (!pageTable)
    {
        // Allocate a new page table
        Address table;
        m_phys->allocate(&size, &table);

        // Map a new page table
        m_pageDirectory[ DIRENTRY(virt) ] = table | PAGE_PRESENT | PAGE_WRITE | flags;

        // Flush and clear the page table
        pageTable = getPageTable(virt);
        tlb_flush(pageTable);
        MemoryBlock::set(pageTable, 0, PAGESIZE);
    }
    // Map the physical page to a virtual address
    m_pageDirectory[ DIRENTRY(virt) ] = m_pageDirectory[ DIRENTRY(virt) ] | flags;
    tlb_flush(pageTable);
    pageTable[ TABENTRY(virt) ] = (phys & PAGEMASK) | flags;
    tlb_flush(virt);
    return virt;
}

Address IntelMemory::lookup(Address virt)
{
    Address *pageTable = getPageTable(virt);

    if (!pageTable)
        return ZERO;
    else
        return pageTable[ TABENTRY(virt) ] & PAGEMASK;
}

bool IntelMemory::access(Address virt, Size size, Access flags)
{
    for (Size i = 0; i < size; i += PAGESIZE)
    {
        Address *pageTable = getPageTable(virt);

        if (!pageTable || !(pageTable[ TABENTRY(virt) ] & flags))
            return false;
    }
    return true;
}

void IntelMemory::unmap(Address virt)
{
    Address *pageTable = getPageTable(virt);
    
    // Remove the mapping and flush TLB cache
    pageTable[ TABENTRY(virt) ] = 0;
    tlb_flush(virt);
}

void IntelMemory::release(Address virt)
{
    Address physical = lookup(virt);

    if (physical)
        m_phys->release(physical);

    unmap(virt);
}

void IntelMemory::releaseRegion(Memory::Region region)
{
    Range r = range(region);

    // Release physical pages of the whole region
    for (Size i = 0; i < r.size; i += PAGESIZE)
    {
        Address *table = getPageTable(r.virt + i);

        if (table)
            m_phys->release(table[TABENTRY(r.virt + i)] & PAGEMASK);
    }
}

Address IntelMemory::findFree(Size size, Memory::Region region)
{
    Range r = range(region);
    Size currentSize = 0;
    Address addr = r.virt, currentAddr = 0;

    while (addr < r.virt+r.size && currentSize < size)
    {
        Address *table = getPageTable(addr);

        // Does the page table exist at all?
        if (!table)
        {
            if (!currentAddr) currentAddr = addr;
            currentSize += PAGESIZE * PAGETAB_MAX;
            addr += PAGESIZE * PAGETAB_MAX;
        }
        // Is this virtual address unused?
        else if (!(table[TABENTRY(addr)] & PAGE_PRESENT))
        {
            if (!currentAddr)
                currentAddr = addr;
            currentSize += PAGESIZE; addr += PAGESIZE;
        }
        // Used. Reset the search
        else
        {
            currentSize = 0; currentAddr = 0; addr += PAGESIZE;
        }
    }
    return currentAddr;
}

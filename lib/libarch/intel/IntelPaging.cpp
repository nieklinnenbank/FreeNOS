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
#include "IntelPaging.h"

/* Temporary definitions. Need to be put in a separate class */
#define PAGE_NONE       0
#define PAGE_PRESENT    1
#define PAGE_READ       0
#define PAGE_EXEC       0
#define PAGE_WRITE      2
#define PAGE_USER       4

IntelPaging::IntelPaging(Address pageDirectory, BitAllocator *phys)
    : Memory(pageDirectory, phys)
{
    // Default to the local page directory
    m_pageTableBase = PAGEDIR_LOCAL;
    m_pageDirectory = ((Address *) m_pageTableBase) + (PAGEDIR_LOCAL >> PAGESHIFT);

    if (pageDirectory)
    {
        // Find a free range for the page table mappings
        // TODO: only search inside the PageTable range!!!
        for (Size i = 0; i < PAGEDIR_MAX; i++)
        {
            if (!(m_pageDirectory[i] & PAGE_PRESENT))
            {
                m_pageTableBase = i * PAGETAB_MAX * PAGESIZE;
                m_pageDirectory = ((Address *) m_pageTableBase) + (PAGEDIR_LOCAL >> PAGESHIFT);
                break;
            }
        }
        // Modify the local page directory to insert the mapping
        Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
        localDirectory[ DIRENTRY(m_pageTableBase) ] =
            (Address) pageDirectory | PAGE_WRITE | PAGE_PRESENT;
        tlb_flush_all();
    }
}

IntelPaging::~IntelPaging()
{
    if (m_pageTableBase != PAGEDIR_LOCAL)
    {
        Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
        localDirectory[ DIRENTRY(m_pageTableBase) ] = 0;
        tlb_flush_all();
    }
}

Memory::Result IntelPaging::create()
{
    return Success;
}

Memory::Result IntelPaging::initialize()
{
    return Success;
}

Memory::Range IntelPaging::range(Memory::Region region)
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

Address * IntelPaging::getPageTable(Address virt)
{
    if (!(m_pageDirectory[ DIRENTRY(virt) ] & PAGE_PRESENT))
        return (Address *) ZERO;
    else
        return ((Address *) m_pageTableBase) + (((virt & PAGEMASK) >> DIRSHIFT) * PAGETAB_MAX);
}

Memory::Result IntelPaging::map(Address phys, Address virt, Access acc)
{
    Size size = PAGESIZE;

    // Must have a virtual address
    if (!virt)
    {
        FATAL("invalid ZERO virtual address");
        return InvalidAddress;
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
        m_pageDirectory[ DIRENTRY(virt) ] = table | PAGE_PRESENT | PAGE_WRITE | flags(acc);

        // Flush and clear the page table
        pageTable = getPageTable(virt);
        tlb_flush(pageTable);
        MemoryBlock::set(pageTable, 0, PAGESIZE);
    }
    // Map the physical page to a virtual address
    m_pageDirectory[ DIRENTRY(virt) ] = m_pageDirectory[ DIRENTRY(virt) ] | flags(acc);
    tlb_flush(pageTable);
    pageTable[ TABENTRY(virt) ] = (phys & PAGEMASK) | flags(acc);
    tlb_flush(virt);
    return Success;
}

Address IntelPaging::lookup(Address virt)
{
    Address *pageTable = getPageTable(virt);

    if (!pageTable)
        return ZERO;
    else
        return pageTable[ TABENTRY(virt) ] & PAGEMASK;
}

u32 IntelPaging::flags(Access acc)
{
    u32 f = 0;

    if (acc & Memory::Present)  f |= PAGE_PRESENT;
    if (acc & Memory::Writable) f |= PAGE_WRITE;
    if (acc & Memory::User)     f |= PAGE_USER;

    return f;
}

Memory::Access IntelPaging::access(Address virt)
{
    Address *pageTable = getPageTable(virt);
    Access acc = None;

    if (pageTable)
    {
        Address entry = pageTable[TABENTRY(virt)];

        if (entry & PAGE_PRESENT) acc |= Memory::Present | Memory::Readable;
        if (entry & PAGE_WRITE)   acc |= Memory::Writable;
        if (entry & PAGE_USER)    acc |= Memory::User;
    }
    return acc;
}

Memory::Result IntelPaging::unmap(Address virt)
{
    Address *pageTable = getPageTable(virt);
    
    // Remove the mapping and flush TLB cache
    pageTable[ TABENTRY(virt) ] = 0;
    tlb_flush(virt);
    return Success;
}

Memory::Result IntelPaging::release(Address virt)
{
    Address physical = lookup(virt);

    if (physical)
        m_phys->release(physical);

    unmap(virt);
    return Success;
}

Memory::Result IntelPaging::releaseRegion(Memory::Region region)
{
    Range r = range(region);

    // Release physical pages of the whole region
    for (Size i = 0; i < r.size; i += PAGESIZE)
    {
        Address *table = getPageTable(r.virt + i);

        if (table)
            m_phys->release(table[TABENTRY(r.virt + i)] & PAGEMASK);
    }
    return Success;
}

Address IntelPaging::findFree(Size size, Memory::Region region)
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
            if (!currentAddr)
                currentAddr = addr;
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

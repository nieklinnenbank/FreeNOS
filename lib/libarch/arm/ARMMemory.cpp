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
#include "ARMCore.h"
#include "ARMMemory.h"

#warning pageDirectory must be 16KB aligned.

ARMMemory::ARMMemory(Address pageDirectory, BitArray *memoryMap) : Memory(pageDirectory, memoryMap)
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
        // Modify the local page directory to insert the mapping
        Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
        localDirectory[ DIRENTRY(m_pageTableBase) ] =
            (Address) pageDirectory | PAGE_WRITE | PAGE_PRESENT | PAGE_RESERVE;
        tlb_flush_all();
    }
    // ask the kernel to map the page directory.
    // TODO: coreserver should just run as ring0, and map it directly????
    if (!isKernel)
    {
        // Ask the kernel for the physical memory map. Remap it as writable.
        if (!m_memoryMap)
        {
            SystemInformation info;
            m_memoryMap = new BitArray( info.memorySize / PAGESIZE, (u8 *)info.memoryBitArray );
        }
    }
}

ARMMemory::~ARMMemory()
{
    if (m_pageTableBase != PAGEDIR_LOCAL)
    {
        Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
        localDirectory[ DIRENTRY(m_pageTableBase) ] = 0;
        tlb_flush_all();
    }
}

Address * ARMMemory::getPageTable(Address virt)
{
    if (!(m_pageDirectory[ DIRENTRY(virt) ] & PAGE_PRESENT))
        return (Address *) ZERO;
    else
        return ((Address *) m_pageTableBase) + (((virt & PAGEMASK) >> DIRSHIFT) * PAGETAB_MAX);
}

Address ARMMemory::map(Address phys, Address virt, Access flags)
{
    // find unused physical page if not specified
    if (!phys)
        phys = m_memoryMap->setNext(1) * PAGESIZE;

    // find unused virtual address if not specified.
    if (!virt)
        virt = findFree(PAGESIZE);

    Address *pageTable = getPageTable(virt);

    // Does the page table exist?
    if (!pageTable)
    {
        // Allocate a new page table
        Address table = m_memoryMap->setNext(1) * PAGESIZE;

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

Address ARMMemory::lookup(Address virt)
{
    Address *pageTable = getPageTable(virt);

    if (!pageTable)
        return ZERO;
    else
        return pageTable[ TABENTRY(virt) ] & PAGEMASK;
}

bool ARMMemory::access(Address virt, Size size, Access flags)
{
    for (Size i = 0; i < size; i += PAGESIZE)
    {
        Address *pageTable = getPageTable(virt);

        if (!pageTable || !(pageTable[ TABENTRY(virt) ] & flags))
            return false;
    }
    return true;
}

void ARMMemory::unmap(Address virt)
{
    Address *pageTable = getPageTable(virt);
    
    // Remove the mapping and flush TLB cache
    pageTable[ TABENTRY(virt) ] = 0;
    tlb_flush(virt);
}

void ARMMemory::release(Address virt)
{
    unmap(virt);
    m_memoryMap->unset(virt >> PAGESHIFT);
}

void ARMMemory::releaseAll()
{
    // Walk page directory
    for (Size i = 0; i < PAGEDIR_MAX; i++)
    {
        // Skip pinned tables and non-present tables
        if (!(m_pageDirectory[i] & PAGE_PRESENT) ||
             (m_pageDirectory[i] & PAGE_PIN))
            continue;

        Address *table = getPageTable(i * PAGETAB_MAX * PAGESIZE);

        // Walk page table
        for (Size j = 0; j < PAGETAB_MAX; j++)
        {
            if (table[j] & PAGE_PRESENT && !(table[j] & PAGE_PIN))
                m_memoryMap->unset(table[j] >> PAGESHIFT);
        }
    }
}

Address ARMMemory::findFree(Size size)
{
    Address addr = 0;
    Size bytes = 0;

    // Walk page directory
    for (Size i = 0; i < PAGEDIR_MAX && bytes < size; i++)
    {
        // Is the pagetable present?
        if (!(m_pageDirectory[i] & (PAGE_PRESENT|PAGE_RESERVE)))
        {
            if (!addr)
                addr = (i * PAGETAB_MAX * PAGESIZE);
            bytes += PAGETAB_MAX * PAGESIZE;
            continue;
        }
        // We dont need to search reserved page directory entries
        if (m_pageDirectory[i] & PAGE_RESERVE)
            continue;

        Address *pageTable = getPageTable(i * PAGETAB_MAX * PAGESIZE);

        // Walk the page tables
        for (Size j = 0; j < PAGETAB_MAX && bytes < size; j++)
        {
            if (!(pageTable[j] & (PAGE_PRESENT|PAGE_RESERVE)))
            {
                if (!addr)
                    addr  = (i * PAGETAB_MAX * PAGESIZE) + (j * PAGESIZE);

                bytes += PAGESIZE;
            }
            else
            {
                addr  = 0;
                bytes = 0;
            }
        }
    }
    return addr;
}

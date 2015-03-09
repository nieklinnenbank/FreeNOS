/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include "IntelMemory.h"
#include <FreeNOS/Kernel.h>
#include <FreeNOS/Process.h>
#include <MemoryBlock.h>
#include <Types.h>

IntelMemory::IntelMemory(Size memorySize) : Memory(memorySize)
{
    m_remPageDir = PAGEDIRADDR_REMOTE;
    m_remPageTab = ZERO;
    m_myPageDir  = PAGEDIRADDR;
    m_myPageTab  = ZERO;

    /* Marks kernel memory used. */
    allocatePhysical(0x00400000);
}

Address IntelMemory::map(Address paddr, Address vaddr, MemoryAccess flags)
{
    /* Virtual address specified? */
    if (vaddr == ZERO)
    {
        vaddr = findFree(PAGETABFROM, PAGEDIRADDR);
    }
    /* Point to the correct page table. */
    m_myPageTab = PAGETABADDR(vaddr);
    
    /* Do we have the page table in memory? */
    if (!(m_myPageDir[DIRENTRY(vaddr)] & PAGE_PRESENT))
    {
        /* Then first allocate new page table. */
        Address newPageTab  = allocatePhysical(PAGESIZE);
        newPageTab |= PAGE_PRESENT | PAGE_RW | getProtectionFlags(flags);

        /* Map the new page table into memory. */
        m_myPageDir[DIRENTRY(vaddr)] = newPageTab;
        tlb_flush(m_myPageTab);

        /* Zero the new page table. */
        MemoryBlock::set(m_myPageTab, 0, PAGESIZE);
    }
    /* Map physical to virtual address. */
    m_myPageTab[TABENTRY(vaddr)] = (paddr & PAGEMASK) | getProtectionFlags(flags);
    tlb_flush(vaddr);

    /* Success. */
    return vaddr;
}

Address IntelMemory::map(Process *p, Address paddr,
                         Address vaddr, MemoryAccess flags)
{
    // The memory must be marked used
    if (!isAllocated(paddr))
    {
        FATAL("physical memory not mapped: " << vaddr << " -> " << paddr);
        for (;;);
    }

    /* Map remote pages. */
    mapRemote((IntelProcess *)p, vaddr);

    /* Virtual address specified? */
    if (vaddr == ZERO)
    {
        vaddr = findFree(PAGETABFROM_REMOTE, m_remPageDir);
    }
    /* Repoint to the correct (remote) page table. */
    m_remPageTab = PAGETABADDR_FROM(vaddr, PAGETABFROM_REMOTE);
    
    /* Does the remote process have the page table in memory? */
    if (!(m_remPageDir[DIRENTRY(vaddr)] & PAGE_PRESENT))
    {
        /* Nope, allocate a page table first. */
        Address newPageTab  = allocatePhysical(PAGESIZE);
        newPageTab |= PAGE_PRESENT | PAGE_RW | getProtectionFlags(flags);
        
        /* Map the new page table into remote memory. */
        m_remPageDir[DIRENTRY(vaddr)] = newPageTab;
        
        /* Update caches. */
        tlb_flush(m_remPageTab);
        
        /* Zero the new page. */
        MemoryBlock::set(m_remPageTab, 0, PAGESIZE);
    }
    /* Map physical address to remote virtual address. */
    m_remPageTab[TABENTRY(vaddr)] = (paddr & PAGEMASK) | getProtectionFlags(flags);
    tlb_flush(vaddr);

    /* Success. */
    return (Address) vaddr;
}

Address IntelMemory::findFree(Address pageTabFrom, Address *pageDirPtr)
{
    Address  vaddr = 0xa0000000;
    Address *pageTabPtr = PAGETABADDR_FROM(vaddr, pageTabFrom);

    /* Find a free virtual address. */
    while (pageDirPtr[DIRENTRY(vaddr)] & PAGE_PRESENT &&
           pageTabPtr[TABENTRY(vaddr)] & PAGE_PRESENT)
    {
        /* Look for the next page in line. */
        vaddr     += PAGESIZE;
        pageTabPtr = PAGETABADDR_FROM(vaddr, pageTabFrom);
    }
    return vaddr;
}

Address IntelMemory::lookup(Process *p, Address vaddr)
{
    Address ret = ZERO;

    /* Map remote page tables. */
    mapRemote((IntelProcess *)p, vaddr);
    
    /* Lookup the address, if mapped. */
    if (m_remPageDir[DIRENTRY(vaddr)] & PAGE_PRESENT &&
        m_remPageTab[TABENTRY(vaddr)] & PAGE_PRESENT)
    {
        ret = m_remPageTab[TABENTRY(vaddr)];
    }
    return ret;
}

void IntelMemory::mapRemote(IntelProcess *p, Address pageTabAddr,
                            Address pageDirAddr, MemoryAccess flags)
{
    /* Map remote page directory and page table. */
    m_myPageDir[DIRENTRY(pageDirAddr)] =
        p->getPageDirectory() | (PAGE_PRESENT|PAGE_RW|PAGE_PINNED|getProtectionFlags(flags));

    m_remPageTab = PAGETABADDR_FROM(pageTabAddr, PAGETABFROM_REMOTE);
    
    /* Refresh entire TLB cache. */
    tlb_flush_all();
}

bool IntelMemory::access(Process *p, Address vaddr, Size sz, MemoryAccess flags)
{
    Size bytes = 0;
    Address vfrom = vaddr;
    ulong prot = getProtectionFlags(flags);

    /* Map remote pages. */
    mapRemote((IntelProcess *)p, vaddr);

    /* Verify protection bits. */
    while (bytes < sz &&
           m_remPageDir[DIRENTRY(vaddr)] & prot &&
           m_remPageTab[TABENTRY(vaddr)] & prot)
    {
        vaddr += PAGESIZE;
        bytes += ((vfrom & PAGEMASK) + PAGESIZE) - vfrom;
        vfrom  = vaddr & PAGEMASK;
        m_remPageTab = PAGETABADDR_FROM(vaddr, PAGETABFROM_REMOTE);
    }
    /* Do we have a match? */
    return (bytes >= sz);
}

void IntelMemory::release(Process *p)
{
    /* Map page tables. */
    mapRemote((IntelProcess *)p, 0x0);

    /* Mark all our physical pages free. */
    for (Size i = 0; i < 1024; i++)
    {
        /* May we release these physical pages? */
        if ((m_remPageDir[i] & PAGE_PRESENT) && !(m_remPageDir[i] & PAGE_PINNED))
        {
            /* Repoint page table. */
            m_remPageTab = PAGETABADDR_FROM(i * PAGESIZE * 1024,
                                          PAGETABFROM_REMOTE);

            /* Scan page table. */
            for (Size j = 0; j < 1024; j++)
            {
                if (m_remPageTab[j] & PAGE_PRESENT && !(m_remPageTab[j] & PAGE_PINNED))
                {
                    releasePhysical(m_remPageTab[j]);
                }
            }
        }
    }
}

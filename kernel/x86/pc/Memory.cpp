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

#include "Memory.h"
#include <FreeNOS/Kernel.h>
#include <FreeNOS/Process.h>
#include <MemoryBlock.h>
#include <Types.h>

X86Memory::X86Memory() : Memory(), remPageDir(PAGEDIRADDR_REMOTE),
                         remPageTab(ZERO), myPageDir(PAGEDIRADDR),
                         myPageTab(ZERO)
{
}

Address X86Memory::mapVirtual(Address paddr, Address vaddr, ulong prot)
{
    /* Virtual address specified? */
    if (vaddr == ZERO)
    {
        vaddr = findFree(PAGETABFROM, PAGEDIRADDR);
    }
    /* Point to the correct page table. */
    myPageTab = PAGETABADDR(vaddr);
    
    /* Do we have the page table in memory? */
    if (!(myPageDir[DIRENTRY(vaddr)] & PAGE_PRESENT))
    {
        /* Then first allocate new page table. */
        Address newPageTab  = memory->allocatePhysical(PAGESIZE);
        newPageTab |= PAGE_PRESENT | PAGE_RW | prot;

        /* Map the new page table into memory. */
        myPageDir[DIRENTRY(vaddr)] = newPageTab;
        tlb_flush(myPageTab);

        /* Zero the new page table. */
        MemoryBlock::set(myPageTab, 0, PAGESIZE);
    }
    /* Map physical to virtual address. */
    myPageTab[TABENTRY(vaddr)] = (paddr & PAGEMASK) | prot;
    tlb_flush(vaddr);

    /* Success. */
    return vaddr;
}

Address X86Memory::mapVirtual(Process *p, Address paddr,
                              Address vaddr, ulong prot)
{
    /* Map remote pages. */
    mapRemote((X86Process *)p, vaddr);

    /* Virtual address specified? */
    if (vaddr == ZERO)
    {
        vaddr = findFree(PAGETABFROM_REMOTE, remPageDir);
    }
    /* Repoint to the correct (remote) page table. */
    remPageTab = PAGETABADDR_FROM(vaddr, PAGETABFROM_REMOTE);
    
    /* Does the remote process have the page table in memory? */
    if (!(remPageDir[DIRENTRY(vaddr)] & PAGE_PRESENT))
    {
        /* Nope, allocate a page table first. */
        Address newPageTab  = memory->allocatePhysical(PAGESIZE);
        newPageTab |= PAGE_PRESENT | PAGE_RW | prot;
        
        /* Map the new page table into remote memory. */
        remPageDir[DIRENTRY(vaddr)] = newPageTab;
        
        /* Update caches. */
        tlb_flush(remPageTab);
        
        /* Zero the new page. */
        MemoryBlock::set(remPageTab, 0, PAGESIZE);
    }
    /* Map physical address to remote virtual address. */
    remPageTab[TABENTRY(vaddr)] = (paddr & PAGEMASK) | prot;
    tlb_flush(vaddr);

    /* Success. */
    return (Address) vaddr;
}

Address X86Memory::findFree(Address pageTabFrom, Address *pageDirPtr)
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

Address X86Memory::lookupVirtual(Process *p, Address vaddr)
{
    Address ret = ZERO;

    /* Map remote page tables. */
    mapRemote((X86Process *)p, vaddr);
    
    /* Lookup the address, if mapped. */
    if (remPageDir[DIRENTRY(vaddr)] & PAGE_PRESENT &&
        remPageTab[TABENTRY(vaddr)] & PAGE_PRESENT)
    {
        ret = remPageTab[TABENTRY(vaddr)];
    }
    return ret;
}

void X86Memory::mapRemote(X86Process *p, Address pageTabAddr,
                          Address pageDirAddr, ulong prot)
{
    /* Map remote page directory and page table. */
    myPageDir[DIRENTRY(pageDirAddr)] =
        p->getPageDirectory() | (PAGE_PRESENT|PAGE_RW|PAGE_PINNED|prot);
    remPageTab = PAGETABADDR_FROM(pageTabAddr, PAGETABFROM_REMOTE);
    
    /* Refresh entire TLB cache. */
    tlb_flush_all();
}

bool X86Memory::access(Process *p, Address vaddr, Size sz, ulong prot)
{
    Size bytes = 0;
    Address vfrom = vaddr;

    /* Map remote pages. */
    mapRemote((X86Process *)p, vaddr);

    /* Verify protection bits. */
    while (bytes < sz &&
           remPageDir[DIRENTRY(vaddr)] & prot &&
           remPageTab[TABENTRY(vaddr)] & prot)
    {
        vaddr += PAGESIZE;
        bytes += ((vfrom & PAGEMASK) + PAGESIZE) - vfrom;
        vfrom  = vaddr & PAGEMASK;
        remPageTab = PAGETABADDR_FROM(vaddr, PAGETABFROM_REMOTE);
    }
    /* Do we have a match? */
    return (bytes >= sz);
}

void X86Memory::releaseAll(Process *p)
{
    /* Map page tables. */
    mapRemote((X86Process *)p, 0x0);

    /* Mark all our physical pages free. */
    for (Size i = 0; i < 1024; i++)
    {
        /* May we release these physical pages? */
        if ((remPageDir[i] & PAGE_PRESENT) && !(remPageDir[i] & PAGE_PINNED))
        {
            /* Repoint page table. */
            remPageTab = PAGETABADDR_FROM(i * PAGESIZE * 1024,
                                          PAGETABFROM_REMOTE);

            /* Scan page table. */
            for (Size j = 0; j < 1024; j++)
            {
                if (remPageTab[j] & PAGE_PRESENT && !(remPageTab[j] & PAGE_PINNED))
                {
                    memory->releasePhysical(remPageTab[j]);
                }
            }
        }
    }
}

INITOBJ(X86Memory, memory, VMEMORY)

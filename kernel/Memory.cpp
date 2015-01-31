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

#include <Arch/Memory.h>
#include "Multiboot.h"
#include "Kernel.h"
#include <Init.h>
#include <Allocator.h>
#include <BubbleAllocator.h>
#include <PoolAllocator.h>
#include <Types.h>

Size Memory::memorySize, Memory::memoryAvail;
u8  *Memory::memoryMap, *Memory::memoryMapEnd;

Memory::Memory()
{
    /* Marks kernel memory used. */
    allocatePhysical(0x00400000, 0);
    
    /* Marks boot module memory. */
    for (Size i = 0; i < multibootInfo.modsCount; i++)
    {
        MultibootModule *mod  = &((MultibootModule *) multibootInfo.modsAddress)[i];
        Size modSize = mod->modEnd - mod->modStart;

        /* Mark memory used. */
        allocatePhysical(modSize, mod->modStart);
    }
}

Size Memory::getTotalMemory()
{
    return memorySize;
}
        
Size Memory::getAvailableMemory()
{
    return memoryAvail;
}

void Memory::initialize()
{
    Address page = 0x00300000;
    Size meta = sizeof(BubbleAllocator) + sizeof(PoolAllocator);
    Allocator *bubble, *pool;

    /* Save memory size. */
    memorySize  = (multibootInfo.memLower + multibootInfo.memUpper) * 1024;
    memoryAvail = memorySize;
    
    /* Allocate memoryMap */
    memoryMap    = (u8 *)(&kernelEnd);
    memoryMapEnd = memoryMap + (memorySize / PAGESIZE / 8);

    /* Clear memory map. */
    for (u8 *p = memoryMap; p < memoryMapEnd; p++)
    {
        *p = 0;
    }
    /* Setup the dynamic memory heap. */
    bubble = new (page) BubbleAllocator();
    pool   = new (page + sizeof(BubbleAllocator)) PoolAllocator();
    pool->setParent(bubble);
    
    /* Setup the heap region (1MB). */
    bubble->region(page + meta, (1024 * 1024) - meta);

    /* Set default allocator. */
    Allocator::setDefault(pool);
}

Address Memory::allocatePhysical(Size sz, Address paddr)
{
    Address start = paddr & PAGEMASK, end = memorySize;
    Address from  = 0, count = 0;

    /* Loop the memoryMap for a free block. */
    for (Address i = start; i < end; i += PAGESIZE)
    {
        if (!isMarked(i))
        {
            /* Remember this page. */
            if (!count)
            {
                from  = i;
                count = 1;
            }
            else
                count++;

            /* Are there enough contigious pages? */
            if (count * PAGESIZE >= sz)
            {
                for (Address j = from; j < from + (count * PAGESIZE); j += PAGESIZE)
                {
                    setMark(j, true);
                }
                memoryAvail -= count * PAGESIZE;
                return from;
            }
        }
        else
        {
            from = count = 0;
        }
    }
    /* Out of memory! */
    return (Address) ZERO;
}

void Memory::releasePhysical(Address addr)
{
    setMark(addr & PAGEMASK, false);
    memoryAvail += PAGESIZE;
}

Address Memory::allocateVirtual(Address vaddr, ulong prot)
{
    /* Allocate a new physical page. */
    Address newPage = allocatePhysical(PAGESIZE);
    
    /* Map it to the requested virtual address. */
    return mapVirtual(newPage, vaddr, prot);
}

Address Memory::allocateVirtual(Process *p, Address vaddr, ulong prot)
{
    /* Allocate new physical page. */
    Address newPage = allocatePhysical(PAGESIZE);
    
    /* Map it into the target process. */
    return mapVirtual(p, newPage, vaddr, prot);
}

bool Memory::isMarked(Address addr)
{
    Size index = (addr >> PAGESHIFT) / 8;
    Size bit   = (addr >> PAGESHIFT) % 8;
    
    return memoryMap[index] & (1 << bit);
}

void Memory::setMark(Address addr, bool marked)
{
    Size index = (addr >> PAGESHIFT) / 8;
    Size bit   = (addr >> PAGESHIFT) % 8;

    if (marked)
        memoryMap[index] |=  (1 << bit);
    else
        memoryMap[index] &= ~(1 << bit);
}

INITCLASS(Memory, initialize, PMEMORY)

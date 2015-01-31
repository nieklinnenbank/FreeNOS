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

#include <API/VMCtl.h>
#include <API/ProcessCtl.h>
#include <Arch/Memory.h>
#include <Macros.h>
#include <ProcessID.h>
#include "VMCtlAllocator.h"

VMCtlAllocator::VMCtlAllocator(Size size)
{
    /* Set heap pointers. */
    heapStart = VMCTL_ALLOC_START;
    heapEnd   = VMCTL_ALLOC_START;
    allocated = ZERO;

    /* Allocate the given bytes. */
    allocate(&size);
}

VMCtlAllocator::VMCtlAllocator(VMCtlAllocator *p)
    : heapStart(p->heapStart), heapEnd(p->heapEnd),
      allocated(p->allocated)
{
}

Address VMCtlAllocator::allocate(Size *size)
{
    Address ret = heapStart + allocated;
    MemoryRange range;
    Size bytes;

    /* Start allocating. */
    for (bytes = 0; bytes < *size; bytes += PAGESIZE)
    {
	range.virtualAddress  = ret + bytes;
	range.physicalAddress = ZERO;
	range.bytes = PAGESIZE;
    
	VMCtl(SELF, Map, &range);
    }
    /* Update end-of-heap pointer. */
    heapEnd = ret + bytes;

    /* Update count. */
    allocated += bytes;

    /* Success. */
    *size = bytes;
    return ret;
}

void VMCtlAllocator::release(Address addr)
{
    // TODO
}

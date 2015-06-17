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

ARMMemory::ARMMemory(Address pageDirectory, BitAllocator *alloc)
    : Memory(pageDirectory, alloc)
{
}

ARMMemory::~ARMMemory()
{
}

Memory::Range ARMMemory::range(Region region)
{
    Memory::Range r;
    return r;
}


Address * ARMMemory::getPageTable(Address virt)
{
    return ZERO;
}

Address ARMMemory::map(Address phys, Address virt, Access flags)
{
    return ZERO;
}

Address ARMMemory::lookup(Address virt)
{
    return ZERO;
}

bool ARMMemory::access(Address virt, Size size, Access flags)
{
    return true;
}

void ARMMemory::unmap(Address virt)
{
}

void ARMMemory::release(Address virt)
{
}

void ARMMemory::releaseRegion(Memory::Region region)
{
}

Address ARMMemory::findFree(Size size, Memory::Region region)
{
    return ZERO;
}

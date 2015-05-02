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

#include <Log.h>
#include "ARMMemory.h"

ARMMemory::ARMMemory(Size memorySize)
    : Memory(memorySize)
{
    NOTICE("memorySize:" << memorySize);
}

Address ARMMemory::map(Address paddr, Address vaddr, MemoryAccess flags)
{
    DEBUG("");
    return 0;
}

Address ARMMemory::map(Process *p, Address paddr, Address vaddr, MemoryAccess flags)
{
    DEBUG("");
    return 0;
}

Address ARMMemory::lookup(Process *p, Address vaddr)
{
    DEBUG("");
    return 0;
}

bool ARMMemory::access(Process *p,
                       Address vaddr,
                       Size sz,
                       MemoryAccess flags)
{
    DEBUG("");
    return true;
}

void ARMMemory::release(Process *p)
{
    DEBUG("");
}

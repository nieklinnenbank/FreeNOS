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
#include <FreeNOS/System.h>
#include <Macros.h>
#include "MemoryAllocator.h"

MemoryAllocator::MemoryAllocator(Address base, Size size)
{
    // set members
    m_base      = base;
    m_allocated = 0;
    m_size      = 0;
}

MemoryAllocator::MemoryAllocator(MemoryAllocator *p)
{
    m_base      = p->m_base;
    m_allocated = p->m_allocated;
    m_size      = p->m_size;
}

Size MemoryAllocator::size()
{
    return m_size;
}

Size MemoryAllocator::available()
{
    return m_size - m_allocated;
}

Address MemoryAllocator::base()
{
    return m_base;
}

Allocator::Result MemoryAllocator::allocate(Size *size, Address *addr, Size align)
{
    Arch::Memory memory;
    Memory::Range range;

    // Update variables
    *addr = m_base + m_allocated;
    *size = CEIL(*size, PAGESIZE);
    *size *= PAGESIZE;

    // Allocate memory range
    range.virt  = *addr;
    range.phys  = ZERO;
    range.size  = *size;
    range.access = Memory::Present |
                   Memory::User |
                   Memory::Readable |
                   Memory::Writable;
    memory.mapRange(&range);

    // Update count
    m_allocated += range.size;

    // Success
    return Success;
}

Allocator::Result MemoryAllocator::release(Address addr)
{
    // TODO
    return InvalidAddress;
}

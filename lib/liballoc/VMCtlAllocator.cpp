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

#include <FreeNOS/API.h>
#include <FreeNOS/System.h>
#include <Macros.h>
#include "VMCtlAllocator.h"

VMCtlAllocator::VMCtlAllocator(Address base, Size size)
{
    // set members
    m_base      = base;
    m_allocated = 0;
    m_size      = 0;
}

VMCtlAllocator::VMCtlAllocator(VMCtlAllocator *p)
{
    m_base      = p->m_base;
    m_allocated = p->m_allocated;
    m_size      = p->m_size;
}

Size VMCtlAllocator::size()
{
    return m_size;
}

Size VMCtlAllocator::available()
{
    return m_size - m_allocated;
}

Address VMCtlAllocator::base()
{
    return m_base;
}

Allocator::Result VMCtlAllocator::allocate(Size *size, Address *addr)
{
    Memory::Range range;
    Size bytes;

    // Set address
    *addr = m_base + m_allocated;

    /* Start allocating. */
    for (bytes = 0; bytes < *size; bytes += PAGESIZE)
    {
        range.virt  = *addr + bytes;
        range.phys  = ZERO;
        range.size  = PAGESIZE;
        range.access = Memory::Present |
                       Memory::User |
                       Memory::Readable |
                       Memory::Writable;
    
        VMCtl(SELF, Map, &range);
    }
    // Update count
    m_allocated += bytes;

    // Success
    *size = bytes;
    return Success;
}

Allocator::Result VMCtlAllocator::release(Address addr)
{
    // TODO
    return InvalidAddress;
}

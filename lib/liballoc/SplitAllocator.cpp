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

#include "SplitAllocator.h"

SplitAllocator::SplitAllocator(const Allocator::Range physRange,
                               const Allocator::Range virtRange,
                               const Size pageSize)
    : Allocator(physRange)
    , m_alloc(physRange, pageSize)
    , m_virtRange(virtRange)
    , m_pageSize(pageSize)
{
}

Size SplitAllocator::available() const
{
    return m_alloc.available();
}

Allocator::Result SplitAllocator::allocate(Allocator::Range & args)
{
    return m_alloc.allocate(args, 0);
}

Allocator::Result SplitAllocator::allocate(Allocator::Range & phys,
                                           Allocator::Range & virt)
{
    Result r = m_alloc.allocate(phys, 0);

    if (r == Success)
    {
        virt.address   = toVirtual(phys.address);
        virt.size      = phys.size;
        virt.alignment = phys.alignment;
    }

    return r;
}

Allocator::Result SplitAllocator::allocate(const Address addr)
{
    return m_alloc.allocate(addr);
}

Allocator::Result SplitAllocator::release(Address addr)
{
    return m_alloc.release(addr);
}

Address SplitAllocator::toVirtual(const Address phys) const
{
    const Size mappingDiff = base() - m_virtRange.address;
    return phys - mappingDiff;
}

Address SplitAllocator::toPhysical(const Address virt) const
{
    const Size mappingDiff = base() - m_virtRange.address;
    return virt + mappingDiff;
}

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

#include <Assert.h>
#include <Macros.h>
#include <MemoryBlock.h>
#include "Allocator.h"

Allocator * Allocator::m_default = ZERO;

Allocator::Allocator()
    : m_parent(ZERO)
{
    MemoryBlock::set(&m_range, 0, sizeof(m_range));
}

Allocator::Allocator(const Allocator::Range range)
    : m_parent(ZERO)
    , m_range(range)
{
    assert(m_range.alignment >= sizeof(u32));
    assert(m_range.size >= sizeof(u32));
    assert((m_range.size % m_range.alignment) == 0);
}

Allocator::~Allocator()
{
}

void Allocator::setParent(Allocator *parent)
{
    m_parent = parent;
}

Allocator * Allocator::parent()
{
    return m_parent;
}

Allocator * Allocator::getDefault()
{
    return m_default;
}

void Allocator::setDefault(Allocator *alloc)
{
    m_default = alloc;
}

Size Allocator::size() const
{
    return m_range.size;
}

Address Allocator::base() const
{
    return m_range.address;
}

Size Allocator::alignment() const
{
    return m_range.alignment;
}

Size Allocator::available() const
{
    return m_parent ? m_parent->available() : ZERO;
}

Allocator::Result Allocator::allocate(Allocator::Range & range)
{
    return OutOfMemory;
}

Allocator::Result Allocator::release(const Address addr)
{
    return InvalidAddress;
}

Address Allocator::aligned(const Address addr, const Size boundary) const
{
    Address corrected = addr;

    if (addr % boundary)
        corrected += boundary - (addr % boundary);

    return corrected;
}

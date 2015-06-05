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

#include "BitAllocator.h"

BitAllocator::BitAllocator(Address base, Size size, Size chunkSize)
    : Allocator(), m_array(size / chunkSize)
{
    m_base      = base;
    m_chunkSize = chunkSize;
}

Size BitAllocator::size()
{
    return m_array.size() * m_chunkSize;
}

Size BitAllocator::available()
{
    return m_array.count(false) * m_chunkSize;
}

BitArray * BitAllocator::getBitArray()
{
    return &m_array;
}

Allocator::Result BitAllocator::allocate(Size *size, Address *addr)
{
    Size num = (*size) / m_chunkSize;

    if ((*size) % m_chunkSize)
        num++;

    *addr = m_array.setNext(num) * m_chunkSize;
    return Success;
}

Allocator::Result BitAllocator::allocate(Address addr)
{
    if (isAllocated(addr))
        return InvalidAddress;        

    m_array.set(addr / m_chunkSize);
    return Success;
}

bool BitAllocator::isAllocated(Address addr)
{
    return m_array.isSet(addr / m_chunkSize);
}

Allocator::Result BitAllocator::release(Address addr)
{
    // TODO: sanity check addr

    m_array.unset(addr / m_chunkSize);
    return Success;
}

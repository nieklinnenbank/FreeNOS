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

#include <Assert.h>
#include "BitAllocator.h"

BitAllocator::BitAllocator(const Allocator::Range range,
                           const Size chunkSize,
                           u8 *bitmap)
    : Allocator(range)
    , m_array(range.size / chunkSize, bitmap)
    , m_chunkSize(chunkSize)
    , m_lastBit(0)
{
}

Size BitAllocator::chunkSize() const
{
    return m_chunkSize;
}

Size BitAllocator::available() const
{
    return m_array.count(false) * m_chunkSize;
}

Allocator::Result BitAllocator::allocate(Allocator::Range & args)
{
    Allocator::Result result = allocateFrom(args, m_lastBit);
    if (result == OutOfMemory)
    {
        return allocateFrom(args, 0);
    } else {
        return result;
    }
}

Allocator::Result BitAllocator::allocateFrom(Allocator::Range & args,
                                             const Size startBit)
{
    Size num = (args.size) / m_chunkSize;
    BitArray::Result result;
    Size bit, alignment = 1;

    if ((args.size) % m_chunkSize)
        num++;

    if (args.alignment)
    {
        if (args.alignment % m_chunkSize)
            return InvalidAlignment;
        else
            alignment = args.alignment / m_chunkSize;
    }

    result = m_array.setNext(&bit, num, startBit, alignment);
    if (result != BitArray::Success)
        return OutOfMemory;

    args.address = base() + (bit * m_chunkSize);
    assert(isAllocated(args.address));
    m_lastBit = bit;
    return Success;
}

Allocator::Result BitAllocator::allocateAt(const Address addr)
{
    assert(!isAllocated(addr));

    m_array.set((addr - base()) / m_chunkSize);
    return Success;
}

bool BitAllocator::isAllocated(const Address addr) const
{
    assert(addr >= base());
    assert(addr < base() + size());
    assert(((addr - base()) % m_chunkSize) == 0);

    return m_array.isSet((addr - base()) / m_chunkSize);
}

Allocator::Result BitAllocator::release(const Address addr)
{
    assert(isAllocated(addr));

    m_array.unset((addr - base()) / m_chunkSize);
    return Success;
}

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

#include <Types.h>
#include "BubbleAllocator.h"

BubbleAllocator::BubbleAllocator(const Allocator::Range range)
    : Allocator(range)
    , m_allocated(0)
{
}

Size BubbleAllocator::available() const
{
    return size() - m_allocated;
}

Allocator::Result BubbleAllocator::allocate(Allocator::Range & args)
{
    Size needed = aligned(args.size, alignment());

    // Do we still have enough room?
    if (m_allocated + needed <= size())
    {
        args.address = base() + m_allocated;
        m_allocated += needed;
        return Success;
    }
    // No more memory available
    return OutOfMemory;
}

Allocator::Result BubbleAllocator::release(const Address addr)
{
    // BubbleAllocator never releases memory
    return InvalidAddress;
}

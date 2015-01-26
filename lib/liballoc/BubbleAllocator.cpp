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

BubbleAllocator::BubbleAllocator()
{
    region(0, 0);
}

BubbleAllocator::BubbleAllocator(Address start, Size size)
{
    region(start, size);
}

Address BubbleAllocator::allocate(Size *sz)
{
    Size needed = aligned(*sz);

    /* Do we still have enough room? */
    if (current + needed < start + size)
    {
	current += needed;
	return (Address) (current - needed);
    }
    /* No more memory available. */
    return (Address) 0;
}

void BubbleAllocator::release(Address addr)
{
}

void BubbleAllocator::region(Address addr, Size size)
{
    this->start   = (u8 *) addr;
    this->current = (u8 *) addr;
    this->size    = size;
}

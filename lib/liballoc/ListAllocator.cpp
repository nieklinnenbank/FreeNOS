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

#include <string.h>
#include <Macros.h>
#include <Types.h>
#include "ListAllocator.h"

ListAllocator::ListAllocator()
    : head(ZERO), tail(ZERO)
{
    region(0, 0);
}

ListAllocator::ListAllocator(Address addr, Size size)
    : head(ZERO), tail(ZERO)
{
    region(addr, size);
}

MemBlock * ListAllocator::findFreeBlock(Size size)
{
    Size sz;
    Address addr;

    /* Search all available blocks. */
    for (MemBlock *b = head; b; b = b->next)
    {
	/* Is this block big enough? */
	if (b->free && b->size >= size)
	{
	    return b;
	}
    }
    /* If no blocks are available, allocate from parent. */
    if (parent)
    {
	/* We want a new memory block from our parent. */
	sz = size;
	
	/* Ask for more blocks from our parent. */
	if ((addr = parent->allocate(&sz)))
	{
	    region(addr, sz);
	}
	/* Now we try our tail. */
	if (tail->size >= size)
	{
	    return tail;
	}
    }
    return ZERO;
}

Address ListAllocator::allocate(Size *size)
{
    MemBlock *b;
    
    /* First try to find a free MemBlock. */
    if((b = findFreeBlock(*size)) == ZERO)
    {
	return ZERO;
    }
    /* Split it if possible. */
    if (*size + sizeof(MemBlock) < b->size)
    {
	MemBlock *n = (MemBlock *) (((Address) (b + 1)) + *size);
	n->size  = b->size - (*size) - sizeof(MemBlock);
	b->size  = *size;
	n->free  = TRUE;
	n->next  = b->next;
	n->prev  = b;
	
	if (b->next != ZERO)
	    b->next->prev = n;
	b->next  = n;
    }
    /* Return the block. */
    b->free = FALSE;
    return (Address) (b + 1);
}

void ListAllocator::release(Address addr)
{
    MemBlock *b = (MemBlock *) (addr - sizeof(MemBlock));

    /* Merge with previous. */
    if (b->prev && b->prev->free)
    {
        b->prev->size += (b->size + sizeof(MemBlock));
        b->prev->next  = b->next;
        if (b->next) b->next->prev = b->prev;
    }
    /* Merge with next */
    else if (b->next && b->next->free)
    {
        b->size += (b->next->size + sizeof(MemBlock));
        b->next  = b->next->next;
        if (b->next->next) b->next->prev = b;
    }
    /* Mark free. */
    b->free = TRUE;
}

void ListAllocator::region(Address addr, Size size)
{
    /* Fill in limit and head. */
    limit = size;
    
    /* Set the head node. */
    if (!head)
    {
        /* Initialize the head node. */
	if((head = (MemBlock *) addr))
	{
	    /* Mark it free, set size. */
    	    head->size = limit - sizeof(MemBlock);
	    head->free = TRUE;
    	    head->prev = ZERO;
    	    head->next = ZERO;
	}
	tail = head;
    }
    /* Append the region to the tail. */
    else
    {
	MemBlock *b = (MemBlock *) addr;
	b->prev = tail;
	tail->next = b;
	tail = b;
	b->next = ZERO;
	b->size = size;
	b->free = TRUE;
    }
}

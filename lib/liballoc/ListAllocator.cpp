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
#include <Types.h>
#include "ListAllocator.h"

#if 1
#define REVALIDATE() \
    /* Search all available regions. */ \
    for (MemRegion *reg = regions; reg; reg = reg->next) \
    { \
	/* Then walk all it's blocks. */ \
        for (MemBlock *blk = reg->blocks; blk; blk = blk->next) \
	{ \
	    /* Must be a sane memory block. */ \
	    assert(blk->magic  == LISTALLOC_MAGIC); \
	    assert(blk->next   != blk); \
	    assert(blk->prev   != blk); \
	    assert(blk->region == reg); \
	    assert(blk->prev   == ZERO || blk->prev < blk); \
	    assert(blk->next   == ZERO || blk->next > blk); \
	    assert(blk->next   == ZERO || blk->next == (MemBlock *)(((Address)blk) + blk->size + sizeof(MemBlock))); \
	    assert(blk->size > 0); \
	    assert(blk->size < 1024 * 1024 * 16); \
	    assert(blk->size <= blk->region->size); \
	} \
    }
#else
#define REVALIDATE()
#endif
	    
ListAllocator::ListAllocator()
    : regions(ZERO)
{
}

ListAllocator::ListAllocator(Address addr, Size size)
    : regions(ZERO)
{
    region(addr, size);
}

MemBlock * ListAllocator::findFreeBlock(Size size, bool askParent = true)
{
    Size sz;
    Address addr;

    /* Loop all available memory blocks. */
    for (MemRegion *r = regions; r; r = r->next)
    {
	/* Does this region have enough memory? */
	if (r->free >= size)
	{
    	    /* Search all it's available blocks. */
    	    for (MemBlock *b = r->blocks; b; b = b->next)
	    {
		/* Must be a sane memory block. */
	        assert(b->magic  == LISTALLOC_MAGIC);
	        assert(b->next   != b);
	        assert(b->prev   != b);
		assert(b->region == r);
    
		/* Is this block big enough? */
	        if (b->free && b->size >= size)
		{
		    return b;
		}
	    }
	}
    }
    /* If no blocks are available, allocate from parent. */
    if (parent && askParent)
    {
	/* We want a new memory block from our parent. */
	sz = size + sizeof(MemRegion) + sizeof(MemBlock);
	
	/* Ask for more blocks from our parent. */
	if ((addr = parent->allocate(&sz)))
	{
	    region(addr, sz);
	}
	/* Try again. */
	return findFreeBlock(size, false);
    }
    /* Out of memory. */
    return ZERO;
}

Address ListAllocator::allocate(Size *size)
{
    MemBlock *b, *n;
    Address a;
    
    REVALIDATE();
    
    /* First try to find a free MemBlock. */
    if ((b = findFreeBlock(*size)) == ZERO)
    {
	return ZERO;
    }
    /* Split it if possible. */
    if (*size + sizeof(MemBlock) * 4 < b->size)
    {
	/* Create new block. */
	a  = (Address) (b + 1);
	a += *size;
	n  = (MemBlock *) a;
	
	/* Fill it. */
	n->magic  = LISTALLOC_MAGIC;
	n->size   = b->size - (*size) - sizeof(MemBlock);
	n->region = b->region;
	b->size   = *size;
	n->free   = TRUE;
	n->next   = b->next;
	n->prev   = b;
	if (b->next != ZERO)
	    b->next->prev = n;
	b->next   = n;

	REVALIDATE();
    }
    /* Return the block. */
    b->region->free -= *size;
    b->free  = FALSE;
    
    REVALIDATE();
    
    /* Success. */
    return (Address) (b + 1);
}

void ListAllocator::release(Address addr)
{
    MemBlock *b = (MemBlock *) (addr - sizeof(MemBlock));

    REVALIDATE();

    /* Sane block given? */
    assertRead(addr);
    assertWrite(addr);
    assert(b->free   != TRUE);
    assert(b->magic  == LISTALLOC_MAGIC);
    assert(b->prev   != b);
    assert(b->next   != b);
    assert(b->region != ZERO);

    /* Mark free. */
    b->free = TRUE;
    b->region->free += b->size;

    /* Merge with previous. */
    if (b->prev && b->prev->free)
    {
        b->prev->size += (b->size + sizeof(MemBlock));
        b->prev->next  = b->next;
        if (b->next) b->next->prev = b->prev;
	
	REVALIDATE();
    }
    /* Merge with next */
    else if (b->next && b->next->free)
    {
        b->size += (b->next->size + sizeof(MemBlock));
        if (b->next->next) b->next->next->prev = b;
        b->next  = b->next->next;

	REVALIDATE();
    }
    REVALIDATE();
}

void ListAllocator::region(Address addr, Size size)
{
    MemRegion *r = (MemRegion *) (addr);
    MemBlock  *b = (MemBlock *)  (addr + sizeof(MemRegion));

    REVALIDATE();

    /* Assume sane arguments. */
    assert(size > sizeof(MemRegion) + sizeof(MemBlock));
    assertRead(addr);
    assertWrite(addr);
    
    /* Initialize region. */
    r->magic  = LISTALLOC_MAGIC;
    r->size   = size - sizeof(MemRegion) - sizeof(MemBlock);
    r->free   = r->size;
    r->next   = regions;
    r->blocks = b;

    /* Initialize block. */
    b->magic  = LISTALLOC_MAGIC;
    b->free   = TRUE;
    b->prev   = ZERO;
    b->next   = ZERO;
    b->size   = r->size;
    b->region = r;

    /* Make us the new regions head. */
    regions = r;
    
    REVALIDATE();
}

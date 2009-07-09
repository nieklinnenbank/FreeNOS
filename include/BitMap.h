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

#ifndef __BITMAP_H
#define __BITMAP_H

#include "Macros.h"
#include "Assert.h"
#include <string.h>

/**
 * Class capable of (un)marking bits inside a binary bitmap.
 */
class BitMap
{
    public:

	/**
	 * Class constructor.
	 * @param newMap Pointer to the bitmap to manage.
	 * @param cnt Number of bits.
	 */
	BitMap(u8 *newMap = ZERO, Size cnt = ZERO)
	    :  count(cnt), free(cnt)
	{
	    map = newMap;
	}
    
	/**
	 * Mark a given bit used.
	 * @param bit Bit number to mark used.
	 */
	void mark(Size bit)
	{
	    assert(bit < count);
	    assertRead(map);
	    assertWrite(map);
	
	    /* Only mark if the bit is free. */
	    if (!isMarked(bit))
	    {
		map[bit / 8] |= 1 << (bit % 8);
		free--;
	    }
        }
	
	/**
	 * Mark a range of bits inside the map.
	 * @param from Bit to start with.
	 * @param to End bit (inclusive).
	 */
	void markRange(Size from, Size to)
	{
	    for (Size i = from; i <= to; i++)
	    {
		mark(i);
	    }
	}
	
	/**
	 * Marks the next free bit used.
	 * @return Bit number on success and -1 otherwise.
	 */
	Error markNext()
	{
	    u32 *ptr;
	    Size num = count / sizeof(u32);

    	    /* At least one, and include partially used bytes. */
    	    if (!num || count % sizeof(u32))
	    {
		num++;
	    }
    	    /* Scan bitmap as fast as possible. */
    	    for (Size i = 0; i < num; i++)
    	    {
        	/* Point to the correct offset. */
        	ptr = ((u32 *) (map)) + i;

        	/* Any blocks free? */
        	if (*ptr != (u32) ~ZERO)
        	{
            	    /* Find the first free bit. */
            	    for (Size bit = 0; bit < sizeof(u32) * 8; bit++)
            	    {
            		if (!(*ptr & (1 << bit)))
                	{
                    	    *ptr |= (1 << bit);
                	    free--;
			    return bit + (sizeof(u32) * 8 * i);
                	}
            	    }
		}
            }
	    /* No free bits left! */
	    return -1;
        }
	
	/**
	 * Unmarks the given bit.
	 * @param bit Bit number to unmark.
	 */
	void unmark(Size bit)
	{
	    assert(bit < count);
	    assertRead(map);
	    assertWrite(map);
	    
	    /* Only unmark if the bit is marked now. */
	    if (isMarked(bit))
	    {
		map[bit / 8] &= ~(1 << (bit % 8));
		free++;
	    }
	}
    
	/**
	 * Verify if a given bit is marked.
	 * @param bit Bit number to check.
	 * @return True if marked, false otherwise.
	 */
	bool isMarked(Size bit) const
	{
	    assert(bit < count);
	    assertRead(map);
	
	    return map[bit / 8] & (1 << (bit % 8));
	}

	/**
	 * Retrieve a pointer to the internal bitmap.
	 * @return Internal bitmap.
	 */
	u8 * getMap() const
	{
	    return map;
	}

	/**
	 * Use the given pointer as the bitmap buffer.
	 * @param newMap New bitmap pointer.
	 * @param newCount New number of bits. ZERO to keep the old value.
	 */
	void setMap(u8 *newMap, Size newCount = ZERO)
	{
	    /* Set bits count. */
	    if (newCount)
	    {
		count = newCount;
	    }
	    /* Reassign to the new map. */
	    map = newMap;
	}
	
	/**
	 * Clears the given map.
	 */
	void clear()
	{
	    Size bytes = (count / 8);
	    
	    /* Partial byte. */
	    if (count % 8)
	    {
		bytes++;
	    }
	    /* Zero it. */
	    memset(map, 0, bytes);
	    
	    /* Reset free count. */
	    free = count;
	}
	
	/**
	 * Get the number of bits used in the map.
	 * @return Number of used bits.
	 */
	Size getUsed() const
	{
	    return (count - free);
	}
	
	/**
	 * Get the number of bits available in the map.
	 * @return Number of free bits.
	 */
	Size getFree() const
	{
	    return free;
	}
    
    private:
    
	/** Total number of bits in the map. */
	Size count;
        
	/** Unmarked bits remaining. */
	Size free;

	/** Bitmap which represents free and used blocks. */
	u8 *map;
};

#endif /* __BITMAP_H */

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

#include "BitMap.h"
#include "MemoryBlock.h"

BitMap::BitMap(u8 *newMap, Size cnt) : count(cnt), free(cnt)
{
    map = newMap;
}

void BitMap::mark(Size bit)
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

void BitMap::markRange(Size from, Size to)
{
    for (Size i = from; i <= to; i++)
    {
        mark(i);
    }
}

Error BitMap::markNext()
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

void BitMap::unmark(Size bit)
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

bool BitMap::isMarked(Size bit) const
{
    assert(bit < count);
    assertRead(map);

    return map[bit / 8] & (1 << (bit % 8));
}

u8 * BitMap::getMap() const
{
    return map;
}

void BitMap::setMap(u8 *newMap, Size newCount)
{
    /* Set bits count. */
    if (newCount)
    {
        count = newCount;
    }
    /* Reassign to the new map. */
    map = newMap;
}

void BitMap::clear()
{
    Size bytes = (count / 8);

    /* Partial byte. */
    if (count % 8)
    {
        bytes++;
    }
    /* Zero it. */
    MemoryBlock::set(map, 0, bytes);

    /* Reset free count. */
    free = count;
}

Size BitMap::getUsed() const
{
    return (count - free);
}

Size BitMap::getFree() const
{
    return free;
}

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

BitMap::BitMap(u8 *map, Size size)
{
    m_map  = map ? map : new u8[size];
    m_size = size;
    m_free = size;
    clear();
}

void BitMap::mark(Size bit)
{
    assert(bit < m_size);
    assertRead(m_map);
    assertWrite(m_map);

    /* Only mark if the bit is free. */
    if (!isMarked(bit))
    {
        m_map[bit / 8] |= 1 << (bit % 8);
        m_free--;
    }
}

void BitMap::unmark(Size bit)
{
    assert(bit < (m_size / 8));
    assertRead(m_map);
    assertWrite(m_map);

    /* Only unmark if the bit is marked now. */
    if (isMarked(bit))
    {
        m_map[bit / 8] &= ~(1 << (bit % 8));
        m_free++;
    }
}

bool BitMap::isMarked(Size bit) const
{
    assert(bit < (m_size / 8));
    assertRead(m_map);

    return m_map[bit / 8] & (1 << (bit % 8));
}

void BitMap::markRange(Size from, Size to)
{
    for (Size i = from; i <= to; i++)
    {
        mark(i);
    }
}

Error BitMap::markNext(Size count, Size start)
{
    Size from = 0, found = 0;

    /* Loop bitmap for free bits. */
    for (Size i = start; i < m_size; i++)
    {
        if (!isMarked(i))
        {
            /* Remember this bit. */
            if (!found)
            {
                from  = i;
                found = 1;
            }
            else
                found++;

            /* Are there enough contigious bits? */
            if (found >= count)
            {
                markRange(from, i);
                return from;
            }
        }
        else
        {
            from = found = 0;
        }
    }
    /* No free bits left! */
    return -1;
}

u8 * BitMap::getMap() const
{
    return m_map;
}

void BitMap::setMap(u8 *map, Size size)
{
    /* Set bits count. */
    if (size)
    {
        m_size = size;
    }
    /* Reassign to the new map. */
    m_map = map;
}

void BitMap::clear()
{
    Size bytes = (m_size / 8);

    /* Partial byte. */
    if (m_size % 8)
    {
        bytes++;
    }
    /* Zero it. */
    MemoryBlock::set(m_map, 0, bytes);

    /* Reset free count. */
    m_free = m_size;
}

Size BitMap::getUsed() const
{
    return (m_size - m_free);
}

Size BitMap::getFree() const
{
    return m_free;
}

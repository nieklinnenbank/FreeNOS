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

#include "Assert.h"
#include "BitArray.h"
#include "MemoryBlock.h"

BitArray::BitArray(const Size bitCount, u8 *array)
{
    m_array = array ? array : new u8[calculateBitmapSize(bitCount)];
    m_allocated = (array == ZERO);
    m_bitCount  = bitCount;
    m_set = 0;

    clear();
}

BitArray::~BitArray()
{
    if (m_allocated)
    {
        delete[] m_array;
    }
}

Size BitArray::size() const
{
    return m_bitCount;
}

Size BitArray::count(const bool on) const
{
    return on ? m_set : m_bitCount - m_set;
}

void BitArray::set(const Size bit, const bool value)
{
    // Check if the bit is inside the array
    if (bit >= m_bitCount)
    {
        return;
    }

    // Check current value
    bool current = m_array[bit / 8] & (1 << (bit % 8));

    // Update the bit only if needed (and update administration)
    if (current != value)
    {
        if (value)
        {
            m_array[bit / 8] |= 1 << (bit % 8);
            m_set++;
        }
        else
        {
            m_array[bit / 8] &= ~(1 << (bit % 8));
            m_set--;
        }
    }
}

void BitArray::unset(const Size bit)
{
    set(bit, false);
}

bool BitArray::isSet(const Size bit) const
{
    assert(bit < m_bitCount);

    return m_array[bit / 8] & (1 << (bit % 8));
}

void BitArray::setRange(const Size from, const Size to)
{
    for (Size i = from; i <= to; i++)
    {
        set(i, true);
    }
}

BitArray::Result BitArray::setNext(Size *bit,
                                   const Size count,
                                   const Size start,
                                   const Size boundary)
{
    Size from = 0, found = 0;

    // Loop BitArray for unset bits
    for (Size i = start; i < m_bitCount;)
    {
        // Try to fast-forward 32 bits to search
        if (m_bitCount > 32 && i < m_bitCount - 32 && ((u32 *)m_array)[i / 32] == 0xffffffff)
        {
            from = found = 0;

            if (i & 31)
                i += (32 - (i % 32));
            else
                i += 32;
            continue;
        }
        // Try to fast-forward 8 bits to search
        else if (m_bitCount > 8 && i < m_bitCount - 8 && m_array[i / 8] == 0xff)
        {
            from = found = 0;

            if (i & 7)
                i += (8 - (i % 8));
            else
                i += 8;
            continue;
        }
        else if (!isSet(i))
        {
            // Remember this bit
            if (!found)
            {
                if (!(i % boundary))
                {
                    from  = i;
                    found = 1;
                }
            }
            else
            {
                found++;
            }

            // Are there enough contigious bits?
            if (found >= count)
            {
                setRange(from, i);
                *bit = from;
                return Success;
            }
        }
        else
        {
            from = found = 0;
        }

        // Move to the next bit
        i++;
    }
    // No unset bits left!
    return OutOfMemory;
}

u8 * BitArray::array() const
{
    return m_array;
}

void BitArray::setArray(u8 *map, const Size bitCount)
{
    // Set bits count
    if (bitCount)
    {
        m_bitCount = bitCount;
    }

    // Cleanup old array, if needed
    if (m_array && m_allocated)
    {
        delete[] m_array;
    }

    // Reassign to the new map
    m_array = map;
    m_allocated = false;
    m_set   = 0;

    // Recalculate set bits
    for (Size i = 0; i < m_bitCount; i++)
    {
        if (isSet(i))
        {
            m_set++;
        }
    }
}

void BitArray::clear()
{
    // Zero it
    MemoryBlock::set(m_array, 0, calculateBitmapSize(m_bitCount));

    // Reset set count
    m_set = 0;
}

bool BitArray::operator[](const Size bit) const
{
    return isSet(bit);
}

bool BitArray::operator[](const int bit) const
{
    return isSet(bit);
}

inline Size BitArray::calculateBitmapSize(const Size bitCount) const
{
    const Size bytes = bitCount / 8;

    if (bitCount % 8)
        return bytes + 1;
    else
        return bytes;
}

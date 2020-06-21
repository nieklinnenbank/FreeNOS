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

#ifndef __LIBSTD_BITARRAY_H
#define __LIBSTD_BITARRAY_H

#include "Macros.h"
#include "Assert.h"
#include "Types.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

/**
 * Represents an array of bits.
 */
class BitArray
{
  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        InvalidArgument,
        OutOfMemory
    };

  public:

    /**
     * Class constructor.
     *
     * @param bitCount Number of bits to manage.
     * @param array Optional pointer to pre-allocated bits array to manage.
     */
    BitArray(const Size bitCount, u8 *array = ZERO);

    /**
     * Class destructor.
     */
    virtual ~BitArray();

    /**
     * Returns the maximum size of this Container.
     *
     * @return size The maximum size of this Container.
     */
    Size size() const;

    /**
     * Get the number of bits in the map which have the given value.
     *
     * @param on True to get the number of 1-bits, false for 0-bits.
     *
     * @return Number of bits with the given value.
     */
    Size count(const bool on) const;

    /**
     * Sets the given bit to the given value.
     *
     * @param bit Bit number to set.
     * @param value Value to set.
     */
    void set(const Size bit, const bool value = true);

    /**
     * Set a range of bits inside the map to 1.
     *
     * @param from Bit to start with.
     * @param to End bit (inclusive).
     */
    void setRange(const Size from, const Size to);

    /**
     * Sets the next unset bit(s).
     *
     * @param bit Start bit number on success.
     * @param count Number of consequetive bits required.
     * @param offset Start bit number to start searching at inside the BitArray.
     * @param boundary First bit number must be on the given alignment boundary.
     *
     * @return Result code.
     */
    Result setNext(Size *bit,
                   const Size count = 1,
                   const Size offset = 0,
                   const Size boundary = 1);

    /**
     * Sets the given bit to zero.
     *
     * @param bit Bit number to unset.
     */
    void unset(const Size bit);

    /**
     * Set all bits to zero.
     */
    void clear();

    /**
     * Verify if a given bit is set.
     *
     * @param bit Bit number to check.
     *
     * @return True if marked, false otherwise.
     */
    bool isSet(const Size bit) const;

    /**
     * Retrieve a pointer to the internal BitArray.
     *
     * @return Internal BitArray.
     */
    u8 * array() const;

    /**
     * Use the given pointer as the BitArray buffer.
     *
     * @param array New bits array pointer.
     * @param bitCount New number of bits. ZERO to keep the old value.
     */
    void setArray(u8 *array, const Size bitCount = ZERO);

    /**
     * Retrieve the value of the given bit.
     *
     * @param bit Bit number to find
     *
     * @return True if set, false otherwise
     */
    bool operator[](const Size bit) const;

    /**
     * Retrieve the value of the given bit
     *
     * @param bit Bit number to find
     *
     * @return True if set, false otherwise
     */
    bool operator[](const int bit) const;

  private:

    /**
     * Calculate required size of bitmap array in bytes.
     *
     * @param bitCount Minimum number of bits needed to store.
     *
     * @return Size of bitmap array in bytes
     */
    Size calculateBitmapSize(const Size bitCount) const;

  private:

    /** Total number of bits in the array. */
    Size m_bitCount;

    /** Set bits in the array. */
    Size m_set;

    /** Array containing the bits. */
    u8 *m_array;

    /** True if m_array was allocated interally. */
    bool m_allocated;
};

/**
 * @}
 * @}
 */

#endif /* __LIBSTD_BITARRAY_H */

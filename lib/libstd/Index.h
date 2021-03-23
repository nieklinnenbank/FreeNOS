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

#ifndef __LIB_LIBSTD_INDEX_H
#define __LIB_LIBSTD_INDEX_H

#include "Assert.h"
#include "Types.h"
#include "Macros.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

/**
 * Index is a N-sized array of pointers to items of type T.
 */
template <class T, const Size N> class Index
{
  public:

    /**
     * Constructor.
     */
    Index()
        : m_count(0)
    {
        for (Size i = 0; i < N; i++)
        {
            m_array[i] = ZERO;
        }
    }

    /**
     * Adds the given item, if possible.
     *
     * @param position On output the position of the item in the Index.
     * @param item Pointer to the item to add.
     *
     * @return True on success, false otherwise.
     */
    virtual bool insert(Size & position, T *item)
    {
        // Check if we are full
        if (m_count == N)
        {
            return false;
        }
        // The item must point to an object
        else if (item == ZERO)
        {
            return false;
        }

        // There is space, add the item.
        for (Size i = 0; i < N; i++)
        {
            if (m_array[i] == ZERO)
            {
                m_array[i] = item;
                m_count++;
                position = i;
                return true;
            }
        }

        // Should not be reached
        assert(false);
        return false;
    }

    /**
     * Adds the given item, if possible.
     *
     * @param item Pointer to the item to add.
     *
     * @return True on success, false otherwise.
     */
    virtual bool insert(T *item)
    {
        Size ignored = 0;
        return insert(ignored, item);
    }

    /**
     * Inserts the given item at the given position.
     *
     * If an item exists at the given position, it will be replaced by the given item.
     *
     * @param position The position to insert the item.
     * @param item The item to insert
     *
     * @return True on success, false otherwise.
     */
    virtual bool insertAt(const Size position, T *item)
    {
        // Position must be in range of the array
        if (position >= N)
        {
            return false;
        }
        // The item must point to an object
        else if (item == ZERO)
        {
            return false;
        }

        // Increment counter only when needed
        if (m_array[position] == ZERO)
        {
            m_count++;
        }

        m_array[position] = item;
        return true;
    }

    /**
     * Removes the item at the given position.
     *
     * @param position The position of the item to remove.
     *
     * @return bool Whether removing the item succeeded.
     */
    virtual bool remove(const Size position)
    {
        // Position must be in range of the array
        if (position >= N)
        {
            return false;
        }

        // See if the item exists
        if (!m_array[position])
        {
            return false;
        }

        m_array[position] = ZERO;
        assert(m_count >= 1);
        m_count--;
        return true;
    }

    /**
     * Removes and delete()'s all items.
     */
    void deleteAll()
    {
        for (Size i = 0; i < N; i++)
        {
            if (m_array[i] != ZERO)
            {
                delete m_array[i];
                m_array[i] = ZERO;
            }
        }

        m_count = 0;
    }

    /**
     * Returns the item at the given position.
     *
     * @param position The position of the item to get.
     *
     * @return Pointer to the item at the given position or ZERO if no item available.
     */
    virtual T * get(const Size position) const
    {
        // Position must be in range of the array
        if (position >= N)
        {
            return ZERO;
        }

        return m_array[position];
    }

    /**
     * Check if the given item is stored in this Sequence.
     */
    virtual bool contains(const T *item) const
    {
        for (Size i = 0; i < N; i++)
        {
            if (m_array[i] == item)
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Size of the Index.
     */
    virtual Size size() const
    {
        return N;
    }

    /**
     * Item count in the Index.
     */
    virtual Size count() const
    {
        return m_count;
    }

   /**
     * Returns the item at the given position in the Index.
     *
     * @param i The position of the item to return.
     *
     * @return the Item at position i or ZERO if no item available.
     */
    T * operator [] (const Size i)
    {
        return get(i);
    }

  private:

    /** Array of pointers to items. */
    T* m_array[N];

    /** Amount of valid pointers in the array. */
    Size m_count;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBSTD_INDEX_H */

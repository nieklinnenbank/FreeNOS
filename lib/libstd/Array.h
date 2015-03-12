/*
 * Copyright (C) 2015 Niek Linnenbank
 * Copyright (C) 2009 Coen Bijlsma
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

#ifndef __LIBSTD_ARRAY_H
#define __LIBSTD_ARRAY_H

#include "Assert.h"
#include "Comparable.h"
#include "Types.h"
#include "Macros.h"

/** Default size of an Array */
#define ARRAY_DEFAULT_SIZE  64

/**
 * This is a wrapper class for an array and contains some extra
 * functionality, somewhat like the Arrays class in Java.
 */
template <class T> class Array : public Comparable<Array<T> >
{
  public:

    /**
     * Constructor.
     * Initializes the array with the given Size.
     *
     * @param size The maximum size of the array
     */
    Array(Size size = ARRAY_DEFAULT_SIZE)
    {
        assert(size > 0);

        m_size  = size;
        m_count = 0;
        m_array = new T[m_size];

        clear();
        
    }
    
    /**
     * Copy constructor.
     *
     * @param a Array pointer to copy from.
     */
    Array(Array<T> *a)
    {
        assert(a->m_size > 0);

        m_size  = a->m_size;
        m_count = a->m_count;
        m_array = new T[m_size];
        
        for (Size i = 0; i < m_size; i++)
        {
            m_array[i] = a->m_array[i];
        }
    }

    /**
     * Destructor.
     */
    ~Array()
    {
        delete[] m_array;
    }    

    /**
     * Adds the given item to the Array, if possible.
     *
     * @param item The item to add to the Array.
     * @return Position of the item in the Array or -1 on failure.
     */
    int insert(T item)
    {
        for (Size i = 0; i < m_size; i++)
        {
            if (!m_array[i])
            {
                m_array[i] = item;
                m_count++;
                return i;
            }
        }
        return -1;
    }
    
    /**
     * Inserts the given item at the given position.
     * If an item exists at the given position, it will be replaced by the given item.
     *
     * @param position The position to insert the item.
     * @param item The item to insert
     * @return bool Whether inserting the item at the given position succeeded.
     */
    bool insert(Size position, T item)
    {
        if (position >= m_size)
        {
            return false;
        }
        m_array[position] = item;
        m_count++;
        return true;
    }
    
    /**
     * Removes the item at the given position.
     *
     * @param position The position of the item to remove.
     * @return bool Whether removing the item succeeded.
     */
    bool remove(Size position)
    {
        if (position >= m_size || position == 0)
        {
            return false;
        }
        m_array[position] = ZERO;
        m_count--;
        return true;
    }

    /**
     * Removes all items from the array.
     */
    void clear()
    {
        for (Size i = 0; i < m_size; i++)
        {
            m_array[i] = ZERO;
        }
    }
    
    /**
     * Returns the item at the given position.
     *
     * @param position The position of the item to get.
     * @return The item at the given position.
     */
    T get(Size position) const
    {
        if (position >= m_size)
        {
            return ZERO;
        }
        return m_array[position];
    }
    
    /**
     * Returns the maximum size of this Array.
     *
     * @return size The maximum size of this Array.
     */
    Size size() const
    {
        return m_size;
    }

    /**
     * Returns the number of items inside the Array.
     * @return Number of items inside the Array.
     */
    Size count() const
    {
        return m_count;
    }
    
    /**
     * Returns a shallow copy of this Array.
     * Shallow means that the copy will point to the same values as the original.
     * @return A clone of this Array.
     */
    Array<T> clone() const
    {
        Array<T> array(m_size);
        
        for (Size i = 0; i < m_size; i++)
        {
            if (m_array[i])
                array.insert(i, m_array[i]);
        }
        return array;
    }

    /**
     * Compare this Array to another Array.
     */
    virtual int compareTo(const Array<T> &t) const
    {
        // Size must be equal
        if (t.m_size != this->m_size)
        {
            return this->m_size - t.m_size;
        }
    
        // All elements must be equal
        for (Size s = 0; s < m_size; s++)
        {
            if (this->get(s) != t.get(s))
            {
                return s + 1;
            }
        }
        return 0;

    }
    
    /**
     * Find the first empty index inside the Array.
     * @return Index of empty position or -1 if full.
     */
    int findEmpty() const
    {
        for (Size i = 0; i < m_size; i++)
        {
            if (!m_array[i])
                return i;
        }
        return -1;
    }

    /**
     * Returns the item at the given position in the Array.
     * @param i The index of the item to return.
     * @return the Item at position i, or NULL of it doesn't exist.
     */
    T operator [] (int i) const
    {
        return (i >= 0 && i < (int) m_size) ? m_array[i] : ZERO;
    }
    
  protected:

    /** The actual array where the data is stored. */
    T *m_array;
    
    /** The maximum size of the array. */
    Size m_size;

    /** Number of used items in the array. */
    Size m_count;
};

#endif /* __LIBSTD_ARRAY_H */

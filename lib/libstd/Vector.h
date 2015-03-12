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

#ifndef __LIBSTD_VECTOR_H
#define __LIBSTD_VECTOR_H

#include "Macros.h"
#include "Types.h"
#include "Assert.h"
#include "Array.h"

/**
 * Simple vector template class.
 *
 * @author Niek Linnenbank
 * @author Coen Bijlsma (_expand())
 */
template <class T> class Vector : public Array<T>
{
  public:

    /**
     * Insert an item in the first empty position.
     * @param t Item to insert.
     * @return Index of the item on success or -1 otherwise.
     */
    int insert(T t)
    {
        assertRead(t);
    
        if ((this->m_count + 1) == this->m_size)
        {
            expand();
        }
        return Array<T>::insert(t);
    }
    
    /**
     * Insert an item in the m_vector.
     * @param pos Position in the m_vector.
     * @param t Data item to te inserted.
     */
    bool insert(Size pos, T t)
    {
        assertRead(t);
    
        if (pos < this->m_size)
        {
            expand(pos - this->m_size + 1);
        }
        return Array<T>::insert(pos, t);
    }

  private:

    /**
     * Expand the Vector.
     */
    void expand(Size increase = ARRAY_DEFAULT_SIZE)
    {
        T* arr = new T[this->m_size + increase];
        
        // Copy the old array in the new one
        for (Size i = 0; i < this->m_size; i++)
        {
            arr[i] = this->m_array[i];
        }   
        // Set the new items to 0
        for (Size i = this->m_size; i < (this->m_size + increase); i++)
        {
            arr[i] = 0;
        }
        // Clean up the old array and set the new one
        delete this->m_array;
        this->m_array = arr;
        this->m_size += increase;
    }
};

#endif /* __LIBSTD_VECTOR_H */

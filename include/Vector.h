/**
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

#ifndef __VECTOR_H
#define __VECTOR_H

#include <string.h>
#include "Macros.h"
#include "Types.h"
#include "Assert.h"

/** Default size of a Vector. */
#define VECTOR_DEFAULT_SIZE	64

/**
 * Simple array template class.
 */
template <class T> class Vector
{
    public:

	/**
	 * Class constructor.
	 * @param sz Maximum number of item this Vector can hold.
	 */
	Vector(Size sz = VECTOR_DEFAULT_SIZE) : _size(sz)
	{
	    assert(sz > 0);
	    vec = new T*[_size];
	    
	    for (uint i = 0; i < sz; i++)
		vec[i] = 0;
	}
	
	/**
	 * Class destructor.
	 */
	~Vector()
	{
	    assertWrite(vec);
	    delete vec;
	}
    
	/**
	 * Insert an item in the first empty position.
	 * @param t Item to insert.
	 * @return Index of the item on success or -1 otherwise.
	 */
	int insert(T *t)
	{
	    assertRead(t);
	
	    for (Size i = 0; i < _size; i++)
	    {
		if (vec[i] == ZERO)
		{
		    vec[i] = t;
		    return i;
		}
	    }
	    return -1;
	}
    
	/**
	 * Insert an item in the Vector.
	 * @param pos Position in the Vector.
	 * @param t Data item to te inserted.
	 */
	void insert(Size pos, T *t)
	{
	    assertRead(t);
	
	    if (pos < _size)
	    {
		vec[pos] = t;
	    }
	}
    
	/**
	 * Remove an item from the list.
	 * @param pos Position of the item in the Vector.
	 */
	void remove(Size pos)
	{
	    if (pos < _size)
	    {
		vec[pos] = ZERO;
	    }
	}
	
	/**
	 * Get an item from the Vector.
	 * @param pos Position in the Vector of the requested item.
	 * @return Item from the Vector.
	 */
	T* get(Size pos)
	{
	    if (pos < _size)
	    {
	        assertRead(vec[pos]);
		return vec[pos];
	    }
	    return (ZERO);
	}

	/**
	 * Retrieve the size of the Vector.
	 * @return Size of the internal array.
	 */
	Size size() const
	{
	    return _size;
	}
    
        /**
	 * Lookup an item in the Vector.
	 * @param Index of the item to find.
	 * @return Pointer to the associated item on success or ZERO otherwise.
	 */
	T* operator [] (int i) const
	{
	    return (i >= 0 && i < (int) _size) ? vec[i] : ZERO;
	}

    private:

	/** Array of item pointers. */
	T **vec;
	
	/** Size of the array. */
	Size _size;
};

#endif /* __VECTOR_H */

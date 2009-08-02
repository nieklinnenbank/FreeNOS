/*
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

#ifndef __ARRAY_H
#define __ARRAY_H

#include "Assert.h"
#include "Comparable.h"
#include "Types.h"

/** Default size of an Array */
#define ARRAY_DEFAULT_SIZE	64

/**
 * This is a wrapper class for an array and contains some extra
 * functionality, somewhat like the Arrays class in Java.
 */
template <class T> class Array
{
    public:

	/**
	 * Constructor.
	 * Initializes the array with the given Size.
	 * @param size The maximum size of the array
	 */
	Array(Size size = ARRAY_DEFAULT_SIZE) : _size(size)
	{
	    assert(size > 0);
	    _array = new T*[_size];
		
	    for( Size i = 0; i < _size; i++)
	    {
		_array[i] = 0;
	    }
	}
	
	/**
	 * Copy constructor.
	 * @param a Array pointer to copy from.
	 */
	Array(Array<T> *a) : _size(a->_size)
	{
	    assert(_size > 0);
	    _array = new T*[_size];
	    
	    for (Size i = 0; i < _size; i++)
	    {
		_array[i] = a->_array[i];
	    }
	}
	
	/**
	 * Adds the given item to the Array, if possible.
	 * @param item The item to add to the Array.
	 * @return bool Whether adding the item to the Array succeeded.
	 */
	int insert(T* item)
	{
	    for(Size i = 0; i < _size; i++)
	    {
		if( _array[i] == ZERO )
		{
		    _array[i] = item;
		    return i;
		}
	    }
	    return -1;
	}
	
	/**
	 * Inserts the given item at the given position. If an item exists
	 * at the given position, it will be replaced by the given item.
	 * @param position The position to insert the item.
	 * @param item The item to insert
	 * @return bool Whether inserting the item at the given position
	 * succeeded.
	 */
	bool insert(Size position, T* item)
	{
	    if( position >= _size )
	    {
		return false;
	    }
	    _array[position] = item;
	    return true;
	}
	
	/**
	 * Removes the item at the given position.
	 * @param position The position of the item to remove.
	 * @return bool Whether removing the item succeeded.
	 */
	bool remove(Size position)
	{
	    if( position >= _size || position == 0 )
	    {
		return false;
	    }
	    _array[position] = (T*) NULL;
	    return true;
	}
	
	/**
	 * Returns the item at the given position.
	 * @param position The position of the item to get.
	 * @return The item at the given position.
	 */
	T* get(Size position)
	{
	    if( position >= _size )
	    {
		return ZERO;
	    }
	    return _array[position];
	}
	
	/**
	 * Returns the maximum size of this Array.
	 * @return size The maximum size of this Array.
	 */
	Size size() const
	{
	    return _size;
	}
	
	/**
	 * Returns a shallow copy of this Array.
	 * @return A clone of this Array.
	 */
	Array<T> clone()
	{
	    Array<T> array(_size);
		
	    for( Size s = 0; s < _size; s++)
	    {
		array.insert(s, _array[s]);
	    }
	    return array;
	}
	
	/**
	 * Compares this Array to the given one.
	 * @param t The Array to compare this one to.
	 * @return Whether this Array and the given one are equal.
	 */
	bool equals(const Array<T> &t)
	{
	    if( *t == this )
	    {
		return true;
	    }
	    if( t.size() != this->size() )
	    {
		return false;
	    }
	
	    for( Size s = 0; s < _size; s++ )
	    {
		if( this->get(s) != t.get(s) )
		{
		    return false;
		}
	    }
	    return true;
	}
	
	/**
	 * Read the array byte-wise.
	 * @param index Index of the item inside the Array to read.
	 * @return The item at the given index.
	 */
	u8 valueAt(Size index) const
	{
	    assert(index < _size);
	    return (u8) _array[index];
	}
	
	/**
	 * Returns the item at the given position in the Array.
	 * @param i The index of the item to return.
	 * @return the Item at position i, or NULL of it doesn't exist.
	 */
	T* operator [] (int i) const
	{
	    return( i >= 0 && i < (int) _size) ? _array[i] : (T *) NULL;
	}
	
	/**
	 * Appends the given Array to this one.
	 * @param value The Array to append to this one.
	 * @return This Array.
	 */
	Array& operator+=  (Array<T>& value)
	{
	    T** array = new T*( _size + value.size() );
		
	    for( Size s = 0; s < _size; s++ )
	    {
		array[s] = this->get(s);
	    }
	    for( Size s = 0; s < value.size(); s++ )
	    {
		array[(s + _size - 1)] = value.get(s);
	    }
	    delete _array;
	    _array = array;
	    _size += value.size();
	    return *this;
	}
	
	/**
	 * Appends the contents of the given Array to this one.
	 * @param value The array to append to this one.
	 * @return The result.
	 */
	Array& operator+ (Array<T>& value)
	{
	    return Array(*this) += value;
	}

    private:

	/** The actual array where the data is stored. */
	T** _array;
	
	/** The maximum size of the array. */
	Size _size;
};

#endif /* __ARRAY_H */

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

#ifndef __COMPARABLE_H
#define __COMPARABLE_H

#include "Types.h"

/**
 * Objects which can be compared to eachother.
 */
template <class T> class Comparable
{
    public:

	/**
	 * Class destructor.
	 */
	virtual ~Comparable() {}

	/**
	 * Test if an object is equal to an other object.
	 * @param t Object instance.
	 * @return True if equal, false otherwise.
	 */
	virtual bool equals(const T &t) = 0;
	
	/**
	 * Compares this Comparable to the given
	 * Comparable and returns whether this Comparable
	 * is equal to, less, or greater then the given Comparable.
	 * @param c The Comparable to compare us to.
	 * @return an int < 0, 0, > 0 if we are respectively less then,
	 * equal to or greater then the given Comparable.
	 */
	virtual int compareTo(const T &t) = 0;
	
	/**
	 * Get the size of the object.
	 * @return Size in bytes.
	 */
	virtual Size size() const = 0;
	
	/**
	 * Read an object byte-wise (e.g. for hashing).
	 * @param index Offset to read.
	 * @return Unsigned byte.
	 */
	virtual u8 valueAt(Size index) const = 0;
};

#endif /* __COMPARABLE_H */

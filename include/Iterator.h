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

#ifndef __ITERATOR_H
#define __ITERATOR_H

/**
 * Abstracts the iteration process.
 */
template <class T, class P> class Iterator
{
    public:

	/**
	 * Destructor.
	 */
	virtual ~Iterator() {}

	/**
	 * Restart iterating using the given instance.
	 * @param P Begin iterating here.
	 */
	virtual void reset(P) = 0;

	/**
	 * Check if there is more to iterate.
	 * @return true if more items, false if not.
	 */
	virtual bool hasNext() const = 0;
	
	/**
	 * Get the current item.
	 * @return Pointer to the current item.
	 */
	virtual T* current() = 0;
	
	/**
	 * Fetch the next item.
	 * @return Pointer to the next item on the iterator.
	 */
	virtual T* next() = 0;

	/**
	 * Simple wrapper around next().
	 */
	void operator ++(int num)
	{
	    next();
	}
};

#endif /* __ITERATOR_H */

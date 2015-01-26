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

#ifndef __LISTITERATOR_H
#define __LISTITERATOR_H

#include "Macros.h"
#include "Iterator.h"
#include "List.h"
#include "Assert.h"

/**
 * Iterate through a List.
 */
template <class T> class ListIterator : public Iterator<T, List<T> *>
{
    public:

	/**
	 * Empty constructor.
	 */
	ListIterator()
	{
	    list = ZERO;
	    cur  = nxt = ZERO;
	}

	/**
	 * Class constructor.
	 * @param lst Points to the List to iterate.
	 */
	ListIterator(List<T> *lst)
	{
	    assertRead(lst);
	    reset(lst);
	}
	
	/**
	 * Class constructor.
	 * @param lst Reference to the List to iterate.
	 */
	ListIterator(List<T> &lst)
	{
	    reset(&lst);
	}

	/**
	 * Reset the iterator.
	 */
	void reset(List<T> *lst)
	{
	    assertRead(lst);
	    list = lst;
	    cur  = list ? list->firstNode() : ZERO;
	    nxt  = cur  ? cur->next : ZERO;
	}

	/**
	 * Get current item in the List.
	 * @return Current item.
	 */
	T* current()
	{
	    return cur ? cur->data : ZERO;
	}
	
	/**
	 * Check if there is more on the List to iterate.
	 * @return true if more items, false if not.
	 */
	bool hasNext() const
	{
	    return (cur != ZERO);
	}
	
	/**
	 * Fetch the next item.
	 */
	T* next()
	{
	    cur = nxt;
	    if (cur)
	    {
	        nxt = cur->next;
	    }
	    return current();
	}
	
	/**
	 * Post increment operator.
	 */
	void operator++(int n)
	{
	    next();
	}
	
	private:
	
	    /** Points to the List to iterate. */
	    List<T> *list;

	    /** Current and next item. */
	    ListNode<T> *cur, *nxt;
};

#endif /* __LISTITERATOR_H */

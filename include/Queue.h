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

#ifndef __QUEUE_H
#define __QUEUE_H

#include "Macros.h"
#include "List.h"
#include "ListIterator.h"
#include "Assert.h"

/**
 * Queue template class.
 */
template <class T> class Queue
{

    public:
    
	/**
         * Put a new item in the queue.
         * @param t Pointer to the item to enqueue.
         */
        void enqueue(T *t)
	{
	    assertRead(t);
	    list.insertTail(t);
	}
    
        /**
         * Remove the oldest element from the queue (FIFO).
	 * @param t Element to remove (explicitly), or ZERO for the oldest.
         * @return The oldest element.
         */
        T* dequeue(T *t = ZERO)
	{
	    if (!t)
	    {
		t = list.head();
	    }
	    if (t)
	    {
		list.remove(t);
		return t;
	    }
	    else
		return ZERO;
	}

    private:
    
	/** Linked list to implement the queue. */
	List<T> list;
};

#endif /* __QUEUE_H */

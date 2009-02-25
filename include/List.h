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

#ifndef __LIST_H
#define __LIST_H

#include "Macros.h"
#include "Assert.h"

/**
 * Represents an item on the List.
 */
template <class T> class ListNode
{
    public:

        /**
	 * Class constructor.
	 * @param t Data value.
	 * @param p Previous ListNode.
	 * @param n Next ListNode.
	 */
	ListNode (T *t, ListNode *p, ListNode *n)
	    : data(t), prev(p), next(n) 
	{
	    assertRead(t);
	}

    	/** User data. */
	T *data;
	
	/** Previous and next node. */
	ListNode *prev, *next;
};

/**
 * Simple linked list template class.
 */
template <class T> class List
{
    
    public:

	/**
	 * Class constructor.
	 */
	List()
	{
	    headNode = tailNode = 0;
	}
	
	/**
	 * Class destructor.
	 */
	~List()
	{
	    while (headNode)
	    {
		ListNode<T> *tmp = headNode;
		headNode = headNode->next;
		delete tmp;
	    }
	}

	/**
	 * Insert an item at the head.
	 * @param t Data item to te inserted.
	 */
	void insertHead(T *t)
	{
	    assertRead(t);
	    
	    ListNode<T> *n = new ListNode<T>(t, ZERO, headNode);
	    
	    if (headNode)
		headNode->prev = n;
	    if (!tailNode)
		tailNode = n;
	    headNode = n;
	}
	
	/**
	 * Insert an item at the tail.
	 * @param t Item to insert.
	 */
	void insertTail(T *t)
	{
	    assertRead(t);
	
	    ListNode<T> *n = new ListNode<T>(t, tailNode, ZERO);
	    
	    if (tailNode)
		tailNode->next = n;
	    if (!headNode)
		headNode = n;
	    tailNode = n;
	}
    
	/**
	 * Remove an item from the list.
	 * @param t Item to remove.
	 */
	void remove(T *t)
	{
	    assertRead(t);
	
	    for (ListNode<T> *i = headNode; i; i = i->next)
	    {	    
		if (*i->data == t)
		{
		    if (i->prev)
			i->prev->next = i->next;

		    if (i == headNode)
			headNode = i->next;

		    if (i->next)
			i->next->prev = i->prev;

		    if (i == tailNode)
			tailNode = i->prev;

		    delete i;
		    break;
		}
	    }
	}

        /**
	 * Check whether an element is on the List.
	 * @param t The element to find.
	 * @return true if the element is on the List, false otherwise.
	 */
	bool contains(T *t)
	{
	    assertRead(t);
	
	    for (ListNode<T> *i = headNode; i; i = i->next)
	    {
	        if (*i->data == t)
		{
	            return true;
		}
	    }
	    return false;
	}
	
	/**
	 * Clears the entire List.
	 * @param free Invoke a delete() operation on each entry.
	 */
	void clear(bool free = false)
	{
	    for (ListNode<T> *i = headNode, *next; i && (next = i->next); i = i->next)
	    {
		if (free)
		    delete i->data;
		delete i;
	    }
	    headNode = tailNode = ZERO;
	}
	
	/**
	 * Get the head ListNode of the List.
	 * @return First ListNode on the List.
	 */
	T* head()
	{
	    return headNode ? headNode->data : ZERO;
	}
	
	/**
	 * Get the tail of the List.
	 * @return Last entry on the list (tail).
	 */
	T* tail()
	{
	    return tailNode ? tailNode->data : ZERO;
	}

	/**
	 * Check if the List is empty.
	 * @return true if empty, false if not.
	 */
	bool isEmpty() const
	{
	    return headNode ? false : true;
	}

	/** Head of the List. */
	ListNode<T> *headNode, *tailNode;
};

#endif /* __LIST_H */

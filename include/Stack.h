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

#ifndef __STACK_H
#define __STACK_H

#include "StackNode.h"

/**
 * This is an implementaion of the stack datastructure.
 */
template <class T> class Stack
{
    public:

	/**
	 * Constructor.
	 * @param top The top node of this Stack;
	 */
	Stack(StackNode<T>* top) : _top(top)
	{
	}
	
	/**
	 * Constructor
	 */
	Stack() {}
	
	/**
	 * Destructor
	 */
	virtual Stack() {}
	
	/**
	 * Pushes the given node on top of the Stack
	 * @param node The Node to put on top of the Stack
	 */
	void push(StackNode<T>* node)
	{
		if( _top == NULL )
		{
			_top = node;
			return;
		}
		
		StackNode<T>* temp = _top;
		_top = node;
		_top->setNext(temp);
	}
	
	/**
	 * Returns the node that is on top of the Stack and
	 * sets the new top Node to be the next Node of the one
	 * returned here.
	 */
	StackNode<T>* pop()
	{
		StackNode<T>* temp = _top;
		_top = temp->getNext();
		return temp;
	}
	
	/**
	 * Returns the length of this Stack
	 */
	Size length()
	{
		Size s = 0;
		
		for( StackNode<T>* node = _top; node != NULL; node = node->getNext() )
		{
			s++;
		}
		
		return s;
	}
	
	/**
	 * Returns the data from the top Node
	 */
	T* peek()
	{
		return _top->getData();
	}
    private:

	StackNode<T>* _top;
}

#endif /* __STACK_H */

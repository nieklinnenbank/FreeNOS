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

#ifndef __STACKNODE_H
#define __STACKNODE_H

/**
 * 
 */
template <class T> class StackNode
{
    public:

	StackNode(T* data, Node* next = NULL) : _data(data), _next(next)
	{
	}
	
	virtual ~StackNode() {}
	
	T* getData() const
	{
		return _data;
	}
	
	StackNode<T>* getNext() const
	{
		return _next;
	}
	
	void setData(T* data)
	{
		_data = data;
	}
	
	void setNext(StackNode<T>* next)
	{
		_next = next;
	}
	
    private:
	T* _data;
	StackNode<T>* _next;
}

#endif /* __STACKNODE_H */

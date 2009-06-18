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

	Node(T* data, Node* next = NULL) : _data(data), _next(next)
	{
	}
	
	virtual ~Node() {}
	
	T* getData() const
	{
		return _data;
	}
	
	Node* getNext() const
	{
		return _next;
	}
	
	void setData(T* data)
	{
		_data = data;
	}
	
	void setNext(Node* next)
	{
		_next = next;
	}
	
    private:
	T* _data;
	Node* _next;
}

#endif /* __STACKNODE_H */

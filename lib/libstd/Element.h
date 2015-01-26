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

#ifndef __ELEMENT_H
#define __ELEMENT_H

/**
 * This class represents an Element in (i.e.) a Stack
 */
template <class T> class Element
{
    public:

	/**
	 * Constructor.
	 * @param data The data stored in this node.
	 * @param next A pointer to the next node.
	 */
	Element(T* data, Node* next = NULL) : _data(data), _next(next)
	{
	}
	
	/**
	 * Destructor
	 */
	virtual ~Element() {}
	
	/**
	 * Returns the data stored in this node.
	 */
	T* getData() const
	{
	    return _data;
	}
	
	/**
	 * Returns the pointer to the next node.
	 */
	Element<T>* getNext() const
	{
	    return _next;
	}
	
	/**
	 * Sets the data stored in this node.
	 * @param data The data to store.
	 */
	void setData(T* data)
	{
	    _data = data;
	}
	
	/**
	 * Sets the pointer to the next node.
	 * @param next The pointer to the next node.
	 */
	void setNext(Element<T>* next)
	{
	    _next = next;
	}
	
    private:
    
	T* _data;
	Element<T>* _next;
};

#endif /* __ELEMENT_H */

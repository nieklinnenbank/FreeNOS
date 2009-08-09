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

#ifndef __INTEGER_H
#define __INTEGER_H

#include "Types.h"
#include "Comparable.h"

/**
 * Abstracts simple numbers.
 */
template <class Int> class Integer : public Comparable<Integer<Int> >
{
    public:

	/**
	 * Empty constructor.
	 */
	Integer() : value(0)
	{
	}

	/**
	 * Constructor.
	 * @param v An number.
	 */
	Integer(Int v) : value(v)
	{
	}

	/**
	 * Destructor.
	 */
	~Integer()
	{
	}

	/**
	 * Assignment operator.
	 * @param v New value of the Integer.
	 */
	void operator = (Int v)
	{
	    value = v;
	}
	
	/**
	 * Assignment operator.
	 * @param i Integer pointer.
	 */
	void operator = (Integer<Int> *i)
	{
	    value = i->value;
	}

        /**
	 * Compare two Integers's.
         * @param b Constant Integer instance.
         * @return True if equal, false otherwise.
         */
	bool equals (const Integer<Int> &i)
	{
	    return value == i.value;
        }
        
        /**
         * Compares two Integers.
         * @param i Constant Integer instance.
         * @return an int < 0, 0 or > 0 if this Integer is
         * less than, equal to or greater than i.
         */
        int compareTo(const Integer<Int> &i)
        {
            if( value == i.value )
            {
                return 0;
            } else if( value < i.value )
            {
                return -1;
            }
            
            return 1;
        }
	
        /**
	 * Compare two Integers's.
         * @param b Integer instance pointer.
         * @return True if equal, false otherwise.
         */	
	bool equals (Integer<Int> *i)
	{
	    return value == i->value;
	}

	/**
	 * Compare us with a basic integer.
	 * @param v Simple integer type to compare with.
	 * @return True if equal, false otherwise.
	 */
	bool equals(Int v)
	{
	    return value == v;
	}
	
	/**
	 * Comparision operator.
	 * @param Integer instance pointer to compare us with.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (Integer<Int> *i)
	{
	    return equals(i->value);
	}

	/**
         * Gets the size of an Integer.
         * @return Size of an Integer.
         */
        Size size() const
        {
	    return sizeof(Int);
        }
    
        /**
         * Read the Integer byte-wise.
         * @param index Indicates the byte to read.
         * @return Byte value of the Integer
         */
	u8 valueAt(Size index) const
        {
	    return (value >> index) & 0xff;
        }

	/**
	 * Dereference operator.
	 * @return Integer value.
	 */
	Int operator *()
	{
	    return value;
	}
	
	/**
	 * Increment operator.
	 * @param num Number of increments?
	 */
	void operator ++(int num)
	{
	    value++;
	}

	/**
	 * Decrement operator.
	 * @param num Number of decrements?
	 */	
	void operator --(int num)
	{
	    value--;
	}

    private:
    
        /** Integer value. */
	Int value;
};

#endif /* __INTEGER_H */

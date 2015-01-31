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

#ifndef __LIBPARSE_DELIMETER_H
#define __LIBPARSE_DELIMETER_H

/**
 * Represents a delimeter
 */
class Delimeter
{
    public:

	/**
	 * Constructor
	 *
	 * @param open The char that indicates that
	 * a string should be tokenized from here.
	 * @param close The char that indicates that
	 * the current token lasts until here.
	 */
	Delimeter(char open, char close);
	
	/**
	 * Destructor
	 */
	virtual ~Delimeter() {};
	
	/**
	 * Returns the char that is used to indicate
	 * that a string should be tokenized from here.
	 */
	char getOpen() const;
	
	/**
	 * Returns the char that indicates that
	 * the current token lasts until here.
	 */
	char getClose() const;

    private:
	
	char open;
	char close;
};

#endif /* __LIBPARSE_DELIMETER_H */

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

#ifndef __LIBPARSE_COMMANDLINEOPTION_H
#define __LIBPARSE_COMMANDLINEOPTION_H

#include <String.h>

/**
 * Represents a commandline option
 */
template <class K, class V> class CommandLineOption
{
    public:

	/**
	 * Constructor
	 */
	CommandLineOption(K key, V value = NULL) : _key(key), _value(value)
	{
	}
	
	/**
	 * Destructor
	 */
	virtual ~CommandLineOption();
	
	K getKey()
	{
		return _key;
	}
	
	V getValue()
	{
		return _value;
	}
	

    private:

	K _key;
	V _value;
};

#endif
#endif /* __LIBPARSE_COMMANDLINEOPTION_H */

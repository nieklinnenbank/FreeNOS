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

#ifndef __LIBPARSE_URI_H
#define __LIBPARSE_URI_H

#include <Array.h>
#include <String.h>

/**
 * Represents an URI according to RFC 1630.
 * This rfc is located at http://tools.ietf.org/html/rfc1630
 */
class URI
{
    public:

	/**
	 * Constructor
	 */
	URI(String uri);
	
	/**
	 * Destructor
	 */
	virtual ~URI() {};
	
	/**
	 * Returns the scheme of this URI
	 */
	String* getScheme() const;

    protected:

	/** Reserved characters array */
	static Array<String> _reserved;
	
	/** The raw URI. */
	String _uri;
	
	/** The scheme of the URI. */
	String _scheme;
};

#endif /* __LIBPARSE_URI */

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

#ifndef __LIBPARSE_URL_H
#define __LIBPARSE_URL_H

#include "URI.h"

#define KNOWN_SCHEMES "ftp http gopher mailto news nntp telnet wais file prospero"

/**
 * Represents an URL according to RFC 1738
 *
 * Foo.
 *
 * @see http://tools.ietf.org/html/rfc1738
 */
class URL : public URI
{
    public:

        /**
         * Constructor
         */
        URL(char* uri);
        
        /**
         * Destructor
         */
        virtual ~URL();
        
        /**
         * Splits this URL 
         */
        char** split();
        
};

#endif /* __LIBPARSE_URL */

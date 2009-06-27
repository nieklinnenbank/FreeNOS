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

#include <String.h>
#include <string.h>

#define RESERVED_GENDELIMS ":/?#[]@"
#define RESERVED_SUBDELIMS "!$&'()*+,;="
#define UNRESERVED_CHARS \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-._~"
#define URI_SCHEME_SEPARATOR ':'
#define URI_QUERY_SEPARATOR '?'
#define URI_FRAGMENT_SEPARATOR '#'

/**
 * Represents an URI according to RFC 3986.
 *
 * Note that the URI class itself is only capable of identifying
 * the various components defined in the given URI. The contents
 * of these components have to be validated by the subclass that
 * knows the scheme.
 * For instance, given the following URI:
 * file:///etc/passwd
 * the class FileURI knows all about the correct format of that URI.
 *
 * On a second note, it's important to remember that an URI does NOT
 * have to refer to an accessable resource, as an URI only provides
 * identification of that resource.
 *
 * @see http://tools.ietf.org/html/rfc3986
 */
class URI
{
    public:

        /**
         * Constructor
         */
        URI(char* uri);
        
        /**
         * Destructor
         */
        virtual ~URI();
        
        /**
         * Returns the scheme of this URI
         */
        char* getScheme() const;
        
        /**
         *
         */
        char* getHierarchical() const;
        
        /**
         *
         */
        char* getQuery() const;
        
        /**
         *
         */
        char* getFragment() const;
        
        /**
         * Returns the raw URI string as given in the
         * constructor.
         */
        char* getRawURI() const;
        
        /**
         * Returns whether the given URI is equal
         * to this URI.
         * @param uri The URI to compare to this URI.
         */
        bool equals(URI& uri);
        
        /**
         * Normalizes this URI. That means that percent-encoded
         * unreserved characters are decoded and replaced in the
         * returned String. This is needed for - amongs other things -
         * determining whether two URI's are equal.
         */
         char* normalize();

    protected:

        /** The raw URI. */
        char* _uri;
        
        /** The normalized URI. */
        char* _normalized;
        
        /** The hierarchical part of the URI. */
        char* _hierarchical;
        
        /** The query part of the URI. */
        char* _query;
        
        /** The fragment part of the URI. */
        char* _fragment;

        /**
         * Decodes the given encoded char* (hex)
         * and returns the decoded char.
         * @param encoded The char* to decode.
         * @return char The decoded char.
         */
        char _decode(char* encoded);

    private:

        /** The scheme of the URI. */
        char* _scheme;
        
};

#endif /* __LIBPARSE_URI */

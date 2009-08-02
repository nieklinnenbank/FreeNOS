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
         * Returns the scheme of this URI as a lowercase string.
         * The scheme part of an URI is mandatory and the URI class
         * guarantees that this value never is NULL.
         * @return char* The scheme if this URI 
         */
        char* getScheme() const;
        
        /**
         * Returns the hierarchical part of this URI.
         * The hierarchical part is the chunk of URI between
         * the scheme and the optional query and/or fragment.
         * The hierarchical part of an URI is mandatory and the URI class
         * guarantees that this value never is NULL.
         * @return char* The hierarchical part of this URI.
         */
        char* getHierarchical() const;
        
        /**
         * Returns the query part of this URI.
         * The query part of an URI starts with a question mark (?).
         * The query is optional so the returned value may or may not be NULL.
         * @return char* The query part of this URI.
         */
        char* getQuery() const;
        
        /**
         * Returns the fragment part of this URI.
         * The fragment part of an URI starts with a number sign (#).
         * The fragment is optional so the returned value may or may not be NULL.
         * @return char* The fragment part of this URI.
         */
        char* getFragment() const;
        
        /**
         * Returns the raw URI string as given in the constructor.
         * @return char* The raw URI string as given in the constructor.
         */
        char* getRawURI() const;
        
        /**
         * Returns whether the given URI is equal
         * to this URI.
         * Before comparison is carried out, both this URI and the
         * given URI are normalized. The normalized values are compared
         * and the result of that comparison is returned.
         * @param uri The URI to compare to this URI.
         * @return Whether the given URI equals this URI.
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
        char* uri;
        
        /** The normalized URI. */
        char* normalized;
        
        /** The hierarchical part of the URI. */
        char* hierarchical;
        
        /** The query part of the URI. */
        char* query;
        
        /** The fragment part of the URI. */
        char* fragment;

        /** The scheme of the URI. */
        char* scheme;
        
        /**
         * Decodes the given encoded char* (hex)
         * and returns the decoded char.
         * @param encoded The char* to decode.
         * @return char The decoded char.
         */
        char decode(char* encoded);
        
};

#endif /* __LIBPARSE_URI */

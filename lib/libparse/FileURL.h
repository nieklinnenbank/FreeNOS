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

#ifndef __LIBPARSE_FILEURL_H
#define __LIBPARSE_FILEURL_H

#include <Vector.h>
#include <String.h>
#include "URL.h"

#define FILEURL_DEFAULT_SEPARATOR '/'

/**
 * Represents a file url
 */
class FileURL : public URL
{
        
    public:
    
        /**
         * Constructor
         */
        FileURL(char* url);
        
        /**
         * Destructor
         */
        virtual ~FileURL();
        
        /**
         * Splits this FileURL in pieces separated by sep.
         * So if, for example, the FileURL to split is the following:
         * file:///var/log/apache2/access.log
         * the URL will by default be splitted in var, log, apache2, access.log
         *
         * @return Array<String> The splitted url.
         */
        Vector<String>* split();
        
        /**
         * Returns the original full path as a String instance.
         * @return String instance containing the original full path.
         */
        String* full();
        
        /**
         * Returns the URL of our parent's path.
         * If we are the root path, (FileURL*)0 is returned.
         * @return the parent path.
         */
        FileURL* parent();
        
        /**
         * Returns the name of the last element in this FileURL.
         * @return Name of the base.
         */
        String* base();
        
        /**
         * Returns the length of our full path.
         * @return the length.
         */
        Size length();
    
    private:
        
        /* The parts in this FileURL. */
        Vector<String>* splitted;
        
        /* The original full path as a String instance. */
        String* fullPath;
        
        /* Our parent path */
        FileURL* par;
        
        /* The length of the original path */
        Size len;
};

#endif /* __LIBPARSE_FILEURL */

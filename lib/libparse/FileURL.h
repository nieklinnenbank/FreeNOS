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
         *
         * @param sep The separater character. This defaults 
         * to FILEURL_DEFAULT_SEPARATOR
         * @return Array<String> The splitted url.
         */
        Vector<String>* split(char sep = FILEURL_DEFAULT_SEPARATOR);
    
    private:
        
        Vector<String>* _splitted;
};

#endif /* __LIBPARSE_FILEURL */

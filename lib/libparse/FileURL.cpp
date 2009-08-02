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

#include <stdlib.h> // EXIT_SUCCESS & EXIT_FAILURE
#include <stdio.h>
#include <string.h>
#include <StringBuffer.h>
#include "StringTokenizer.h"
#include "FileURL.h"

FileURL::FileURL(char* url) : URL(url)
{
    if( strcasecmp( "file", scheme ) != 0 )
    {
        printf("Not a FileURL: %s.\n", uri);
        exit(EXIT_FAILURE);
    }
    
    splitted = (Vector<String>*)0;
    fullPath = (String*)0;
    par = (FileURL*)0;
    len = strlen(hierarchical);
}

FileURL::~FileURL()
{
    if( this->splitted )
    {
        for( Size size = 0; size < this->splitted->count(); size++ )
        {
            delete this->splitted->get(size);
        }
        
        delete this->splitted;
    }
    
    if( this->fullPath )
    {
        delete this->fullPath;
    }
    
    if( this->par )
    {
        delete this->par;
    }
}

Vector<String>* FileURL::split()
{

    /* Are we already splitted? */
    if( !splitted )
    {
        /* Example: file:///var/log/apache2/access.log */
    
        String hier(hierarchical);
    
        if( hier.startsWith("//") )
        {
            hier = hier.substring(2);
        }
        
        StringTokenizer st(hier, FILEURL_DEFAULT_SEPARATOR);

        Vector<String>* ret = new Vector<String>();

        while( st.hasNext() )
        {
            String* s  = new String(st.next());
            String* trim = s->trim();
            
            if( trim != (String*)0 && trim->size() > 0)
            {
                ret->insert( trim );
            }
            delete s;
        }

        splitted = ret;
    }
    
    return splitted;
}

String* FileURL::full()
{
    if( !fullPath )
    {
        fullPath = new String(hierarchical);
    }
    
    return fullPath;
}

FileURL* FileURL::parent()
{
    if( ! par )
    {
        /* Check if we are the root path */
        if( split()->count() == 1 )
        {
            par = (FileURL*)0;
        } else {
            StringBuffer* sb = new StringBuffer();
            
            sb->append("file://");
            
            for(Size pos = 0; pos < splitted->count() - 1; pos++ )
            {
                sb->append(FILEURL_DEFAULT_SEPARATOR);
                String* part = splitted->get(pos);
                sb->append( part->operator*() );
            }
            
            par = new FileURL( sb->toString()->operator*() );
            delete sb;
        }
    }
    
    return par;
}

String* FileURL::base()
{
    Size count = split()->count();
    return splitted->get(count - 1);
}

Size FileURL::length()
{
    return len;
}

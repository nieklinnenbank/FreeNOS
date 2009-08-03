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

#include <API/ProcessCtl.h>
#include <FreeNOS/CPU.h>
#include <Macros.h>
#include <Types.h>
#include <Config.h>
#include <string.h>
#include "Time.h"

Time::Time()
{
}

Error Time::initialize()
{
    
    /* Done! */
    return ESUCCESS;
}

Error Time::read(s8 *buffer, Size size, Size offset)
{
    /* PHONY read */
    if( offset >= 10 )
    {
        return 0;
    }
    
    memcpy((void*)buffer, (const void*)"1234567890", size);
    
    return (Error) size;
}

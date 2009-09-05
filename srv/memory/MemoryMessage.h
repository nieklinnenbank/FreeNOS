/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __MEMORY_MEMORY_MESSAGE_H
#define __MEMORY_MEMORY_MESSAGE_H

/** 
 * @defgroup memory MemoryServer (Trusted Memory Server) 
 * @{   
 */

#include <API/IPCMessage.h>
#include <API/VMCtl.h>
#include <Types.h>
#include <Macros.h>
#include <Error.h>

/**
 * Actions which can be specified in an MemoryMessage.
 */
typedef enum MemoryAction
{
    /* Shared mappings. */
    CreateShared   = 4,
    DeleteShared   = 5,
    ReleaseShared  = 6,
    ListShared     = 7,
    
    /* Private mappings. */
    CreatePrivate  = 8,
    ReservePrivate = 9,
    ReleasePrivate = 10,
    ListPrivate    = 11,
    
    /* Diagnostics. */
    SystemMemory   = 12,
    ProcessMemory  = 13,
}
MemoryAction;

/**
 * Memory operation message.
 */
typedef struct MemoryMessage : public Message, public MemoryRange
{
    union
    {
	/** Action to perform. */
        MemoryAction action;
	
	/** Result code. */
	Error result;
    };
    
    /** Shared memory key. */
    char *key;
    
    /** Length of the key. */
    Size keyLength;
    
    /** Indicates if a shared mapping is newly created, or reused. */
    bool created;
}
MemoryMessage;

/**
 * @}
 */

#endif /* __MEMORY_MEMORY_MESSAGE_H */

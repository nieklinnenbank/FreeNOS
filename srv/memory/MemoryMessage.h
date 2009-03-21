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

#ifndef __MEMORY_MEMORYMESSAGE_H
#define __MEMORY_MEMORYMESSAGE_H

#include <api/IPCMessage.h>
#include <Types.h>
#include <Macros.h>
#include <Error.h>

/**
 * Actions which can be specified in an MemoryMessage.
 */
typedef enum MemoryAction
{
    HeapGrow    = 0,
    HeapShrink  = 1,
    MemoryUsage = 2,
}
MemoryAction;

/**
 * Memory operation message.
 */
typedef struct MemoryMessage : public Message
{
    /**
     * Default constructor.
     */
    MemoryMessage() : action(HeapGrow), bytes(ZERO)
    {
    }

    /**
     * Assignment operator.
     * @param m MemoryMessage pointer to copy from.
     */
    void operator = (MemoryMessage *m)
    {
	from   = m->from;
	type   = m->type;
	action = m->action;
	bytes  = m->bytes;
    }

    /**
     * Get the current system wide memory usage.
     */
    void usage()
    {
	action = MemoryUsage;
	ipc(MEMSRV_PID, SendReceive, sizeof(MemoryMessage));
    }

    union
    {
	/** Action to perform. */
        MemoryAction action;
	
	/** Result code. */
	Error result;
    };

    /** Indicates a number of bytes. */
    Size bytes, bytesFree;

    /** Start and end addresses (e.g. of the heap). */
    Address startAddr, endAddr;
    
    /** Unused. */
    ulong unused;
}
MemoryMessage;

#endif /* __MEMORY_MEMORYMESSAGE_H */

/**
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

#ifndef __MEMORY_MEMORYSERVER_H
#define __MEMORY_MEMORYSERVER_H

#include <api/IPCMessage.h>
#include <api/VMCtl.h>
#include <api/SystemInfo.h>
#include <arch/Process.h>
#include <arch/Memory.h>
#include <Types.h>

/** Starting address of the heap. */
#define HEAP_START	0xe0000000

/** Maximum size of the heap (256 MB). */
#define HEAP_MAX	(1024 * 1024 * 256)

/** Ending address of the heap. */
#define HEAP_END	(HEAP_START + HEAP_MAX - MEMALIGN)

/**
 * Actions which can be specified in an MemoryMessage.
 */
typedef enum MemoryAction
{
    HeapGrow    = 0,
    HeapShrink  = 1,
    MemoryUsage = 2,
    MemoryOK    = 3,
    MemoryError = 4,
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
    MemoryMessage() : action(MemoryError), bytes(ZERO)
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

    /** Action to perform. */
    MemoryAction action;

    /** Indicates a number of bytes. */
    Size bytes, bytesFree;

    /** Start and end addresses (e.g. of the heap). */
    Address startAddr, endAddr;
}
MemoryMessage;

/**
 * Memory management server.
 */
class MemoryServer
{
    public:
    
	/**
	 * Class constructor function.
	 */
	MemoryServer();
	
	/**
	 * Enters an infinite loop, serving incoming memory requests.
	 * @return Never.
	 */
	int run();
	
    private:
    
	/**
	 * Allocate more memory pages for the heap of a process.
	 * @param msg Request message.
	 * @param reply Reply message.
	 */
	void doGrow(MemoryMessage *msg, MemoryMessage *reply);

	/**
	 * Releases memory pages from the heap of a process.
	 * @param msg Request message.
	 * @param reply Reply message.
	 */
	void doShrink(MemoryMessage *msg, MemoryMessage *reply);
    
	/**
	 * Retrieve the physical memory usage.
	 * @param msg Request message.
	 * @param reply Reply message.
	 */
	void doUsage(MemoryMessage *msg, MemoryMessage *reply);

	/** Heap pointers. */
	Address heaps[MAX_PROCS];
};

#endif /* __MEMORY_MEMORYSERVER_H */

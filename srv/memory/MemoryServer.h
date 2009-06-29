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

#ifndef __MEMORY_MEMORYSERVER_H
#define __MEMORY_MEMORYSERVER_H

#include <API/IPCMessage.h>
#include <API/VMCtl.h>
#include <API/SystemInfo.h>
#include <FreeNOS/Process.h>
#include <FreeNOS/Memory.h>
#include <IPCServer.h>
#include <Types.h>
#include <Macros.h>
#include <Error.h>
#include "MemoryMessage.h"

/** Starting address of the heap. */
#define HEAP_START	(0xe0000000)

/** Maximum size of the heap (256 MB). */
#define HEAP_MAX	(1024 * 1024 * 256)

/** Ending address of the heap. */
#define HEAP_END	(HEAP_START + HEAP_MAX - MEMALIGN)

/**
 * Memory management server.
 */
class MemoryServer : public IPCServer<MemoryServer, MemoryMessage>
{
    public:
    
	/**
	 * Class constructor function.
	 */
	MemoryServer();

    private:
    
	/**
	 * Allocate more memory pages for the heap of a process.
	 * @param msg Request message.
	 */
	void doGrow(MemoryMessage *msg);

	/**
	 * Releases memory pages from the heap of a process.
	 * @param msg Request message.
	 */
	void doShrink(MemoryMessage *msg);
    
	/**
	 * Retrieve the physical memory usage.
	 * @param msg Request message.
	 */
	void doUsage(MemoryMessage *msg);

	/**
	 * Copies the heap pointer from another process.
	 * @param msg Request message.
	 */
	void doClone(MemoryMessage *msg);
	
	/**
	 * Resets heap pointer from a killed process.
	 * @param msg Request message.
	 */
	void doReset(MemoryMessage *msg);

	/** Heap pointers. */
	Address heaps[MAX_PROCS];
};

#endif /* __MEMORY_MEMORYSERVER_H */

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

#ifndef __MEMORY_MEMORY_SERVER_H
#define __MEMORY_MEMORY_SERVER_H

/**
 * @defgroup memory MemoryServer (Trusted Memory Server)
 * @{  
 */

#include <API/IPCMessage.h>
#include <API/VMCtl.h>
#include <API/SystemInfo.h>
#include <FreeNOS/Process.h>
#include <FreeNOS/Memory.h>
#include <IPCServer.h>
#include <UserProcess.h>
#include <FileSystemMount.h>
#include <List.h>
#include <ListIterator.h>
#include <String.h>
#include <Types.h>
#include <Macros.h>
#include "MemoryMessage.h"

/**
 * Describes a shared memory region.
 */
typedef struct SharedMemory
{
    /** Unique identifier key. */
    String key;
    
    /** Physical start address. */
    Address address;
    
    /** Size of the shared memory region, in bytes. */
    Size size;
}
SharedMemory;

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
	 * Create a new private mapping.
	 * @param msg Request message.
	 */
	void createPrivate(MemoryMessage *msg);

	/**
	 * Reserve virtual memory space for a mapping.
	 * @param Request message.
	 */
	void reservePrivate(MemoryMessage *msg);
    
	/**
	 * Release an private memory mapping.
	 * @param msg Request message.
	 */
	void releasePrivate(MemoryMessage *msg);
    
	/**
	 * Create a shared mapping if not exising, and map it.
	 * @param msg Request message.
	 */
	void createShared(MemoryMessage *msg);
	
	/**
	 * Removes a shared mapping, only if unused.
	 * @param msg Request message.
	 */
	void deleteShared(MemoryMessage *msg);
	
	/**
	 * Unmap a shared mapping.
	 * @param msg Request message.
	 */
	void releaseShared(MemoryMessage *msg);
    
	/**
	 * Retrieve the system memory usage.
	 * @param msg Request message.
	 */
	void systemMemory(MemoryMessage *msg);

	/**
	 * Find a free virtual memory range.
	 * @param procID Process identity number.
	 * @param size Number of bytes needed.
	 * @return Virtual start address if found, ZERO otherwise.
	 */
	Address findFreeRange(ProcessID procID, Size size);
	
	/**
	 * Attempt to map a virtual address range.
	 * @param procID Process identity.
	 * @param range Describes the memory range to map.
	 * @return ESUCCESS on success, or an error code on failure.
	 */
	Error insertMapping(ProcessID procID, MemoryRange *range);
	
	
	/**
	 * Creates and maps a shared mapping.
	 * @param procID Process identity to map the shared region for.
	 * @param key Unique key for the shared region.
	 * @param size Size in bytes.
	 * @param range Outputs the memory location of the mapping.
	 * @param created Output a boolean indicating if we created a new shared mapping,
	 *                or reused an existing one.
	 * @return Pointer to a SharedMemory object.
	 */
	SharedMemory * insertShared(ProcessID procID, char *key,
				    Size size, MemoryRange *range, bool *created = ZERO);

	/**
	 * Lookup a shared memory region with the specified key.
	 * @param key Unique key to search for.
	 * @return SharedMemory pointer if found, ZERO otherwise.
	 */
	SharedMemory * findShared(char *key);
	
	/** Keeps track of all current shared memory regions. */
	List<SharedMemory *> shared;
	
	/** Pointer to the user process table. */
	UserProcess *procs;
	
	/** Pointer to the filesystem mounts table. */
	FileSystemMount *mounts;
};

/**
 * @}
 */

#endif /* __MEMORY_MEMORY_SERVER_H */

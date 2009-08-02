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

#ifndef __SHARED_H
#define __SHARED_H

#include <MemoryMessage.h>
#include "Assert.h"
#include "Comparable.h"
#include "Types.h"
#include "String.h"
#include "Config.h"

/**
 * Enables sharing objects between processes.
 * @note While possible, sharing memory pointers is likely to
 *       effect refering invalid memory regions. Try to use simple
 *       datastructures with this class.
 */
template <class T> class Shared
{
    public:

	/**
	 * Empty constructor function.
	 */
	Shared<T>()
	    : object(ZERO), key(ZERO), count(ZERO)
	{
	}

	/**
	 * Constructor function.
	 * @param key Unique key. Reuse it in another process
	 *            to access the share.
	 * @param count The number of T's shared.
	 */
	Shared<T>(const char *key, Size count = 1)
	{
	    load(key, count);
	}

	/**
	 * Load a shared memory object by it's key.
	 * @param key Unique key.
	 * @param count Number of objects shared.
	 * @return True if successfull, false otherwise.
	 */
	bool load(const char *key, Size count)
	{
	    MemoryMessage mem;
	
	    /* Initialize variables. */
	    this->key   = new String(key);
	    this->count = count;
	    
	    /* Ask the memory server. */
	    mem.action = CreateShared;
	    mem.bytes  = sizeof(T) * count;
	    mem.key    = (char *) key;
	    mem.keyLength  = strlen(key);
	    mem.protection = PAGE_RW;
	    mem.virtualAddress  = ZERO;
	    mem.physicalAddress = ZERO;
	    mem.ipc(MEMSRV_PID, SendReceive, sizeof(mem));
	    
	    /* Set object pointer. */
	    object = (T *) mem.virtualAddress;
	    
	    /* Clear new mappings. */
	    if (mem.created)
	    {
		memset(object, 0, size());
	    }
	    /* Done. */
	    return mem.result == ESUCCESS;
	}

	/**
	 * Retrieve the object at the given index.
	 * @param index Index number. Must be >= 0 and < count.
	 * @see count
	 * @return Pointer to the object, if found.
	 */
	T * get(Size index = 0)
	{
	    return index < count ? &object[index] : ZERO;
	}
	
	/**
	 * Retrieve the total size of the shared object(s).
	 * @return Size in bytes.
	 */
	Size size() const
	{
	    return sizeof(T) * this->count;
	}
	
	/**
	 * Dereferencing operator.
	 * @return A pointer to the shared object.
	 */
	T * operator *()
	{
	    return get();
	}
	
	/**
	 * Indexing operator.
	 * @param index Index number.
	 * @return A pointer to the shared object at the given index.
	 */
	T * operator [](Size index)
	{
	    return get(index);
	}

    private:

	/** Pointer to the shared mapping. */
	T *object;

	/** Unique key of the shared object. */
	String *key;
    
	/** The number of shared objects of type T. */
	Size count;
};

#endif /* __SHARED_H */

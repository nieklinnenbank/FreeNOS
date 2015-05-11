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

#ifndef __API_VMCTL_H
#define __API_VMCTL_H

#include <FreeNOS/API.h>
#include <System/Constant.h>
#include <System/Function.h>
#include <Types.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/**
 * Memory operations which may be used as an argument to VMCtl().
 */
typedef enum MemoryOperation
{
    Map            = 0,
    LookupVirtual  = 1,
    LookupPhysical = 2,
    Access         = 3,
    MapTables      = 4,
    UnMapTables    = 5,
}
MemoryOperation;

/**
 * Describes a range of memory pages.
 */
// TODO: move to libarch.
typedef struct MemoryRange
{
    /**
     * Constructor initializes the MemoryRange.
     */
    MemoryRange()
    {
        virtualAddress  = ZERO;
        physicalAddress = ZERO;
        bytes      = PAGESIZE;
        access     = Memory::Present | Memory::User | Memory::Readable | Memory::Writable;
    }
    /** Virtual address start. */
    Address virtualAddress;
    
    /** Physical address start. */
    Address physicalAddress;
    
    /** Number of pages requested, in bytes. */
    Size bytes;
    
    /**
     * Used to signify a number of free bytes.
     * @note Not used by the kernel, but the MemoryServer.
     * @see MemoryServer
     */
    Size free;
    
    /** Page access flags. */
    Memory::MemoryAccess access;
}
MemoryRange;

/**
 * Prototype for user applications. Examines and modifies virtual memory pages.
 * @param procID Remote process.
 * @param op Determines which operation to perform.
 * @param range Describes the memory pages to operate on.
 * @return Zero on success or error code on failure.
 */
inline Error VMCtl(ProcessID procID, MemoryOperation op,
                        MemoryRange *range = ZERO)
{
    return trapKernel3(API::VMCtlNumber, procID, op, (Address) range);
}

extern Error VMCtlHandler(ProcessID procID, MemoryOperation op, MemoryRange *range);

/**
 * @}
 */

#endif /* __API_VMCTL_H */

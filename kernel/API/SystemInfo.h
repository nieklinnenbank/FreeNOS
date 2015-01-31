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

#ifndef __API_SYSTEMINFO_H
#define __API_SYSTEMINFO_H

#include <FreeNOS/API.h>
#include <FreeNOS/Scheduler.h>
#include <FreeNOS/Multiboot.h>
#include <Arch/Memory.h>
#include <FreeNOS/Config.h>
#include <FreeNOS/Kernel.h>
#include <Error.h>
#include <Types.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/** SystemCall number for SystemInfo(). */
#define SYSTEMINFO 6

/**
 * Forward declaration.
 * @see SystemInformation
 */
class SystemInformation;

/**
 * Prototype for user applications. Retrieves system information.
 * @param buf Target buffer.
 */
inline Error SystemInfo(SystemInformation *info)
{
    return trapKernel1(SYSTEMINFO, (Address) info);
}

/**
 * System information structure.
 */
typedef struct SystemInformation
{
    /**
     * Constructor function.
     */
    SystemInformation()
    {
	SystemInfo(this);
    }

    /** System version. */
    ulong version;
    
    /** Boot commandline. */
    char cmdline[64];
    
    /** Total and available memory in bytes. */
    Size memorySize, memoryAvail;

    /**
     * Multiboot modules.
     */
    struct
    {
	/** Physical addresses. */
	Address modStart, modEnd;
	
	/** Configured boot string. */
	char string[32];
    }
    modules[32];
    
    /** Number of modules. */
    Size moduleCount;
}
SystemInformation;

/**
 * @}
 */

#endif /* __API_SYSTEMINFO_H */

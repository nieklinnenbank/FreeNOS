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

#include <FreeNOS/System.h>
#include <FreeNOS/Config.h>
#include <FreeNOS/Kernel.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/**
 * Forward declaration.
 * @see SystemInformation
 */
struct SystemInformation;
class BitAllocator;

/**
 * Prototype for user applications. Retrieves system information.
 * @param buf Target buffer.
 */
inline Error SystemInfo(SystemInformation *info)
{
    return trapKernel1(API::SystemInfoNumber, (Address) info);
}

/**
 * Kernel prototype.
 */
extern Error SystemInfoHandler(SystemInformation *info);

/**
 * System information structure.
 */
// TODO: replace this with CoreInfo.
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

    /** Core Identifier */
    uint coreId;

    /** BootImage physical address */
    Address bootImageAddress;

    /** BootImage size */
    Size bootImageSize;

    Address coreChannelAddress;
    Size coreChannelSize;

    /** Timer counter */
    uint timerCounter;
}
SystemInformation;

/**
 * @}
 */

#endif /* __API_SYSTEMINFO_H */

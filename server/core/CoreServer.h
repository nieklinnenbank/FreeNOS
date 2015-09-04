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

#ifndef __CORE_CORESERVER_H
#define __CORE_CORESERVER_H

/**
 * @defgroup core CoreServer
 * @{  
 */

#include <FreeNOS/System.h>
#include <FreeNOS/API.h>
#include <IPCServer.h>
#include <List.h>
#include <ListIterator.h>
#include <String.h>
#include <Types.h>
#include <Macros.h>
#include <ExecutableFormat.h>
#include <CoreInfo.h>
#include "CoreMessage.h"

#ifdef INTEL
#include <intel/IntelMP.h>
#endif /* INTEL */

/**
 * Represents a single Core in a Central Processing Unit (CPU).
 *
 * Each core in a system will run its own instance of CoreServer.
 * CoreServers will communicate and collaborate together to implement functionality.
 */
class CoreServer : public IPCServer<CoreServer, CoreMessage>
{
  private:

    /** The default kernel for starting new cores. */
    static const char *kernelPath;

  public:

    /**
     * Result codes.
     */    
    enum Result
    {
        Success,
        NotFound,
        BootError,
        ExecError,
        OutOfMemory
    };

    /**
     * Class constructor function.
     */
    CoreServer();

    Result initialize();

    Result bootCore(uint coreId, CoreInfo *info, MemoryRegion *regions);

    Result discover();

    Result loadKernel();

  private:

#ifdef INTEL
    IntelMP m_cores;
#endif /* INTEL */

    ExecutableFormat *m_kernel;

    MemoryRegion m_regions[16]; 

    int m_numRegions;    
};

/**
 * @}
 */

#endif /* __CORE_CORESERVER_H */

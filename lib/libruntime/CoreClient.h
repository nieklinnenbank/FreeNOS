/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __LIB_LIBRUNTIME_CORECLIENT_H
#define __LIB_LIBRUNTIME_CORECLIENT_H

#include <FreeNOS/API/ProcessID.h>
#include <Types.h>
#include "Core.h"

struct CoreMessage;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libruntime
 * @{
 */

/**
 * CoreClient provides a simple interface to a CoreServer.
 *
 * @see CoreServer
 */
class CoreClient
{
  public:

    /**
     * Class constructor function.
     *
     * @param pid Optional ProcessID of the CoreServer.
     */
    CoreClient(const ProcessID pid = CORESRV_PID);

    /**
     * Get number of processor cores in the system.
     *
     * @param numCores On output, contains the number of processor cores.
     *
     * @return Result code
     */
    Core::Result getCoreCount(Size &numCores) const;

    /**
     * Create a new process on a different core.
     *
     * @param coreId Specifies the core on which the process will be created.
     * @param programAddr Virtual address of the loaded program to start.
     * @param programSize Size of the loaded program in bytes.
     * @param programCmd Command-line string for starting the program.
     *
     * @return Result code
     */
    Core::Result createProcess(const Size coreId,
                               const Address programAddr,
                               const Size programSize,
                               const char *programCmd) const;

  private:

    /**
     * Send an IPC request to the CoreServer
     *
     * @param msg Reference to the CoreMessage to send
     *
     * @return Result code
     */
    Core::Result request(CoreMessage &msg) const;

  private:

    /** ProcessID of the CoreServer */
    const ProcessID m_pid;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBRUNTIME_CORECLIENT_H */

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

#ifndef __LIB_LIBRUNTIME_RECOVERYCLIENT_H
#define __LIB_LIBRUNTIME_RECOVERYCLIENT_H

#include <FreeNOS/API/ProcessID.h>
#include <Types.h>
#include "Recovery.h"

struct RecoveryMessage;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libruntime
 * @{
 */

/**
 * RecoveryClient provides a simple interface to the local core's RecoveryServer
 *
 * @see RecoveryServer
 */
class RecoveryClient
{
  public:

    /**
     * Class constructor function.
     *
     * @param pid Optional ProcessID of the RecoveryServer.
     */
    RecoveryClient(const ProcessID pid = RECOVERY_PID);

    /**
     * Restart a process.
     *
     * @param pid Process identifier of the program to restart.
     *
     * @return Result code
     */
    Recovery::Result restartProcess(const ProcessID pid) const;

  private:

    /**
     * Send an IPC request to the RecoveryServer
     *
     * @param msg Reference to the RecoveryMessage to send
     *
     * @return Result code
     */
    Recovery::Result request(RecoveryMessage &msg) const;

  private:

    /** ProcessID of the RecoveryServer */
    const ProcessID m_pid;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBRUNTIME_RECOVERYCLIENT_H */

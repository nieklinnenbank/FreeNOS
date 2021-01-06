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

#ifndef __SERVER_RECOVERY_RECOVERYSERVER_H
#define __SERVER_RECOVERY_RECOVERYSERVER_H

#include <ChannelServer.h>
#include <Types.h>
#include "Recovery.h"
#include "RecoveryMessage.h"

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup recovery
 * @{
 */

/**
 * Recovery Server
 *
 * Provides fault tolerance to servers by restarting on errors (recovery)
 *
 * @todo Support automatic restart of a process when a CPU exception occurs
 *
 * @todo Pro-actively send ping/pong requests to processes to verify they
 *       are still running properly and are not inside an infinite loop.
 *
 * @todo Modify FileSystemServer to use new peek() and seek()
 *       functions of MemoryChannel to support mid-transaction recovery.
 *
 * @todo Update various servers to actually use the DatastoreServer for storing persistent data
 *       that needs to be preserved on restart. For example, the block cache in TmpFileSystem.
 *
 * @todo Add a fault injection tester that modifies the memory of running programs to introduce/simulate
 *       software errors and verify that the system keeps running while the programs are restarted.
 */
class RecoveryServer : public ChannelServer<RecoveryServer, RecoveryMessage>
{
  private:

  public:

    /**
     * Class constructor function.
     */
    RecoveryServer();

  private:

    /**
     * Restart a process.
     *
     * @param msg RecoveryMessage pointer
     */
    void restartProcess(RecoveryMessage *msg);

    /**
     * Overwrite the given process by fetching a fresh program data copy.
     *
     * @param pid Process identifier
     * @param path Path to the program data to use
     *
     * @return True if success, false otherwise
     */
    bool reloadProgram(const ProcessID pid,
                       const char *path) const;

    /**
     * Release and unmap program data
     *
     * @param pid Process identifier
     *
     * @return True if success, false otherwise
     */
    bool cleanupProgram(const ProcessID pid) const;

    /**
     * Overwrite process with given program data.
     *
     * @param pid Process identifier
     * @param program Address containing the program data
     * @param size Number of bytes in the program data
     *
     * @return True if success, false otherwise
     */
    bool rewriteProgram(const ProcessID pid,
                        const Address program,
                        const Size size) const;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_RECOVERY_RECOVERYSERVER_H */

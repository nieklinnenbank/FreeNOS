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

#include <FreeNOS/API.h>
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
#include "CoreMessage.h"

/**
 * Represents a single Core in a Central Processing Unit (CPU).
 *
 * Each core in a system will run its own instance of CoreServer.
 * CoreServers will communicate and collaborate together to implement functionality.
 */
class CoreServer : public IPCServer<CoreServer, CoreMessage>
{
  public:
    
    /**
     * Class constructor function.
     */
    CoreServer();

  private:
    
    /**
     * Create a new private mapping.
     * @param msg Request message.
     */
    void createPrivate(CoreMessage *msg);

    /**
     * Release an private memory mapping.
     * @param msg Request message.
     */
    void releasePrivate(CoreMessage *msg);
    
    /**
     * Read the user process table.
     * @param msg Incoming message.
     */
    void readProcessHandler(CoreMessage *msg);

    /**
     * Terminate a process.
     * @param msg Incoming message.
     */
    void exitProcessHandler(CoreMessage *msg);
    
    /**
     * Create a process.
     * @param msg Incoming message.
     */
    void spawnProcessHandler(CoreMessage *msg);

    /**
     * Waits until a process has died.
     * @param msg Incoming message.
     */
    void waitProcessHandler(CoreMessage *msg);

    /**
     * Get Filesystem mounts.
     */
    void getMountsHandler(CoreMessage *msg);

    /**
     * (Un)set FileSystem mount.
     */
    void setMountHandler(CoreMessage *msg);

    /** User Process table. */
    UserProcess *procs;
    
    /** Pointer to the filesystem mounts table. */
    FileSystemMount *mounts;
};

/**
 * @}
 */

#endif /* __CORE_CORESERVER_H */

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

#include <API/SystemInfo.h>                                                       
#include <FreeNOS/BootImage.h>
#include <FreeNOS/Process.h>
#include <FileSystemMessage.h>
#include <FileSystem.h>
#include <BootModule.h>
#include <String.h>
#include <Error.h>
#include <Macros.h>
#include <ProcessID.h>
#include "ProcessMessage.h"
#include "ProcessServer.h"
#include <stdio.h>

ProcessServer::ProcessServer()
    : IPCServer<ProcessServer, ProcessMessage>(this)
{
    MemoryMessage mem;

    /* Register message handlers. */
    addIPCHandler(ReadProcess,  &ProcessServer::readProcessHandler);
    addIPCHandler(ExitProcess,  &ProcessServer::exitProcessHandler,  false);
    addIPCHandler(SpawnProcess, &ProcessServer::spawnProcessHandler);
    addIPCHandler(CloneProcess, &ProcessServer::cloneProcessHandler, false);
    addIPCHandler(WaitProcess,  &ProcessServer::waitProcessHandler,  false);
    addIPCHandler(SetCurrentDirectory, &ProcessServer::setCurrentDirectory);

    /* Load shared objects. */
    procs.load(USER_PROCESS_KEY, MAX_PROCS);
    files = new Array<Shared<FileDescriptor> >(MAX_PROCS);
}

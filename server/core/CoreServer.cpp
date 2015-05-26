/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <FreeNOS/API.h>
#include <FreeNOS/System.h>
#include <BootImage.h>
#include <Runtime.h>
#include "CoreServer.h"
#include "CoreMessage.h"
#include <stdio.h>
#include <string.h>

CoreServer::CoreServer()
    : IPCServer<CoreServer, CoreMessage>(this)
{
    SystemInformation info;
    VirtualMemory::Range range;
    BootImage *image;
    BootSymbol *symbol;

    /* Register message handlers. */
    addIPCHandler(CreatePrivate,  &CoreServer::createPrivate);
    addIPCHandler(ReleasePrivate, &CoreServer::releasePrivate);
    addIPCHandler(ReadProcess,    &CoreServer::readProcessHandler);
    addIPCHandler(GetMounts,      &CoreServer::getMountsHandler);
    addIPCHandler(SetMount,       &CoreServer::setMountHandler);
    addIPCHandler(ExitProcess,    &CoreServer::exitProcessHandler,  false);
    addIPCHandler(SpawnProcess,   &CoreServer::spawnProcessHandler);
    addIPCHandler(WaitProcess,    &CoreServer::waitProcessHandler,  false);

    /* Allocate a user process table. */
    procs = new UserProcess[MAX_PROCS];
    memset(procs, 0, sizeof(UserProcess) * MAX_PROCS);

    /* Allocate FileSystemMounts table. */
    mounts = new FileSystemMount[FILESYSTEM_MAXMOUNTS];
    memset(mounts, 0, sizeof(FileSystemMount) * FILESYSTEM_MAXMOUNTS);

    /* We only guarantee that / and /dev, /proc are mounted. */
    strlcpy(mounts[0].path, "/dev", PATHLEN);
    strlcpy(mounts[1].path, "/", PATHLEN);
    strlcpy(mounts[2].path, "/proc", PATHLEN);
    mounts[0].procID  = DEVSRV_PID;
    mounts[0].options = ZERO;
    mounts[1].procID  = ROOTSRV_PID;
    mounts[1].options = ZERO;
    mounts[2].procID  = 13;
    mounts[2].options = ZERO;

    // Attempt to load the boot image
    range.virt    = ZERO;
    range.phys    = info.bootImageAddress;
    range.access  = Arch::Memory::Present | Arch::Memory::User | Arch::Memory::Readable;
    range.size    = info.bootImageSize;
    VMCtl(SELF, Map, &range);
    
    image = (BootImage *) range.virt;

    /* Loop all embedded programs. */
    for (Size j = 0; j < image->symbolTableCount; j++)
    {
        /* Read out the next program. */
        symbol = (BootSymbol *)(((Address)image) + image->symbolTableOffset);
        symbol += j;

        if (symbol->type != BootProgram)
            continue;

        /* Write commandline. */
        snprintf(procs[j].command, COMMANDLEN,
                "[%s]", symbol->name);

        /* Set user and group identities. */
        procs[j].userID  = 0;
        procs[j].groupID = 0;
    }
}

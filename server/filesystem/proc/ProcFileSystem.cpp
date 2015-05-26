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

#include <FreeNOS/API.h>
#include <FreeNOS/Process.h>
#include <CoreMessage.h>
#include <CoreServer.h>
#include <FileSystemPath.h>
#include <PseudoFile.h>
#include "ProcRootDirectory.h"
#include "ProcFileSystem.h"

// Local copy of the UserProcess table
UserProcess procs[MAX_PROCS];

char * ProcFileSystem::states[] =
{
    "Running",
    "Ready",
    "Stopped",
    "Sleeping",
};

ProcFileSystem::ProcFileSystem(const char *path)
    : FileSystem(path)
{
    rootDir = new ProcRootDirectory(this);
    setRoot(rootDir);
}

void ProcFileSystem::refresh()
{
    CoreMessage msg;
    ProcessInfo info;
    String slash("/");
    Directory *procDir;

    // TODO: memory leak! Cleanup the whole cache first... (currently broken)
    // clearFileCache();
    rootDir->clear();

    /* Update root. */
    rootDir->insert(DirectoryFile, ".");
    rootDir->insert(DirectoryFile, "..");
    
    /* Reinsert into the cache. */
    insertFileCache(rootDir, ".");
    insertFileCache(rootDir, "..");

    // Refresh UserProcess table
    msg.action = ReadProcess;
    msg.buffer = procs;
    msg.number = ZERO;
    msg.type   = IPCType;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // Insert processes pseudo files
    for (int i = 0; i < MAX_PROCS; i++)
    {
        // Skip unused PIDs
        if (!procs[i].command[0])
            continue;

        // Per-process directory
        procDir = new Directory;
        procDir->insert(DirectoryFile, ".");
        procDir->insert(DirectoryFile, "..");
        procDir->insert(RegularFile, "cmdline");
        procDir->insert(RegularFile, "status");
        rootDir->insert(DirectoryFile, "%u", i);

        // Insert into the cache
        insertFileCache(procDir, "%u",    i);
        insertFileCache(procDir, "%u/.",  i);
        insertFileCache(rootDir, "%u/..", i);

        // Set commandline
        insertFileCache(new PseudoFile("%s", procs[i].command),
                        "%u/cmdline", i);

        // Request kernel's process information
        ProcessCtl(i, InfoPID, (Address) &info);

        // Process status
        insertFileCache(new PseudoFile("%s", states[info.state]),
                        "%u/status",  i);
    }    
}

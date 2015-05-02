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

#include <API/IPCMessage.h>
#include <FreeNOS/Process.h>
#include <ProcessMessage.h>
#include <ProcessServer.h>
#include <FileSystemPath.h>
#include <PseudoFile.h>
#include "ProcRootDirectory.h"
#include "ProcFileSystem.h"

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
    ProcessMessage msg;
    ProcessID pid = 0;
    UserProcess uproc;    
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

    /* Read processes from process server. */
    // TODO: very inefficient. Use ONE message to read the whole table.
    while (true)
    {
	/* Fill message. */
        msg.action = ReadProcess;
        msg.buffer = &uproc;
	msg.number = pid;

	/* Read next process. */
	if (IPCMessage(PROCSRV_PID, API::SendReceive, &msg, sizeof(msg)) || msg.result)
	{
	    break;
	}
	/* Per-process directory. */
	procDir = new Directory;
        procDir->insert(DirectoryFile, ".");
        procDir->insert(DirectoryFile, "..");
        procDir->insert(RegularFile, "cmdline");
        procDir->insert(RegularFile, "status");
        rootDir->insert(DirectoryFile, "%u", msg.number);

	/* Reinsert into the cache. */
	insertFileCache(procDir, "%u",    msg.number);
	insertFileCache(procDir, "%u/.",  msg.number);
	insertFileCache(rootDir, "%u/..", msg.number);

	/* Command line string. */
	insertFileCache(new PseudoFile("%s", uproc.command),
			"%u/cmdline", msg.number);
	
	/* Process status. */
	insertFileCache(new PseudoFile("%s", states[uproc.state]),
		        "%u/status",  msg.number);
	
	/* Move to next PID. */
	pid = msg.number + 1;
    }    
}

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

#include <api/IPCMessage.h>
#include <arch/Process.h>
#include <ProcessMessage.h>
#include <ProcessServer.h>
#include <FileSystemPath.h>
#include <Error.h>
#include "ProcFile.h"
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
    FileSystemPath slash("/");

    rootDir = new ProcRootDirectory(this);
    root    = new FileCache(&slash, rootDir, ZERO);
}

void ProcFileSystem::refresh()
{
    ProcessMessage msg;
    ProcessID pid = 0;
    UserProcess uproc;    
    Directory *procPidDir;
    String slash("/");
    char tmp[PATHLEN];

    /* Clear the current cache. */
    clearFileCache();

    /* Read processes from process server. */
    while (true)
    {
	/* Fill message. */
        msg.action = ReadProcess;
        msg.buffer = &uproc;
	msg.number = pid;
	
	/* Read next process. */
	if (IPCMessage(PROCSRV_PID, SendReceive, &msg) || msg.result)
	{
	    break;
	}
	/* Add entry to root. */
	snprintf(tmp, sizeof(tmp), "%u", msg.number);
	rootDir->addEntry(tmp, DT_DIR);
	
	/* Per-process directory. */
	procPidDir = new Directory;
	procPidDir->addEntry("cmdline", DT_REG);
	procPidDir->addEntry("status",  DT_REG);
	
	/* Insert per-process directory to cache. */
	insertFileCache(procPidDir, "%u", msg.number);

	/* Command line string. */
	insertFileCache(new ProcFile(uproc.command),
			"%u/cmdline", msg.number);
	
	/* Process status. */
	insertFileCache(new ProcFile(states[uproc.state]),
		        "%u/status",  msg.number);
	
	/* Move to next PID. */
	pid = msg.number + 1;
    }    
}

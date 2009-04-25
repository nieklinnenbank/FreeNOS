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
    String slash("/");
    Directory *procDir;
    char tmp[PATHLEN];

    /* Clear the current cache. */
    clearFileCache();
    
    /* Update root. */
    rootDir->insertEntry(".", DT_DIR);
    rootDir->insertEntry("..", DT_DIR);
    insertFileCache(rootDir, ".");
    insertFileCache(rootDir, "..");

    /* Read processes from process server. */
    while (true)
    {
	/* Fill message. */
        msg.action = ReadProcess;
        msg.buffer = &uproc;
	msg.number = pid;

	/* Read next process. */
	if (IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg)) || msg.result)
	{
	    break;
	}
	snprintf(tmp, sizeof(tmp), "%u", msg.number);
	
	/* Per-process directory. */
	procDir = new Directory;
	procDir->insertEntry(".",       DT_DIR);
	procDir->insertEntry("..",      DT_DIR);
	procDir->insertEntry("cmdline", DT_REG);
	procDir->insertEntry("status",  DT_REG);
	rootDir->insertEntry(tmp,      DT_DIR);
	insertFileCache(procDir, "%u",    msg.number);
	insertFileCache(procDir, "%u/.",  msg.number);
	insertFileCache(rootDir, "%u/..", msg.number);

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

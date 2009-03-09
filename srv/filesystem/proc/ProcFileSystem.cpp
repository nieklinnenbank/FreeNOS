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

#include <arch/Process.h>
#include <ProcessMessage.h>
#include <ProcessServer.h>
#include "ProcFile.h"
#include "ProcFileSystem.h"
#include <Error.h>

char * ProcFileSystem::states[] =
{
    "Running",
    "Ready",
    "Stopped",
    "Sleeping",
};

bool done = false;

ProcFileSystem::ProcFileSystem(const char *path)
    : FileSystem(path)
{
    cacheHit(ZERO);
}

FileCache * ProcFileSystem::cacheHit(FileCache *ch)
{
    ProcessMessage msg;
    ProcessID pid = 0;
    UserProcess uproc;    
    Directory *rootDir = new Directory;
    Directory *procPidDir;
    String slash("/");
    char tmp[PATHLEN];

    if (!done) done = true;
    else return ch;

    /* Clear the current cache. */
    if (ch) ch->count++;
    clearFileCache(cache[&slash]);

    /* Add the root directory to the cache. */
    insertFileCache(rootDir, "/");

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
	insertFileCache(procPidDir, "/%u", msg.number);

	/* Command line string. */
	insertFileCache(new ProcFile(uproc.command, strlen(uproc.command)),
			"/%u/cmdline", msg.number);
	
	/* Process status. */
	insertFileCache(new ProcFile(states[uproc.state], strlen(states[uproc.state])),
		        "/%u/status",  msg.number);
	
	/* Move to next PID. */
	pid = msg.number + 1;
    }    
    /* Just return input. */
    if (ch) ch->count--;
    return ch;
}

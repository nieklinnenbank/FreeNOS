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

#include <api/SystemInfo.h>
#include <Version.h>
#include <ProcessServer.h>
#include <stdio.h>
#include <dirent.h>
#include "ProcFile.h"

UserProcess procs[MAX_PROCS];

Error ProcRootFile::read(u8 *buffer, Size sz, Size offset)
{
    ProcessMessage msg;
    Size bytes = 0;
    Dirent *dirent = (Dirent *) buffer;
    
    /* Fill message. */
    msg.action = ReadProcess;
    msg.buffer = procs;
    
    /* Request process server. */
    IPCMessage(PROCSRV_PID, SendReceive, &msg);
    
    /* Fill in dirent entries. */
    for (Size i = offset / sizeof(Dirent); i < MAX_PROCS && bytes + sizeof(Dirent) < sz; i++)
    {
	/* Is this entry filled? */
	if (procs[i].command[0])
	{
	    snprintf(dirent->d_name, DIRLEN, "%u", i);
	    dirent->d_type = DT_DIR;
	    dirent++;
	    bytes += sizeof(Dirent);
	}
    }
    return bytes;
}

Error ProcVersionFile::read(u8 *buffer, Size sz, Size offset)
{
    char *version = "FreeNOS " RELEASE;
    Size len = strlen(version);
    
    /* End of file reached? */
    if (offset >= len)
    {
	return 0;
    }
    memcpy((char *) buffer, version + offset, sz);
    return sz;
}

Error ProcCmdLineFile::read(u8 *buffer, Size sz, Size offset)
{
    SystemInformation info;
    Size len = strlen(info.cmdline);

    /* End of file reached? */
    if (offset >= len)
    {
	return 0;
    }    
    /* Write kernel commandline. */
    memcpy((char *) buffer, info.cmdline + offset, sz);
    return sz;
}

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
#include <Arch/Memory.h>
#include <ProcessServer.h>
#include <ProcessMessage.h>
#include <ProcessID.h>
#include <Types.h>
#include <string.h>
#include <errno.h>
#include "unistd.h"

int forkexec(const char *path, const char *argv[])
{
    ProcessMessage msg;
    char *arguments = new char[PAGESIZE];
    uint count = 0;

    /* Fill in arguments. */
    while (argv[count] && count < PAGESIZE / ARGV_SIZE)
    {
	strlcpy(arguments + (ARGV_SIZE * count), argv[count], ARGV_SIZE);
	count++;
    }    
    /* We want to spawn a new process. */
    msg.action    = SpawnProcess;
    msg.path      = (char *) path;
    msg.arguments = arguments;
    msg.number    = count;
    
    /* Ask process server. */
    IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg));

    /* Set errno. */
    errno = msg.result;
    
    /* Cleanup. */
    delete arguments;
    
    /* All done. */
    return errno == ESUCCESS ? (int) msg.number : -1;
}

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
#include <String.h>
#include <Types.h>
#include <Log.h>
#include <stdio.h>
#include "CoreMessage.h"
#include "CoreServer.h"

void CoreServer::spawnProcessHandler(CoreMessage *msg)
{
    char path[PATHLEN];
    ProcessID pid;

    // Read out the name of the process
    if ((msg->result = VMCopy(msg->from, API::Read, (Address) path,
                             (Address) msg->path, PATHLEN) < 0))
    {
        return;
    }

    // Create new process
    pid = ProcessCtl(ANY, Spawn, msg->number);

    DEBUG("spawn: " << procs[msg->from].command << "[" << msg->from << "] => " << path << "[" << pid << "]");

    // Set process name
    snprintf(procs[pid].command, COMMANDLEN, "%s", path);

    // Fill UserProcess structure
    procs[pid].userID  = procs[msg->from].userID;
    procs[pid].groupID = procs[msg->from].groupID;
    procs[pid].parent  = msg->from;

    // Success
    msg->number = pid;
    msg->result = ESUCCESS;
}

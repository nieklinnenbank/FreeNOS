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
#include "CoreServer.h"
#include "CoreMessage.h"
#include <errno.h>

void CoreServer::waitProcessHandler(CoreMessage *msg)
{
    DEBUG("wait: " << procs[msg->from].command << "[" << msg->from << "] => " << msg->number);

    if (msg->number < MAX_PROCS && msg->number != ANY &&
	procs[msg->number].command[0])
    {
	procs[msg->from].waitProcessID = msg->number;
    }
    else
    {
	msg->result = EINVAL;
        msg->type   = IPCType;
	IPCMessage(msg->from, API::Send, msg, sizeof(*msg));
    }
}

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

#include <API/VMCopy.h>
#include <API/ProcessCtl.h> 
#include <FreeNOS/Process.h> 
#include "ProcessMessage.h"
#include "ProcessServer.h"
#include <errno.h>

void ProcessServer::readProcessHandler(ProcessMessage *msg)
{
    ProcessInfo info;

    /* Find the next process, starting at the given PID. */
    for (Size i = msg->number; i < MAX_PROCS; i++)
    {
	if (procs[i]->command[0])
	{
	    /* Request kernel's process information. */
	    ProcessCtl(i, InfoPID, (Address) &info);
	    
	    /* Update entry. */
	    procs[i]->state = info.state;
	
	    /* Copy buffer. */
	    VMCopy(msg->from, API::Write, (Address) (procs[i]),
				          (Address) (msg->buffer), sizeof(UserProcess));
	    msg->result = ESUCCESS;
	    msg->number = i;
	    return;
	}
    }
    msg->result = ENOENT;
}

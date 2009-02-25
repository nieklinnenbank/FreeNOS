/**
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
#include <arch/API.h>
#include <arch/Init.h>
#include <arch/Memory.h>
#include <arch/Scheduler.h>
#include <Error.h>

int IPCMessageHandler(ProcessID id, Action action, UserMessage *msg)
{
    ArchProcess *proc;
    UserMessage *newMessage;

    /* Verify memory read/write access. */
    if (!memory->access(scheduler->current(), (Address) msg, sizeof(UserMessage)))
    {
	return EFAULT;
    }
    /* Enforce correct fields. */
    msg->from = scheduler->current()->getID();
    msg->type = IPCType;

    /* Handle IPC request appropriately. */
    switch (action)
    {
	case Send:
	case SendReceive:
  
	    /* Find the remote process to send to. */
	    if (!(proc = Process::byID(id)))
	    {
		return ENOSUCH;
	    }
	    proc->getMessageQueue()->enqueue(new UserMessage(msg));
	    proc->wakeup();
	    if (action == Send) break;

	case Receive:

	    /* Wait until we have a message. */
	    while (true)
	    {
		if ((newMessage = scheduler->current()->getMessageQueue()->dequeue()))
		{
		    *msg = *newMessage;
		    delete newMessage;
		    break;
		}
		/* Let some other process run while we wait. */
		scheduler->current()->setState(Sleeping);
		scheduler->executeNext();
	    }
	default:
	    ;
    }
    /* Success. */
    return 0;
}

INITAPI(IPCMESSAGE, IPCMessageHandler)

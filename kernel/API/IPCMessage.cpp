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

#include <Log.h>
#include <ListIterator.h>
#include <FreeNOS/API.h>
#include <FreeNOS/Kernel.h>
#include "IPCMessage.h"

Error IPCMessageHandler(ProcessID id, Operation action, UserMessage *msg, Size size)
{
    Process *proc;
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Memory *memory = Kernel::instance->getMemory();
    
    /* Verify memory read/write access. */
    if (size > MAX_MESSAGE_SIZE || !memory->access(procs->current(),
                                                  (Address) msg, sizeof(UserMessage)))
    {
        return EFAULT;
    }
    /* Enforce correct fields. */
    msg->from = procs->current()->getID();
    msg->type = IPCType;

    DEBUG("#" << msg->from << " " << action << " -> #" << id);

    /* Handle IPC request appropriately. */
    switch (action)
    {
        case Send:
        case SendReceive:
  
            /* Find the remote process to send to. */
            if (!(proc = procs->get(id)))
            {
                return ESRCH;
            }
            /* Put our message on their list, and try to let them execute! */
            proc->getMessages()->insertHead(new UserMessage(msg, size));
            proc->setState(Process::Ready);

            if (action == SendReceive && proc != procs->current())
            {
                procs->current()->setState(Process::Sleeping);
                procs->schedule(proc);
            }
            if (action == Send)
                break;
            
        case Receive:

            /* Block until we have a message. */
            while (true)
            {
                /* Look for a message, with origin 'id'. */
                for (ListIterator<UserMessage> i(procs->current()->getMessages());
                     i.hasNext(); i++)
                {
                    if (i.current()->from == id || id == ANY)
                    {
                        MemoryBlock::copy(msg, i.current()->data, size < i.current()->size ?
                                                       size : i.current()->size);
                        procs->current()->getMessages()->remove(i.current());
                        delete i.current();
                        return 0;
                    }
                }
                /* Let some other process run while we wait. */
                procs->current()->setState(Process::Sleeping);
                procs->schedule();
            }

        default:
            return EINVAL;
    }
    /* Success. */
    return 0;
}

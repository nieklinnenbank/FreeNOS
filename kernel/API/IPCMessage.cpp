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

Error IPCMessageHandler(ProcessID id, API::Operation action, Message *msg, Size size)
{
    Process *proc;
    ProcessManager *procs = Kernel::instance->getProcessManager();

    // Sanity check the incoming message.
    if (!msg)
    {
        FATAL("Message == 0"); for(;;);
    }
    if (msg->type != IPCType)
    {
        FATAL("Message->type != IPCType"); for(;;);
    }
    if (size > MAX_MESSAGE_SIZE)
    {
        FATAL("size > MAX"); for(;;);
    }

    // Enforce correct fields
    msg->from = procs->current()->getID();
    msg->type = IPCType;

    DEBUG("#" << msg->from << " " << action << " -> #" << id);

    /* Handle IPC request appropriately. */
    switch (action)
    {
        case API::Send:
        case API::SendReceive:
        {
            /* Find the remote process to send to. */
            if (!(proc = procs->get(id)))
            {
                return API::NotFound;
            }
            // Copy the message
            UserMessage *copy = new UserMessage;
            copy->from = msg->from;
            copy->type = msg->type;
            MemoryBlock::copy(copy, msg, size);

            // Put our message on their list, and try to let them execute!
            proc->getMessages()->prepend(copy);
            proc->setState(Process::Ready);

            if (action == API::SendReceive && proc != procs->current())
            {
                procs->current()->setState(Process::Sleeping);
                procs->schedule(proc);
            }
            if (action == API::Send)
                break;
        }
            
        case API::Receive:

            proc = procs->current();

            // Block until we have a message
            while (true)
            {
                /* Look for a message, with origin 'id'. */
                for (ListIterator<Message *> i(proc->getMessages()); i.hasCurrent(); i++)
                {
                    Message *m = i.current();

                    if (!m)
                    {
                        FATAL("empty message from queue"); for(;;);
                    }
                    if (m->type > FaultType)
                    {
                        FATAL("invalid type from queue"); for(;;);
                    }

                    // TODO: #warning i.current() gets corrupted here!
                    if (m->from == id || id == ANY)
                    {
                        MemoryBlock::copy(msg, m, size);
                        i.remove();
                        delete m;
                        return API::Success;
                    }
                }
                /* Let some other process run while we wait. */
                procs->current()->setState(Process::Sleeping);
                procs->schedule();
            }

        default:
            FATAL("unknown action: " << (int) action); for(;;);
            return API::InvalidArgument;
    }
    /* Success. */
    return API::Success;
}

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

#include "CoreServer.h"
#include "CoreMessage.h"

void CoreServer::createPrivate(CoreMessage *msg)
{
    MemoryRange range;

    /* Try the given virtual addresses, if any. */
    if (msg->virtualAddress >= (1024*1024*16))
    {
	msg->virtualAddress &= PAGEMASK;
    }
    /* Otherwise, search for a free range. */
    else if (!(msg->virtualAddress = findFreeRange(msg->from, msg->bytes)))
    {
	msg->result = ENOMEM;
	return;
    }
    /* Set mapping flags. */
    // TODO: only allow pinned pages for uid == 0!
    msg->access &= Memory::Pinned | Memory::Reserved | Memory::Readable | Memory::Writable;
    msg->access |= Memory::Present | Memory::User;
    
    /* Try to map the range. */
    msg->result = insertMapping(msg->from, msg);
}

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

#include "MemoryServer.h"
#include "MemoryMessage.h"

void MemoryServer::createPrivate(MemoryMessage *msg)
{
    MemoryRange range;

    //
    // TODO: verify uid == 0, when physStart != ZERO
    //       i.e. only superuser may specify physical memory explicitely.
    //       we can do this if we have shared mappings, and mapped the
    //       user process table into our address space.
    //

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
    msg->protection &= PAGE_PINNED  | PAGE_RESERVED | PAGE_RW;
    msg->protection |= PAGE_PRESENT | PAGE_USER;
    
    /* Try to map the range. */
    msg->result = insertMapping(msg->from, msg);
}

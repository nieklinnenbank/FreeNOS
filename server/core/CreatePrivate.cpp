/*
 * Copyright (C) 2015 Niek Linnenbank
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
    Memory::Range range;
    ProcessInfo info;

    // TODO: keep a local copy of the ProcessInfo in the user process table
    //       That will save a system call here.
    ProcessCtl(msg->from, InfoPID, (Address) &info);

    // Set mapping flags
#warning use regions!
    Arch::Memory mem(info.pageDirectory);
    range.virt   = msg->virt & PAGEMASK;
    range.size   = msg->size;    
    range.phys   = msg->phys;
    range.access = msg->access;

    if (!range.virt)
        range.virt = mem.findFree(range.size, Memory::UserPrivate);

    // Try to map the range
    mem.mapRange(&range);

    msg->virt = range.virt;
    msg->phys = range.phys;
    msg->result = ESUCCESS;
}

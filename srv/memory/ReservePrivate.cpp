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

void MemoryServer::reservePrivate(MemoryMessage *msg)
{
    Address *pageDir;
    
    /* Verify virtual addresses. */
    if (!(msg->virtualAddress >= 1024*1024*16))
    {
	msg->result = EINVAL;
	return;
    }
    /* Point page directory. */
    pageDir = (Address *) PAGETABADDR_FROM(PAGETABFROM,
					   PAGEUSERFROM);
    /* Map page directory. */
    VMCtl(msg->from, MapTables);

    /* Loop directory. Mark them reserved. */
    for (Address i = msg->virtualAddress;
		 i < msg->virtualAddress + msg->bytes;
	         i += (PAGESIZE * PAGETAB_MAX))
    {
	pageDir[DIRENTRY(i)] |= PAGE_RESERVED;
    }
    /* Unmap. */
    VMCtl(msg->from, UnMapTables);
    
    /* Done. */
    msg->result = ESUCCESS;
}

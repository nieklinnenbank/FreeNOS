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

#include <API/SystemInfo.h>
#include <API/IPCMessage.h>
#include <MemoryMessage.h>
#include <IOBuffer.h>
#include <Types.h>
#include "GRUBModule.h"

GRUBModule::GRUBModule(const char *name)
{
    SystemInformation info;
    MemoryMessage mem;

    /* Search for the boot module. */
    for (Size i = 0; i < info.moduleCount; i++)
    {
        if (!strcmp(info.modules[i].string, (char *) name))
        {
            /* Ask memory server to map GRUB module. */
            mem.action     = CreatePrivate;
            mem.protection = PAGE_PINNED;
            mem.bytes      = info.modules[i].modEnd -
                             info.modules[i].modStart;
            mem.virtualAddress  = ZERO;
            mem.physicalAddress = info.modules[i].modStart;
            mem.ipc(MEMSRV_PID, SendReceive, sizeof(mem));

            /* Update our state. */
            size   = info.modules[i].modEnd -
            	     info.modules[i].modStart;
            buffer = (char *) mem.virtualAddress;

            /* Success! */
	    break;
	}
    }
}

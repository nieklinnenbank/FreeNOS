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
#include <String.h>

int SystemInfoHandler(SystemInformation *info)
{
    /* Verify memory access. */
    if (!memory->access(scheduler->current(), (Address) info,
	                sizeof(SystemInformation)))
    {
        return EFAULT;
    }
    /* Fill in our current information. */
    info->version     = 0;
    info->memorySize  = memory->getTotalMemory();
    info->memoryAvail = memory->getAvailableMemory();
    info->moduleCount = multibootInfo.modsCount;
    String::strlcpy(info->cmdline, (char *)multibootInfo.cmdline, 64);
    
    /* Include multiboot modules information. */
    for (Size i = 0; i < info->moduleCount; i++)
    {
	MultibootModule *m = (MultibootModule *)(multibootInfo.modsAddress +
						 sizeof(MultibootModule) * i);
	info->modules[i].modStart   = m->modStart;
	info->modules[i].modEnd     = m->modEnd;
	info->modules[i].string[31] = ZERO;
	String::strlcpy(info->modules[i].string, (char *)m->string, 32);
    }
    return 0;
}

INITAPI(SYSTEMINFO, SystemInfoHandler)

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

#include <api/VMCtl.h>
#include <Error.h>
#include <Config.h>

int VMCtlHandler(ProcessID procID, Address paddr, Address vaddr,
		 ulong prot = PAGE_PRESENT|PAGE_USER|PAGE_RW)
{
    ArchProcess *proc;
    
    /* Find the given process. */
    if (!(proc = Process::byID(procID)) &&
        !(procID == SELF && (proc = scheduler->current())))
    {
	return ENOSUCH;
    }
    /* Map the memory page. */
    if (prot & PAGE_PRESENT)
    {
	memory->mapVirtual(proc, paddr ? paddr : memory->allocatePhysical(PAGESIZE),
			   vaddr, prot & ~PAGEMASK);
    }
    /* Release memory page (if not pinned). */
    else if (!memory->access(proc, vaddr, PAGE_PINNED))
    {
	memory->releasePhysical(memory->virtualToPhysical(proc, vaddr));
    }
    /* Success. */
    return (0);
}

INITAPI(VMCTL, VMCtlHandler)

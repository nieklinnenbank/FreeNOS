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

#include <api/VMCopy.h>
#include <arch/Process.h>
#include <arch/API.h>
#include <arch/Memory.h>
#include <Error.h>

int VMCopyHandler(ProcessID procID, Action how, Address ours, Address theirs, Size sz)
{
    ArchProcess *proc;
    Address paddr, tmpAddr;
    Size bytes = 0, pageOff, total = 0;
    
    /* Find the corresponding ArchProcess. */
    if (!(proc = ArchProcess::byID(procID)))
    {
	return ENOSUCH;
    }
    /* Verify memory addresses. */
    if (!memory->access(scheduler->current(), ours, sz) ||
        !memory->access(proc, theirs, sz))
    {
	return EFAULT;
    }
    /* Keep on going until all memory is processed. */
    while (total < sz)
    {
	/* Update variables. */
	paddr   = memory->virtualToPhysical(proc, theirs);
	pageOff = theirs & ~PAGEMASK;
	bytes   = (PAGESIZE - pageOff) < sz ? (PAGESIZE - pageOff) : sz;
		
	/* Valid address? */
	if (!paddr) break;
		
	/* Map the physical page. */
	tmpAddr = memory->mapVirtual(paddr);

	/* Process the action appropriately. */
        switch (how)
	{
	    case Read:
		memcpy((void *)ours, (void *)(tmpAddr + pageOff), bytes);
		break;
			
	    case Write:
		memcpy((void *)(tmpAddr + pageOff), (void *)ours, bytes);
		break;
	    
	    default:
		;
	}	
	/* Remove mapping. */
	memory->mapVirtual((Address) 0, (Address) tmpAddr, 0);
	ours   += bytes;
	total  += bytes;
    }
    /* Success. */
    return 0;
}

INITAPI(VMCOPY, VMCopyHandler)

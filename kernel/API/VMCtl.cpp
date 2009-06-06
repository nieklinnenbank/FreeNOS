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

#include <API/VMCtl.h>
#include <Error.h>
#include <Config.h>

Address VMCtlHandler(Operation action, ProcessID procID, Address paddr,
		     Address vaddr, ulong prot = PAGE_PRESENT|PAGE_USER|PAGE_RW)
{
    ArchProcess *proc = ZERO;
    Address page = ZERO;
    
    /* Find the given process. */
    if (!(proc = Process::byID(procID)))
    {
	return ESRCH;
    }
    switch (action)
    {
	case Lookup:    
	    return memory->lookupVirtual(proc, vaddr);

	case Map:
	    /* Map the memory page. */
	    if (prot & PAGE_PRESENT)
	    {
		page = paddr ? paddr : memory->allocatePhysical(PAGESIZE);
		memory->mapVirtual(proc, page, vaddr, prot & ~PAGEMASK);
	    }
	    /* Release memory page (if not pinned). */
	    else if (!memory->access(proc, vaddr, PAGE_PINNED))
	    {
		page = memory->lookupVirtual(proc, vaddr) & PAGEMASK;
		memory->releasePhysical(page);
	    }
	    break;
	    
	default:
	    return EINVAL;
	
    }
    /* Success. */
    return (page);
}

INITAPI(VMCTL, VMCtlHandler)

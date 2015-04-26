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

#include <FreeNOS/Kernel.h>
#include <FreeNOS/Config.h>
#include <Error.h>
#include "VMCtl.h"
#include "ProcessID.h"

#warning Do not depend on Intel specific functions in a generic API

Error VMCtlHandler(ProcessID procID, MemoryOperation op, MemoryRange *range)
{
#ifdef __i386__
    ProcessManager *procs = Kernel::instance->getProcessManager();
    IntelMemory *memory = (IntelMemory *) Kernel::instance->getMemory();
    Process *proc = ZERO;
    Address  page     = ZERO;
    Address *remotePG = (Address *) PAGETABADDR_FROM(PAGETABFROM,
                                                     PAGEUSERFROM);
    
    /* Find the given process. */
    if (procID == SELF)
        proc = procs->current();
    else if (!(proc = procs->get(procID)))
    {
        return ESRCH;
    }

    /* Validate the given MemoryRange pointer, if needed. */
    if (op != MapTables && op != UnMapTables &&
        !memory->access(procs->current(),
                       (Address) range, sizeof(MemoryRange)))
    {
        return EFAULT;
    }
    /* Perform operation. */
    switch (op)
    {
        case LookupVirtual:
            range->physicalAddress = memory->lookup(proc, range->virtualAddress);
            break;

        case LookupPhysical:
            return EINVAL;
            //return memory->isMarked(range->physicalAddress);

        case Map:

            /* Map the memory page. */
            if (range->access & Memory::Present)
            {
                /* Acquire physical page(s) first. */
                if (!range->physicalAddress)
                {
                    range->physicalAddress = memory->allocatePhysical(range->bytes);
                }
                /* Insert virtual page(s). */
                for (Size i = 0; i < range->bytes; i += PAGESIZE)
                {
                    memory->map(proc,
                                range->physicalAddress + i,
                                range->virtualAddress  + i,
                                range->access);
                }
            }
            /* Release memory page(s). */
            else
            {
                for (Size i = 0; i < range->bytes; i += PAGESIZE)
                {
                    /* Don't release pinned pages. */
                    if (memory->access(proc, range->virtualAddress + i,
                                       Memory::Pinned))
                        continue;
                
                    if ((page = memory->lookup(proc,
                                               range->virtualAddress + i)))
                    {
                        memory->releasePhysical(page & PAGEMASK);
                    }
                }
            }
            break;

        case Access:
            return memory->access(proc, range->virtualAddress,
                                        range->bytes, range->access);
            
        case MapTables:

            /* Map remote page tables. */
            memory->mapRemote((IntelProcess *)proc, 0,
                             (Address) PAGETABADDR_FROM(PAGETABFROM, PAGEUSERFROM),
                              Memory::Present | Memory::User | Memory::Writable); // PAGE_USER);
            
            /* Temporarily allow userlevel access to the page tables. */
            for (Size i = 0; i < PAGEDIR_MAX; i++)
            {
                if (!(remotePG[i] & PAGE_USER))
                {
                    remotePG[i] |= PAGE_MARKED;
                }
                remotePG[i] |= PAGE_USER;
            }
            /* Flush caches. */
            tlb_flush_all();
            break;
            
        case UnMapTables:

            /* Remove userlevel access where needed. */
            for (Size i = 0; i < PAGEDIR_MAX; i++)
            {
                if (remotePG[i] & PAGE_MARKED)
                {
                    remotePG[i] &= ~PAGE_USER;
                }
            }
            /* Flush caches. */
            tlb_flush_all();
            break;
            
        default:
            return EINVAL;
        
    }
#endif
    /* Success. */
    return 0;
}

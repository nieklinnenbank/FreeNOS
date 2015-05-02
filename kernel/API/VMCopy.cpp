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


#include "VMCopy.h"
#include <FreeNOS/Config.h>
#include <FreeNOS/Process.h>
#include <FreeNOS/API.h>
#include <FreeNOS/Kernel.h>
#include <FreeNOS/System/Constant.h>
#include <MemoryBlock.h>

#warning Do not depend on Intel specific flags for generic APIs

Error VMCopyHandler(ProcessID procID, API::Operation how, Address ours,
                    Address theirs, Size sz)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Memory *memory = Kernel::instance->getMemory();
    Process *proc;
    Address paddr, tmpAddr;
    Size bytes = 0, pageOff, total = 0;
    
#ifdef __i386__

    /* Find the corresponding Process. */
    if (!(proc = procs->get(procID)))
    {
        return API::NotFound;
    }
    /* Verify memory addresses. */
    if (!memory->access(procs->current(), ours, sz) ||
        !memory->access(proc, theirs, sz))
    {
        return API::AccessViolation;
    }
    /* Keep on going until all memory is processed. */
    while (total < sz)
    {
        /* Update variables. */
        paddr   = memory->lookup(proc, theirs) & PAGEMASK;
        pageOff = theirs & ~PAGEMASK;
        bytes   = (PAGESIZE - pageOff) < (sz - total) ?
                  (PAGESIZE - pageOff) : (sz - total);
                
        /* Valid address? */
        if (!paddr) break;
                
        /* Map the physical page. */
        tmpAddr = memory->map(paddr);

        /* Process the action appropriately. */
        switch (how)
        {
            case API::Read:
                MemoryBlock::copy((void *)ours, (void *)(tmpAddr + pageOff), bytes);
                break;
                        
            case API::Write:
                MemoryBlock::copy((void *)(tmpAddr + pageOff), (void *)ours, bytes);
                break;
            
            default:
                ;
        }       
        /* Remove mapping. */
        memory->map((Address) 0, (Address) tmpAddr, Memory::None);
        ours   += bytes;
        theirs += bytes;
        total  += bytes;
    }
#endif
    /* Success. */
    return total;
}

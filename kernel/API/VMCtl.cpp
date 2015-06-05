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

#include <BitAllocator.h>
#include "VMCtl.h"
#include "ProcessID.h"

Error VMCtlHandler(ProcessID procID, MemoryOperation op, Memory::Range *range)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Process *proc = ZERO;
    Error ret = API::Success;
    
    // Find the given process
    if (procID == SELF)
        proc = procs->current();
    else if (!(proc = procs->get(procID)))
    {
        return API::NotFound;
    }

    Address *localDirectory = ((Address *) PAGEDIR_LOCAL) + (PAGEDIR_LOCAL >> PAGESHIFT);
    switch (op)
    {
        case MapTables:
            // Insert the page directory into the selected virtual address
            // TODO: ofcourse, this needs proper access checking.
            // TODO: why not use a Arch::Memory instead???

            // Modify the local page directory to insert the mapping
            localDirectory[ DIRENTRY(range->virt) ] =
                range->phys | PAGE_WRITE | PAGE_PRESENT | PAGE_RESERVE | PAGE_USER;
            tlb_flush_all();
            return API::Success;
            
        case UnMapTables:
            localDirectory[ DIRENTRY( range->virt ) ] = 0;
            tlb_flush_all();
            return API::Success;

        default:
            break;
    }

    Arch::Memory mem(proc->getPageDirectory(),
                     Kernel::instance->getMemory()->getBitArray());

    // Perform operation
    switch (op)
    {
        case LookupVirtual:
            range->phys = mem.lookup(range->virt);
            break;

        case LookupPhysical:
            ret = API::InvalidArgument;
            break;

        case Map:
            mem.mapRange(range);
            break;

        case UnMap:
            mem.releaseRange(range);
            break;

        case Access:
            ret = (API::Error) mem.access(range->virt,
                                          range->size,
                                          range->access);
            break;
            
        default:
            ret = API::InvalidArgument;
            break;
    }
    // Done
    return ret;
}

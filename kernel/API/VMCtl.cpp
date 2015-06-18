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

    Arch::Memory mem(proc->getPageDirectory(),
                     Kernel::instance->getMemory());

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
            if (!range->virt)
                range->virt = mem.findFree(range->size, Memory::UserPrivate);
            mem.mapRange(range);
            break;

        case UnMap:
            mem.releaseRange(range);
            break;

        case Access:
            ret = (API::Error) mem.access(range->virt);
            break;
            
        default:
            ret = API::InvalidArgument;
            break;
    }
    // Done
    return ret;
}

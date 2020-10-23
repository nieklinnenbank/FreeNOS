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

#include <FreeNOS/System.h>
#include <FreeNOS/ProcessManager.h>
#include <MemoryBlock.h>
#include <SplitAllocator.h>
#include "VMCopy.h"

API::Result VMCopyHandler(const ProcessID procID,
                          const API::Operation how,
                          const Address ours,
                          const Address theirs,
                          const Size sz)
{
    ProcessManager *procs = Kernel::instance()->getProcessManager();
    MemoryContext::Result memResult = MemoryContext::Success;
    Size bytes = 0, pageOff, total = 0;
    Address paddr, vaddr;
    Address ourAddr = ours, theirAddr = theirs;
    Process *proc;

    DEBUG("");

    // Find the corresponding Process
    if (procID == SELF)
        proc = procs->current();
    else if (!(proc = procs->get(procID)))
        return API::NotFound;

    MemoryContext *local  = procs->current()->getMemoryContext();
    MemoryContext *remote = proc->getMemoryContext();

    // Keep on going until all memory is processed
    while (total < sz)
    {
        // Update variables
        if (how == API::ReadPhys)
            paddr = theirAddr & PAGEMASK;
        else if (remote->lookup(theirAddr, &paddr) != MemoryContext::Success)
            return API::AccessViolation;

        assert(!(paddr & ~PAGEMASK));
        pageOff = theirAddr & ~PAGEMASK;
        bytes   = (PAGESIZE - pageOff) < (sz - total) ?
                  (PAGESIZE - pageOff) : (sz - total);

        // Valid address?
        if (!paddr) break;

        // Map their address into our local address space
        if (local->findFree(PAGESIZE, MemoryMap::KernelPrivate, &vaddr) != MemoryContext::Success)
            return API::RangeError;

        if ((memResult = local->map(vaddr, paddr,
                                    Memory::Readable | Memory::Writable)) != MemoryContext::Success)
        {
            ERROR("failed to map physical address " << (void *)paddr << ": " << (int)memResult);
            return API::IOError;
        }

        // Process the action appropriately
        switch (how)
        {
            case API::Read:
            case API::ReadPhys:
                MemoryBlock::copy((void *)ourAddr, (void *)(vaddr + pageOff), bytes);
                break;

            case API::Write:
                MemoryBlock::copy((void *)(vaddr + pageOff), (void *)ourAddr, bytes);
                break;

            default:
                ;
        }

        // Unmap, which must always succeed
        memResult = local->unmap(vaddr);
        assert(memResult == MemoryContext::Success);

        // Update counters
        ourAddr   += bytes;
        theirAddr += bytes;
        total     += bytes;
    }

    return API::Success;
}

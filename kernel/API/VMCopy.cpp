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

#include <MemoryBlock.h>
#include "VMCopy.h"

Error VMCopyHandler(ProcessID procID, API::Operation how, Address ours,
                    Address theirs, Size sz)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Process *proc;
    Address paddr;
    Size bytes = 0, pageOff, total = 0;

    // Find the corresponding Process
    if (!(proc = procs->get(procID)))
    {
        return API::NotFound;
    }
    // TODO: Verify memory addresses
    BitArray *bits = Kernel::instance->getMemory()->getMemoryBitArray();
    Arch::Memory local(0, bits);
    Arch::Memory remote(proc->getPageDirectory(), bits);

    // Keep on going until all memory is processed
    while (total < sz)
    {
        /* Update variables. */
        paddr   = remote.lookup(theirs);
        pageOff = theirs & ~PAGEMASK;
        bytes   = (PAGESIZE - pageOff) < (sz - total) ?
                  (PAGESIZE - pageOff) : (sz - total);
                
        /* Valid address? */
        if (!paddr) break;
                
        // Map their address into our local address space
        Address tmp = local.map(paddr, ZERO, Arch::Memory::Present  |
                                             Arch::Memory::User     |
                                             Arch::Memory::Readable |
                                             Arch::Memory::Writable);

        /* Process the action appropriately. */
        switch (how)
        {
            case API::Read:
                MemoryBlock::copy((void *)ours, (void *)(tmp + pageOff), bytes);
                break;
                        
            case API::Write:
                MemoryBlock::copy((void *)(tmp + pageOff), (void *)ours, bytes);
                break;
            
            default:
                ;
        }       
        // Unmap
        local.unmap(tmp);

        // Update counters
        ours   += bytes;
        theirs += bytes;
        total  += bytes;
    }
    return total;
}

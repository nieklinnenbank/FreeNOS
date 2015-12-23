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

#include <FreeNOS/System.h>
#include "VMShare.h"
#include "ProcessID.h"

Error createShare(Process *proc, Memory::Range *range)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();
    MemoryShare *localShare = ZERO;
    MemoryShare *remoteShare = ZERO;
    MemoryContext *localMem  = procs->current()->getMemoryContext();
    MemoryContext *remoteMem = proc->getMemoryContext();

    if (range->size == 0)
        return API::InvalidArgument;

    // Allocate MemoryShare objects
    localShare  = new MemoryShare;
    if (!localShare)
        return API::OutOfMemory;

    remoteShare = new MemoryShare;
    if (!remoteShare)
    {
        delete localShare;
        return API::OutOfMemory;
    }
    // Fill the local share object
    localShare->pid        = proc->getID();
    localShare->coreId     = Kernel::instance->getCoreInfo()->coreId;
    localShare->range.phys = 0;
    localShare->range.size = range->size;
    localShare->range.access = Memory::User | range->access;

    // Map in the local process
    if (localMem->findFree(range->size, MemoryMap::UserPrivate, &localShare->range.virt) != MemoryContext::Success ||
        localMem->mapRange(&localShare->range) != MemoryContext::Success)
    {
        delete localShare;
        delete remoteShare;
        return API::OutOfMemory;
    }
    // Fill the remote share object
    remoteShare->pid          = procs->current()->getID();
    remoteShare->coreId       = localShare->coreId;
    remoteShare->range.phys   = localShare->range.phys;
    remoteShare->range.size   = localShare->range.size;
    remoteShare->range.access = localShare->range.access;
    remoteShare->range.virt   = 0;

    // Map in the remote process
    if (remoteMem->findFree(range->size, MemoryMap::UserPrivate, &remoteShare->range.virt) != MemoryContext::Success ||
        remoteMem->mapRange(&remoteShare->range) != MemoryContext::Success)
    {
        delete localShare;
        delete remoteShare;
        return API::OutOfMemory;
    }
    // TODO: for MemoryChannel: update access permissions

    // Zero out the whole data buffer
    MemoryBlock::set((Address *)localShare->range.virt, 0, localShare->range.size);

    // insert into both process's shares
    procs->current()->getShares().insert(*localShare);
    proc->getShares().insert(*remoteShare);

    // Update parameter output
    range->phys = localShare->range.phys;
    range->virt = localShare->range.virt;
    return API::Success;
}

Error readShare(MemoryShare *shares)
{
    Process *proc = Kernel::instance->getProcessManager()->current();
    Index<MemoryShare> & sh = proc->getShares();
    Size count = sh.count();

    for (Size i = 0; i < count; i++)
    {
        MemoryBlock::copy(shares++, sh.get(i), sizeof(MemoryShare));
    }
    return API::Success;
}

Error VMShareHandler(ProcessID procID, API::Operation op, Address parameter)
{
    // TODO: use the share.range.physicalAddress as identifier. its unique.
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Process *proc = ZERO;
    Error ret = API::Success;
    
    // Find the given process
    if (procID == SELF)
    {
        if (op != API::Read)
            return API::InvalidArgument;
        else
            proc = procs->current();
    }
    else if (!(proc = procs->get(procID)))
    {
        return API::NotFound;
    }

    switch (op)
    {
        case API::Create:
            ret = createShare(proc, (Memory::Range *) parameter);
            break;

        case API::Read:
            ret = readShare((MemoryShare *) parameter);
            break;

        default:
            return API::InvalidArgument;
    }

    // Done
    return ret;
}

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
#include <MemoryChannel.h>
#include <List.h>
#include <ListIterator.h>
#include <SplitAllocator.h>
#include "ProcessEvent.h"

#warning merge with Process please

ProcessShares::ProcessShares(ProcessID pid)
{
    m_pid    = pid;
    m_memory = ZERO;
    m_kernelChannel = new MemoryChannel;
}

ProcessShares::~ProcessShares()
{
    ProcessManager *procs = Kernel::instance->getProcessManager();
    List<ProcessID> pids;

    // Cleanup members
    delete m_kernelChannel;

    // Make a list of unique process IDs which
    // have a share with this Process
    Size count = m_shares.count();
    for (Size i = 0; i < count; i++)
    {
        const MemoryShare *sh = m_shares.get(i);
        if (sh && !pids.contains(sh->pid))
            pids.append(sh->pid);
    }

    // Raise process terminated events
    for (ListIterator<ProcessID> i(pids); i.hasCurrent(); i++)
    {
        ProcessID pid = i.current();
        Process *proc = procs->get(pid);
        if (proc)
        {
            ProcessEvent event;
            event.type = ProcessTerminated;
            event.number = m_pid;
            proc->raiseEvent(&event);
        }
    }
}

const ProcessID ProcessShares::getProcessID() const
{
    return m_pid;
}

MemoryContext * ProcessShares::getMemoryContext()
{
    return m_memory;
}

ProcessShares::Result ProcessShares::setMemoryContext(MemoryContext *ctx)
{
    m_memory = ctx;
    return Success;
}

ProcessShares::Result ProcessShares::createShare(ProcessID pid,
                                                 Size coreId,
                                                 Size tagId,
                                                 Address virt,
                                                 Size size)
{
    MemoryShare *share = ZERO;

    if (size == 0 || size % PAGESIZE)
        return InvalidArgument;

    // Allocate MemoryShare objects
    share  = new MemoryShare;
    if (!share)
        return OutOfMemory;

    // Fill the share object
    share->pid        = pid;
    share->coreId     = coreId;
    share->tagId      = tagId;
    share->range.virt = virt;
    share->range.size = size;
    m_memory->lookup(share->range.virt, &share->range.phys);
    m_memory->access(share->range.virt, &share->range.access);

    // insert into shares list
    m_shares.insert(*share);
    return Success;
}

ProcessShares::Result ProcessShares::createShare(ProcessShares & instance,
                                                 ProcessShares::MemoryShare *share)
{
    MemoryShare *localShare = ZERO;
    MemoryShare *remoteShare = ZERO;
    MemoryContext *localMem  = m_memory;
    MemoryContext *remoteMem = instance.getMemoryContext();
    Address paddr, vaddr;

    if (share->range.size == 0)
        return InvalidArgument;

    // Check if the share already exists
    if (readShare(share) == Success)
        return AlreadyExists;

    // Allocate local
    localShare = new MemoryShare;
    if (!localShare)
        return OutOfMemory;

    // Allocate remote
    remoteShare = new MemoryShare;
    if (!remoteShare)
    {
        delete localShare;
        return OutOfMemory;
    }
    // Allocate actual pages
    if (Kernel::instance->getAllocator()->allocateLow(share->range.size, &paddr) != Allocator::Success)
        return OutOfMemory;

    // Zero out the pages
    vaddr = (Address) Kernel::instance->getAllocator()->toVirtual(paddr);
    MemoryBlock::set((void *)vaddr, 0, share->range.size);
    for (Size i = 0; i < share->range.size; i+=PAGESIZE)
        cache1_clean(vaddr + i);

    // Fill the local share object
    localShare->pid        = instance.getProcessID();
    localShare->coreId     = Kernel::instance->getCoreInfo()->coreId;
    localShare->tagId      = share->tagId;
    localShare->range.phys = paddr;
    localShare->range.size = share->range.size;
    localShare->range.access = Memory::User | share->range.access;

    // Map in the local process
    if (localMem->findFree(localShare->range.size, MemoryMap::UserPrivate, &localShare->range.virt) != MemoryContext::Success ||
        localMem->mapRange(&localShare->range) != MemoryContext::Success)
    {
        delete localShare;
        delete remoteShare;
        return OutOfMemory;
    }
    // Fill the remote share object
    remoteShare->pid          = m_pid;
    remoteShare->coreId       = localShare->coreId;
    remoteShare->tagId        = localShare->tagId;
    remoteShare->range.phys   = localShare->range.phys;
    remoteShare->range.size   = localShare->range.size;
    remoteShare->range.access = localShare->range.access;

    // Map in the remote process
    if (remoteMem->findFree(remoteShare->range.size, MemoryMap::UserPrivate, &remoteShare->range.virt) != MemoryContext::Success ||
        remoteMem->mapRange(&remoteShare->range) != MemoryContext::Success)
    {
        delete localShare;
        delete remoteShare;
        return OutOfMemory;
    }
    // TODO: for MemoryChannel: update access permissions, ro/rw for data/feedback

    // insert into shares list
    m_shares.insert(*localShare);
    instance.m_shares.insert(*remoteShare);

    // raise event on the remote process
    Process *proc = Kernel::instance->getProcessManager()->get(instance.getProcessID());
    ProcessEvent event;
    event.type = ShareCreated;
    MemoryBlock::copy(&event.share, remoteShare, sizeof(*remoteShare));
    proc->raiseEvent(&event);

    // Update parameter outputs
    MemoryBlock::copy(share, localShare, sizeof(*share));
    return Success;
}

ProcessShares::Result ProcessShares::readShare(MemoryShare *share)
{
    Size count     = m_shares.count();
    const MemoryShare *s = 0;

    for (Size i = 0; i < count; i++)
    {
        if ((s = m_shares.get(i)) != ZERO)
        {
            if (s->pid == share->pid &&
                s->coreId == share->coreId &&
                s->tagId == share->tagId)
            {
                MemoryBlock::copy(share, s, sizeof(MemoryShare));
                return Success;
            }
        }
    }
    return NotFound;
}

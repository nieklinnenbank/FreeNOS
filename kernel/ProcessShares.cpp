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
#include <MemoryContext.h>
#include <MemoryChannel.h>
#include <List.h>
#include <ListIterator.h>
#include <SplitAllocator.h>
#include "ProcessEvent.h"
#include "ProcessManager.h"

ProcessShares::ProcessShares(ProcessID pid)
{
    m_pid    = pid;
    m_memory = ZERO;
}

ProcessShares::~ProcessShares()
{
    ProcessManager *procs = Kernel::instance()->getProcessManager();
    List<ProcessID> pids;

    // Make a list of unique process IDs which
    // have a share with this Process
    Size size = m_shares.size();
    for (Size i = 0; i < size; i++)
    {
        MemoryShare *sh = m_shares.get(i);
        if (sh)
        {
            if (!pids.contains(sh->pid))
                pids.append(sh->pid);

            releaseShare(sh, i);
        }
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
            procs->raiseEvent(proc, &event);
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
    MemoryContext::Result result = MemoryContext::Success;

    if (size == 0 || size % PAGESIZE)
        return InvalidArgument;

    // This code currently does not support intra-core IPC
    assert(coreId == coreInfo.coreId);

    // Allocate MemoryShare objects
    share  = new MemoryShare;
    if (!share)
    {
        ERROR("failed to allocate MemoryShare");
        return OutOfMemory;
    }

    // Fill the share object
    share->pid        = pid;
    share->coreId     = coreId;
    share->tagId      = tagId;
    share->range.virt = virt;
    share->range.size = size;

    // For the kernel channel, set to unattached to ensure releaseShare() always works
    share->attached   = !(pid == KERNEL_PID);

    // Translate to physical address
    if ((result = m_memory->lookup(share->range.virt, &share->range.phys)) != MemoryContext::Success)
    {
        ERROR("failed to translate share virtual address at " <<
             (void *)share->range.virt << ": " << (int)result);
        delete share;
        return MemoryMapError;
    }
    assert(!(share->range.phys & ~PAGEMASK));

    // Retrieve memory access permissions
    if ((result = m_memory->access(share->range.virt, &share->range.access)) != MemoryContext::Success)
    {
        ERROR("failed to retrieve share access permissions for virtual address " <<
             (void *)share->range.virt << ": " << (int)result);
        delete share;
        return MemoryMapError;
    }

    // insert into shares list
    Size idx = 0;
    m_shares.insert(idx, share);
    return Success;
}

ProcessShares::Result ProcessShares::createShare(ProcessShares & instance,
                                                 ProcessShares::MemoryShare *share)
{
    MemoryShare *localShare = ZERO;
    MemoryShare *remoteShare = ZERO;
    MemoryContext *localMem  = m_memory;
    MemoryContext *remoteMem = instance.getMemoryContext();
    Arch::Cache cache;
    Allocator::Range allocPhys, allocVirt;
    Size idx = 0;

    if (share->range.size == 0)
        return InvalidArgument;

    // Check if the share already exists
    if (findShare(share->pid, share->coreId, share->tagId) != ZERO)
        return AlreadyExists;

    // Check if the remote share is still detaching
    remoteShare = instance.findShare(m_pid, share->coreId, share->tagId);
    if (remoteShare && !remoteShare->attached)
    {
        return DetachInProgress;
    }

    // Allocate local
    localShare = new MemoryShare;
    if (!localShare)
    {
        ERROR("failed to allocate MemoryShare for local process");
        return OutOfMemory;
    }

    // Allocate remote
    remoteShare = new MemoryShare;
    if (!remoteShare)
    {
        ERROR("failed to allocate MemoryShare for remote process");
        delete localShare;
        return OutOfMemory;
    }

    // Allocate actual pages
    allocPhys.address = 0;
    allocPhys.size = share->range.size;
    allocPhys.alignment = PAGESIZE;

    if (Kernel::instance()->getAllocator()->allocate(allocPhys, allocVirt) != Allocator::Success)
    {
        ERROR("failed to allocate pages for MemoryShare");
        return OutOfMemory;
    }

    // Zero out the pages
    MemoryBlock::set((void *) allocVirt.address, 0, share->range.size);
    for (Size i = 0; i < share->range.size; i+=PAGESIZE)
        cache.cleanData(allocVirt.address + i);

    // Fill the local share object
    localShare->pid        = instance.getProcessID();
    localShare->coreId     = Kernel::instance()->getCoreInfo()->coreId;
    localShare->tagId      = share->tagId;
    localShare->range.phys = allocPhys.address;
    localShare->range.size = share->range.size;
    localShare->range.access = Memory::User | share->range.access;
    localShare->attached   = true;

    // Map in the local process
    if (localMem->findFree(localShare->range.size, MemoryMap::UserShare, &localShare->range.virt) != MemoryContext::Success ||
        localMem->mapRangeContiguous(&localShare->range) != MemoryContext::Success)
    {
        ERROR("failed to map MemoryShare in local process");
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
    remoteShare->attached     = true;

    // Map in the remote process
    if (remoteMem->findFree(remoteShare->range.size, MemoryMap::UserShare, &remoteShare->range.virt) != MemoryContext::Success ||
        remoteMem->mapRangeContiguous(&remoteShare->range) != MemoryContext::Success)
    {
        ERROR("failed to map MemoryShare in remote process");
        delete localShare;
        delete remoteShare;
        return OutOfMemory;
    }
    // insert into shares list
    m_shares.insert(idx, localShare);
    instance.m_shares.insert(idx, remoteShare);

    // raise event on the remote process
    ProcessManager *procs = Kernel::instance()->getProcessManager();
    Process *proc = procs->get(instance.getProcessID());
    ProcessEvent event;
    event.type   = ShareCreated;
    event.number = m_pid;
    MemoryBlock::copy(&event.share, remoteShare, sizeof(*remoteShare));
    procs->raiseEvent(proc, &event);

    // Update parameter outputs
    MemoryBlock::copy(share, localShare, sizeof(*share));
    return Success;
}

ProcessShares::Result ProcessShares::removeShares(ProcessID pid)
{
    const Size size = m_shares.size();
    MemoryShare *s = 0;

    for (Size i = 0; i < size; i++)
    {
        if ((s = m_shares.get(i)) != ZERO)
        {
            if (s->pid != pid)
                continue;

            releaseShare(s, i);
        }
    }
    return Success;
}

ProcessShares::Result ProcessShares::releaseShare(MemoryShare *s, Size idx)
{
    assert(s->coreId == coreInfo.coreId);

    // Only release physical memory if both processes have detached.
    // Note that in case all memory shares for a certain ProcessID have
    // been detached but not yet released, and due to a very unlikely race
    // condition (ProcessID reuse) a new memory share was just created (before old ones were released)
    // the new memory share would also be detached here, resulting in a memory
    // share with is detached in this process but attached and useless in the
    // other process.
    if (s->attached)
    {
        Process *proc = Kernel::instance()->getProcessManager()->get(s->pid);
        if (proc)
        {
            ProcessShares & shares = proc->getShares();
            const Size size = shares.m_shares.size();

            // Mark all process shares detached in the other process
            for (Size i = 0; i < size; i++)
            {
                MemoryShare *otherShare = shares.m_shares.get(i);
                if (otherShare)
                {
                    assert(otherShare->coreId == coreInfo.coreId);

                    if (otherShare->pid == m_pid && otherShare->coreId == s->coreId)
                    {
                        otherShare->attached = false;
                    }
                }
            }
        }
    }
    else
    {
        // Only release physical memory pages if the other
        // process already detached earlier
        Allocator *alloc = Kernel::instance()->getAllocator();

        for (Size i = 0; i < s->range.size; i += PAGESIZE)
            alloc->release(s->range.phys + i);
    }

    // Unmap the share
    m_memory->unmapRange(&s->range);

    // Release the share object
    delete s;

    // Remove share from our administration
    if (!m_shares.remove(idx))
    {
        return NotFound;
    }
    else
    {
        return Success;
    }
}

ProcessShares::MemoryShare * ProcessShares::findShare(const ProcessID pid,
                                                      const Size coreId,
                                                      const Size tagId)
{
    const Size size = m_shares.size();

    for (Size i = 0; i < size; i++)
    {
        MemoryShare *s = m_shares.get(i);

        if (s != ZERO)
        {
            assert(s->coreId == coreInfo.coreId);

            if (s->pid == pid && s->coreId == coreId && s->tagId == tagId)
            {
                return s;
            }
        }
    }

    return ZERO;
}

ProcessShares::Result ProcessShares::readShare(MemoryShare *share)
{
    const MemoryShare *s = findShare(share->pid, share->coreId, share->tagId);
    if (s != ZERO)
    {
        MemoryBlock::copy(share, s, sizeof(MemoryShare));
        return Success;
    }

    return NotFound;
}

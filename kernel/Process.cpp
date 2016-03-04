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
#include <FreeNOS/API.h>
#include <Index.h>
#include <MemoryBlock.h>
#include <MemoryChannel.h>
#include <SplitAllocator.h>
#include "Process.h"
#include "ProcessEvent.h"

Process::Process(ProcessID id, Address entry, bool privileged, const MemoryMap &map)
    : m_id(id), m_map(map), m_shares(id)
{
    m_state         = Stopped;
    m_kernelStack   = 0;
    m_userStack     = 0;
    m_pageDirectory = 0;
    m_parent        = 0;
    m_waitId        = 0;
    m_wakeups       = 0;
    m_entry         = entry;
    m_privileged    = privileged;
    m_memoryContext = ZERO;
    m_kernelChannel = new MemoryChannel;
    MemoryBlock::set(&m_sleepTimer, 0, sizeof(m_sleepTimer));
}
    
Process::~Process()
{
    delete m_kernelChannel;

    if (m_memoryContext)
    {
        m_memoryContext->releaseRegion(MemoryMap::UserData);
        m_memoryContext->releaseRegion(MemoryMap::UserHeap);
        m_memoryContext->releaseRegion(MemoryMap::UserStack);
        m_memoryContext->releaseRegion(MemoryMap::UserPrivate);
        delete m_memoryContext;
    }
}

ProcessID Process::getID() const
{
    return m_id;
}

ProcessID Process::getParent() const
{
    return m_parent;
}

ProcessID Process::getWait() const
{
    return m_waitId;
}

Process::State Process::getState() const
{
    return m_state;
}

ProcessShares & Process::getShares()
{
    return m_shares;
}

const Timer::Info * Process::getSleepTimer() const
{
    return &m_sleepTimer;
}

Address Process::getPageDirectory() const
{
    return m_pageDirectory;
}

Address Process::getUserStack() const
{
    return m_userStack;
}

Address Process::getKernelStack() const
{
    return m_kernelStack;
}

MemoryContext * Process::getMemoryContext()
{
    return m_memoryContext;
}

bool Process::isPrivileged() const
{
    return m_privileged;
}

void Process::setState(Process::State st)
{
    m_state = st;
}

void Process::setParent(ProcessID id)
{
    m_parent = id;
}

void Process::setWait(ProcessID id)
{
    m_waitId = id;
}

void Process::setSleepTimer(const Timer::Info *sleepTimer)
{
    MemoryBlock::copy(&m_sleepTimer, sleepTimer, sizeof(m_sleepTimer));
}

void Process::setPageDirectory(Address addr)
{
    m_pageDirectory = addr;
}

void Process::setUserStack(Address addr)
{
    m_userStack = addr;
}

void Process::setKernelStack(Address addr)
{
    m_kernelStack = addr;
}

Process::Result Process::raiseEvent(ProcessEvent *event)
{
    // Write the message. Be sure to flush the caches because
    // the kernel has mapped the channel pages separately in low memory.
    m_kernelChannel->write(event);
    m_kernelChannel->flush();

    // Wakeup the Process, if needed
    return wakeup();
}

Process::Result Process::initialize()
{
    Memory::Range range;
    Address paddr, vaddr;
    Arch::Cache cache;

    // Allocate two pages for the kernel event channel
    if (Kernel::instance->getAllocator()->allocateLow(PAGESIZE*2, &paddr) != Allocator::Success)
        return OutOfMemory;

    // Translate to virtual address in kernel low memory
    vaddr = (Address) Kernel::instance->getAllocator()->toVirtual(paddr);
    MemoryBlock::set((void *)vaddr, 0, PAGESIZE*2);
    cache.cleanData(vaddr);
    cache.cleanData(vaddr + PAGESIZE);

    // Map data and feedback pages in userspace
    range.phys   = paddr;
    range.access = Memory::User | Memory::Readable;
    range.size   = PAGESIZE * 2;
    m_memoryContext->findFree(range.size, MemoryMap::UserPrivate, &range.virt);
    m_memoryContext->mapRange(&range);

    // Remap the feedback page with write permissions
    m_memoryContext->unmap(range.virt + PAGESIZE);
    m_memoryContext->map(range.virt + PAGESIZE,
                         range.phys + PAGESIZE, Memory::User | Memory::Readable | Memory::Writable);

    // Create shares entry
    m_shares.setMemoryContext(m_memoryContext);
    m_shares.createShare(KERNEL_PID, Kernel::instance->getCoreInfo()->coreId, 0, range.virt, range.size);

    // Setup the kernel event channel
    m_kernelChannel->setMode(Channel::Producer);
    m_kernelChannel->setMessageSize(sizeof(ProcessEvent));
    m_kernelChannel->setVirtual(vaddr, vaddr + PAGESIZE);

    return Success;
}

Process::Result Process::wakeup()
{
    m_wakeups++;

    if (m_state != Running)
        m_state = Ready;

    MemoryBlock::set(&m_sleepTimer, 0, sizeof(m_sleepTimer));
    return Success;
}

Process::Result Process::sleep(Timer::Info *timer)
{
    if (!m_wakeups)
    {
        m_state = Sleeping;
        
        if (timer)
            MemoryBlock::copy(&m_sleepTimer, timer, sizeof(m_sleepTimer));

        return Success;
    }
    m_wakeups = 0;
    return WakeupPending;
}

bool Process::operator==(Process *proc)
{
    return proc->getID() == m_id;
}

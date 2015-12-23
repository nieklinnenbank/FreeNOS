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

#include <FreeNOS/API.h>
#include <Index.h>
#include <MemoryBlock.h>
#include "Process.h"

Process::Process(ProcessID id, Address entry, bool privileged, const MemoryMap &map)
    : m_id(id), m_map(map)
{
    m_state         = Stopped;
    m_kernelStack   = 0;
    m_userStack     = 0;
    m_pageDirectory = 0;
    m_parent        = 0;
    m_waitId        = 0;
    m_entry         = entry;
    m_privileged    = privileged;
    m_memoryContext = ZERO;
    MemoryBlock::set(&m_sleepTimer, 0, sizeof(m_sleepTimer));
}
    
Process::~Process()
{
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

Index<MemoryShare> & Process::getShares()
{
    return m_shares;
}

const Timer::Info & Process::getSleepTimer() const
{
    return m_sleepTimer;
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

List<Message *> * Process::getMessages()
{
    return &m_messages;
}

bool Process::operator==(Process *proc)
{
    return proc->getID() == m_id;
}

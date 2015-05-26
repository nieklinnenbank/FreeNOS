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

#include "Process.h"

Process::Process(ProcessID id, Address addr)
    : m_id(id)
{
    m_state         = Stopped;
    m_kernelStack   = 0;
    m_userStack     = 0;
    m_pageDirectory = 0;
}
    
Process::~Process()
{
}

ProcessID Process::getID() const
{
    return m_id;
}

Process::State Process::getState() const
{
    return m_state;
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

void Process::setState(Process::State st)
{
    m_state = st;
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

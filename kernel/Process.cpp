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

#include "Process.h"
#include <Types.h>

Process::Process(ProcessID id, Address addr)
    : m_id(id)
{
    m_state = Stopped;
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

void Process::setState(Process::State st)
{
    m_state = st;
}

List<UserMessage *> * Process::getMessages()
{
    return &m_messages;
}

bool Process::operator==(Process *proc)
{
    return proc->getID() == m_id;
}

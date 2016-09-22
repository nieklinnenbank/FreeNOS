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
#include <Log.h>
#include "ProcessManager.h"

// TODO: replace Vector with an Index..

ProcessManager::ProcessManager(Scheduler *scheduler)
    : m_procs(MAX_PROCS)
{
    DEBUG("m_procs = " << MAX_PROCS);
    m_scheduler = scheduler;
    m_current   = ZERO;
    m_previous  = ZERO;
    m_idle      = ZERO;
}

ProcessManager::~ProcessManager()
{
}

Scheduler * ProcessManager::getScheduler()
{
    return m_scheduler;
}

Process * ProcessManager::create(Address entry, const MemoryMap &map)
{
    Process *proc = new Arch::Process(m_procs.count(), entry, false, map);

    // Insert to the process table
    if (proc && proc->initialize() == Process::Success)
    {
        m_procs.insert(proc);
        return proc;
    }
    return ZERO;
}

Process * ProcessManager::get(ProcessID id)
{
    // TODO: replace with an Index to make this more easy.
    Process **p = (Process **) m_procs.get(id);
    return p ? *p : ZERO;
}

void ProcessManager::remove(Process *proc, uint exitStatus)
{
    if (proc == m_previous)
        m_previous = ZERO;

    if (proc == m_idle)
        m_idle = ZERO;

    if (proc == m_current)
        m_current = ZERO;

    // Wakeup any Processes which are waiting for this Process
    Size size = m_procs.size();

    for (Size i = 0; i < size; i++)
    {
        if (m_procs[i] != ZERO &&
            m_procs[i]->getState() == Process::Waiting &&
            m_procs[i]->getWait() == proc->getID())
        {
            m_procs[i]->setState(Process::Ready);
            m_procs[i]->setWait(exitStatus);
        }
    }

    // Remove process from administration
    m_procs[proc->getID()] = ZERO;

    // Free the process memory
    delete proc;
}

void ProcessManager::schedule(Process *proc)
{
    /* If needed, let the scheduler select a new process */
    if (!proc)
    {
        proc = m_scheduler->select(&m_procs, m_idle);
        
        /* If no process ready, let us idle. */
        if (!proc)
            proc = m_idle;
    }

    if (!proc)
    {
        FATAL("no process found to run!"); for(;;);
    }

    /* Only execute if its a different process */
    if (proc != m_current)
    {
        m_previous = m_current;
        m_current  = proc;

        if (m_previous && m_previous->getState() == Process::Running)
            m_previous->setState(Process::Ready);

        proc->setState(Process::Running);
        proc->execute(m_previous);
    }
}

Process * ProcessManager::current()
{
    return m_current;
}

Process * ProcessManager::previous()
{
    return m_previous;
}

void ProcessManager::setIdle(Process *proc)
{
    m_idle = proc;
}

Vector<Process *> * ProcessManager::getProcessTable()
{
    return &m_procs;
}

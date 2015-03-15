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

#include <Log.h>
#include <Error.h>
#include "ProcessManager.h"
#include <System/Function.h>

ProcessManager::ProcessManager(ProcessFactory *factory,
                               ProcessScheduler *scheduler)
    : m_procs(MAX_PROCS)
{
    m_factory   = factory;
    m_scheduler = scheduler;
    m_current   = ZERO;
    m_previous  = ZERO;
    m_idle      = ZERO;
}

ProcessManager::~ProcessManager()
{
}

Process * ProcessManager::create(Address entry)
{
    Process *proc = m_factory->createProcess(m_procs.count(), entry);
    ProcessID id  = m_procs.put(proc);
    assert(id == proc()->getID());

    return proc;
}

Process * ProcessManager::get(ProcessID id)
{
    return m_procs[id];
}

void ProcessManager::remove(Process *proc)
{
    if (proc == m_previous)
        m_previous = ZERO;

    if (proc == m_idle)
        m_idle = ZERO;

    if (proc == m_current)
    {
        FATAL("removing currently executing process"); for(;;);
        m_current = ZERO;
    }
    /* Remove process from administration */
    m_procs[proc->getID()] = ZERO;

    /* Free the process memory */
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
        proc->execute();
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

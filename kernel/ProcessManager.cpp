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

ProcessManager::ProcessManager()
    : m_procs(MAX_PROCS)
{
    DEBUG("m_procs = " << MAX_PROCS);

    m_current   = ZERO;
    m_idle      = ZERO;
}

ProcessManager::~ProcessManager()
{
}

Process * ProcessManager::create(Address entry, const MemoryMap &map, bool readyToRun)
{
    Process *proc = new Arch::Process(m_procs.count(), entry, false, map);

    // Insert to the process table
    if (proc && proc->initialize() == Process::Success)
    {
        m_procs.insert(proc);

        if (readyToRun)
        {
            proc->wakeup(true);
            m_scheduler.enqueue(proc);
        }

        if (m_current != 0)
            proc->setParent(m_current->getID());

        return proc;
    }
    return ZERO;
}

Process * ProcessManager::get(ProcessID id)
{
    Process **p = (Process **) m_procs.get(id);
    return p ? *p : ZERO;
}

void ProcessManager::remove(Process *proc, uint exitStatus)
{
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
            m_procs[i]->setWaitResult(exitStatus);
            m_procs[i]->wakeup(true);
            m_scheduler.enqueue(m_procs[i]);
        }
    }

    // Remove process from administration and schedule
    m_procs[proc->getID()] = ZERO;
    m_scheduler.dequeue(proc, true);

    // Free the process memory
    delete proc;
}

Process * ProcessManager::schedule(Process *proc)
{
    // If needed, let the scheduler select a new process
    if (!proc)
    {
        proc = m_scheduler.select(&m_procs, m_idle);

        // If no process ready, let us idle
        if (!proc)
            proc = m_idle;
    }

    if (!proc)
    {
        FATAL("no process found to run!"); for(;;);
    }

    // Only execute if its a different process
    if (proc != m_current)
    {
        Process *previous = m_current;

        m_current = proc;
        proc->execute(previous);
        return m_current;
    }
    return (Process *) NULL;
}

Process * ProcessManager::current()
{
    return m_current;
}

void ProcessManager::setIdle(Process *proc)
{
    m_idle = proc;
}

Vector<Process *> * ProcessManager::getProcessTable()
{
    return &m_procs;
}

ProcessManager::Result ProcessManager::wait(Process *proc)
{
    if (m_current->wait(proc->getID()) != Process::Success)
    {
        ERROR("process ID " << m_current->getID() << " failed to wait");
        return IOError;
    }

    if (m_scheduler.dequeue(m_current) != Scheduler::Success)
    {
        ERROR("process ID " << m_current->getID() << " not removed from Scheduler");
        return IOError;
    }

    return Success;
}

ProcessManager::Result ProcessManager::sleep(const Timer::Info *timer, bool ignoreWakeups)
{
    Process::Result result;

    if (!m_current)
    {
        ERROR("no current process found");
        return InvalidArgument;
    }

    result = m_current->sleep(timer, ignoreWakeups);
    switch (result)
    {
        case Process::WakeupPending:
            return WakeupPending;

        case Process::Success:
            if (m_scheduler.dequeue(m_current) != Scheduler::Success)
            {
                ERROR("process ID " << m_current->getID() << " not removed from Scheduler");
                return IOError;
            }
            break;

        default:
            ERROR("failed to sleep process ID " << m_current->getID() <<
                  ": result: " << (uint) result);
            return IOError;
    }

    return Success;
}

ProcessManager::Result ProcessManager::wakeup(Process *proc)
{
    Process::Result result;

    if ((result = proc->wakeup()) != Process::Success)
    {
        ERROR("failed to wakeup process ID " << proc->getID() <<
              ": result: " << (uint) result);
        return IOError;
    }

    if (m_scheduler.enqueue(proc) != Scheduler::Success)
    {
        ERROR("process ID " << proc->getID() << " not added to Scheduler");
        return IOError;
    }

    return Success;
}

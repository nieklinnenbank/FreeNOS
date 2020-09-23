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
#include <ListIterator.h>
#include "Scheduler.h"
#include "ProcessEvent.h"
#include "ProcessManager.h"

ProcessManager::ProcessManager()
    : m_procs()
    , m_interruptNotifyList(256)
{
    DEBUG("m_procs = " << MAX_PROCS);

    m_scheduler = new Scheduler();
    m_current   = ZERO;
    m_idle      = ZERO;
    m_interruptNotifyList.fill(ZERO);
}

ProcessManager::~ProcessManager()
{
    if (m_scheduler != NULL)
    {
        delete m_scheduler;
    }
}

Process * ProcessManager::create(const Address entry,
                                 const MemoryMap &map,
                                 const bool readyToRun,
                                 const bool privileged)
{
    Size pid = 0;

    // Insert a dummy to determine the next available PID
    if (!m_procs.insert(pid, (Process *) ~ZERO))
    {
        return ZERO;
    }

    // Create the new Process
    Process *proc = new Arch::Process(pid, entry, privileged, map);
    if (!proc)
    {
        ERROR("failed to allocate Process");
        m_procs.remove(pid);
        return ZERO;
    }

    // Initialize the Process
    const Process::Result result = proc->initialize();
    if (result != Process::Success)
    {
        ERROR("failed to initialize Process: result = " << (int) result);
        m_procs.remove(pid);
        delete proc;
        return ZERO;
    }

    // Overwrite dummy with actual Process
    m_procs.insertAt(pid, proc);

    // Report to scheduler, if requested
    if (readyToRun)
    {
        resume(proc);
    }

    // Assign parent, if any
    if (m_current != 0)
    {
        proc->setParent(m_current->getID());
    }

    return proc;
}

Process * ProcessManager::get(const ProcessID id)
{
    return m_procs.get(id);
}

void ProcessManager::remove(Process *proc, const uint exitStatus)
{
    if (proc == m_idle)
        m_idle = ZERO;

    if (proc == m_current)
        m_current = ZERO;

    // Notify processes which are waiting for this Process
    const Size size = m_procs.size();
    for (Size i = 0; i < size; i++)
    {
        if (m_procs[i] != ZERO &&
            m_procs[i]->getState() == Process::Waiting &&
            m_procs[i]->getWait() == proc->getID())
        {
            const Process::Result result = m_procs[i]->join(exitStatus);
            if (result != Process::Success)
            {
                FATAL("failed to join() PID " << m_procs[i]->getID() <<
                      ": result = " << (int) result);
            }

            const Result r = enqueueProcess(m_procs[i]);
            if (r != Success)
            {
                FATAL("failed to enqueue() PID " << m_procs[i]->getID() <<
                      ": result = " << (int) r);
            }
        }
    }

    // Unregister any interrupt events for this process
    unregisterInterruptNotify(proc);

    // Remove process from administration and schedule
    m_procs.remove(proc->getID());

    if (proc->getState() == Process::Ready)
    {
        const Result result = dequeueProcess(proc, true);
        if (result != Success)
        {
            FATAL("failed to dequeue PID " << proc->getID());
        }
    }

    const Size countRemoved = m_sleepTimerQueue.remove(proc);
    assert(countRemoved <= 1U);
    (void) countRemoved;

    // Free the process memory
    delete proc;
}

ProcessManager::Result ProcessManager::schedule()
{
    const Timer *timer = Kernel::instance()->getTimer();
    const Size sleepTimerCount = m_sleepTimerQueue.count();

    // Let the scheduler select a new process
    Process *proc = m_scheduler->select();

    // If no process ready, let us idle
    if (!proc)
        proc = m_idle;

    if (!proc)
    {
        FATAL("no process found to run!");
    }

    // Try to wakeup processes that are waiting for a timer to expire
    for (Size i = 0; i < sleepTimerCount; i++)
    {
        Process *p = m_sleepTimerQueue.pop();
        const Timer::Info & procTimer = p->getSleepTimer();

        if (timer->isExpired(procTimer))
        {
            const Result result = wakeup(p);
            if (result != Success)
            {
                FATAL("failed to wakeup PID " << p->getID());
            }
        }
        else
        {
            m_sleepTimerQueue.push(p);
        }
    }

    // Only execute if its a different process
    if (proc != m_current)
    {
        Process *previous = m_current;
        m_current = proc;
        proc->execute(previous);
    }

    return Success;
}

Process * ProcessManager::current()
{
    return m_current;
}

void ProcessManager::setIdle(Process *proc)
{
    const Result result = dequeueProcess(proc, true);
    if (result != Success)
    {
        FATAL("failed to dequeue PID " << proc->getID());
    }

    m_idle = proc;
}

ProcessManager::Result ProcessManager::wait(Process *proc)
{
    if (m_current->wait(proc->getID()) != Process::Success)
    {
        ERROR("process ID " << m_current->getID() << " failed to wait");
        return IOError;
    }

    return dequeueProcess(m_current);
}

ProcessManager::Result ProcessManager::stop(Process *proc)
{
    const Process::State state = proc->getState();
    const Process::Result result = proc->stop();
    if (result != Process::Success)
    {
        ERROR("failed to stop PID " << proc->getID() << ": result = " << (int) result);
        return IOError;
    }

    if (state == Process::Ready)
    {
        return dequeueProcess(proc);
    }
    else
    {
        return Success;
    }
}

ProcessManager::Result ProcessManager::resume(Process *proc)
{
    const Process::Result result = proc->resume();
    if (result != Process::Success)
    {
        ERROR("failed to resume PID " << proc->getID() << ": result = " << (int) result);
        return IOError;
    }

    return enqueueProcess(proc);
}

ProcessManager::Result ProcessManager::reset(Process *proc, const Address entry)
{
    if (proc == m_current)
    {
        ERROR("cannot reset current Process");
        return IOError;
    }

    proc->reset(entry);
    return Success;
}

ProcessManager::Result ProcessManager::sleep(const Timer::Info *timer, const bool ignoreWakeups)
{
    const Process::Result result = m_current->sleep(timer, ignoreWakeups);
    switch (result)
    {
        case Process::WakeupPending:
            return WakeupPending;

        case Process::Success: {
            const Result res = dequeueProcess(m_current);
            if (res != Success)
            {
                FATAL("failed to dequeue PID " << m_current->getID());
            }

            if (timer)
            {
                assert(!m_sleepTimerQueue.contains(m_current));
                m_sleepTimerQueue.push(m_current);
            }
            break;
        }

        default:
            ERROR("failed to sleep process ID " << m_current->getID() <<
                  ": result: " << (uint) result);
            return IOError;
    }

    return Success;
}

ProcessManager::Result ProcessManager::wakeup(Process *proc)
{
    const Process::Result result = proc->wakeup();

    switch (result)
    {
        case Process::WakeupPending:
            return Success;

        case Process::Success:
            return enqueueProcess(proc);

        default:
            ERROR("failed to wakeup process ID " << proc->getID() <<
                  ": result: " << (uint) result);
            return IOError;
    }
}

ProcessManager::Result ProcessManager::raiseEvent(Process *proc, const struct ProcessEvent *event)
{
    const Process::Result result = proc->raiseEvent(event);

    switch (result)
    {
        case Process::WakeupPending:
            return Success;

        case Process::Success:
            return enqueueProcess(proc);

        default:
            ERROR("failed to raise event in process ID " << proc->getID() <<
                  ": result: " << (uint) result);
            return IOError;
    }
}

ProcessManager::Result ProcessManager::registerInterruptNotify(Process *proc, const u32 vec)
{
    // Create List if necessary
    if (!m_interruptNotifyList[vec])
    {
        m_interruptNotifyList.insert(vec, new List<Process *>());
    }

    // Check for duplicates
    if (m_interruptNotifyList[vec]->contains(proc))
        return AlreadyExists;

    // Append the Process
    m_interruptNotifyList[vec]->append(proc);
    return Success;
}

ProcessManager::Result ProcessManager::unregisterInterruptNotify(Process *proc)
{
    // Remove the Process from all notify lists
    for (Size i = 0; i < m_interruptNotifyList.size(); i++)
    {
        List<Process *> *lst = m_interruptNotifyList[i];
        if (lst)
        {
            lst->remove(proc);
        }
    }

    return Success;
}

ProcessManager::Result ProcessManager::interruptNotify(const u32 vector)
{
    List<Process *> *lst = m_interruptNotifyList[vector];
    if (lst)
    {
        ProcessEvent event;
        event.type   = InterruptEvent;
        event.number = vector;

        for (ListIterator<Process *> i(lst); i.hasCurrent(); i++)
        {
            if (raiseEvent(i.current(), &event) != Success)
            {
                ERROR("failed to raise InterruptEvent for IRQ #" << vector <<
                      " on Process ID " << i.current()->getID());
                return IOError;
            }
        }
    }

    return Success;
}

ProcessManager::Result ProcessManager::enqueueProcess(Process *proc, const bool ignoreState)
{
    if (m_scheduler->enqueue(proc, ignoreState) != Scheduler::Success)
    {
        ERROR("process ID " << proc->getID() << " not added to Scheduler");
        return IOError;
    }

    const Size countRemoved = m_sleepTimerQueue.remove(proc);
    assert(countRemoved <= 1U);
    (void) countRemoved;

    return Success;
}

ProcessManager::Result ProcessManager::dequeueProcess(Process *proc, const bool ignoreState) const
{
    if (m_scheduler->dequeue(proc, ignoreState) != Scheduler::Success)
    {
        ERROR("process ID " << proc->getID() << " not removed from Scheduler");
        return IOError;
    }

    return Success;
}

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

#include "Scheduler.h"
#include "Kernel.h"
#include <List.h>
#include <ListIterator.h>
#include <Macros.h>

Scheduler::Scheduler()
    : currentProcess(ZERO), oldProcess(ZERO), idleProcess(ZERO)
{
    queuePtr.reset(&queue);
}

void Scheduler::executeNext()
{
    Process *next;

    /* Save the old process. */
    oldProcess = currentProcess ? currentProcess : ZERO;

    /* Process any pending wakeups. */
    for (ListIterator<Process> i(Process::getWakeups()); i.hasNext(); i++)
    {
        /* Only wakeup when it sleeps. */
        if (i.current()->getState() == Sleeping)
        {
            i.current()->setState(Ready);
            Process::getWakeups()->remove(i.current());
        }
    }
    /* Find the next ready Process in line. */
    if (!(next = findNextReady()))
    {
        next = idleProcess;
    }
    /* Update current. */
    currentProcess = next;
    
    /* Run it. */
    if (currentProcess != oldProcess)
    {
        currentProcess->execute();
    }
}

void Scheduler::executeAttempt(Process *p)
{
    /* Don't switch if it's the current process. */
    if (p == currentProcess) return;
    
    /* Wakeup process if needed. */
    if (p->getState() == Sleeping)
    {
        p->setState(Ready);
        Process::getWakeups()->remove(p);
    }
    /* Update pointers. */
    oldProcess = currentProcess;
    currentProcess = p;
    
    /* Execute it. */
    p->execute();
}

void Scheduler::enqueue(Process *proc)
{
    queue.insertTail(proc);
}

void Scheduler::dequeue(Process *proc)
{
    queue.remove(proc);
    queuePtr.reset(&queue);

    if (currentProcess == proc)
        currentProcess = ZERO;

    if (oldProcess == proc)
        oldProcess = ZERO;
}

Process * Scheduler::findNextReady()
{
    Process *ret = ZERO, *saved = ZERO;

    while (!ret)
    {
        /* Search the whole list. */
        while (queuePtr.hasNext())
        {
            /* Save the current. */
            if (!saved)
                saved = queuePtr.current();

            /* We walked the whole list already. */
            else if (queuePtr.current() == saved)
            {
                return ret;
            }
            /* Is this process ready? */
            if (queuePtr.current()->getState() == Ready)
            {
                ret = queuePtr.current();
                queuePtr++;
                return ret;
            }
            /* Try the next. */
            queuePtr++;
        }
        /* Start again at the front. */
        queuePtr.reset(&queue);
    }
    return ret;
}

Process * Scheduler::current()
{
    return currentProcess;
}

Process * Scheduler::old()
{
    return oldProcess;
}

void Scheduler::setIdle(Process *p)
{
    idleProcess = p;
    queue.remove(p);
}

INITOBJ(Scheduler, scheduler, SCHEDULER)

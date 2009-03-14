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

#include <arch/Scheduler.h>
#include <arch/Init.h>
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
    ArchProcess *next;

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

void Scheduler::executeAttempt(ArchProcess *p)
{
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

void Scheduler::enqueue(ArchProcess *proc)
{
    queue.insertTail(proc);
}

void Scheduler::dequeue(ArchProcess *proc)
{
    queue.remove(proc);
    queuePtr.reset(&queue);

    if (currentProcess == proc)
	currentProcess = ZERO;

    if (oldProcess == proc)
	oldProcess = ZERO;
}

ArchProcess * Scheduler::findNextReady()
{
    ArchProcess *ret = ZERO, *saved = ZERO;

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

INITOBJ(Scheduler, scheduler, SCHEDULER)

/**
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
#include <Queue.h>
#include <Macros.h>

Scheduler::Scheduler() : currentProcess(ZERO), oldProcess(ZERO)
{
}

void Scheduler::executeNext()
{
    ArchProcess *next;

    /* Push current process back on the queue. */
    if (currentProcess)
    {
	queue.enqueue(currentProcess);
	oldProcess = currentProcess;
    }
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
    /* Grab first process. */
    next = queue.dequeue();
    
    /* Find the next ready Process in line. */
    while (next->getState() != Ready)
    {
	queue.enqueue(next);
	next = queue.dequeue();
    }
    /* Update current. */
    currentProcess = next;
    
    /* Run it. */
    if (currentProcess != oldProcess)
    {
	currentProcess->execute();
    }
}

void Scheduler::enqueue(ArchProcess *proc)
{
    queue.enqueue(proc);
}

void Scheduler::dequeue(ArchProcess *proc)
{
    queue.dequeue(proc);
}

INITOBJ(Scheduler, scheduler, SCHEDULER)

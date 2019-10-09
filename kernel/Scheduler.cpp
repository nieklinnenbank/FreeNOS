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

#include <Log.h>
#include "Kernel.h"
#include "Scheduler.h"

Scheduler::Scheduler()
{
    DEBUG("");

    m_index = 0;
}

Scheduler::Result Scheduler::enqueue(Process *proc, bool ignoreState)
{
    if (proc->getState() != Process::Ready && !ignoreState)
    {
        ERROR("process ID " << proc->getID() << " not in Ready state");
        return InvalidArgument;
    }

    return Success;
}

Scheduler::Result Scheduler::dequeue(Process *proc, bool ignoreState)
{
    if (proc->getState() == Process::Ready && !ignoreState)
    {
        ERROR("process ID " << proc->getID() << " is in Ready state");
        return InvalidArgument;
    }

    return Success;
}

Process * Scheduler::select(Vector<Process *> *procs, Process *idle)
{
    Size size = procs->size();

    for (Size i = 0; i < size; i++)
    {
        // Increment process index
        m_index = (m_index + 1) % size;

        // Pick the process
        Process *p = procs->at(m_index);

        if (p)
        {
            Process::State state = p->getState();

            // Wakeup the process if its sleeptimer expired
            if (state == Process::Sleeping && Kernel::instance->getTimer()->isExpired(p->getSleepTimer()))
                p->wakeup();

            // Select this process if it wants to run
            if (p != idle && state == Process::Ready)
            {
                return p;
            }
        }
    }
    return (Process *) NULL;
}

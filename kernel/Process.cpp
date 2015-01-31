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
#include <Array.h>
#include <Types.h>
#include <ProcessID.h>
#include <ListIterator.h>
#include <Arch/Interrupt.h>
#include <Types.h>
#include <Array.h>
#include <List.h>
#include "Scheduler.h"

Array<Process> Process::procs(MAX_PROCS);
List<Process> Process::wakeups;

Process::Process(Address addr) : status(Stopped)
{
    pid = procs.insert(this);
}
    
Process::~Process()
{
    wakeups.remove(this);
    procs.remove(pid);
}

ProcessID Process::getID()
{
    return pid;
}
        
ProcessState Process::getState()
{
    return status;
}

void Process::setState(ProcessState st)
{
    status = st;
}

void Process::wakeup()
{
    ulong t = irq_disable();
    wakeups.insertTail(this);
    irq_restore(t);
}

List<Process> * Process::getWakeups()
{
    return &wakeups;
}

List<UserMessage> * Process::getMessages()
{
    return &messages;
}

Array<Process> * Process::getProcessTable()
{
    return &procs;
}

Process * Process::byID(ProcessID id)
{
    if (id == SELF && scheduler->current())
        return scheduler->current();
    else
        return procs[id];
}

bool Process::operator == (Process *p)
{
    return this->pid == p->pid;
}

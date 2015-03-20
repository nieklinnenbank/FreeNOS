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
#include <FreeNOS/API.h>
#include <FreeNOS/Kernel.h>
#warning Do not depend on IntelProcess for ProcessCtl()
#include <intel/IntelProcess.h>
#include "ProcessCtl.h"

void interruptNotify(CPUState *st, Process *p)
{
    ProcessManager *procs = Kernel::instance->getProcessManager();

    p->getMessages()->prepend(new UserMessage(new InterruptMessage(IRQ_REG(st)),
						 sizeof(InterruptMessage)));
    p->setState(Process::Ready);
}

Error ProcessCtlHandler(ProcessID procID, ProcessOperation action, Address addr)
{
    IntelProcess *proc = ZERO;
    Memory *memory = Kernel::instance->getMemory();
    ProcessInfo *info = (ProcessInfo *) addr;
    ProcessManager *procs = Kernel::instance->getProcessManager();

    DEBUG("#" << procs->current()->getID() << " " << action << " -> " << procID << " (" << addr << ")");

    /* Verify memory address. */
    if (action == InfoPID)
    {
	if (!memory->access(procs->current(), addr, sizeof(ProcessInfo)))
	{
	    return EFAULT;
	}
    }
    /* Does the target process exist? */
    if(action != GetPID && action != Spawn)
    {
        if (procID == SELF)
            proc = (IntelProcess *) procs->current();
        else if (!(proc = (IntelProcess *)procs->get(procID)))
	    return ESRCH;
    }
    /* Handle request. */
    switch (action)
    {
	case Spawn:
	    proc = (IntelProcess *) procs->create(addr);
	    return proc->getID();
	
	case KillPID:
            procs->remove(proc);
	    break;

	case GetPID:
	    return procs->current()->getID();

	case Schedule:
            procs->schedule();
	    break;

	case Resume:
	    proc->setState(Process::Ready);
	    break;
	
	case AllowIO:
            return EINVAL;
	    //proc->IOPort(addr, true);
	    //break;
	
	case WatchIRQ:
	    Kernel::instance->hookInterrupt(IRQ(addr),
		(InterruptHandler *)interruptNotify, (ulong)proc);
	    Kernel::instance->enableIRQ(addr, true);
	    break;
	
	case InfoPID:
	    info->id    = proc->getID();
	    info->state = proc->getState();
	    info->stack = proc->getStack();
	    info->pageDirectory = proc->getPageDirectory();
	    break;
	    
	case SetStack:
	    proc->setStack(addr);
	    break;
    }
    return 0;
}

Log & operator << (Log &log, ProcessOperation op)
{
    switch (op)
    {
        case Spawn:     log.write("Spawn"); break;
        case KillPID:   log.write("KillPID"); break;
        case GetPID:    log.write("GetPID"); break;
        case Schedule:  log.write("Schedule"); break;
        case Resume:    log.write("Resume"); break;
        case AllowIO:   log.write("AllowIO"); break;
        case WatchIRQ:  log.write("WatchIRQ"); break;
        case InfoPID:   log.write("InfoPID"); break;
        case SetStack:  log.write("SetStack"); break;
    }
    return log;
}

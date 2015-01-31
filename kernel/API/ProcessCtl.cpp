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

#include <API/ProcessCtl.h>
#include <API/IPCMessage.h>
#include <Arch/Kernel.h>
#include <Arch/Memory.h>
#include <Error.h>

void interruptNotify(CPUState *st, Process *p)
{
    p->getMessages()->insertHead(new UserMessage(new InterruptMessage(IRQ_REG(st)),
						 sizeof(InterruptMessage)));
    p->wakeup();
}

int ProcessCtlHandler(ProcessID procID, ProcessOperation action, Address addr)
{
    X86Process *proc = ZERO;
    ProcessInfo *info = (ProcessInfo *) addr;

    /* Verify memory address. */
    if (action == InfoPID)
    {
	if (!memory->access(scheduler->current(), addr, sizeof(ProcessInfo)))
	{
	    return EFAULT;
	}
    }
    /* Does the target process exist? */
    if(action != GetPID && action != Spawn && !(proc = (X86Process *) Process::byID(procID)))
    {
	return ESRCH;
    }
    /* Handle request. */
    switch (action)
    {
	case Spawn:
	    proc = (X86Process *) kernel->createProcess(addr);
	    return proc->getID();
	
	case KillPID:
	    delete proc;
	    break;

	case GetPID:
	    return scheduler->current()->getID();

	case Schedule:
	    scheduler->executeNext();
	    break;

	case Resume:
	    proc->setState(Ready);
	    scheduler->enqueue(proc);
	    break;
	
	case AllowIO:
	    proc->IOPort(addr, true);
	    break;
	
	case WatchIRQ:
	    kernel->hookInterrupt(IRQ(addr),
		(InterruptHandler *)interruptNotify, (ulong)proc);
	    kernel->enableIRQ(addr, true);
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

INITAPI(PROCESSCTL, ProcessCtlHandler)

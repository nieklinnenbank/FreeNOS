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

#include <api/ProcessCtl.h>
#include <api/IPCMessage.h>
#include <arch/Init.h>
#include <arch/Kernel.h>
#include <Error.h>

void interruptNotify(CPUState *st, ArchProcess *p)
{
    UserMessage *m = (UserMessage *) new InterruptMessage(IRQ_REG(st));
    p->getMessages()->insertHead(m);
    p->wakeup();
}

int ProcessCtlHandler(ProcessID procID, ProcessAction action, Address addr)
{
    ArchProcess *proc = ZERO;
    ProcessInfo *info = (ProcessInfo *) addr;

    /* Verify memory address. */
    if (action == Info)
    {
	if (!memory->access(scheduler->current(), addr, sizeof(ProcessInfo)))
	{
	    return EFAULT;
	}
    }
    /* Does the target process exist? */
    else if(action != GetPID && !(proc = Process::byID(procID)))
    {
	return ENOSUCH;
    }
    /* Handle request. */
    switch (action)
    {
	case Spawn:
	    return EINVALID;
	
	case Kill:
	    delete proc;
	    break;

	case GetPID:
	    return scheduler->current()->getID();
	
	case AllowIO:
	    proc->IOPort(addr, true);
	    break;
	
	case WatchIRQ:
	    kernel->hookInterrupt(IRQ(addr),
		(InterruptHandler *)interruptNotify, (ulong)proc);
	    kernel->enableIRQ(addr, true);
	    break;
	
	case Info:
	    
	    /* Find next. */
	    while (procID < MAX_PROCS && !(proc = Process::byID(procID)))
		procID++;
	    
	    if (!proc)
		return ENOSUCH;
	
	    info->id    = proc->getID();
	    info->state = proc->getState();
    }
    return 0;
}

INITAPI(PROCESSCTL, ProcessCtlHandler)

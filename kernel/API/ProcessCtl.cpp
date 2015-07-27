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

#include <FreeNOS/API.h>
#include <FreeNOS/Kernel.h>
#include <FreeNOS/Config.h>
#include <FreeNOS/Process.h>
#include <Log.h>
#include "ProcessCtl.h"

void interruptNotify(CPUState *st, Process *p)
{
    InterruptMessage *msg = new InterruptMessage;
    msg->from   = KERNEL_PID;
    msg->type   = IRQType;
    msg->vector = IRQ_REG(st);

    p->getMessages()->prepend(msg);
    p->setState(Process::Ready);
}

Error ProcessCtlHandler(ProcessID procID, ProcessOperation action, Address addr)
{
    Process *proc = ZERO;
    ProcessInfo *info = (ProcessInfo *) addr;
    ProcessManager *procs = Kernel::instance->getProcessManager();

    DEBUG("#" << procs->current()->getID() << " " << action << " -> " << procID << " (" << addr << ")");

    // TODO: Verify memory address

    // Does the target process exist?
    if(action != GetPID && action != Spawn)
    {
        if (procID == SELF)
            proc = procs->current();
        else if (!(proc = procs->get(procID)))
            return API::NotFound;
    }
    // Handle request
    switch (action)
    {
    case Spawn:
        proc = procs->create(addr);
        proc->setParent(procs->current()->getID());
        return proc->getID();
    
    case KillPID:
        procs->remove(proc, addr); // Addr contains the exit status
        procs->schedule();
        break;

    case GetPID:
        return procs->current()->getID();

    case GetParent:
        return procs->current()->getParent();

    case Schedule:
        procs->schedule();
        break;

    case Resume:
        proc->setState(Process::Ready);
        break;

    case WatchIRQ:
        Kernel::instance->hookInterrupt(IRQ(addr), (InterruptHandler *)interruptNotify, (ulong)proc);
        break;

    case EnableIRQ:
        Kernel::instance->enableIRQ(addr, true);
        break;

    case DisableIRQ:
        Kernel::instance->enableIRQ(addr, false);
        break;
    
    case InfoPID:
        info->id    = proc->getID();
        info->state = proc->getState();
        info->userStack     = proc->getUserStack();
        info->kernelStack   = proc->getKernelStack();
        info->pageDirectory = proc->getPageDirectory();
        info->parent = proc->getParent();
        break;

    case WaitPID:
        procs->current()->setWait(proc->getID());
        procs->current()->setState(Process::Waiting);
        procs->schedule();
        return procs->current()->getWait(); // contains the exit status of the other process
        
    case SetStack:
        proc->setUserStack(addr);
        break;
    }
    return API::Success;
}

Log & operator << (Log &log, ProcessOperation op)
{
    switch (op)
    {
        case Spawn:     log.write("Spawn"); break;
        case KillPID:   log.write("KillPID"); break;
        case GetPID:    log.write("GetPID"); break;
        case GetParent: log.write("GetParent"); break;
        case Schedule:  log.write("Schedule"); break;
        case Resume:    log.write("Resume"); break;
        case WatchIRQ:  log.write("WatchIRQ"); break;
        case EnableIRQ: log.write("EnableIRQ"); break;
        case InfoPID:   log.write("InfoPID"); break;
        case WaitPID:   log.write("WaitPID"); break;
        case SetStack:  log.write("SetStack"); break;
        default:        log.write("???"); break;
    }
    return log;
}

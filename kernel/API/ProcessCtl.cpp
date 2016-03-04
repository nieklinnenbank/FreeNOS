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

#include <FreeNOS/System.h>
#include <FreeNOS/Kernel.h>
#include <FreeNOS/Config.h>
#include <FreeNOS/Process.h>
#include <FreeNOS/ProcessEvent.h>
#include <Log.h>
#include "ProcessCtl.h"

void interruptNotify(CPUState *st, Process *p)
{
    ProcessEvent event;

    event.type   = InterruptEvent;
    event.number = IRQ_REG(st);

    p->raiseEvent(&event);
}

Error ProcessCtlHandler(ProcessID procID,
                        ProcessOperation action,
                        Address addr,
                        Address output)
{
    Process *proc = ZERO;
    ProcessInfo *info = (ProcessInfo *) addr;
    ProcessManager *procs = Kernel::instance->getProcessManager();
    Timer *timer;
    Arch::MemoryMap map;

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
        proc = procs->create(addr, map);
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
        // increment wakeup counter and set process ready
        proc->wakeup();
        break;

    case WatchIRQ:
        Kernel::instance->hookIntVector(IRQ(addr), (InterruptHandler *)interruptNotify, (ulong)proc);
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

    case InfoTimer:
        if (!(timer = Kernel::instance->getTimer()))
            return API::NotFound;
        
        timer->getCurrent((Timer::Info *) addr); // TODO: check access...
        break;

    /*    
    case WaitTimer:
        procs->current()->setSleepTimer((const Timer::Info *)addr); // TODO: check access...
        procs->current()->setState(Process::Sleeping);
        procs->schedule();
        // TODO: set a Timer::Info field for the process. Then when scheduling, the process
        // will only be allowed to run until after the Timer::Info time has arrived (for sleep).
        break;
     */

    case EnterSleep:
        // only sleeps the process if no pending wakeups
        if (procs->current()->sleep((Timer::Info *)addr) == Process::Success)
            procs->schedule();

        if (output && ((timer = Kernel::instance->getTimer())))
            timer->getCurrent((Timer::Info *) output); // TODO: check access...
        break;

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
        case Spawn:     log.append("Spawn"); break;
        case KillPID:   log.append("KillPID"); break;
        case GetPID:    log.append("GetPID"); break;
        case GetParent: log.append("GetParent"); break;
        case WatchIRQ:  log.append("WatchIRQ"); break;
        case EnableIRQ: log.append("EnableIRQ"); break;
        case DisableIRQ:log.append("DisableIRQ"); break;
        case InfoPID:   log.append("InfoPID"); break;
        case WaitPID:   log.append("WaitPID"); break;
        case InfoTimer: log.append("InfoTimer"); break;
        case EnterSleep: log.append("EnterSleep"); break;
        case Schedule:  log.append("Schedule"); break;
        case Resume:    log.append("Resume"); break;
        case SetStack:  log.append("SetStack"); break;
        default:        log.append("???"); break;
    }
    return log;
}

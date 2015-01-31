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

#include <FreeNOS/Scheduler.h>
#include <Types.h>
#include <ListIterator.h>
#include <MemoryBlock.h>
#include "CPU.h"
#include "Process.h"
#include "Memory.h"

X86Process::X86Process(Address entry) : Process(entry)
{
    Address *pageDir, *tmpStack, *ioMap;
    CPUState *regs;

    /* Allocate page directory. */
    pageDirAddr = memory->allocatePhysical(PAGESIZE);
    pageDir     = (Address *) memory->mapVirtual(pageDirAddr);

    /* One page for the I/O bitmap. */
    ioMapAddr   = memory->allocatePhysical(PAGESIZE);
    ioMap       = (Address *) memory->mapVirtual(ioMapAddr);

    /* Clear them first. */
    MemoryBlock::set(pageDir,   0, PAGESIZE);
    MemoryBlock::set(ioMap,  0xff, PAGESIZE);

    /* Setup mappings. */
    pageDir[0] = kernelPageDir[0];
    pageDir[DIRENTRY(PAGETABFROM) ] = pageDirAddr | PAGE_PRESENT | PAGE_RW;
    pageDir[DIRENTRY(PAGEUSERFROM)] = pageDirAddr | PAGE_PRESENT | PAGE_RW;

    /* Point stacks. */
    stackAddr       = 0xc0000000 - MEMALIGN;
    kernelStackAddr = 0xd0000000 - MEMALIGN;

    /* Allocate stacks. */
    for (int i = 0; i < 4; i++)
    {
        memory->allocateVirtual(this, stackAddr - (i * PAGESIZE),
                                PAGE_PRESENT | PAGE_USER | PAGE_RW);
        memory->allocateVirtual(this, kernelStackAddr - (i * PAGESIZE),
                                PAGE_PRESENT | PAGE_RW);
    }
    /* Map kernel stack. */
    tmpStack = (Address *) memory->mapVirtual(
				memory->lookupVirtual(this, kernelStackAddr) & PAGEMASK);
	
    /* Setup initial registers. */
    regs = (CPUState *) (((u32)tmpStack) + PAGESIZE - sizeof(CPUState));
    MemoryBlock::set(regs, 0, sizeof(CPUState));
    regs->ss0    = KERNEL_DS_SEL;
    regs->fs     = USER_DS_SEL;
    regs->gs     = USER_DS_SEL;
    regs->es     = USER_DS_SEL;
    regs->ds     = USER_DS_SEL;
    regs->ebp    = stackAddr;
    regs->esp0   = kernelStackAddr;
    regs->eip    = entry;
    regs->cs     = USER_CS_SEL;
    regs->eflags = 0x202;
    regs->esp3   = stackAddr;
    regs->ss3    = USER_DS_SEL;
    
    /* Repoint our stack. */
    stackAddr = kernelStackAddr - sizeof(CPUState) + MEMALIGN;

    /* Release temporary mappings. */
    memory->mapVirtual((Address) 0, (Address) pageDir, 0);
    memory->mapVirtual((Address) 0, (Address) tmpStack, 0);
    memory->mapVirtual((Address) 0, (Address) ioMap, 0);
}

X86Process::~X86Process()
{
    /* Remove ourselves from the scheduler. */	
    scheduler->dequeue(this);

    /* Mark all our pages free. */
    memory->releaseAll(this);
}

void X86Process::IOPort(u16 port, bool enabled)
{
    Address tmp = memory->mapVirtual(ioMapAddr);
    kernelTss.setPort(port, enabled, (u8 *) tmp);
    memory->mapVirtual((Address) 0, tmp, 0);
}

void X86Process::execute()
{
    /* Refresh I/O bitmap. */
    memory->mapVirtual(ioMapAddr, (Address) &kernelioBitMap);

    /* Perform a context switch. */
    contextSwitch( scheduler->old() ? &((X86Process *)scheduler->old())->stackAddr
				    :  ZERO,
		   pageDirAddr,
		   stackAddr,
		  &kernelTss,
		   kernelStackAddr);
}

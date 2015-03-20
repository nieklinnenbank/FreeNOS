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

#include <Types.h>
#include <MemoryBlock.h>
#include <FreeNOS/Kernel.h>
#include <FreeNOS/ProcessScheduler.h>
#include "IntelCPU.h"
#include "IntelMemory.h"
#include "IntelProcess.h"

IntelProcess::IntelProcess(ProcessID id, Address entry)
    : Process(id, entry)
{
    Address *pageDir, *tmpStack, *ioMap;
    IntelMemory *memory = (IntelMemory *) Kernel::instance->getMemory();
    CPUState *regs;

    /* Allocate page directory. */
    pageDirAddr = memory->allocatePhysical(PAGESIZE);
    pageDir     = (Address *) memory->map(pageDirAddr);

    /* One page for the I/O bitmap. */
    ioMapAddr   = memory->allocatePhysical(PAGESIZE);
    ioMap       = (Address *) memory->map(ioMapAddr);

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
        memory->allocate(this, stackAddr - (i * PAGESIZE),
                                Memory::Present | Memory::User | Memory::Writable | Memory::Readable);
        memory->allocate(this, kernelStackAddr - (i * PAGESIZE),
                                Memory::Present | Memory::Readable | Memory::Writable);
    }
    /* Map kernel stack. */
    tmpStack = (Address *) memory->map(
				memory->lookup(this, kernelStackAddr) & PAGEMASK);
	
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
    memory->map((Address) 0, (Address) pageDir, Memory::None);
    memory->map((Address) 0, (Address) tmpStack, Memory::None);
    memory->map((Address) 0, (Address) ioMap, Memory::None);
}

IntelProcess::~IntelProcess()
{
    Memory *memory = Kernel::instance->getMemory();

    /* Mark all our pages free. */
    memory->release(this);
}

void IntelProcess::IOPort(u16 port, bool enable)
{
    Memory *memory = Kernel::instance->getMemory();

    Address tmp = memory->map(ioMapAddr);
    kernelTss.setPort(port, enable, (u8 *) tmp);
    memory->map((Address) 0, tmp, Memory::None);
}

void IntelProcess::execute()
{
    Memory *memory = Kernel::instance->getMemory();
    ProcessManager *procs = Kernel::instance->getProcessManager();
    IntelProcess *old = (IntelProcess *) procs->previous();

    /* Refresh I/O bitmap. */
    memory->map(ioMapAddr, (Address) &kernelioBitMap);

#warning FIX this. Context switching should not depend on a previous process. Split up into two functions: saveContext() restoreContext()

    /* Perform a context switch. */
    contextSwitch( old ? &old->stackAddr : ZERO,
		   pageDirAddr,
		   stackAddr,
		  &kernelTss,
		   kernelStackAddr);
}

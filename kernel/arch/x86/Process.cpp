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

#include <arch/Process.h>
#include <arch/Memory.h>
#include <arch/CPU.h>
#include <arch/Scheduler.h>
#include <Types.h>
#include <string.h>
#include <ListIterator.h>

x86Process::x86Process(Address entry) : Process(entry)
{
    Address *pageDir, miscTabAddr, *miscTab, *tmpStack, *ioMap;
    CPUState *regs;

    /* Allocate page directory. */
    pageDirAddr = memory->allocatePhysical(PAGESIZE);
    pageDir     = (Address *) memory->mapVirtual(pageDirAddr);

    /* Allocate a page, for mapping temporary kernel pages. */
    miscTabAddr = memory->allocatePhysical(PAGESIZE);
    miscTab     = (Address *) memory->mapVirtual(miscTabAddr);

    /* One page for the I/O bitmap. */
    ioMapAddr   = memory->allocatePhysical(PAGESIZE);
    ioMap       = (Address *) memory->mapVirtual(ioMapAddr);

    /* Clear them first. */
    memset(pageDir,   0, PAGESIZE);
    memset(miscTab,   0, PAGESIZE);
    memset(ioMap,  0xff, PAGESIZE);

    /* Setup mappings. */
    pageDir[0] = kernelPageDir[0];
    pageDir[DIRENTRY(PAGETABFROM) ] = pageDirAddr | PAGE_PRESENT | PAGE_RW;
    pageDir[DIRENTRY(PAGEMISCFROM)] = miscTabAddr | PAGE_PRESENT | PAGE_RW;

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
				memory->virtualToPhysical(this, kernelStackAddr));
	
    /* Setup initial registers. */
    regs = (CPUState *) (((u32)tmpStack) + PAGESIZE - sizeof(CPUState));
    memset(regs, 0, sizeof(CPUState));
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
    memory->mapVirtual((Address) 0, (Address) miscTab, 0);
    memory->mapVirtual((Address) 0, (Address) tmpStack, 0);
    memory->mapVirtual((Address) 0, (Address) ioMap, 0);
}

x86Process::~x86Process()
{
    Address *pd = PAGEDIRADDR;
    Address *pt = 0;
    
    /* Mark all our physical pages free. */
    for (Size i = 0; i < 1024; i++)
    {
	if (pd[i] & PAGE_PRESENT)
	{
	    pt = PAGETABADDR(i * PAGESIZE * 1024);
	    
	    for (Size i = 0; i < 1024; i++)
	    {
		if (pt[i] & PAGE_PRESENT && !(pt[i] & PAGE_PINNED))
		{
		    memory->releasePhysical(pt[i]);
		}
	    }
	}
    }
    /* Clear current/old pointer, if it is us. */
    if (scheduler->current() == this)
	scheduler->setCurrent(ZERO);
    if (scheduler->old() == this)
	scheduler->setOld(ZERO);

    /* Remove ourselves from the scheduler. */	
    scheduler->dequeue(this);
}

void x86Process::IOPort(u16 port, bool enabled)
{
    Address tmp = memory->mapVirtual(ioMapAddr);
    kernelTss.setPort(port, enabled, (u8 *) tmp);
    memory->mapVirtual((Address) 0, tmp, 0);
}

void x86Process::execute()
{
    /* Refresh I/O bitmap. */
    memory->mapVirtual(ioMapAddr, (Address) &kernelioBitMap);

    /* Saves the current process state. */
    if (scheduler->old())
    {
	asm volatile
	(
	    /* Setup stack for IRETD later on. */
	    "pushf\n"
	    "push %%cs\n"
	    "push $resume\n"

	    /* Save registers on the current stack (CPUState). */
	    "pushl $0\n"
	    "pushl $0\n"
	    "pusha\n"
	    "pushl %%ss\n"
	    "pushl %%ds\n"
	    "pushl %%es\n"
	    "pushl %%fs\n"
	    "pushl %%gs\n"

	    /* Save current stack pointer. */
	    "pushl %%esp\n"
	    "popl %%eax\n"
	    "movl %%eax, %0\n" : "=r"(scheduler->old()->stackAddr)
	);
    }
    /* Reload page directory, stack and TSS. */
    asm volatile
    (
	"movl %%eax, %%cr3\n"
	"movl %%ebx, %%esp\n"
	"movl %%edx, 4(%%ecx)\n"
	"movl $0x10, 8(%%ecx)\n"
	:: "a"(pageDirAddr),
	   "b"(stackAddr),
	   "c"(&kernelTss),
	   "d"(kernelStackAddr)
    );
    /* Restores the old process state. */
    asm volatile
    (
	/* Restore registers. */
	"popl %gs\n"
	"popl %fs\n"
	"popl %es\n"
	"popl %ds\n"
	"popl %ss\n"
	"popa\n"
	"addl $8, %esp\n"
	
	/* Resume execution. */
	"iret\n"
	"\n"
	"resume:\n"
    );
}

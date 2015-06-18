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

#include <FreeNOS/System.h>
#include <MemoryBlock.h>
#include <BitAllocator.h>
#include "IntelProcess.h"

// TODO: this should not be done here. Try to use libarch's Memory class.
#define PAGE_PRESENT    1
#define PAGE_USER       4

IntelProcess::IntelProcess(ProcessID id, Address entry, bool privileged)
    : Process(id, entry, privileged)
{
    Address stack, stackBase, *pageDir;
    BitAllocator *memory = Kernel::instance->getMemory();
    CPUState *regs;
    Arch::Memory local(0, memory);
    Arch::Memory::Range range;
    Size dirSize = PAGESIZE;
    u16 dataSel = privileged ? KERNEL_DS_SEL : USER_DS_SEL;
    u16 codeSel = privileged ? KERNEL_CS_SEL : USER_CS_SEL;

    // Allocate and map page directory
    memory->allocate(&dirSize, &m_pageDirectory);

    pageDir = (Address *) local.findFree(PAGESIZE, Memory::KernelPrivate);
    local.map(m_pageDirectory, (Address) pageDir,
              Arch::Memory::Present |
              Arch::Memory::Readable |
              Arch::Memory::Writable);

    // Initialize page directory
    for (Size i = 0; i < PAGEDIR_MAX; i++)
        pageDir[i] = 0;


    pageDir[0] = kernelPageDir[0];
    // TODO: this should not be done here. Try to use libarch's Memory class.
    pageDir[DIRENTRY(PAGEDIR_LOCAL) ] = m_pageDirectory | PAGE_PRESENT | PAGE_USER;
    local.unmap((Address)pageDir);

    // Obtain memory mappings
    Arch::Memory mem(m_pageDirectory, memory);

    // User stack.
    range.phys   = 0;
    range.virt   = mem.range(Memory::UserStack).virt;
    range.size   = mem.range(Memory::UserStack).size;
    range.access = Arch::Memory::Present |
                   Arch::Memory::User |
                   Arch::Memory::Readable |
                   Arch::Memory::Writable;
    mem.mapRange(&range);
    setUserStack(range.virt + range.size - MEMALIGN);

    // Kernel stack.
    range.phys   = 0;
    range.virt   = mem.range(Memory::KernelStack).virt;
    range.size   = mem.range(Memory::KernelStack).size;
    range.access = Arch::Memory::Present |
                   Arch::Memory::Writable;
    mem.mapRange(&range);
    setKernelStack(range.virt + range.size - sizeof(CPUState)
                                           - sizeof(IRQRegs0)
                                           - sizeof(CPURegs));

    // Map kernel stack
    range.virt = local.findFree(range.size, Memory::KernelPrivate);
    stack      = range.virt;
    local.mapRange(&range);
    stackBase  = stack + range.size;

    // loadCoreState: struct CPUState
    regs = (CPUState *) stackBase - 1;
    MemoryBlock::set(regs, 0, sizeof(CPUState));
    regs->seg.ss0    = KERNEL_DS_SEL;
    regs->seg.fs     = dataSel;
    regs->seg.gs     = dataSel;
    regs->seg.es     = dataSel;
    regs->seg.ds     = dataSel;
    regs->regs.ebp   = m_userStack;
    regs->regs.esp0  = m_kernelStack;
    regs->irq.eip    = entry;
    regs->irq.cs     = codeSel;
    regs->irq.eflags = INTEL_EFLAGS_DEFAULT |
                       INTEL_EFLAGS_IRQ;
    regs->irq.esp3   = m_userStack;
    regs->irq.ss3    = dataSel;

    // restoreState: iret
    IRQRegs0 *irq = (IRQRegs0 *) regs - 1;
    irq->eip = (Address) loadCoreState;
    irq->cs  = KERNEL_CS_SEL;
    irq->eflags = INTEL_EFLAGS_DEFAULT;

    // restoreState: popa
    CPURegs *pusha = (CPURegs *) irq - 1;
    MemoryBlock::set(pusha, 0, sizeof(CPURegs));
    pusha->ebp  = m_kernelStack - sizeof(CPURegs);
    pusha->esp0 = pusha->ebp;

    local.unmapRange(&range);
}

IntelProcess::~IntelProcess()
{
    Arch::Memory mem(getPageDirectory(), Kernel::instance->getMemory());

    // Release regions
    mem.releaseRegion(Memory::KernelStack);
    mem.releaseRegion(Memory::UserData);
    mem.releaseRegion(Memory::UserHeap);
    mem.releaseRegion(Memory::UserStack);
    mem.releaseRegion(Memory::UserPrivate);
}

void IntelProcess::execute(Process *previous)
{
    IntelProcess *p = (IntelProcess *) previous;

    switchCoreState( p ? &p->m_kernelStack : ZERO,
                     m_pageDirectory,
                     m_kernelStack );
}

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

#include <FreeNOS/API.h>
#include <FreeNOS/ProcessManager.h>
#include <FreeNOS/System.h>
#include <Macros.h>
#include <List.h>
#include <ListIterator.h>
#include <SplitAllocator.h>
#include <Vector.h>
#include <MemoryBlock.h>
#include <String.h>
#include <BootImage.h>
#include <intel/IntelMap.h>
#include "IntelKernel.h"
#include "IntelBoot.h"

extern C void executeInterrupt(CPUState state)
{
    Kernel::instance->executeInterrupt(state.vector, &state);
}

IntelKernel::IntelKernel(Memory::Range kernel, Memory::Range memory)
    : Kernel(kernel, memory)
{
    IntelMap map;

    /* ICW1: Initialize PIC's (Edge triggered, Cascade) */
    IO::outb(PIC1_CMD, 0x11);
    IO::outb(PIC2_CMD, 0x11);
    
    /* ICW2: Remap IRQ's to interrupts 32-47. */
    IO::outb(PIC1_DATA, PIC_IRQ_BASE);
    IO::outb(PIC2_DATA, PIC_IRQ_BASE + 8);

    /* ICW3: PIC2 is connected to PIC1 via IRQ2. */
    IO::outb(PIC1_DATA, 0x04);
    IO::outb(PIC2_DATA, 0x02);

    /* ICW4: 8086 mode, fully nested, not buffered, no implicit EOI. */
    IO::outb(PIC1_DATA, 0x01);
    IO::outb(PIC2_DATA, 0x01);

    /* OCW1: Disable all IRQ's for now. */
    IO::outb(PIC1_DATA, 0xff);
    IO::outb(PIC2_DATA, 0xff);

    /* Let the i8253 timer run continuously (square wave). */
    IO::outb(PIT_CMD, 0x36);
    IO::outb(PIT_CHAN0, PIT_DIVISOR & 0xff);
    IO::outb(PIT_CHAN0, PIT_DIVISOR >> 8);
    
    /* Make sure to enable PIC2 and the i8253. */
    enableIRQ(2, true);
    enableIRQ(0, true);

    // Install interruptRun() callback
    interruptRun = ::executeInterrupt;

    /* Setup exception handlers. */
    for (int i = 0; i < 17; i++)
    {
        hookInterrupt(i, exception, 0);
    }
    /* Setup IRQ handlers. */
    for (int i = 17; i < 256; i++)
    {
        /* Trap gate. */
        if (i == 0x90)
            hookInterrupt(0x90, trap, 0);

        /* Hardware Interrupt. */
        else
            hookInterrupt(i, interrupt, 0);
    }
    /* Install PIT (i8253) IRQ handler. */
    hookInterrupt(IRQ(0), clocktick, 0);

    // Initialize TSS Segment
    gdt[KERNEL_TSS].limitLow    = sizeof(TSS) + (0xfff / 8);
    gdt[KERNEL_TSS].baseLow     = ((Address) &kernelTss) & 0xffff;
    gdt[KERNEL_TSS].baseMid     = (((Address) &kernelTss) >> 16) & 0xff;
    gdt[KERNEL_TSS].type        = 9;
    gdt[KERNEL_TSS].privilege   = 0;
    gdt[KERNEL_TSS].present     = 1;
    gdt[KERNEL_TSS].limitHigh   = 0;
    gdt[KERNEL_TSS].granularity = 8;
    gdt[KERNEL_TSS].baseHigh    = (((Address) &kernelTss) >> 24) & 0xff;

    // Fill the Task State Segment (TSS).
    MemoryBlock::set(&kernelTss, 0, sizeof(TSS));
    kernelTss.ss0    = KERNEL_DS_SEL;
    kernelTss.esp0   = 0;
    kernelTss.bitmap = sizeof(TSS);
    ltr(KERNEL_TSS_SEL);
}

void IntelKernel::enableIRQ(u32 irq, bool enabled)
{
    if (enabled)
    {
        if (irq < 8)
            IO::outb(PIC1_DATA, IO::inb(PIC1_DATA) & ~(1 << irq));
        else
            IO::outb(PIC2_DATA, IO::inb(PIC2_DATA) & ~(1 << (irq - 8)));
    }
    else
    {
        if (irq < 8)
            IO::outb(PIC1_DATA, IO::inb(PIC1_DATA) | (1 << irq));
        else
            IO::outb(PIC2_DATA, IO::inb(PIC2_DATA) | (1 << (irq - 8)));
    }
}

// TODO: this is a generic function?
void IntelKernel::exception(CPUState *state, ulong param)
{
    IntelCore core;
    ProcessManager *procs = Kernel::instance->getProcessManager();

    ERROR("Exception in Process: " << procs->current()->getID());
    core.logException(state);

    assert(procs->current() != ZERO);
    procs->remove(procs->current());
    procs->schedule();
}

void IntelKernel::interrupt(CPUState *state, ulong param)
{
    /* End of Interrupt to slave. */
    if (IRQ(state->vector) >= 8)
    {
        IO::outb(PIC2_CMD, PIC_EOI);
    }
    /* End of Interrupt to master. */
    IO::outb(PIC1_CMD, PIC_EOI);
}

void IntelKernel::trap(CPUState *state, ulong param)
{
    state->regs.eax = Kernel::instance->getAPI()->invoke(
        (API::Number) state->regs.eax,
                      state->regs.ecx,
                      state->regs.ebx,
                      state->regs.edx,
                      state->regs.esi,
                      state->regs.edi
    );
}

void IntelKernel::clocktick(CPUState *state, ulong param)
{
    Kernel::instance->enableIRQ(0, true);
    Kernel::instance->getProcessManager()->schedule();
}

bool IntelKernel::loadBootImage()
{
    MultibootModule *mod;
    BootImage *image;

    // Startup boot modules
    for (Size n = 0; n < multibootInfo.modsCount; n++)
    {
        // Map MultibootModule struct
        // TODO: too many arguments. Make an easier wrapper.
        mod = (MultibootModule *) multibootInfo.modsAddress;
        mod += n;
        String str((char *)(mod->string));

        // Mark its memory used
        for (Address a = mod->modStart; a < mod->modEnd; a += PAGESIZE)
            m_alloc->allocate(a);

        // Is this a BootImage?
        if (str.match("*.img.gz"))
        {
            image = (BootImage *) mod->modStart;

            // Verify this is a correct BootImage
            if (image->magic[0] == BOOTIMAGE_MAGIC0 &&
                image->magic[1] == BOOTIMAGE_MAGIC1 &&
                image->layoutRevision == BOOTIMAGE_REVISION)
            {
                m_bootImageAddress = mod->modStart;
                m_bootImageSize    = mod->modEnd - mod->modStart;

                // TODO: move this to Kernel.cpp
                // Loop BootPrograms
                for (Size i = 0; i < image->symbolTableCount; i++)
                    loadBootProcess(image, mod->modStart, i);
            }
            return true;
        }
    }
    return false;
}

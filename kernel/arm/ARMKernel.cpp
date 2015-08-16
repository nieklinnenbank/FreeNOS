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

#include <Log.h>
#include <SplitAllocator.h>
#include <FreeNOS/API.h>
#include <arm/ARMInterrupt.h>
#include <arm/ARMConstant.h>
#include <arm/BCM2835Interrupt.h>
#include "ARMKernel.h"

ARMKernel::ARMKernel(Memory::Range kernel,
                     Memory::Range memory,
                     ARMInterrupt *intr,
                     Address tags)
    : Kernel(kernel, memory), m_tags(tags)
{    
    NOTICE("");

    m_intr = intr;
    intr->install(ARMInterrupt::UndefinedInstruction, undefinedInstruction);
    intr->install(ARMInterrupt::SoftwareInterrupt, trap);
    intr->install(ARMInterrupt::PrefetchAbort, prefetchAbort);
    intr->install(ARMInterrupt::DataAbort, dataAbort);
    intr->install(ARMInterrupt::Reserved, reserved);
    intr->install(ARMInterrupt::IRQ, interrupt);
    intr->install(ARMInterrupt::FIQ, interrupt);

    // Enable clocks and irqs
    m_timer.setInterval( 250 ); /* trigger timer interrupts at 250Hz (clock runs at 1Mhz) */
    m_intr->enableIRQ(BCM_IRQ_SYSTIMERM1);

    // Set ARMCore modes
    ARMControl ctrl;
    ctrl.unset(ARMControl::AlignmentCorrect);
    ctrl.set(ARMControl::AlignmentFaults);
    ctrl.unset(ARMControl::BigEndian);
}

void ARMKernel::enableIRQ(u32 vector, bool enabled)
{
    DEBUG("vector =" << vector << "enabled =" << enabled);

    if (enabled)
        m_intr->enableIRQ(vector);
    else
        m_intr->disableIRQ(vector);
}

void ARMKernel::interrupt(CPUState state)
{
    ARMKernel *kernel = (ARMKernel *) Kernel::instance;

    //DEBUG("");

    // TODO: remove BCM2835 specific code
    if (kernel->m_intr->isTriggered(BCM_IRQ_SYSTIMERM1))
    {
        kernel->m_timer.next();
        kernel->getProcessManager()->schedule();
    }
    else
    {
        for (uint i = 0; i < 64; i++)
        {
            if (kernel->m_intr->isTriggered(i))
                kernel->executeInterrupt(i, &state);
        }
    }
}

void ARMKernel::undefinedInstruction(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}

void ARMKernel::prefetchAbort(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}

void ARMKernel::dataAbort(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}


void ARMKernel::reserved(CPUState state)
{
    ARMCore core;
    core.logException(&state);
    FATAL("procId = " << Kernel::instance->getProcessManager()->current()->getID());
    for(;;);
}

void ARMKernel::trap(CPUState state)
{
    //DEBUG("procId = " << Kernel::instance->getProcessManager()->current()->getID() << " api = " << state.r0);

    state.r0 = Kernel::instance->getAPI()->invoke(
        (API::Number) state.r0,
                      state.r1,
                      state.r2,
                      state.r3,
                      state.r4,
                      state.r5
    );
}

bool ARMKernel::loadBootImage()
{
    BootImage *image;

    DEBUG("");

    Memory::Range range = m_tags.getInitRd2();
    DEBUG("initrd = " << range.phys << " (" << range.size << ")");

    // TODO: most of this code should be moved to the generic Kernel.
    image = (BootImage *) range.phys;

    // Verify this is a correct BootImage
    if (image->magic[0] == BOOTIMAGE_MAGIC0 &&
        image->magic[1] == BOOTIMAGE_MAGIC1 &&
        image->layoutRevision == BOOTIMAGE_REVISION)
    {
        m_bootImageAddress = range.phys;
        m_bootImageSize    = range.size;

       // Mark its memory used
        for (Size i = 0; i < m_bootImageSize; i += PAGESIZE)
            m_alloc->allocate(m_bootImageAddress + i);

        // Loop BootPrograms
        for (Size i = 0; i < image->symbolTableCount; i++)
            loadBootProcess(image, m_bootImageAddress, i);
    }
    else
        ERROR("invalid magic on BootImage");

    return true;
}

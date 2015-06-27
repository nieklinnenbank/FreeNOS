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
#include <arm/ARMInterrupt.h>
#include "ARMKernel.h"

ARMKernel::ARMKernel(Memory::Range kernel,
                     Memory::Range memory,
                     ARMInterrupt *intr)
    : Kernel(kernel, memory)
{    
    NOTICE("");

    intr->install(ARMInterrupt::UndefinedInstruction, exception);
    intr->install(ARMInterrupt::SoftwareInterrupt, trap);
    intr->install(ARMInterrupt::PrefetchAbort, exception);
    intr->install(ARMInterrupt::DataAbort, exception);
    intr->install(ARMInterrupt::Reserved, exception);
    intr->install(ARMInterrupt::IRQ, interrupt);
    intr->install(ARMInterrupt::FIQ, interrupt);

    ARMPaging mmu(0, m_memory);
    mmu.initialize();
}

void ARMKernel::enableIRQ(uint vector, bool enabled)
{
    DEBUG("vector =" << vector << "enabled =" << enabled);
    //m_interruptControl->enable(vector, enabled);
}

void ARMKernel::interrupt(CPUState state)
{
    DEBUG("");
}

void ARMKernel::exception(CPUState state)
{
    DEBUG("");
}

void ARMKernel::trap(CPUState state)
{
    DEBUG("");
}

bool ARMKernel::loadBootImage()
{
    DEBUG("");
    return true;
}

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
#include <BitAllocator.h>
#include <arm/ARMInterrupt.h>
#include "ARMKernel.h"

ARMKernel::ARMKernel(Memory::Range kernel,
                     Memory::Range memory,
                     ARMInterrupt *intr,
                     Address tags)
    : Kernel(kernel, memory), m_tags(tags)
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
    FATAL(""); for(;;);
}

void ARMKernel::trap(CPUState state)
{
    DEBUG("");
}

bool ARMKernel::loadBootImage()
{
    Arch::Memory virt(0, m_memory);
    BootImage *image;

    DEBUG("");

    Memory::Range range = m_tags.getInitRd2();
    DEBUG("initrd = " << range.phys << " (" << range.size << ")");

    // TODO: most of this code should be moved to the generic Kernel.
    // Map the BootImage into our address space
    range.virt   = virt.findFree(range.size, Memory::KernelPrivate);
    range.access = Arch::Memory::Present | Arch::Memory::Readable;
    virt.mapRange(&range);
    image = (BootImage *) range.virt;

    // Verify this is a correct BootImage
    if (image->magic[0] == BOOTIMAGE_MAGIC0 &&
        image->magic[1] == BOOTIMAGE_MAGIC1 &&
        image->layoutRevision == BOOTIMAGE_REVISION)
    {
        m_bootImageAddress = range.phys;
        m_bootImageSize    = range.size;

       // Mark its memory used
        for (Size i = 0; i < m_bootImageSize; i += PAGESIZE)
            m_memory->allocate(m_bootImageAddress + i);

        // Loop BootPrograms
        for (Size i = 0; i < image->symbolTableCount; i++)
            loadBootProcess(image, m_bootImageAddress, i);
    }
    else
        ERROR("invalid magic on BootImage");

    return true;
}

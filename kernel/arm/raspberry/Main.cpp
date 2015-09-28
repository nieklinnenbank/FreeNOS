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

#include <FreeNOS/Config.h>
#include <FreeNOS/Support.h>
#include <FreeNOS/System.h>
#include <FreeNOS/arm/ARMKernel.h>
#include <Macros.h>
#include <arm/ARMControl.h>
#include <arm/BCM2835Interrupt.h>
#include <arm/BCMSysTimer.h>
#include "RaspiSerial.h"

extern C int kernel_main(u32 r0, u32 r1, u32 r2)
{
    Arch::MemoryMap mem;
    BCM2835Interrupt irq;
    ARMTags tags(r2);
    Memory::Range initrd = tags.getInitRd2();

    // Fill coreInfo
    MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
    coreInfo.bootImageAddress = initrd.phys;
    coreInfo.bootImageSize    = initrd.size;
    coreInfo.kernel.phys      = 0;
    coreInfo.kernel.size      = MegaByte(4);
    coreInfo.memory.phys      = 0;
    coreInfo.memory.size      = MegaByte(512);

    // Initialize heap
    Kernel::heap( MegaByte(3), MegaByte(1) );

    // TODO: put this in the boot.S, or maybe hide it in the support library? maybe a _run_main() or something.
    constructors();

    // Open the serial console as default Log
    RaspiSerial console;
    console.setMinimumLogLevel(Log::Notice);

    // Create the kernel
    ARMKernel kernel(&irq, &coreInfo);

    // Print some info
    DEBUG("ATAGS = " << r2);
    ARMControl ctrl;
    DEBUG("MainID = " << ctrl.read(ARMControl::MainID));
    ctrl.write(ARMControl::UserProcID, 11223344);
    DEBUG("UserProcID = " << ctrl.read(ARMControl::UserProcID));

    // Run the kernel
    return kernel.run();
}

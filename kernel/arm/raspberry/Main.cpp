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
#include <Macros.h>
#include <MemoryBlock.h>
#include <arm/ARMControl.h>
#include <arm/ARMPaging.h>
#include <arm/broadcom/BroadcomInterrupt.h>
#include <arm/broadcom/BroadcomTimer.h>
#include <PL011.h>
#include <DeviceLog.h>
#include "RaspberryKernel.h"

extern Address __start, __end, __bootimg;

static u32 ALIGN(16 * 1024) SECTION(".data") tmpPageDir[4096];

extern C int kernel_main(void)
{
    // Invalidate all caches now
    Arch::Cache cache;
    cache.invalidate(Cache::Unified);

#ifdef ARMV7
    // Raise the SMP bit for ARMv7
    ARMControl ctrl;
    ctrl.set(ARMControl::SMPBit);
#endif

    // Fill coreInfo
    BootImage *bootimage = (BootImage *) &__bootimg;
    MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
    coreInfo.bootImageAddress = (Address) (bootimage);
    coreInfo.bootImageSize    = bootimage->bootImageSize;
    coreInfo.kernel.phys      = (Address) &__start;
    coreInfo.kernel.size      = ((Address) &__end - (Address) &__start);
    coreInfo.memory.phys      = RAM_ADDR;
    coreInfo.memory.size      = RAM_SIZE;

    // Prepare early page tables
    Arch::MemoryMap mem;
    ARMPaging paging(&mem, (Address) &tmpPageDir, RAM_ADDR);

    // Activate MMU
    paging.initialize();
    paging.activate(true);

    // Clear BSS
    clearBSS();

    // Initialize heap
    Kernel::initializeHeap();

    // Run all constructors first
    constructors();

    // Open the serial console as default Log
    PL011 pl011(UART0_IRQ);
    pl011.initialize();

    DeviceLog console(pl011);
    console.setMinimumLogLevel(Log::Notice);

    // Create the kernel
    RaspberryKernel kernel(&coreInfo);

    // Run the kernel
    return kernel.run();
}

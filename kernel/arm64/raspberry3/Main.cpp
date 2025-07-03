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


/* https://github.com/bztsrc/raspi3-tutorial/blob/master/03_uart1 */
extern Address __start, __end, __bootimg;

#include <FreeNOS/System.h>
#include <FreeNOS/arm64/ARM64Kernel.h>
#include <MemoryBlock.h>
#include <arm64/ARM64Map.h>
#include <arm64/ARM64Paging.h>
#include <arm64/ARM64PageTable.h>
#include <arm64/ARM64Exception.h>
#include <arm64/ARM64Control.h>
#include <DeviceLog.h>
#include "CoreInfo.h"
#include "BootImage.h"
#include "RaspberryKernel.h"
#include "Support.h"
#include "PL011.h"

static char ALIGN(16 * 1024) SECTION(".data") tmpPageDir[sizeof(ARM64PageTable)];
static char ALIGN(16 * 1024) SECTION(".data") tmpPage1Dir[sizeof(ARM64PageTable)];
static char ALIGN(16 * 1024) SECTION(".data") tmpPage2Dir[sizeof(ARM64PageTable)];

extern C int kernel_main(void)
{
    // Fill coreInfo
    BootImage *bootimage = (BootImage *) &__bootimg;
    MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
    coreInfo.bootImageAddress = (Address) (bootimage);
    coreInfo.bootImageSize    = bootimage->bootImageSize;
    coreInfo.kernel.phys      = (Address) &__start;
    coreInfo.kernel.size      = ((Address) &__end - (Address) &__start);
    coreInfo.memory.phys      = RAM_ADDR;
    coreInfo.memory.size      = RAM_SIZE;

    Arch::MemoryMap mem;
    Address tables[2] = { (Address) tmpPage1Dir, (Address) tmpPage2Dir };
    ARM64Paging paging(&mem, (Address) &tmpPageDir, tables, RAM_ADDR);

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
    PL011 pl011(25);
    pl011.initialize();

    DeviceLog console(pl011);
    console.setMinimumLogLevel(Log::Notice);

    RaspberryKernel kernel(&coreInfo);

    NOTICE("Before accessing memory");
    u64 val = *(volatile u64 *)0xffffffffffffaaffull;
    NOTICE("After accessing memory");
    NOTICE("Bootimg address " << (void *) coreInfo.bootImageAddress);
    NOTICE("Bootimg size " << (void *) coreInfo.bootImageSize);

    // echo everything back
    while(1) {
    }
    return 0;
}

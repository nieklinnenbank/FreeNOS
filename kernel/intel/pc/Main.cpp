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
#include <intel/IntelKernel.h>
#include <intel/IntelSerial.h>
#include <intel/IntelBoot.h>
#include <Macros.h>
#include <Log.h>

#warning take the kernel physical memory base here. it varies per core.

extern C int kernel_main()
{
    // Initialize heap at 3MB offset
    // TODO: fix this
    Kernel::heap( MegaByte(3),
                  MegaByte(1) );

    // Start kernel debug serial console
    // TODO: can I re-use the user-land driver here somehow????
    IntelSerial *serial = new IntelSerial(0x3f8);
    serial->setMinimumLogLevel(Log::Notice);

    // TODO: put this in the boot.S, or maybe hide it in the support library? maybe a _run_main() or something.
    constructors();

    // Kernel memory range (first 4MB, includes 1MB heap)
    Memory::Range kernelRange;
    kernelRange.phys = 0;
    kernelRange.size = MegaByte(4);

    // RAM physical range for this core (SplitAllocator lower memory).
    Memory::Range ramRange;
    ramRange.phys = 0;
    ramRange.size = (multibootInfo.memUpper * 1024) + MegaByte(1);

    // Create and run the kernel
    IntelKernel *kernel = new IntelKernel(kernelRange, ramRange);
    return kernel->run();
}

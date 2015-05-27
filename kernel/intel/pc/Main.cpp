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
#include <intel/IntelKernel.h>
#include <intel/IntelSerial.h>
#include <intel/IntelBoot.h>
#include <Macros.h>
#include <Log.h>

extern C void kernel_main()
{
    // Initialize physical memory.
    Memory::initialize(0x00300000);

    // Start kernel debug serial console
    IntelSerial *serial = new IntelSerial(0x3f8);
    serial->setMinimumLogLevel(Log::Notice);

    // TODO: put this in the boot.S, or maybe hide it in the support library? maybe a _run_main() or something.
    constructors();

    // Create and run the kernel
    IntelKernel *kernel = new IntelKernel(
        (multibootInfo.memLower + multibootInfo.memUpper) * 1024,
         0, 1024 * 1024 * 4
    );
    kernel->run();
}

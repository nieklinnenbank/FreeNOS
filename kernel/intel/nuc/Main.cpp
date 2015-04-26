/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include <Macros.h>
#include <Log.h>
#include <FreeNOS/ProcessScheduler.h>
#include <FreeNOS/Support.h>
#include <intel/IntelKernel.h>
#include <intel/IntelMemory.h>
#include <intel/IntelFactory.h>
#include <intel/IntelSerial.h>
#include <intel/Multiboot.h>

extern C void kernel_main()
{
    // Initialize heap
    Memory::initialize(0x00300000);

    // Start kernel debug serial console
    IntelSerial *serial = new IntelSerial(0x3f8);

    // TODO: put this in the boot.S, or maybe hide it in the support library? maybe a _run_main() or something.
    constructors();

    // TODO: this should be done from the support library too.
    // Later, a user-process should monitor the kernel console buffer and write
    // it to the selected console for the kernel.
#define BANNER \
    "FreeNOS " RELEASE " [" ARCH "/" SYSTEM "] (" BUILDUSER "@" BUILDHOST ") (" COMPILER_VERSION ") " DATETIME "\r\n"

    serial->setMinimumLogLevel(Log::Notice);
    serial->write(BANNER);
    serial->write(COPYRIGHT "\r\n");
    NOTICE("Initializing subsystems");

    // Determine memory size
    Size memorySize = ((multibootInfo.memLower + multibootInfo.memUpper) * 1024);

    // Create subsystems
    IntelFactory *factory   = new IntelFactory();
    IntelMemory *memory     = new IntelMemory(memorySize);
    ProcessScheduler *sched = new ProcessScheduler();
    ProcessManager *proc    = new ProcessManager(factory, sched);
    IntelKernel *kernel     = new IntelKernel(memory, proc);

    // Begin execution
    kernel->run();
}

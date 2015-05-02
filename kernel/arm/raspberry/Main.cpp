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
#include <FreeNOS/arm/ARMKernel.h>
#include <FreeNOS/arm/ARMMemory.h>
#include <FreeNOS/arm/ARMFactory.h>
#include <FreeNOS/ProcessScheduler.h>
#include <FreeNOS/ProcessManager.h>
#include <Macros.h>
#include "RaspiSerial.h"

extern C int kernel_main(void)
{
    // Initialize heap
    Memory::initialize(0x00300000);

    RaspiSerial console;

    // TODO: put this in the boot.S, or maybe hide it in the support library? maybe a _run_main() or something.
    constructors();

    // TODO: this should be done from the support library too.
    // Later, a user-process should monitor the kernel console buffer and write
    // it to the selected console for the kernel.
#define BANNER \
    "FreeNOS " RELEASE " [" ARCH "/" SYSTEM "] (" BUILDUSER "@" BUILDHOST ") (" COMPILER_VERSION ") " DATETIME "\r\n"

    console.setMinimumLogLevel(Log::Debug);
    console.write(BANNER);
    console.write(COPYRIGHT "\r\n");
    NOTICE("Initializing subsystems");

    ARMMemory mem(128 * 1024 * 1024);
    ARMFactory fac;
    ProcessScheduler sched;
    ProcessManager procs(&fac, &sched);
    ARMKernel kernel(&mem, &procs);

    while (true)
	console.put(console.get());

    return 0;
}

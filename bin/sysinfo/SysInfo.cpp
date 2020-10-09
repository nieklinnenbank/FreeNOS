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

#include <FreeNOS/User.h>
#include <Timer.h>
#include <CoreClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include "SysInfo.h"

SysInfo::SysInfo(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("Print global system information");
}

SysInfo::~SysInfo()
{
}

SysInfo::Result SysInfo::exec()
{
    const CoreClient coreClient;
    Size numCores = 1U;
    Timer::Info timer;
    struct timeval tv;
    struct timezone tz;

    // Retrieve number of cores from the CoreServer
    const Core::Result result = coreClient.getCoreCount(numCores);
    if (result != Core::Success)
    {
        printf("failed to retrieve core count from CoreServer: %d\n",
                (int) result);
        return IOError;
    }

    // Retrieve scheduler timer info from the kernel
    ProcessCtl(SELF, InfoTimer, (Address) &timer);
    gettimeofday(&tv, &tz);

    // Retrieve system information
    const SystemInformation info;

    // Print all information to standard output
    printf("Memory Total:     %u KB\r\n"
           "Memory Available: %u KB\r\n"
           "Processor Cores:  %u\r\n"
           "Timer:            %l ticks (%u hertz)\r\n"
           "Uptime:           %l.%us\r\n",
            info.memorySize / 1024,
            info.memoryAvail / 1024,
            numCores,
            (u32) timer.ticks,
            timer.frequency,
            (u32) tv.tv_sec, tv.tv_usec);

    // Done
    return Success;
}

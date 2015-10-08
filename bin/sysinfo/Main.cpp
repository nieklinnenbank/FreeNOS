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

#include <FreeNOS/API.h>
#include <CoreMessage.h>
#include <Timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    SystemInformation info;
    CoreMessage msg;
    Timer::Info timer;

    msg.action = GetCoreCount;
    msg.type = IPCType;
    msg.from = SELF;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    ProcessCtl(SELF, InfoTimer, (Address) &timer);

    printf("Memory Total:     %u KB\r\n"
           "Memory Available: %u KB\r\n"
           "Processor Cores:  %u\r\n"
           "Timer:            %l ticks (%u hertz)\r\n"
           "Uptime:           %u.%us\r\n",
            info.memorySize / 1024,
            info.memoryAvail / 1024,
            msg.coreCount,
            (u32)timer.ticks,
            timer.frequency,
            timer.frequency ? (u32)timer.ticks / timer.frequency : 0,
            timer.frequency ? (u32)timer.ticks % timer.frequency : 0);

    return EXIT_SUCCESS;
}

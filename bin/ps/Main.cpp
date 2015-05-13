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
#include <UserProcess.h>
#include <Types.h>
#include <Macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

UserProcess procs[MAX_PROCS];

char * states[] =
{
    "Running",
    "Ready",
    "Stopped",
    "Sleeping",
};

int main(int argc, char **argv)
{
    CoreMessage msg;
    ProcessInfo info;

    // Receive UserProcess table from the CoreServer
    msg.action = ReadProcess;
    msg.buffer = procs;
    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));

    // Check the result
    if (msg.result != 0)
    {
        printf("%s: failed to receive process table: %s\n",
                argv[0], strerror(msg.result));
        return EXIT_FAILURE;
    }

    // Print header
    printf("ID  PARENT  USER GROUP STATUS     CMD\r\n");

    // Loop processes
    for (int i = 0; i < MAX_PROCS; i++)
    {
        // Skip unused PIDs
        if (!procs[i].command[0])
            continue;

        // Request kernel's process information
        ProcessCtl(i, InfoPID, (Address) &info);

        // Output a line
        printf("%3d %7d %4d %5d %10s %32s\r\n",
                i,
                procs[i].parent,
                procs[i].userID,
                procs[i].groupID,
                states[info.state],
                procs[i].command);
    }
    return EXIT_SUCCESS;
}

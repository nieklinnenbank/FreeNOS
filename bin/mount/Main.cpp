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
#include <FileSystemMount.h>
#include <UserProcess.h>
#include <Runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

UserProcess procs[MAX_PROCS];

int main(int argc, char **argv)
{
    CoreMessage msg;
    FileSystemMount *mounts = getMounts();

    // Obtain the user process table
    msg.action = ReadProcess;
    msg.number = ZERO;
    msg.buffer = procs;

    IPCMessage(CORESRV_PID, API::SendReceive, &msg, sizeof(msg));
    if (msg.result != ESUCCESS)
    {
        printf("%s: failed to receive process table: %s\n",
                argv[0], strerror(msg.result));
        return EXIT_FAILURE;
    }

    // Print header
    printf("PATH       FILESYSTEM\r\n");

    // Print out
    for (Size i = 0; i < FILESYSTEM_MAXMOUNTS; i++)
    {
        if (mounts[i].path[0])
            printf("%10s %s\r\n", mounts[i].path, procs[mounts[i].procID].command);
    }
    // Done
    return EXIT_SUCCESS;
}

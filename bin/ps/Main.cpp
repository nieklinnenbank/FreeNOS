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

#include <FreeNOS/System.h>
#include <Types.h>
#include <Macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char * states[] =
{
    "Running",
    "Ready",
    "Stopped",
    "Sleeping",
    "Waiting"
};

char cmd[PAGESIZE];

int main(int argc, char **argv)
{
    ProcessInfo info;

    // TODO: ask the kernel for the whole process table in one shot.

    // Print header
    printf("ID  PARENT  USER GROUP STATUS     CMD\r\n");

    memset(&cmd, 0, sizeof(cmd));

    // Loop processes
    for (uint i = 0; i < MAX_PROCS; i++)
    {
        // Request kernel's process information
        if (ProcessCtl(i, InfoPID, (Address) &info) != API::NotFound)
        {
            // Get the command
            VMCopy(i, API::Read, (Address) cmd, ARGV_ADDR, PAGESIZE);

            // Output a line
            printf("%3d %7d %4d %5d %10s %32s\r\n",
                    i,
                    info.parent, 0, 0,
                    states[info.state], cmd);
        }
    }
    return EXIT_SUCCESS;
}

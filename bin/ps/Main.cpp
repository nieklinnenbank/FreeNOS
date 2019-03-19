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
#include <unistd.h>
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
    Arch::MemoryMap map;
    Memory::Range range = map.range(MemoryMap::UserArgs);
    String out;
    char line[256];

    // Print header
    out << "ID  PARENT  USER GROUP STATUS     CMD\r\n";
    memset(&cmd, 0, sizeof(cmd));

    // Loop processes
    for (uint i = 0; i < MAX_PROCS; i++)
    {
        // Request kernel's process information
        if (ProcessCtl(i, InfoPID, (Address) &info) != API::NotFound)
        {
            // Get the command
            VMCopy(i, API::Read, (Address) cmd, range.virt, PAGESIZE);

            // Output a line
            snprintf(line, sizeof(line),
                    "%3d %7d %4d %5d %10s %32s\r\n",
                     i, info.parent, 0, 0, states[info.state], cmd);
            out << line;
        }
    }
    // Output the table
    write(1, *out, out.length());
    return EXIT_SUCCESS;
}

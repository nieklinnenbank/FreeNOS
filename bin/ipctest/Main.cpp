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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FileSystemMessage.h>
#include <MemoryChannel.h>

char cmd[PAGESIZE];

int main(int argc, char **argv)
{
    ProcessID pid = 0;
    ProcessInfo info;

    // Find the PID of ipctest server
    // Loop processes
    for (uint i = 0; i < MAX_PROCS; i++)
    {
        // Request kernel's process information
        if (ProcessCtl(i, InfoPID, (Address) &info) != API::NotFound)
        {
            // Get the command
            VMCopy(i, API::Read, (Address) cmd, ARGV_ADDR, PAGESIZE);

            if (strcmp(cmd, "/server/ipctest/server") == 0)
            {
                pid = i;
                break;
            }
        }
    }

    if (!pid)
    {
        printf("%s: failed to find PID for ipctest server\n", argv[0]);
        return 1;
    }
    printf("%s: found ipctest server at PID %d\n", argv[0], pid);

    // Call VMShare to create shared memory mapping for MemoryChannel.
    Memory::Range range;
    range.size = PAGESIZE * 4;
    range.virt = 0;
    range.phys = 0;
    range.access = Memory::User | Memory::Readable | Memory::Writable | Memory::Uncached;

    // Create shared memory mapping
    Error r = VMShare(pid, API::Create, (Address) &range);
    if (r != API::Success)
    {
        printf("%s: VMShare() failed with result = %d\n", argv[0], r);
        return 1;
    }
    printf("%s: mapped shared at: phys=%u virt=%u\n", argv[0], range.phys, range.virt);

    //Address shares;
    //VMShare(SELF, API::Read, (Address) &shares);

    FileSystemMessage msg;
    MemoryChannel send;
    send.setMessageSize(sizeof(msg));
    send.setMode(Channel::Producer);

    if (send.setData(range.phys/*virt*/) != MemoryChannel::Success)
    {
        printf("%s: setData failed\n", argv[0]);
        return 1;
    }
    send.setFeedback(range.phys/*virt*/ + PAGESIZE);

    printf("%s: wakeup ipctest server..\n", argv[0]);
    ProcessCtl(pid, Resume, 0);


    msg.offset = 0x12345678;
    printf("%s: writing message..\n", argv[0]);
    send.write(&msg);
    printf("%s: write finished\n", argv[0]);



    // TODO: Wake up the ipctest server (interrupt) to ensure
    // it sees the new shared mapping for the MemoryChannel.

    // TODO: send message in the memory channel

    return 0;
}

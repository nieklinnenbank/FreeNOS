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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <FileSystemMessage.h>
#include <MemoryChannel.h>
#include <ChannelClient.h>
#include <ChannelRegistry.h>

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

    ChannelRegistry reg;
    ChannelClient client;
    client.setRegistry(&reg);

    if (client.connect(pid) != ChannelClient::Success)
    {
        printf("%s: failed to connect to PID %d\n", argv[0], pid);
        return 1;
    }
    printf("%s: connected to PID %d\n", argv[0], pid);

    FileSystemMessage msg;

    printf("%s: wakeup ipctest server..\n", argv[0]);
    ProcessCtl(pid, Resume, 0);

    msg.offset = 0x12345678;
    printf("%s: writing message..\n", argv[0]);
    if (client.syncSendTo(&msg, pid) != ChannelClient::Success)
    {
        printf("%s: failed to send message to PID %d\n", argv[0], pid);
        return 1;
    }
    printf("%s: write finished\n", argv[0]);
    printf("%s: receiving reply\n", argv[0]);
    // TODO: make the wakeup a counter?
    ProcessCtl(pid, Resume, 0);

    if (client.syncReceiveFrom(&msg, pid) != ChannelClient::Success)
    {
        printf("%s: failed to receive message from PID %d\n", argv[0], pid);
        return 1;
    }
    printf("%s: received reply with result = %d\n", argv[0], msg.result);
    return 0;
}

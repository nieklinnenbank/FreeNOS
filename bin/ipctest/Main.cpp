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
#include <IPCTestMessage.h>
#include <MemoryChannel.h>
#include <ChannelClient.h>

#define IPCTESTSERVER_CMD "/server/ipctest/server"

char cmd[PAGESIZE];

int main(int argc, char **argv)
{
    ProcessID pid = 0;
    ProcessInfo info;
    Arch::MemoryMap map;
    IPCTestMessage msg;
    ChannelClient::Result r;
    Memory::Range range = map.range(MemoryMap::UserArgs);

    /* Find the PID of ipctest server. Loop processes */
    for (uint i = 0; i < MAX_PROCS; i++)
    {
        // Request kernel's process information
        if (ProcessCtl(i, InfoPID, (Address) &info) != API::NotFound)
        {
            // Get the command
            VMCopy(i, API::Read, (Address) cmd, range.virt, PAGESIZE);

            if (strncmp(cmd, IPCTESTSERVER_CMD, strlen(IPCTESTSERVER_CMD)) == 0)
            {
                pid = i;
                break;
            }
        }
    }
    /* Did we find the IPCTestServer process? */
    if (!pid)
    {
        printf("%s: failed to find PID for ipctest server\n", argv[0]);
        return EXIT_FAILURE;
    }
    printf("%s: found ipctest server at PID %d\n", argv[0], pid);

    /* Connect first, to ensure message size matches */
    if ((r = ChannelClient::instance->connect(pid, sizeof(msg))) != ChannelClient::Success)
    {
        printf("%s: failed to connect to ipctest server: error %d\n",
                argv[0], (int) r);
        return EXIT_FAILURE;
    }

    /* Prepare message */
    msg.action = TestActionB;
    msg.data   = 0x12345678;
    printf("%s: sending message with data = %x\n", argv[0], msg.data);

    /* Send message */
    if ((r = ChannelClient::instance->syncSendTo(&msg, pid)) != ChannelClient::Success)
    {
        printf("%s: failed to send message to PID %d: error %d\n",
                argv[0], pid, (int) r);
        return EXIT_FAILURE;
    }
    printf("%s: send finished\n", argv[0]);
    printf("%s: receiving reply\n", argv[0]);

    /* Receive reply from server */
    if ((r = ChannelClient::instance->syncReceiveFrom(&msg, pid)) != ChannelClient::Success)
    {
        printf("%s: failed to receive message from PID %d: error %d\n",
                argv[0], pid, (int) r);
        return EXIT_FAILURE;
    }

    /* Done */
    printf("%s: reply received with data = %x\n", argv[0], msg.data);
    return EXIT_SUCCESS;
}

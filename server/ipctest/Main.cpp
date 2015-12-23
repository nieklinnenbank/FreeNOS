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
#include <MemoryChannel.h>
#include <FileSystemMessage.h>
#include <Types.h>
#include <Log.h>
#include <KernelLog.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    KernelLog log;
    log.setMinimumLogLevel(Log::Debug);

    // Sleep inifinite. We only return in case the process is
    // woken up by an external (wakeup) interrupt.
    DEBUG("WaitTimer");
    Error r = ProcessCtl(SELF, WaitTimer, 0);
    DEBUG("WaitTimer returned: " << (int)r);

    // scan the VMShare table to find incoming/new MemoryChannel
    MemoryShare share;
    VMShare(SELF, API::Read, (Address) &share);
    DEBUG("MemoryChannel at phys = " << (void *) share.range.phys << " and virt = " << (void *) share.range.virt);

    // Assign memory channel
    FileSystemMessage msg;
    MemoryChannel chan;
    chan.setMode(Channel::Consumer);
    chan.setMessageSize(sizeof(msg));
    chan.setData(share.range.phys/*virt*/);
    chan.setFeedback(share.range.phys/*virt*/ + PAGESIZE);

    // Receive message
    DEBUG("receiving");
    chan.read(&msg);
    DEBUG("received message: " << (void *) msg.offset);
    printf("ipctest-server: received message: %x\n", msg.offset);
    return 0;
}

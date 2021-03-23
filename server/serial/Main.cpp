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
#include <KernelLog.h>
#include <DeviceServer.h>
#include "SerialDevice.h"

int main(int argc, char **argv)
{
    KernelLog log;
    DeviceServer server("/dev/serial");

    // Create serial device instance
    SerialDevice::inodeNumber = server.getNextInode();
    SerialDevice *dev = SerialDevice::create();

    // Register to DeviceServer
    Directory *dir = new Directory(server.getNextInode());
    server.registerDirectory(dir, "/serial0");
    server.registerDevice(dev, "/serial0/io");
    server.registerInterrupt(dev, dev->getIrq());

    // Initialize
    const FileSystem::Result result = server.initialize();
    if (result != FileSystem::Success)
    {
        ERROR("failed to initialize: result = " << (int) result);
        return 1;
    }

    // Start serving requests
    return server.run();
}

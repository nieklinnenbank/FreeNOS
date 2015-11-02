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

#include <DeviceServer.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <BroadcomI2C.h>
#include <KernelLog.h>
#include "DigitalPort.h"

int main(int argc, char **argv)
{
    DeviceServer server("groveDigi", CharacterDeviceFile);

    // Setup logging
    Log *log = new KernelLog();
    log->setMinimumLogLevel(Log::Debug);

    // Initialize I2C controller
    BroadcomI2C *i2c = new BroadcomI2C();
    if (argc >= 2 && strcmp(argv[1], "-run") == 0)
    {
        if (i2c->initialize() != BroadcomI2C::Success)
        {
            ERROR("failed to initialize I2C controller");
            return EXIT_FAILURE;
        }
        else
            NOTICE("I2C initialized");
    }

    // Add devices
    for (int i = 0; i < 8; i++)
        server.add(new DigitalPort(i2c, i+1));

    // Start serving requests
    return server.run(argc, argv);
}

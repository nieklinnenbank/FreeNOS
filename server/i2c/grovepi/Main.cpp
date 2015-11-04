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
#include "AnalogPort.h"

#warning add to libposix with this dummy implementation.
#define sleep(sec) for (int i = 0; i < 1000000; i++);

void setRGB(BroadcomI2C *i2c, uint r, uint g, uint b)
{
    u8 command[2];
    command[0] = 0;
    command[1] = 0;
    i2c->setAddress(0x62);
    i2c->write(command, sizeof(command)); // 0,0

    command[0] = 1;
    i2c->write(command, sizeof(command)); // 1,0

    command[0] = 0x8;
    command[1] = 0xaa;
    i2c->write(command, sizeof(command)); // 0x08,0xaa

    command[0] = 4;
    command[1] = r;
    i2c->write(command, sizeof(command)); // 4,r

    command[0] = 3;
    command[1] = g;
    i2c->write(command, sizeof(command)); // 3,g

    command[0] = 2;
    command[1] = b;
    i2c->write(command, sizeof(command)); // 2,b
}

int main(int argc, char **argv)
{
#warning modify Device to provide a name.
#warning ensure that devices each have a fixed reliable name, based on e.g. its hardware address.
    DeviceServer server("groveDigi", CharacterDeviceFile);

    // Setup logging
    Log *log = new KernelLog();
    log->setMinimumLogLevel(Log::Debug);

    // Initialize I2C controller
    BroadcomI2C *i2c = new BroadcomI2C();
    if (argc >= 2 && strcmp(argv[1], "-run") == 0)
    {
        switch (i2c->initialize())
        {
            case BroadcomI2C::Success:
                break;

            case BroadcomI2C::NotFound:
                NOTICE("No I2C controller found");
                return EXIT_SUCCESS;

            default:
                ERROR("failed to initialize I2C controller");
                return EXIT_FAILURE;
        }
        u8 command[4];

        // Set RGB LED
        setRGB(i2c, 0, 0, 255);
        sleep(1);

#if 0
        // Send a version command to the GrovePi
        command[0] = 8;
        command[1] = 0;
        command[2] = 0;
        command[3] = 0;
        i2c->setAddress(0x4);
        i2c->write(command, sizeof(command));

        // TODO: implement sleep
        sleep(1);

//        i2c->setAddress(0x4);
//        i2c->read(command, 1);

        i2c->setAddress(0x4);
        i2c->read(command, sizeof(command));
        
        // Ask firmware version
        NOTICE("GrovePi firmware: " << command[1] << "." << command[2] << "." << command[3]);
#endif
#if 1
        // Set Digipin 3 to input mode
        command[0] = 5;
        command[1] = 3;
        command[2] = 0;
        command[3] = 0;
        i2c->setAddress(0x4);
        i2c->write(command, sizeof(command));
        sleep(1);

        // Set Analog 0 to input mode
        command[0] = 5;
        command[1] = 0;
        command[2] = 0;
        command[3] = 0;
        i2c->setAddress(0x4);
        i2c->write(command, sizeof(command));

        //sleep(1);
        //i2c->setAddress(0x4);
        //i2c->read(command, 1);
        //NOTICE("digi2: " << command[0]);
#endif
    }

    // Add devices
    for (int i = 0; i < 8; i++)
        server.add(new DigitalPort(i2c, i+1));

    for (int j = 0; j < 3; j++)
        server.add(new AnalogPort(i2c, j));

    // Start serving requests
    return server.run(argc, argv);
}

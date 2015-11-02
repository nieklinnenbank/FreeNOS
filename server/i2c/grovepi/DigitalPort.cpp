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

#include <Log.h>
#include "DigitalPort.h"

DigitalPort::DigitalPort(I2C *i2c, uint port)
    : Device()
{
    m_i2c  = i2c;
    m_port = port;
}

Error DigitalPort::initialize()
{
    return ESUCCESS;
}

Error DigitalPort::write(s8 *buffer, Size size, Size offset)
{
    u8 command[4];

    for (Size i = 0; i < size; i++)
    {
        DEBUG("buffer[" << i << "] =" << buffer[i]);
    }
    DEBUG("port =" << m_port);

    // Send a digitalWrite command to the GrovePi
    // including the port number and digital value (HIGH/LOW)
    command[0] = 2;
    command[1] = m_port;
    command[2] = buffer[0] != '0';
    command[3] = 0;
    m_i2c->setAddress(0x4);
    m_i2c->write(command, sizeof(command));

    // Done
    return size;
}

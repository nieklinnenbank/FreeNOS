/*
 * Copyright (C) 2020 Niek Linnenbank
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
#include "SunxiClockControl.h"

SunxiClockControl::Result SunxiClockControl::initialize()
{
    DEBUG("");

    if (m_io.map(IOBase & ~0xfff, PAGESIZE,
                 Memory::User | Memory::Readable |
                 Memory::Writable | Memory::Device) != IO::Success)
    {
        ERROR("failed to map I/O memory");
        return IOError;
    }

    m_io.setBase(m_io.getBase() + (IOBase & 0xfff));
    return Success;
}

SunxiClockControl::Result SunxiClockControl::enable(const SunxiClockControl::Clock clock)
{
    DEBUG("clock = " << (int) clock);

    u32 offset = 0x0;
    u32 bit;

    switch (clock)
    {
        case ClockEmacTx:
            offset = 0x060;
            bit = 17;
            break;

        case ClockEphy:
            offset = 0x070;
            bit = 0;
            break;

        default:
            ERROR("unsupported clock: " << (int) clock);
            return InvalidArgument;
    }

    m_io.set(offset, (1 << bit));
    return Success;
}

SunxiClockControl::Result SunxiClockControl::deassert(const SunxiClockControl::Reset reset)
{
    DEBUG("reset = " << (int) reset);

    u32 offset = 0x0;
    u32 bit;

    switch (reset)
    {
        case ResetEmacTx:
            offset = 0x2c0;
            bit = 17;
            break;

        case ResetEphy:
            offset = 0x2c8;
            bit = 2;
            break;

        default:
            ERROR("unsupported reset: " << (int) reset);
            return InvalidArgument;
    }

    m_io.set(offset, (1 << bit));
    return Success;
}

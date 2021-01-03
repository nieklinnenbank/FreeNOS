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
#include "SunxiSystemControl.h"

SunxiSystemControl::Result SunxiSystemControl::initialize()
{
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

SunxiSystemControl::Result SunxiSystemControl::setupEmac(const uint phyAddr)
{
    DEBUG("phyAddr = " << (void *)phyAddr);

    u32 val = m_io.read(EmacClock);
    val &= ~(u32)(EmacClockMask);
    val |= EmacClockDefault;
    val |= phyAddr << EmacClockPhyShift;
    val |= EmacClockLedPoll;
    val &= ~(u32)(EmacClockShutdown);
    val |= EmacClockSelect;
    val &= ~(u32)(EmacClockRmiiEn);
    m_io.write(EmacClock, val);

    return Success;
}

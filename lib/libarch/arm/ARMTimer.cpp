/*
 * Copyright (C) 2019 Niek Linnenbank
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
#include "ARMCore.h"
#include "ARMTimer.h"

/** Timer enable. Set to enable timer. */
#define CNTP_CTL_ENABLE  (1 << 0)

ARMTimer::ARMTimer()
    : m_frequency(0)
{
}

u32 ARMTimer::getSystemFrequency(void)
{
    return mrc(p15, 0, 0, c14, c0);
}

void ARMTimer::setPL1TimerValue(u32 value)
{
    mcr(p15, 0, 0, c14, c2, value);
}

u64 ARMTimer::getPL1TimerCompare(void)
{
    return mrrc(p15, 2, c14);
}

void ARMTimer::setPL1TimerCompare(u64 value)
{
    mcrr(p15, 2, c14, value);
}

void ARMTimer::setPL1Control(u32 value)
{
    mcr(p15, 0, 1, c14, c2, value);
}

ARMTimer::Result ARMTimer::setFrequency(Size hertz)
{
    m_frequency = hertz;
    tick();
    return Success;
}

ARMTimer::Result ARMTimer::tick()
{
    setPL1TimerValue(getSystemFrequency() / m_frequency);
    setPL1Control(CNTP_CTL_ENABLE);
    return Success;
}

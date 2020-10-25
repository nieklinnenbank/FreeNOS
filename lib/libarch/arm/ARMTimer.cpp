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

#include <FreeNOS/System.h>
#include "ARMCore.h"
#include "ARMTimer.h"

ARMTimer::ARMTimer()
    : m_initialTimerCounter(0)
{
    m_int = ARMTIMER_IRQ;
}

u32 ARMTimer::getSystemFrequency(void) const
{
    u32 f = mrc(p15, 0, 0, c14, c0);

#ifdef SYSTEM_FREQ
    if (f == 0)
    {
        f = SYSTEM_FREQ;
    }
#endif /* SYSTEM_FREQ */

    return f;
}

void ARMTimer::setPL1PhysicalTimerValue(const u32 value)
{
    mcr(p15, 0, 0, c14, c2, value);
}

void ARMTimer::setPL1PhysicalTimerControl(const u32 value)
{
    mcr(p15, 0, 1, c14, c2, value);
}

ARMTimer::Result ARMTimer::setFrequency(const Size hertz)
{
    m_initialTimerCounter = getSystemFrequency() / hertz;
    tick();

    return Timer::setFrequency(hertz);
}

ARMTimer::Result ARMTimer::tick()
{
    setPL1PhysicalTimerValue(m_initialTimerCounter);
    setPL1PhysicalTimerControl(TimerControlEnable);

    return Timer::tick();
}

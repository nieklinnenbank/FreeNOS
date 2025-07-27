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
#include "ARM64Control.h"
#include "ARM64Timer.h"

ARM64Timer::ARM64Timer()
    : m_initialTimerCounter(0)
{
    m_int = ARMTIMER_IRQ;
}

u32 ARM64Timer::getSystemFrequency(void) const
{
    u32 f = ARM64Control::read(ARM64Control::SystemFrequency);

    return f;
}

void ARM64Timer::setPL1PhysicalTimerValue(const u32 value)
{
    u64 f = value;
    ARM64Control::write(ARM64Control::PhysicalTimerValue, f);
}

void ARM64Timer::setPL1PhysicalTimerControl(const u32 value)
{
    u64 f = value;
    ARM64Control::write(ARM64Control::PhysicalTimerControl, f);
}

ARM64Timer::Result ARM64Timer::setFrequency(const Size hertz)
{
    m_initialTimerCounter = getSystemFrequency() / hertz;
    tick();

    return Timer::setFrequency(hertz);
}

ARM64Timer::Result ARM64Timer::tick()
{
    setPL1PhysicalTimerValue(m_initialTimerCounter);
    setPL1PhysicalTimerControl(TimerControlEnable);

    return Timer::tick();
}

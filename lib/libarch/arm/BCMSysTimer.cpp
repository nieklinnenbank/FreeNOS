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

#include "BCMSysTimer.h"

void BCMSysTimer::setInterval(u32 hertz)
{
    m_cycles = BCM_SYSTIMER_FREQ / hertz;

#warning we need to use memory barriers???

    // Use timer slot 1. Enable.
    SYSTIMER_C1  = SYSTIMER_CLO + m_cycles;
    SYSTIMER_CS |= (1 << M1);
}

void BCMSysTimer::next()
{
    // Clear+acknowledge the timer interrupt
    SYSTIMER_CS |= (1 << M1);
    SYSTIMER_C1 += m_cycles;
}

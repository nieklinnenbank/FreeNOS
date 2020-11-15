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

#include <Types.h>
#include <stdio.h>
#include <sys/time.h>
#include "SystemClock.h"

SystemClock::SystemClock()
{
    m_timeval.tv_sec = 0;
    m_timeval.tv_usec = 0;
}

void SystemClock::value(struct timeval & val) const
{
    val.tv_sec = m_timeval.tv_sec;
    val.tv_usec = m_timeval.tv_usec;
}

SystemClock::Result SystemClock::now()
{
    struct timezone tz;

    if (gettimeofday(&m_timeval, &tz) != 0)
    {
        return SystemClock::IOError;
    }
    else
    {
        return SystemClock::Success;
    }
}

void SystemClock::printDiff(const SystemClock & clock) const
{
    printDiff(clock.m_timeval);
}

void SystemClock::printDiff(const struct timeval & stamp) const
{
    const u64 usec1 = (m_timeval.tv_sec * 1000000) + (m_timeval.tv_usec);
    const u64 usec2 = (stamp.tv_sec * 1000000) + (stamp.tv_usec);

    // Print time measured
    printf("%us %uusec\n",
            (uint) ((usec2 - usec1) / 1000000),
            (uint) ((usec2 - usec1) % 1000000));
}

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

#ifndef __LIB_LIBAPP_SYSTEMCLOCK_H
#define __LIB_LIBAPP_SYSTEMCLOCK_H

#include <sys/time.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libapp
 * @{
 */

/**
 * Provides an abstract interface to the system clock.
 */
class SystemClock
{
  public:

    /**
     * Result codes
     */
    enum Result
    {
        Success,
        IOError
    };

  public:

    /**
     * Constructor
     */
    SystemClock();

    /**
     * Get time value
     *
     * @param val Timeval struct
     */
    void value(struct timeval & val) const;

    /**
     * Get the current time
     *
     * @return Result code
     */
    Result now();

    /**
     * Print difference between two clocks to stdout
     *
     * @param clock Clock to compare with
     */
    void printDiff(const SystemClock & clock) const;

    /**
     * Print difference between to timestamps to stdout
     *
     * @param stamp Timestamp to compare with
     */
    void printDiff(const struct timeval & stamp) const;

  private:

    /** Time value */
    struct timeval m_timeval;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBAPP_SYSTEMCLOCK_H */

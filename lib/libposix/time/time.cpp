/**
 * Copyright (C) 2009 Coen Bijlsma
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

#include <time.h>

unsigned long mktime(const unsigned int year, const unsigned int month,
                     const unsigned int day, const unsigned int hour,
                     const unsigned int min, const unsigned int sec)
{
    unsigned int monthVal = month, yearVal = year;

    // 1..12 -> 11,12,1..10
    if( 0 >= (int) (monthVal -= 2))
    {
        monthVal += 12; // Puts Feb last since it has leap day
        yearVal -= 1;
    }

    return ((((unsigned long)
                (yearVal/4 - yearVal/100 + yearVal/400 + 367*monthVal/12 + day) +
                yearVal*365 - 719499
            )*24 + hour // now have hours
        )*60 + min // now have minutes
    )*60 + sec; // finally seconds
}

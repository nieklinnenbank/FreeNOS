/*
 * Copyright (C) 2015 Niek Linnenbank
 * Copyright (C) 2010 Mahmoud Hesham El-Magdoub
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

#include "math.h"

double sqrt( const double fg)
{
    double n = fg / 2.0;
    double lstX = 0.0; 

    if (fg <= 0)
        return 0;

    while(n != lstX)  
    {
        lstX = n;
        n = (n + fg/n) / 2.0;
    }
    return n;
}

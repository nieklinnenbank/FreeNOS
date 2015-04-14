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

#include "Support.h"

int __aeabi_idiv(int numerator, int denominator)
{
    idiv_return r = __aeabi_idivmod(numerator, denominator);
    return r.quotient;
}

uint __aeabi_uidiv(uint numerator, uint denominator)
{
    uidiv_return r = __aeabi_uidivmod(numerator, denominator);
    return r.quotient;
}

idiv_return __aeabi_idivmod(int numerator, int denominator)
{
    idiv_return r;
    int neg = 0;

    r.quotient  = 0;
    r.remainder = 0;

    if (numerator < 0)
    {
        neg = 1;
        numerator = -numerator;
    }

    if (denominator < 0)
    {
        neg ^= 1;
        denominator = -denominator;
    }

    while (numerator >= denominator)
    {
        numerator -= denominator;
        r.quotient++;
    }
    r.remainder = numerator;

    if (neg)
    {
        r.quotient  = -r.quotient;
        r.remainder = -r.remainder;
    }
    return r;
}

uidiv_return __aeabi_uidivmod(uint numerator, uint denominator)
{
    uidiv_return r;
    
    r.quotient  = 0;
    r.remainder = 0;

    while (numerator >= denominator)
    {
        numerator -= denominator;
        r.quotient++;
    }
    r.remainder = numerator;
    return r;
}

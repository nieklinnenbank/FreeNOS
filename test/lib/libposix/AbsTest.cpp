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

#include <TestCase.h>
#include <TestRunner.h>
#include <TestMain.h>
#include <math.h>

TestCase(AbsPositive)
{
    for (int i = 0; i < 100; i++)
        testAssert(abs(i) == i);

    return OK;
}

TestCase(AbsNegative)
{
    testAssert(abs(-1) == 1);
    testAssert(abs(-1000) == 1000);
    testAssert(abs(-99999) == 99999);
    return OK;
}

TestCase(AbsZero)
{
    testAssert(abs(0) == 0);
    testAssert(abs(-0) == 0);
    return OK;
}

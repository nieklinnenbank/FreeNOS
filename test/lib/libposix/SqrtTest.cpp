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

#include <FreeNOS/System.h>
#include <TestCase.h>
#include <TestRunner.h>
#include <TestMain.h>
#include <Macros.h>
#include <math.h>

TestCase(SqrtPositive)
{
    testAssert(sqrt(1) == 1.0);
    testAssert(sqrt(16) == 4.0);
    testAssert(sqrt(100) == 10.0);
    testAssert(doubleEquals(sqrt(32), 5.65685, 0.00001));
    testAssert(doubleEquals(sqrt(91), 9.53939, 0.00001));
    testAssert(doubleEquals(sqrt(99999), 316.22618, 0.00001));
    return OK;
}

TestCase(SqrtNegative)
{
    testAssert(sqrt(-1) == 0);
    testAssert(sqrt(-1000) == 0);
    testAssert(sqrt(-99999) == 0);
    return OK;
}

TestCase(SqrtZero)
{
    testAssert(sqrt(0) == 0);
    testAssert(sqrt(-0) == 0);
    return OK;
}

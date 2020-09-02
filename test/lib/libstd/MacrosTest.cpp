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
#include <Macros.h>

TestCase(DoubleAbsolute)
{
    testAssert(doubleAbsolute(1.0) == 1.0);
    testAssert(doubleAbsolute(10000.0) == 10000.0);
    testAssert(doubleAbsolute(-9999.123) == 9999.123);
    testAssert(doubleAbsolute(0.0) == 0.0);

    return OK;
}

TestCase(DoubleEquals)
{
    testAssert(doubleEquals(1033.12345, 1033.12345, 0.000001));
    testAssert(!doubleEquals(1033.12345, 1033.12346, 0.000001));
    return OK;
}

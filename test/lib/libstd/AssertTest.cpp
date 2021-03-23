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

#include <TestCase.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>

#undef FATAL
#define FATAL(msg) \
    fatalCalled = true;

#undef assert
#undef __LIB_LIBSTD_ASSERT_H
#ifndef __ASSERT__
#define __ASSERT__
#endif
#define __LIB_LIBSTD_LOG_H
#include <Assert.h>

TestCase(AssertCallsFatal)
{
    bool fatalCalled = false;

    // invoke an assertion that always passed
    assert(true);
    testAssert(fatalCalled == false);

    // invoke an assertion that always fails
    assert(false);
    testAssert(fatalCalled == true);

    return OK;
}

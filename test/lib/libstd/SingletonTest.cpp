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
#include <TestInt.h>
#include <TestMain.h>
#include <Singleton.h>

/**
 * Test class using a StrictSingleton
 */
class StrictDummy : public StrictSingleton<StrictDummy>
{
  public:
    StrictDummy()
    {
        constructorCount++;
    }

    static Size constructorCount;
};

Size StrictDummy::constructorCount = 0;

/**
 * Test class using a WeakSingleton
 */
class WeakDummy : public WeakSingleton<WeakDummy>
{
  public:
    WeakDummy() : WeakSingleton<WeakDummy>(this)
    {
    }
};

TestCase(StrictSingletonInstance)
{
    // Verify that the instance is always the same
    testAssert(StrictDummy::instance() == StrictDummy::instance());

    // Object must be constructed only once
    testAssert(StrictDummy::constructorCount == 1);

    return OK;
}

TestCase(WeakSingletonInstance)
{
    testAssert(WeakDummy::instance() == ZERO);

    WeakDummy dummy1;
    testAssert(WeakDummy::instance() == &dummy1);

    WeakDummy dummy2;
    testAssert(WeakDummy::instance() == &dummy2);

    return OK;
}

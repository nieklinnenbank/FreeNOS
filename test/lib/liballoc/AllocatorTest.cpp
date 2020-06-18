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

#include <FreeNOS/System.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <Allocator.h>

TestCase(AllocatorConstruct)
{
    const Allocator::Range range = { 0, PAGESIZE, sizeof(u32) };
    Allocator alloc(range);

    return OK;
}

TestCase(AllocatorDefault)
{
    const Allocator::Range range = { 0, PAGESIZE, sizeof(u32) };
    Allocator alloc(range);

    // Retrieve current default
    Allocator *current = alloc.getDefault();

    // Assign a new default allocator
    alloc.setDefault(&alloc);
    testAssert(alloc.getDefault() == &alloc);

    // Restore default
    alloc.setDefault(current);
    testAssert(alloc.getDefault() == current);
    testAssert(alloc.getDefault() != &alloc);

    return OK;
}

TestCase(AllocatorParent)
{
    const Allocator::Range range = { 0, PAGESIZE, sizeof(u32) };
    Allocator alloc(range);
    Allocator parent(range);

    // The default is no parent
    testAssert(alloc.parent() == NULL);

    // Assign a new parent
    alloc.setParent(&parent);
    testAssert(alloc.parent() == &parent);

    return OK;
}

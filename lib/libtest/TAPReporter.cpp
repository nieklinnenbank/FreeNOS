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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TAPReporter.h"

TAPReporter::TAPReporter(int argc, char **argv)
    : TestReporter(argc, argv)
{
    m_count = 1;
}

void TAPReporter::reportBegin(List<TestInstance *> & tests)
{
    printf("1..%d\r\n", tests.count());
}

void TAPReporter::reportBefore(TestInstance & test)
{
}

void TAPReporter::reportAfter(TestInstance & test, TestResult & result)
{
#warning TODO: how to get the assertion failure text in the test reporter output????

    switch (result)
    {
        case OK:   printf("ok %d %s\r\n", m_count, *test.getName()); break;
        case FAIL: printf("not ok %d %s\r\n", m_count, *test.getName()); break;
        case SKIP: printf("skip %d %s\r\n", m_count, *test.getName()); break;
    }
    m_count++;
}

void TAPReporter::reportFinish(List<TestInstance *> & tests)
{
#ifdef __HOST__
    fflush(stdout);
#endif /* __HOST__ */
}

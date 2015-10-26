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

#include <string.h>
#include "TestReporter.h"

TestReporter::TestReporter(int argc, char **argv)
{
    m_argc = argc;
    m_argv = argv;
    m_ok   = 0;
    m_fail = 0;
    m_skip = 0;
    m_report = true;
    m_showStatistics = !(argc > 1 && strcmp(argv[1], "-n") == 0);
}

uint TestReporter::getOk() const
{
    return m_ok;
}

uint TestReporter::getFailed() const
{
    return m_fail;
}

uint TestReporter::getSkipped() const
{
    return m_skip;
}

void TestReporter::setReport(bool value)
{
    m_report = value;
}

void TestReporter::prepare(TestInstance & test)
{
    if (m_report)
        reportBefore(test);
}

void TestReporter::collect(TestResult & result)
{
    if (m_report)
        reportAfter(result);

    switch (result)
    {
        case OK:   m_ok++;   break;
        case FAIL: m_fail++; break;
        case SKIP: m_skip++; break;
    }
}

void TestReporter::finish()
{
    reportFinish();
}

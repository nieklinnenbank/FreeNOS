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
#include <ListIterator.h>
#include "TestCase.h"
#include "TestSuite.h"
#include "TestRunner.h"
#include "StdoutReporter.h"

TestRunner::TestRunner(int argc, char **argv)
{
    m_argc = argc;
    m_argv = argv;
    m_showStatistics = !(argc > 1 && strcmp(argv[1], "-n") == 0);
    m_reporter = new StdoutReporter(argc, argv);
}

TestRunner::~TestRunner()
{
    delete m_reporter;
}

TestReporter * TestRunner::getReporter()
{
    return m_reporter;
}

int TestRunner::run(void)
{
    for (ListIterator<TestInstance *> i(TestSuite::instance->getTests()); i.hasCurrent(); i++)
    {
        TestInstance *test = i.current();

        m_reporter->prepare(*test);
        TestResult result = test->run();
        m_reporter->collect(result);
    }
    m_reporter->finish();
    return m_reporter->getFailed();
}

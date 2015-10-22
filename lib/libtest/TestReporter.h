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

#ifndef __LIBTEST_TESTREPORTER_H
#define __LIBTEST_TESTREPORTER_H

#include <Types.h>
#include "TestCase.h"

/**
 * Responsible for outputting test results.
 */
class TestReporter
{
  public:

    /**
     * Constructor.
     */
    TestReporter(int argc, char **argv);

    /**
     * Get OK count.
     */
    uint getOk() const;

    /**
     * Get fail count.
     */
    uint getFailed() const;

    /**
     * Get skip count.
     */
    uint getSkipped() const;

    /**
     * Prepare for next test.
     */
    virtual void prepare(TestInstance & test);

    /**
     * Collect test statistics.
     */
    virtual void collect(TestResult & result);

    /**
     * Finish testing.
     */
    virtual void finish();

  protected:

    /**
     * Report start of a test.
     */
    virtual void reportBefore(TestInstance & test) = 0;

    /**
     * Report finish of a test.
     */
    virtual void reportAfter(TestResult & result) = 0;

    /**
     * Report completion of all tests.
     */
    virtual void reportFinish() = 0;

    /** Argument count */
    int m_argc;

    /** Argument values */
    char ** m_argv;

    /** Show final statistics. */
    bool m_showStatistics;

    /** Test statistics */
    uint m_ok, m_fail, m_skip;
};

#endif /* __LIBTEST_TESTREPORTER_H */

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

#ifndef __LIBTEST_TESTCASE_H
#define __LIBTEST_TESTCASE_H

/*
 * Testcases sometimes need to access private members
 * of a class. This define makes sure that a TestCase
 * can access the private members.
 */
#ifndef private
#define private public
#endif

/* Same for protected */
#ifndef protected
#define protected public
#endif

#include <stdio.h>
#include <List.h>
#include <Macros.h>

enum TestResult
{
    OK   = 0,
    FAIL = 1,
    SKIP = 2
};

#define TestCase(name) \
    TestResult name (void); \
    TestInstance instance_##name (QUOTE(name), name); \
    TestResult name (void)

typedef TestResult TestFunction(void);

class TestInstance
{
  public:

    TestInstance(const char *name, TestFunction func);

    TestResult run();

  private:

    const char *m_name;

    TestFunction *m_func;
};


/**
 * Test if the given expression is true and return NOK otherwise.
 *
 * @param expression The expression to test against
 * @return NOK only if the expression evaluates to false
 */
#define testAssert(expression) \
    if(!(expression)) \
    { \
	printf("%s:%d:%s testAssert failed: `%s' .. ", __FILE__, __LINE__,  __FUNCTION__, QUOTE(expression)); \
	return FAIL; \
    }

#endif /* __LIBTEST_TESTCASE_H */

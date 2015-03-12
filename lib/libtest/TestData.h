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

#ifndef __LIBTEST_TESTDATA_H
#define __LIBTEST_TESTDATA_H

#include <Types.h>
#include <Macros.h>
#include <Vector.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __HOST__
#include <sys/time.h>
#else
#include <FreeNOS/System/Function.h>
#endif /* __HOST__ */

/**
 * Generate test data for a certain data type.
 */
template <class T> class TestData
{
  public:

    /**
     * Constructor.
     */
    TestData()
    {
        // Collect seed from process id.
        pid_t pid = getpid();
        unsigned int seed = pid;

        // Collect seed from the current time
#ifdef __HOST__
        struct timeval tv;
        gettimeofday(&tv, NULL);
        seed += tv.tv_sec + tv.tv_usec;
#else
        seed += timestamp();
#endif
        // Seed the random generator
        srandom(seed);
    }

    /**
     * Retrieve previously random generated test data by index.
     *
     * @param index Index of the value to retrieve.
     * @return T value or ZERO if not found.
     */
    T get(int index)
    {
        return m_values[index];
    }

    /**
     * Get random test value by signed range.
     * The generated test value will have index number of the
     * previous test value + 1.
     *
     * @param max Maximum signed range of the value.
     * @param min Minimum signed range of the value.
     * @return T value.
     */
    virtual T value(long max = LONG_MAX, long min = 0);

    /**
     * Get random test value by unsigned range.
     * The generated test value will have index number of the
     * previous test value + 1.
     *
     * @param max Maximum signed range of the value.
     * @param min Minimum signed range of the value.
     * @return T value.
     */
    virtual T uvalue(ulong max = ULONG_MAX, ulong min = 0);

  private:

    /** Vector with generated values. */
    Vector<T> m_values;
};

template<> int TestData<int>::value(long max, long min)
{
    int value = (random() % max);

    if (value < min)
        value = min;

    m_values.insert(value);
    return value;
}

template<> int TestData<int>::uvalue(ulong max, ulong min)
{
    return 0;
}

template<> uint TestData<uint>::uvalue(ulong max, ulong min)
{
    uint value = (((unsigned)random() + (unsigned)random()) % max);

    if (value < min)
        value = min;

    m_values.insert(value);
    return value;
}

template<> uint TestData<uint>::value(long max, long min)
{
    return 0;
}

#endif /* __LIBTEST_TESTDATA_H */

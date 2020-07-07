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

#ifndef __LIBTEST_TESTCHAR_H
#define __LIBTEST_TESTCHAR_H

#include "TestData.h"
#include "TestInt.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libtest
 * @{
 */

/**
 * Generate test data for character strings.
 */
template <class T> class TestChar : public TestData<T>
{
  public:

    /**
     * Constructor.
     *
     * @param min Minimum length of the character strings.
     * @param max Maximum length of the character strings.
     */
    TestChar(Size min, Size max) : TestData<T>()
    {
        m_max = max;
        m_min = min;
    }

    /**
     * Destructor.
     */
    virtual ~TestChar()
    {
        for (Size i = 0; i < this->m_values.count(); i++)
            delete[] this->m_values[i];
    }

    /**
     * Get random test value(s).
     *
     * @param count Number or values to generate.
     *
     * @return The last generated T value.
     */
    virtual T random(Size count = 1);

    /**
     * Get unique random test value(s).
     *
     * @param count Number of unique values to generate.
     *
     * @return The last generated T value.
     */
    virtual T unique(Size count = 1);

    /**
     * Get the length of the generated character string.
     */
    virtual Size length(Size position)
    {
        return m_lengths[position];
    }

  private:

    /** Maximum value */
    Size m_max;

    /** Minimum value */
    Size m_min;

    /** String lengths */
    Vector<Size> m_lengths;
};

template<> char * TestChar<char *>::random(Size count)
{
    char * value = 0;

    while (count--)
    {
        TestInt<uint> sizes(m_min, m_max);
        TestInt<uint> chars(32, 126); /* ' ' till '~' */
        Size len = sizes.random();

        // Fill the buffer with random ASCII characters.
        value = new char[len + 1];
        for (Size i = 0; i < len; i++)
            value[i] = (char) chars.random();

        value[len] = ZERO;

        // Insert to administration
        m_values.insert(value);
        m_lengths.insert(len);
    }
    return value;
}

template<> char * TestChar<char *>::unique(Size count)
{
    const Size charMin = 32, charMax = 126;  /* ' ' till '~' */
    TestInt<uint> chars(charMin, charMax);
    char value[m_max + 1];

    // Clear string buffer
    MemoryBlock::set(value, 0, sizeof(value));

    // Generate 'count' unique strings using the prefix.
    for (Size i = 0; i < count; i++)
    {
        const Size len = (i + 1) < m_max ? (i + 1) : m_max;
        char *buf = new char[sizeof(value)];

        // String is filled by incrementing characters
        const Size numToAdd = i < m_min ? m_min : 1;

        for (Size j = 0; j < numToAdd; j++)
        {
            const char c = (char) ((i / (sizeof(value) - 1)) + charMin);
            value[i % (sizeof(value) - 1)] = c;
        }
        MemoryBlock::copy(buf, value, sizeof(value));

        // Add to administration
        m_values.insert(buf);
        m_lengths.insert(len);
    }

    return m_values[0];
}

/**
 * @}
 * @}
 */

#endif /* __LIBTEST_TESTCHAR_H */

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

class TestData
{
  public:

    TestData();

    int sint(int max = INT_MAX, int min = 0);
    
    unsigned uint(unsigned max = UINT_MAX, unsigned min = 0);
    
    Address address(Address max = ULONG_MAX, Address min = 0);
};

#endif /* __LIBTEST_TESTDATA_H */

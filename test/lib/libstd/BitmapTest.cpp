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
#include <BitMap.h>

TestCase(BitmapConstruct)
{
    BitMap *bm = new BitMap();

    testAssert(bm->m_map != NULL);
    testAssert(bm->m_size == bm->getFree());
    testAssert(bm->getUsed() == 0);

    return OK;
}

TestCase(BitmapMark)
{
    BitMap *bm = new BitMap();

    bm->mark(1);
    testAssert(bm->isMarked(1));
    testAssert(!bm->isMarked(0));

    return OK;
}


int main(int argc, char **argv)
{
    TestRunner tests(argc, argv);
    return tests.run();
}

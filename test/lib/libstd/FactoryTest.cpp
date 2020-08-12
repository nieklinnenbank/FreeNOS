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

#include <TestCase.h>
#include <TestRunner.h>
#include <TestMain.h>
#include <Types.h>
#include <Factory.h>

class MyTestClass : public Factory<MyTestClass>
{
  public:
    MyTestClass()
    : val(1234)
    {
    }

    u32 val;
};

class MyAbstractTestClass : public AbstractFactory<MyAbstractTestClass>
{
  public:
    MyAbstractTestClass()
    : val(4567)
    {
    }

    u32 val;
};

template<> MyAbstractTestClass * AbstractFactory<MyAbstractTestClass>::create()
{
    return new MyAbstractTestClass();
}

TestCase(FactoryCreate)
{
    MyTestClass *obj = MyTestClass::create();

    testAssert(obj != NULL);
    testAssert(obj->val == 1234);
    delete obj;

    return OK;
}

TestCase(AbstractFactoryCreate)
{
    MyAbstractTestClass *obj = MyAbstractTestClass::create();

    testAssert(obj != NULL);
    testAssert(obj->val == 4567);
    delete obj;

    return OK;
}

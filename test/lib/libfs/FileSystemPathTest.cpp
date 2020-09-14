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

#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <FileSystemPath.h>

TestCase(FileSystemPathConstruct)
{
    const String testpath("/mnt/path/to/my/file.txt");

    // Provide absolute path
    FileSystemPath path(*testpath);

    // Verify members
    testString(*path.base(), "file.txt");
    testString(*path.parent(), "/mnt/path/to/my");
    testString(*path.full(), *testpath);
    testAssert(path.split().count() == 5);
    testAssert(path.length() == testpath.length());

    // Provide empty path
    FileSystemPath empty("");
    testString(*empty.base(), "");
    testString(*empty.parent(), "");
    testString(*empty.full(), "");
    testAssert(empty.length() == 0);

    return OK;
}

TestCase(FileSystemPathSplit)
{
    const String testpath("/mnt/path/to/my/file.txt");

    FileSystemPath path(*testpath);
    const List<String> parts = path.split();
    const List<String>::Node *l = parts.head();

    testAssert(parts.count() == 5);
    testAssert(l && l->next);
    testString(*l->data, "mnt");

    l = l->next;
    testAssert(l && l->next);
    testString(*l->data, "path");

    l = l->next;
    testAssert(l && l->next);
    testString(*l->data, "to");

    l = l->next;
    testAssert(l && l->next);
    testString(*l->data, "my");

    l = l->next;
    testAssert(l && l->next == ZERO);
    testString(*l->data, "file.txt");

    return OK;
}

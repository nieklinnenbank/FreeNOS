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

#include <FreeNOS/Config.h>
#include <stdio.h>
#include "POSIXApplication.h"
#include "ApplicationLauncher.h"

POSIXApplication::POSIXApplication(int argc, char **argv)
    : Application(argc, argv)
{
    setVersion(VERSION);
}

POSIXApplication::~POSIXApplication()
{
}

POSIXApplication::Result POSIXApplication::output(const char *string) const
{
    printf("%s", string);
    return Success;
}

int POSIXApplication::runProgram(const char *path, const char **argv)
{
    ApplicationLauncher prog(path, argv);

    const ApplicationLauncher::Result result = prog.exec();
    if (result != ApplicationLauncher::Success)
    {
        return -1;
    }
    else
    {
        return (int) prog.getPid();
    }
}

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

#include <FreeNOS/Config.h>
#include <TestCase.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>
#include <ApplicationLauncher.h>

TestCase(RunMpiPing)
{
    // Launch mpiping program
    const char *args[] = { "/bin/mpiping", ZERO };
    ApplicationLauncher mpiping(TESTROOT "/bin/mpiping", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = mpiping.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Wait for program to finish
    const ApplicationLauncher::Result waitResult = mpiping.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(mpiping.getExitCode() == 0);

    // Done
    return OK;
}

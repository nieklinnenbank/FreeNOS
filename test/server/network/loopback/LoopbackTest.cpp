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
#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <ApplicationLauncher.h>
#include <NetworkServer.h>
#include <Loopback.h>

TestCase(LoopbackConstruct)
{
    NetworkServer server("/networktest/loopback");
    Loopback loop(server);

    return OK;
}

#ifndef __HOST__
TestCase(LoopbackArpPing)
{
    // Launch ping program
    const char *args[] = { "/bin/netping", "-a", "lo", "127.0.0.1", ZERO };
    ApplicationLauncher ping(TESTROOT "/bin/netping", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = ping.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Wait for program to finish
    const ApplicationLauncher::Result waitResult = ping.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(ping.getExitCode() == 0);

    // Done
    return OK;
}

TestCase(LoopbackIcmpPing)
{
    // Launch ping program
    const char *args[] = { "/bin/netping", "-i", "lo", "127.0.0.1", ZERO };
    ApplicationLauncher ping(TESTROOT "/bin/netping", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = ping.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Wait for program to finish
    const ApplicationLauncher::Result waitResult = ping.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(ping.getExitCode() == 0);

    // Done
    return OK;
}
#endif /* __HOST__ */

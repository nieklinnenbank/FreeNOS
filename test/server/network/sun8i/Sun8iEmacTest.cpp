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
#include <Ethernet.h>
#include <BufferedFile.h>
#include <ApplicationLauncher.h>

static bool isRunningOnQemu()
{
    const Ethernet::Address qemuAddr = { .addr = { 0x52, 0x54, 0x0, 0x12, 0x34, 0x56 }};
    BufferedFile ethernetAddr("/network/sun8i/ethernet/address");

    // Read address
    if (ethernetAddr.read() != BufferedFile::Success)
    {
        ERROR("failed to read ethernet address on " << ethernetAddr.path());
        return false;
    }

    return MemoryBlock::compare(&qemuAddr, ethernetAddr.buffer(), sizeof(Ethernet::Address));
}

TestCase(SunxiEmacDhcp)
{
    // Only run this test under Qemu
    if (!isRunningOnQemu())
    {
        return SKIP;
    }

    // Launch dhcp program
    const char *args[] = { "/bin/dhcpc", "sun8i", ZERO };
    ApplicationLauncher dhcpc(TESTROOT "/bin/dhcpc", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = dhcpc.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Wait for program to finish
    const ApplicationLauncher::Result waitResult = dhcpc.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(dhcpc.getExitCode() == 0);

    // Done
    return OK;
}

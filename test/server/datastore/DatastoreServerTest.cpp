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
#include <Types.h>
#include <ApplicationLauncher.h>
#include <DatastoreClient.h>

TestCase(DatastoreServerRegisterBuffer)
{
    const char *args[] = { "launcher", ZERO };
    ApplicationLauncher datastore(TESTROOT "/server/datastore/server", args);

    // Start the DatastoreServer
    const ApplicationLauncher::Result resultCode = datastore.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Connect client
    DatastoreClient datastoreClient(datastore.getPid());

    // Perform IPC
    void *bufptr = ZERO;
    const Datastore::Result result = datastoreClient.registerBuffer("mykey", &bufptr, 4096);
    testAssert(result == Datastore::Success);

    // Terminate the DatastoreServer
    const ApplicationLauncher::Result terminateResult = datastore.terminate();
    testAssert(terminateResult == ApplicationLauncher::Success);

    return OK;
}

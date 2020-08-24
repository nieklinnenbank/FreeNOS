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
#include <POSIXApplication.h>
#include <DatastoreClient.h>
#include <signal.h>

#ifndef __HOST__
#undef BUILDROOT
#define BUILDROOT
#endif /* __HOST__ */

/**
 * Helper class to launch the DatastoreServer program
 */
class DatastoreServerLauncher : public POSIXApplication
{
  private:

    /** Path to the DatastoreServer binary */
    static const char *DatastoreServerPath;

  public:

    /**
     * Constructor
     */
    DatastoreServerLauncher(int argc, char **argv)
        : POSIXApplication(argc, argv)
        , pid(0)
    {
    }

    /**
     * Execute the application.
     *
     * @return Result code
     */
    virtual Result exec()
    {
        const char *args[] = { (char *) "datastore", ZERO };
        int result = runProgram(DatastoreServerPath, args);
        if (result == -1)
        {
            FATAL("could not start DatastoreServer at " << DatastoreServerPath);
            return IOError;
        }

        pid = (ProcessID) result;
        return Success;
    }

    /**
     * Terminate the DatastoreServer.
     *
     * @return ZERO on success, non-zero on failure.
     */
    int terminate() const
    {
        return kill(pid, SIGTERM);
    }

    /** PID of the DatastoreServer. */
    ProcessID pid;
};

const char * DatastoreServerLauncher::DatastoreServerPath = BUILDROOT "/server/datastore/server";

TestCase(DatastoreServerRegisterBuffer)
{
    char *args[] = { (char *)"launcher", ZERO };
    DatastoreServerLauncher launcher(1, args);

    // Start the DatastoreServer
    int exitCode = launcher.run();
    testAssert(exitCode == 0);

    // Connect client
    DatastoreClient datastoreClient(launcher.pid);

    // Perform IPC
    void *bufptr = ZERO;
    const Datastore::Result result = datastoreClient.registerBuffer("mykey", &bufptr, 4096);
    testAssert(result == Datastore::Success);

    // Terminate the DatastoreServer
    int terminateResult = launcher.terminate();
    testAssert(terminateResult == 0);

    return OK;
}

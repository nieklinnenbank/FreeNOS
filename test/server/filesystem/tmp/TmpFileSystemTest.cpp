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
#include <FreeNOS/System.h>
#include <FreeNOS/ProcessManager.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <FileSystem.h>
#include <FileSystemPath.h>
#include <FileDescriptor.h>
#include <ApplicationLauncher.h>
#include <ProcessClient.h>
#include <RecoveryClient.h>
#include <FileSystemClient.h>

TestCase(TmpFileSystemReadWrite)
{
    const char *path = "/tmp/test.txt";
    const char *data = "testing 123 abc";
    const FileSystemClient fs;
    char buf[128];
    Size sz = sizeof(buf);
    Size descriptor = 3;
    FileSystem::FileStat st;

    // Attemp to read a non-existing file
    testAssert(fs.statFile(path, &st) == FileSystem::NotFound);
    testAssert(fs.openFile(path, descriptor) == FileSystem::NotFound);
    testAssert(fs.readFile(descriptor, buf, &sz) == FileSystem::NotFound);
    testAssert(fs.writeFile(descriptor, data, &sz) == FileSystem::NotFound);

    // Create new file
    testAssert(fs.createFile(path, FileSystem::RegularFile, FileSystem::OwnerRW) == FileSystem::Success);

    // File should now exist
    testAssert(fs.statFile(path, &st) == FileSystem::Success);
    testAssert(fs.openFile(path, descriptor) == FileSystem::Success);

    // Attemp to read the file again (empty file)
    testAssert(fs.readFile(descriptor, buf, &sz) == FileSystem::Success);
    testAssert(sz == 0);

    // Write content to the file
    sz = String::length(data);
    testAssert(fs.writeFile(descriptor, data, &sz) == FileSystem::Success);
    testAssert(sz == String::length(data));

    // Read the file data
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(descriptor);
    testAssert(fd != ZERO);
    fd->position = 0;
    testAssert(fs.readFile(descriptor, buf, &sz) == FileSystem::Success);
    testAssert(sz == String::length(data));
    buf[sz] = ZERO;
    testString(buf, data);

    // Delete the file
    testAssert(fs.deleteFile(path) == FileSystem::Success);

    // The file must be gone now
    testAssert(fs.statFile(path, &st) == FileSystem::NotFound);
    testAssert(fs.readFile(descriptor, buf, &sz) == FileSystem::NotFound);
    testAssert(fs.writeFile(descriptor, data, &sz) == FileSystem::NotFound);

    return OK;
}

TestCase(TmpFileSystemRestart)
{
    // Find PID first
    const ProcessID pid = ProcessClient().findProcess("/server/filesystem/tmp/server");
    if (pid == ANY)
    {
        return SKIP;
    }

    // Stop the server
    testAssert(ProcessCtl(pid, Stop) == API::Success);

    // Launch directory list program
    const char *args[] = { "/bin/ls", "-n", "/tmp", ZERO };
    ApplicationLauncher ls(TESTROOT "/bin/ls", args);

    // Start the program
    const ApplicationLauncher::Result resultCode = ls.exec();
    testAssert(resultCode == ApplicationLauncher::Success);

    // Restart the server
    const RecoveryClient recovery;
    testAssert(recovery.restartProcess(pid) == Recovery::Success);

    // Wait for list program to finish
    const ApplicationLauncher::Result waitResult = ls.wait();
    testAssert(waitResult == ApplicationLauncher::Success || waitResult == ApplicationLauncher::NotFound);
    testAssert(ls.getExitCode() == 0);

    // Done
    return OK;
}

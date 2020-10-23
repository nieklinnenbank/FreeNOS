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

#include <FreeNOS/System.h>
#include <FileSystemClient.h>
#include <FileSystemMount.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "Mount.h"

Mount::Mount(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("Mount filesystems on the system");
    parser().registerFlag('w', "wait", "Blocking wait until given filesystem path is mounted");
}

Mount::~Mount()
{
}

void Mount::listMounts() const
{
    const FileSystemClient filesystem;
    const FileSystemMount *mounts;
    const Arch::MemoryMap map;
    const Memory::Range range = map.range(MemoryMap::UserArgs);
    char cmd[PAGESIZE];
    Size numberOfMounts = 0;

    // Get mounted filesystems
    mounts = filesystem.getFileSystems(numberOfMounts);
    assert(mounts != NULL);

    // Print header
    printf("PATH                 FILESYSTEM\r\n");

    // Print out
    for (Size i = 0; i < numberOfMounts; i++)
    {
        if (mounts[i].path[0])
        {
            // Get the command
            const API::Result result = VMCopy(mounts[i].procID, API::Read, (Address) cmd, range.virt, PAGESIZE);
            if (result != API::Success)
            {
                ERROR("VMCopy failed for PID " << mounts[i].procID << ": result = " << (int) result);
                MemoryBlock::copy(cmd, "???", sizeof(cmd));
            }

            printf("%20s %s\r\n", mounts[i].path, cmd);
        }
    }
}

void Mount::waitForMount(const char *path) const
{
    const FileSystemClient filesystem;
    const FileSystem::Result result = filesystem.waitFileSystem(path);

    if (result != FileSystem::Success)
    {
        ERROR("failed to wait for filesystem at " << path << ": result = " << (int) result);
        exit(1);
    }

    assert(result == FileSystem::Success);
}

Mount::Result Mount::exec()
{
    const char *waitPath = arguments().get("wait") ?
                           arguments().get("wait") : ZERO;

    if (waitPath != ZERO)
        waitForMount(waitPath);
    else
        listMounts();

    return Success;
}

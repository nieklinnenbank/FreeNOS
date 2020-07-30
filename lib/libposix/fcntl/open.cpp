/*
 * Copyright (C) 2009 Niek Linnenbank
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
#include "Runtime.h"
#include "errno.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/stat.h"

int open(const char *path, int oflag, ...)
{
    FileSystemClient filesystem;
    FileDescriptor *files = getFiles();
    FileSystem::FileStat st;
    char fullpath[PATH_MAX];

    // Relative or absolute?
    if (path[0] != '/')
    {
        char cwd[PATH_MAX];

        // What's the current working dir?
        getcwd(cwd, PATH_MAX);
        snprintf(fullpath, sizeof(fullpath), "%s/%s", cwd, path);
    }
    else
        strlcpy(fullpath, path, sizeof(fullpath));

    // Ask the FileSystem for the file.
    if (files != NULL)
    {
        FileSystem::Result result = filesystem.statFile(fullpath, &st);

        // Refresh mounts and retry, in case the file did not exist
        if (result == FileSystem::NotFound)
        {
            filesystem.refreshMounts(0);
            result = filesystem.statFile(fullpath, &st);
        }

        // Set errno
        if (result == FileSystem::Success)
        {
            errno = ESUCCESS;

            // Insert into file descriptor table
            for (Size i = 0; i < FILE_DESCRIPTOR_MAX; i++)
            {
                if (!files[i].open)
                {
                    files[i].open  = true;
                    files[i].mount = filesystem.findMount(fullpath);
                    files[i].identifier = 0;
                    files[i].position = 0;
                    strlcpy(files[i].path, fullpath, PATH_MAX);
                    return i;
                }
            }
            // Too many open files
            errno = ENFILE;
        }
        else
        {
            errno = EIO;
        }
    }
    else
        errno = ENOENT;

    return -1;
}

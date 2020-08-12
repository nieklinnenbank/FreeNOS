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

#include <FreeNOS/User.h>
#include <FileSystemClient.h>
#include <FileDescriptor.h>
#include "limits.h"
#include "string.h"
#include "errno.h"
#include "fcntl.h"
#include "sys/stat.h"

int open(const char *path, int oflag, ...)
{
    const FileSystemClient filesystem;
    FileDescriptor *files = getFiles();
    FileSystem::FileStat st;

    // Ask the FileSystem for the file.
    if (files != NULL)
    {
        const FileSystem::Result result = filesystem.statFile(path, &st);

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
                    files[i].identifier = 0;
                    files[i].position = 0;
                    strlcpy(files[i].path, path, PATH_MAX);
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

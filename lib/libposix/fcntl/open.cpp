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
    FileSystem::FileStat st;

    // Ask the FileSystem for the file.
    const FileSystem::Result result = filesystem.statFile(path, &st);
    if (result == FileSystem::Success)
    {
        // Insert into file descriptor table
        Size idx = 0;
        const FileDescriptor::Result result = FileDescriptor::instance()->openEntry(path, idx);
        if (result != FileDescriptor::Success)
        {
            // Too many open files
            errno = ENFILE;
            return -1;
        }
        else
        {
            errno = ESUCCESS;
            return (int) idx;
        }
    }
    else
    {
        // File not found
        errno = ENOENT;
        return -1;
    }
}

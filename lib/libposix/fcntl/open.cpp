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
#include "limits.h"
#include "string.h"
#include "errno.h"
#include "fcntl.h"
#include "sys/stat.h"

int open(const char *path, int oflag, ...)
{
    const FileSystemClient filesystem;
    Size fd = 0;

    // Attempt to open the file
    const FileSystem::Result result = filesystem.openFile(path, fd);
    if (result == FileSystem::Success)
    {
        errno = ESUCCESS;
        return (int) fd;
    }
    else
    {
        // File not found
        errno = ENOENT;
        return -1;
    }
}

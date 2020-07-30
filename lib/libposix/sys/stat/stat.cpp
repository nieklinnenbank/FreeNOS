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

#include <FileSystemClient.h>
#include <Log.h>
#include "errno.h"
#include "limits.h"
#include "string.h"
#include "sys/stat.h"

int stat(const char *path, struct stat *buf)
{
    const FileSystemClient filesystem;
    FileSystem::FileStat st;

    DEBUG("path = " << path);

    // Ask the FileSystem for the information
    const FileSystem::Result result = filesystem.statFile(path, &st);

    // Copy information into buf
    switch (result)
    {
        case FileSystem::Success:
            buf->fromFileStat(&st);
            errno = ESUCCESS;
            break;
        case FileSystem::NotFound:
            errno = ENOENT;
            break;
        default:
            errno = EIO;
            break;
    }

    // Success
    return errno == ESUCCESS ? 0 : -1;
}

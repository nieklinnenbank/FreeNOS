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
#include "errno.h"
#include "unistd.h"

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    // Write the file.
    const FileSystemClient filesystem;
    const FileSystem::Result result = filesystem.writeFile(fildes,
                                                          (const char *)buf,
                                                          &nbyte);

    // Did the write succeed?
    if (result != FileSystem::Success)
    {
        errno = ENOENT;
        return -1;
    }

    return nbyte;
}

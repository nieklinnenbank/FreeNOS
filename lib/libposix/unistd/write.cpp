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
#include <FileDescriptor.h>
#include "errno.h"
#include "unistd.h"

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    // Do we have this file descriptor?
    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(fildes);
    if (!fd || !fd->open)
    {
        errno = ENOENT;
        return -1;
    }

    // Write the file.
    const FileSystemClient filesystem;
    const FileSystem::Result result = filesystem.writeFile(fd->path,
                                                          (const char *)buf,
                                                         &nbyte,
                                                           fd->position);

    // Did the write succeed?
    if (result != FileSystem::Success)
    {
        errno = EIO;
        return -1;
    }

    fd->position += nbyte;
    return nbyte;
}

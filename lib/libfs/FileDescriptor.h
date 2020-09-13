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

#ifndef __LIBPOSIX_FILEDESCRIPTOR_H
#define __LIBPOSIX_FILEDESCRIPTOR_H

#include <Types.h>
#include <Macros.h>
#include <String.h>
#include <MemoryBlock.h>
#include "FileSystemPath.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libposix
 * @{
 */

#define FILE_DESCRIPTOR_MAX 1024

/**
 * Abstracts a file which is opened by a user process.
 */
class FileDescriptor
{
  public:

    FileDescriptor()
    {
        path[0]  = ZERO;
        position = 0;
        open     = false;
    }

    FileDescriptor(const FileDescriptor & fd)
    {
        position = fd.position;
        open     = fd.open;
        MemoryBlock::copy(path, fd.path, FileSystemPath::MaximumLength);
    }

    bool operator == (const FileDescriptor & fd) const
    {
        const String str(path, false);
        return str.equals(fd.path);
    }

    bool operator != (const FileDescriptor & fd) const
    {
        const String str(path, false);
        return !(str.equals(fd.path));
    }

    /** Unique identifier, used by a device driver (minor device ID). */
    Address identifier;

    /** Path to the file. */
    char path[FileSystemPath::MaximumLength];

    /** Current position indicator. */
    Size position;

    /** State of the file descriptor. */
    bool open;
};

/**
 * Get File Descriptors table.
 *
 * @return FileDescriptor array pointer
 */
FileDescriptor * getFiles();

/**
 * Set a new FileDescriptor table.
 *
 * @param files FileDescriptor array pointer to use
 */
void setFiles(FileDescriptor *files);

/**
 * @}
 * @}
 */

#endif /* __LIBPOSIX_FILEDESCRIPTOR_H */

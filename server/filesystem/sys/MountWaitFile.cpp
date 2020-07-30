/*
 * Copyright (C) 2019 Niek Linnenbank
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
#include <FileSystemPath.h>
#include <string.h>
#include "MountWaitFile.h"

MountWaitFile::MountWaitFile()
    : File(FileSystem::RegularFile)
{
    m_access = FileSystem::OwnerRW;
}

MountWaitFile::~MountWaitFile()
{
}

Error MountWaitFile::write(IOBuffer & buffer, Size size, Size offset)
{
    char path[PATHLEN];
    Error r;

    // Check for input size
    if (size >= PATHLEN)
        return EIO;

    // Copy the input path
    if ((r = buffer.read(path, size)) <= 0)
        return r;

    // NULL-terminate
    path[size] = 0;

    // Trim the string
    String str(path);
    str.trim();

    // Search for a mounted filesystem with the given mount path
    FileSystemClient filesystem;
    Size numberOfMounts = 0;
    FileSystemMount *mounts = filesystem.getMounts(numberOfMounts);
    for (Size i = 0; i < numberOfMounts; i++)
    {
        if (mounts[i].path[0] && strcmp(mounts[i].path, *str) == 0)
            return size;
    }

    // Not yet available
    return EAGAIN;
}

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
#include <FileSystemMount.h>
#include <string.h>
#include "MountsFile.h"

MountsFile::MountsFile() : File(FileSystem::RegularFile)
{
    m_access = FileSystem::OwnerRW;

    FileSystemClient filesystem;
    Size numberOfMounts = 0;
    filesystem.getMounts(numberOfMounts);

    m_size = sizeof(FileSystemMount) * numberOfMounts;
}

MountsFile::~MountsFile()
{
}

FileSystem::Error MountsFile::read(IOBuffer & buffer, Size size, Size offset)
{
    FileSystemClient filesystem;
    Size numberOfMounts = 0;
    FileSystemMount *mounts = filesystem.getMounts(numberOfMounts);

    // Bounds checking
    if (offset > 0 || size < m_size)
        return FileSystem::InvalidArgument;

    // Copy the entire mounts table
    return buffer.write(mounts, m_size);
}

FileSystem::Error MountsFile::write(IOBuffer & buffer, Size size, Size offset)
{
    FileSystemMount fs;
    FileSystemClient filesystem;
    Size numberOfMounts = 0;
    FileSystemMount *mounts = filesystem.getMounts(numberOfMounts);
    FileSystem::Error r;

    // Input must be exactly one FileSystemMount struct
    if (size != sizeof(fs))
        return FileSystem::InvalidArgument;

    // Copy the input mount struct
    if ((r = buffer.read(&fs, sizeof(fs))) <= 0)
        return r;

    // Append to our filesystem mounts table
    for (Size i = 0; i < numberOfMounts; i++)
    {
        if (!mounts[i].path[0])
        {
            memcpy((void *)&mounts[i], &fs, sizeof(fs));
            NOTICE("mounted " << mounts[i].path);
            return size;
        }
    }

    // Mounts table is full
    return FileSystem::IOError;
}

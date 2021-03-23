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

#include "FileDescriptor.h"
#include "FileStorage.h"

FileStorage::FileStorage(const char *path, const Size offset)
    : m_path(path)
    , m_fd(0)
    , m_offset(offset)
{
}

FileSystem::Result FileStorage::initialize()
{
    const FileSystem::Result statResult = m_file.statFile(m_path, &m_stat);
    if (statResult != FileSystem::Success)
    {
        return statResult;
    }

    return m_file.openFile(m_path, m_fd);
}

FileSystem::Result FileStorage::read(const u64 offset, void *buffer, const Size size) const
{
    Size sz = size;

    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(m_fd);
    if (!fd || !fd->open)
    {
        return FileSystem::IOError;
    }

    // Update the file position pointer
    fd->position = m_offset + offset;

    return m_file.readFile(m_fd, buffer, &sz);
}

FileSystem::Result FileStorage::write(const u64 offset, void *buffer, const Size size)
{
    Size sz = size;

    FileDescriptor::Entry *fd = FileDescriptor::instance()->getEntry(m_fd);
    if (!fd || !fd->open)
    {
        return FileSystem::IOError;
    }

    // Update the file position pointer
    fd->position = m_offset + offset;

    return m_file.writeFile(m_fd, buffer, &sz);
}

u64 FileStorage::capacity() const
{
    return m_stat.size;
}

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

#include "FileStorage.h"

FileStorage::FileStorage(const char *path, const Size offset)
    : m_path(path)
    , m_offset(offset)
{
    m_file.statFile(path, &m_stat);
}

FileSystem::Error FileStorage::read(u64 offset, void *buffer, Size size)
{
    const FileSystem::Result result = m_file.readFile(m_path, buffer, &size, m_offset + offset);

    if (result == FileSystem::Success)
    {
        return size;
    }
    else
    {
        return result;
    }
}

FileSystem::Error FileStorage::write(u64 offset, void *buffer, Size size)
{
    const FileSystem::Result result = m_file.writeFile(m_path, buffer, &size, m_offset + offset);

    if (result == FileSystem::Success)
    {
        return size;
    }
    else
    {
        return result;
    }
}

u64 FileStorage::capacity() const
{
    return m_stat.size;
}

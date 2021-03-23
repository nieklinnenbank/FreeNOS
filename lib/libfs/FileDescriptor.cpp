/*
 * Copyright (C) 2020 Niek Linnenbank
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

FileDescriptor::FileDescriptor()
    : m_array(ZERO)
{
}

FileDescriptor::Entry * FileDescriptor::getArray(Size & count)
{
    count = m_count;
    return m_array;
}

void FileDescriptor::setArray(Entry *array,
                              const Size count)
{
    m_array = array;
    m_count = count;
}

FileDescriptor::Result FileDescriptor::openEntry(const u32 inode,
                                                 const ProcessID filesystem,
                                                 Size & index)
{
    // Insert into file descriptor table
    for (index = 0; index < m_count; index++)
    {
        if (!m_array[index].open)
        {
            m_array[index].open  = true;
            m_array[index].position = 0;
            m_array[index].inode = inode;
            m_array[index].pid = filesystem;
            return FileDescriptor::Success;
        }
    }

    return FileDescriptor::OutOfFiles;
}

FileDescriptor::Entry * FileDescriptor::getEntry(const Size index)
{
    if (index >= m_count)
    {
        return ZERO;
    }

    return &m_array[index];
}

FileDescriptor::Result FileDescriptor::closeEntry(const Size index)
{
    if (index >= m_count)
    {
        return FileDescriptor::InvalidArgument;
    }

    m_array[index].open = false;
    return FileDescriptor::Success;
}

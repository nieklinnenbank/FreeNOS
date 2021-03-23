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

#include <Log.h>
#include <Assert.h>
#include <Macros.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "BufferedFile.h"

BufferedFile::BufferedFile(const char *path)
    : m_path(path)
    , m_buffer(ZERO)
    , m_size(0)
{
}

BufferedFile::~BufferedFile()
{
    if (m_buffer != ZERO)
    {
        delete[] m_buffer;
    }
}

const char * BufferedFile::path() const
{
    return m_path;
}

const void * BufferedFile::buffer() const
{
    return m_buffer;
}

const Size BufferedFile::size() const
{
    return m_size;
}

BufferedFile::Result BufferedFile::read()
{
    struct stat st;
    int fp;

    // Retrieve file information
    if (::stat(m_path, &st) != 0)
    {
        ERROR("failed to stat input file " << m_path << ": " << strerror(errno));
        return NotFound;
    }
    m_size = st.st_size;

    // Open the file
    if ((fp = ::open(m_path, O_RDONLY)) == -1)
    {
        ERROR("failed to open input file " << m_path << ": " << strerror(errno));
        return IOError;
    }

    // (Re)allocate the internal buffer
    if (m_buffer != ZERO)
    {
        delete[] m_buffer;
    }
    m_buffer = new u8[m_size];
    assert(m_buffer != ZERO);

    // Read input file
    if (::read(fp, m_buffer, st.st_size) <= 0)
    {
        ERROR("failed to read input file " << m_path << ": " << strerror(errno));
        ::close(fp);
        return IOError;
    }

    // Done
    ::close(fp);
    return Success;
}

BufferedFile::Result BufferedFile::write(const void *data, const Size size) const
{
    int fp;

    // Open the file
    if ((fp = ::open(m_path, O_RDWR)) == -1)
    {
        ERROR("failed to open output file " << m_path << ": " << strerror(errno));
        return IOError;
    }

    // Write to the file
    if (::write(fp, data, size) <= 0)
    {
        ERROR("failed to write output file " << m_path << ": " << strerror(errno));
        ::close(fp);
        return IOError;
    }

    // Done
    ::close(fp);
    return Success;
}

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

#include <FreeNOS/User.h>
#include <Assert.h>
#include <MemoryBlock.h>
#include <CoreInfo.h>
#include "IOBuffer.h"

IOBuffer::IOBuffer()
    : m_message(ZERO)
    , m_directMapped(false)
    , m_buffer(ZERO)
    , m_size(0)
    , m_count(0)
{
}

IOBuffer::IOBuffer(const FileSystemMessage *msg)
    : m_message(msg)
    , m_directMapped(false)
    , m_buffer(ZERO)
    , m_size(0)
    , m_count(0)
{
    setMessage(msg);
}

IOBuffer::~IOBuffer()
{
    if (m_buffer)
    {
        if (m_directMapped)
        {
            const API::Result r = VMCtl(SELF, UnMap, &m_directMapRange);
            if (r != API::Success)
            {
                ERROR("failed to unmap remote buffer using VMCtl: result = " << (int) r);
                return;
            }
        }
        else
        {
            delete[] m_buffer;
        }
    }
}

void IOBuffer::setMessage(const FileSystemMessage *msg)
{
    if (msg->action == FileSystem::ReadFile || msg->action == FileSystem::WriteFile)
    {
        // If the remote buffer is page aligned, we can directly map it (unbuffered)
        if (!isKernel && !((const ulong) msg->buffer & ~PAGEMASK))
        {
            // Lookup the physical address
            m_directMapRange.virt = (Address) msg->buffer;
            const API::Result lookResult = VMCtl(msg->from, LookupVirtual, &m_directMapRange);
            if (lookResult != API::Success)
            {
                ERROR("failed to lookup remote buffer using VMCtl: result = " << (int) lookResult);
                return;
            }
            m_directMapRange.size   = msg->size;
            m_directMapRange.access = Memory::User | Memory::Readable | Memory::Writable;
            m_directMapRange.virt   = ZERO;

            // Map the remote buffer directly into our address space
            const API::Result mapResult = VMCtl(SELF, MapContiguous, &m_directMapRange);
            if (mapResult != API::Success)
            {
                ERROR("failed to map remote buffer using VMCtl: result = " << (int) mapResult);
                return;
            }
            m_directMapped = true;
            m_buffer = (u8 *) m_directMapRange.virt;
        }
        else
        {
            m_buffer = new u8[msg->size];
            assert(m_buffer != NULL);
        }
    }
    else
    {
        assert(m_buffer == NULL);
        m_buffer = 0;
    }

    m_message = msg;
    m_size   = msg->size;
    m_count  = 0;
}

Size IOBuffer::getCount() const
{
    return m_count;
}

void IOBuffer::addCount(const Size bytes)
{
    m_count += bytes;
    assert(m_count <= m_size);
}

const FileSystemMessage * IOBuffer::getMessage() const
{
    return m_message;
}

u8 * IOBuffer::getBuffer()
{
    return m_buffer;
}

FileSystem::Result IOBuffer::bufferedRead()
{
    if (!m_directMapped)
    {
        const FileSystem::Result result = read(m_buffer, m_message->size, 0);
        if (result != FileSystem::Success)
        {
            m_count = 0;
            return result;
        }
    }

    m_count = m_message->size;
    return FileSystem::Success;
}

FileSystem::Result IOBuffer::bufferedWrite(const void *buffer, const Size size)
{
    const Size num = m_count + size < m_size ? size : m_size - m_count;

    MemoryBlock::copy(m_buffer + m_count, buffer, num);
    m_count += num;

    return FileSystem::Success;
}

FileSystem::Result IOBuffer::read(void *buffer, const Size size, const Size offset)
{
    m_count = 0;

    if (m_directMapped)
    {
        MemoryBlock::copy(buffer, m_buffer + offset, size);
        return FileSystem::Success;
    }

    const API::Result result = VMCopy(m_message->from, API::Read,
                                     (Address) buffer,
                                     (Address) m_message->buffer + offset, size);
    if (result == API::Success)
    {
        return FileSystem::Success;
    }
    else
    {
        ERROR("VMCopy failed for PID " << m_message->from << ": result = " << (int) result);
        return FileSystem::IOError;
    }
}

FileSystem::Result IOBuffer::write(const void *buffer, const Size size, const Size offset)
{
    m_count = 0;

    if (m_directMapped)
    {
        MemoryBlock::copy(m_buffer + offset, buffer, size);
        return FileSystem::Success;
    }

    const API::Result result = VMCopy(m_message->from, API::Write,
                                     (Address) buffer,
                                     (Address) m_message->buffer + offset, size);
    if (result == API::Success)
    {
        return FileSystem::Success;
    }
    else
    {
        ERROR("VMCopy failed for PID " << m_message->from << ": result = " << (int) result);
        return FileSystem::IOError;
    }
}

FileSystem::Result IOBuffer::flushWrite()
{
    if (m_directMapped)
    {
        m_count = 0;
        return FileSystem::Success;
    }
    else
    {
        return write(m_buffer, m_count, 0);
    }
}

u8 IOBuffer::operator[](Size index) const
{
    return index < m_size ? m_buffer[index] : 0;
}

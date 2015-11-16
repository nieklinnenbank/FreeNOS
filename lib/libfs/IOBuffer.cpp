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

#include "IOBuffer.h"

IOBuffer::IOBuffer(const FileSystemMessage *msg)
    : m_message(msg)
{
    m_buffer = new u8[msg->size];
    m_size   = msg->size;
    m_count  = 0;
}

IOBuffer::~IOBuffer()
{
    delete m_buffer;
}

Size IOBuffer::getCount() const
{
    return m_count;
}

const u8 * IOBuffer::getBuffer() const
{
    return m_buffer;
}

Error IOBuffer::bufferedRead()
{
    return read(m_buffer, m_message->size, 0);
}

Error IOBuffer::bufferedWrite(void *buffer, Size size)
{
    Size i = 0;

    for (i = 0; i < size && m_count < m_size; i++)
    {
        m_buffer[m_count++] = ((u8 *)buffer)[i];
    }
    return i;
}
    
Error IOBuffer::read(void *buffer, Size size, Size offset) const
{
    return VMCopy(m_message->from, API::Read,
                 (Address) buffer,
                 (Address) m_message->buffer + offset, size);
}

Error IOBuffer::write(void *buffer, Size size, Size offset) const
{
    return VMCopy(m_message->from, API::Write,
                 (Address) buffer,
                 (Address) m_message->buffer + offset, size);
}

Error IOBuffer::flush()
{
    return write(m_buffer, m_count, 0);
}

u8 IOBuffer::operator[](Size index)
{
    return index < m_size ? m_buffer[index] : 0;
}

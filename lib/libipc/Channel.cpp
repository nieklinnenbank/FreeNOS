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

#include "Channel.h"

Channel::Channel(const Channel::Mode mode, const Size messageSize)
    : m_mode(mode)
    , m_messageSize(messageSize)
{
}

Channel::~Channel()
{
}

const Size Channel::getMessageSize() const
{
    return m_messageSize;
}

Channel::Result Channel::read(void *buffer)
{
    return NotSupported;
}

Channel::Result Channel::write(const void *buffer)
{
    return NotSupported;
}

Channel::Result Channel::flush()
{
    return NotSupported;
}

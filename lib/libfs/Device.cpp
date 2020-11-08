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

#include "Device.h"

Device::Device(const u32 inode,
               const FileSystem::FileType type)
    : File(inode, type)
{
}

Device::~Device()
{
}

const String & Device::getIdentifier() const
{
    return m_identifier;
}

FileSystem::Result Device::initialize()
{
    return FileSystem::Success;
}

FileSystem::Result Device::interrupt(const Size vector)
{
    return FileSystem::Success;
}

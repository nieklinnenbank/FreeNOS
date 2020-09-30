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

#include <FreeNOS/User.h>
#include <MemoryBlock.h>
#include "BootImageStorage.h"

BootImageStorage::BootImageStorage()
    : m_image(load())
{
}

const BootImage BootImageStorage::bootImage() const
{
    BootImage header;
    read(0, &header, sizeof(header));
    return header;
}

FileSystem::Error BootImageStorage::initialize()
{
    return m_image != ZERO ? FileSystem::Success : FileSystem::IOError;
}

FileSystem::Error BootImageStorage::read(const u64 offset, void *buffer, const Size size) const
{
    const u8 *data = ((const u8 *)(m_image)) + offset;
    MemoryBlock::copy(buffer, data, size);
    return size;
}

u64 BootImageStorage::capacity() const
{
    return m_image->bootImageSize;
}

const BootImage * BootImageStorage::load() const
{
    const SystemInformation info;
    Memory::Range range;

    // Request boot image memory
    range.size   = info.bootImageSize;
    range.access = Memory::User |
                   Memory::Readable;
    range.virt   = ZERO;
    range.phys   = info.bootImageAddress;

    // Map BootImage into our address space
    const API::Result r = VMCtl(SELF, MapContiguous, &range);
    if (r != API::Success)
    {
        ERROR("failed to map BootImage using VMCtl: result = " << (int) r);
        return ZERO;
    }

    return (const BootImage *) range.virt;
}

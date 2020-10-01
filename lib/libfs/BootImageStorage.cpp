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

BootImageStorage::BootImageStorage(const BootImage *image)
    : m_image(image != ZERO ? image : load())
{
}

const BootImage BootImageStorage::bootImage() const
{
    BootImage header;
    read(0, &header, sizeof(header));
    return header;
}

FileSystem::Result BootImageStorage::initialize()
{
    if (m_image == ZERO)
    {
        return FileSystem::IOError;
    }

    if (m_image->magic[0] == BOOTIMAGE_MAGIC0 &&
        m_image->magic[1] == BOOTIMAGE_MAGIC1 &&
        m_image->layoutRevision == BOOTIMAGE_REVISION)
    {
        return FileSystem::Success;
    }
    else
    {
        ERROR("invalid BootImage: signature = " <<
               m_image->magic[0] << ", " << m_image->magic[1] <<
              " revision = " << m_image->layoutRevision);
        return FileSystem::InvalidArgument;
    }
}

FileSystem::Result BootImageStorage::read(const u64 offset, void *buffer, const Size size) const
{
    const u8 *data = ((const u8 *)(m_image)) + offset;
    MemoryBlock::copy(buffer, data, size);
    return FileSystem::Success;
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

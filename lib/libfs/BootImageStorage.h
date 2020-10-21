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

#ifndef __LIB_LIBFS_BOOTIMAGESTORAGE_H
#define __LIB_LIBFS_BOOTIMAGESTORAGE_H

#include <Types.h>
#include <String.h>
#include <BootImage.h>
#include "FileSystem.h"
#include "Storage.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Uses a BootImage as a storage provider.
 */
class BootImageStorage : public Storage
{
  public:

    /**
     * Constructor function.
     *
     * @param image Pointer to mapped BootImage or ZERO to map via kernel
     */
    BootImageStorage(const BootImage *image = ZERO);

    /**
     * Get BootImage header.
     *
     * @return BootImage header.
     */
    const BootImage bootImage() const;

    /**
     * Initialize the Storage device
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Reads data from the boot image.
     *
     * @param offset Offset to start reading from.
     * @param buffer Output buffer.
     * @param size Number of bytes to copied.
     *
     * @return Result code
     */
    virtual FileSystem::Result read(const u64 offset, void *buffer, const Size size) const;

    /**
     * Retrieve maximum storage capacity.
     *
     * @return Storage capacity.
     */
    virtual u64 capacity() const;

  private:

    /**
     * Loads the BootImage into virtual memory.
     *
     * @return Pointer to the BootImage or ZERO on failure
     */
    const BootImage * load() const;

  private:

    /** Pointer to the BootImage */
    const BootImage *m_image;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_BOOTIMAGESTORAGE_H */

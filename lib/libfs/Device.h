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

#ifndef __LIB_LIBFS_DEVICE_H
#define __LIB_LIBFS_DEVICE_H

#include <String.h>
#include "File.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Abstract device class interface.
 */
class Device : public File
{
  public:

    /**
     * Constructor.
     *
     * @param inode Inode number for this Device
     * @param type Type of file
     */
    Device(const u32 inode,
           const FileSystem::FileType type);

    /**
     * Destructor.
     */
    virtual ~Device();

    /**
     * Get unique device identifier.
     *
     * @return String object
     */
    virtual const String & getIdentifier() const;

    /**
     * Initialize the device
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Called when an interrupt has been triggered for this device.
     *
     * @param vector Vector number of the interrupt.
     *
     * @return Result code
     */
    virtual FileSystem::Result interrupt(const Size vector);

  protected:

    /** Unique identifier for this Device. */
    String m_identifier;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_DEVICE_H */

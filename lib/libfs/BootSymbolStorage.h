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

#ifndef __LIB_LIBFS_BOOTSYMBOLSTORAGE_H
#define __LIB_LIBFS_BOOTSYMBOLSTORAGE_H

#include <Types.h>
#include <String.h>
#include <BootImage.h>
#include "FileSystem.h"
#include "Storage.h"
#include "BootImageStorage.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Uses a BootSymbol inside a BootImage as a storage provider.
 */
class BootSymbolStorage : public Storage
{
  public:

    /**
     * Constructor function.
     *
     * @param bootImage BootImageStorage to use for I/O.
     * @param symbolName Name of the BootSymbol entry to use.
     */
    BootSymbolStorage(const BootImageStorage &bootImage,
                      const char *symbolName);

    /**
     * Initialize the Storage device
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Reads data from the BootSymbol.
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
     * Loads the BootSymbol from the BootImage.
     *
     * @return BootSymbol value
     */
    const BootSymbol loadSymbol(const char *name) const;

    /**
     * Load the BootSegment for the given BootSymbol.
     *
     * @param symbol BootSymbol reference
     *
     * @return BootSegment value
     */
    const BootSegment loadSegment(const BootSymbol &symbol) const;

  private:

    /** Read-only reference to the BootImage storage */
    const BootImageStorage &m_bootImage;

    /** BootSymbol value */
    const BootSymbol m_symbol;

    /** BootSegment value */
    const BootSegment m_segment;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_BOOTSYMBOLSTORAGE_H */

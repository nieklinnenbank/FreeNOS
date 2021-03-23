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

#ifndef __FILESYSTEM_LINN_DIRECTORY_H
#define __FILESYSTEM_LINN_DIRECTORY_H
#ifndef __HOST__

#include <Directory.h>
#include <Types.h>
#include "LinnDirectoryEntry.h"
#include "LinnFileSystem.h"
#include "LinnInode.h"
#include "IOBuffer.h"

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup linnfs
 * @{
 */

/**
 * Represents an directory on a LinnFS filesystem.
 *
 * @see Directory
 * @see LinnDirectoryEntry
 * @see LinnFileSystem
 */
class LinnDirectory : public Directory
{
  public:

    /**
     * Constructor function.
     *
     * @param fs Filesystem pointer.
     * @param inode Inode number
     * @param inodeData Inode data pointer.
     *
     * @see LinnFileSystem
     * @see LinnInode
     */
    LinnDirectory(LinnFileSystem *fs,
                  const u32 inode,
                  LinnInode *inodeData);

    /**
     * Read directory entries
     *
     * @param buffer Input/Output buffer to output bytes to.
     * @param size Maximum number of bytes to read on input.
     *             On output, the actual number of bytes read.
     * @param offset Offset inside the file to start reading.
     *
     * @return Result code
     */
    virtual FileSystem::Result read(IOBuffer & buffer,
                                    Size & size,
                                    const Size offset);

    /**
     * @brief Retrieves a File pointer for the given entry name.
     *
     * This function reads a file from disk corresponding
     * to the LinnInode of the given entry name. It returns
     * an File object associated with that LinnInode.
     *
     * @param name Name of the entry to lookup.
     *
     * @return Pointer to a File if found, or ZERO otherwise.
     *
     * @see File
     * @see LinnInode
     */
    virtual File * lookup(const char *name);

  private:

    /**
     * Retrieve a directory entry.
     *
     * @param dent LinnDirectoryEntry buffer pointer.
     * @param name Unique name of the entry.
     *
     * @return True if successful, false otherwise.
     */
    bool getLinnDirectoryEntry(LinnDirectoryEntry *dent,
                               const char *name);

  private:

    /** Filesystem pointer. */
    LinnFileSystem *m_fs;

    /** Inode which describes the directory. */
    LinnInode *m_inodeData;
};

/**
 * @}
 * @}
 */

#endif /* __HOST__ */
#endif /* __FILESYSTEM_EXT2DIRECTORY_H */

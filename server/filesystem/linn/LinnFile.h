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

#ifndef __SERVER_FILESYSTEM_LINN_LINNFILE_H
#define __SERVER_FILESYSTEM_LINN_LINNFILE_H

#include <File.h>
#include <Types.h>
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
 * Represents a file on a mounted LinnFS filesystem.
 */
class LinnFile : public File
{
  public:

    /**
     * Constructor function.
     *
     * @param fs LinnFS filesystem pointer.
     * @param inode Inode number
     * @param inodeData Inode data pointer.
     */
    LinnFile(LinnFileSystem *fs,
             const u32 inode,
             LinnInode *inodeData);

    /**
     * Destructor function.
     */
    virtual ~LinnFile();

    /**
     * @brief Read bytes from the file.
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

  private:

    /** Filesystem pointer. */
    LinnFileSystem *m_fs;

    /** Inode pointer. */
    LinnInode *m_inodeData;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_FILESYSTEM_LINN_LINNFILE_H */

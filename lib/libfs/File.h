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

#ifndef __LIB_LIBFS_FILE_H
#define __LIB_LIBFS_FILE_H

#include <Types.h>
#include "FileSystemMessage.h"
#include "FileSystem.h"
#include "IOBuffer.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Represents a file present on a FileSystem.
 *
 * @see FileSystem
 */
class File
{
  public:

    /**
     * Constructor function.
     *
     * @param inode The inode number of this file.
     * @param type Type of file.
     * @param uid User identity.
     * @param gid Group identity.
     */
    File(const u32 inode,
         const FileSystem::FileType type = FileSystem::RegularFile,
         const UserID uid = ZERO,
         const GroupID gid = ZERO);

    /**
     * Destructor function.
     */
    virtual ~File();

    /**
     * Get inode number
     *
     * @return Inode number
     */
    u32 getInode() const;

    /**
     * Retrieve our filetype.
     *
     * @return FileType object.
     */
    FileSystem::FileType getType() const;

    /**
     * Read bytes from the file
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
     * Write bytes to the file
     *
     * @param buffer Input/Output buffer to input bytes from.
     * @param size Maximum number of bytes to write on input.
     *             On output, the actual number of bytes written.
     * @param offset Offset inside the file to start writing.
     *
     * @return Result code
     */
    virtual FileSystem::Result write(IOBuffer & buffer,
                                     Size & size,
                                     const Size offset);

    /**
     * Retrieve file statistics.
     *
     * @param st File statistics output struct
     *
     * @return Result code
     */
    virtual FileSystem::Result status(FileSystem::FileStat &st);

    /**
     * Check if the File has data ready for reading.
     *
     * When this function returns true, it can be read without blocking.
     *
     * @return Boolean
     */
    virtual bool canRead() const;

    /**
     * Check if the File can be written to.
     *
     * When this function returns true, it can be written without blocking.
     *
     * @return Boolean
     */
    virtual bool canWrite() const;

  protected:

    /** Inode number */
    const u32 m_inode;

    /** Type of this file. */
    const FileSystem::FileType m_type;

    /** Owner of the file. */
    UserID m_uid;

    /** Group of the file. */
    GroupID m_gid;

    /** Access permissions. */
    FileSystem::FileModes m_access;

    /** Size of the file, in bytes. */
    Size m_size;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILE_H */

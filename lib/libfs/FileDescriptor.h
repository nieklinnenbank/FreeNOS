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

#ifndef __LIB_LIBFS_FILEDESCRIPTOR_H
#define __LIB_LIBFS_FILEDESCRIPTOR_H

#include <Types.h>
#include <Singleton.h>
#include "FileSystemPath.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Abstracts files which are opened by a user process.
 */
class FileDescriptor : public StrictSingleton<FileDescriptor>
{
  public:

    /** Default maximum number of files which can be opened */
    static const Size MaximumFiles = 1024;

    /**
     * Describes a single file opened by a user process
     */
    struct Entry
    {
        u32 inode;     /**@< Inode number of the file */
        ProcessID pid; /**@< Process identifier of the filesystem */
        Size position; /**@< Current position indicator. */
        bool open;     /**@< State of the file descriptor. */
    };

    /**
     * Result code
     */
    enum Result
    {
        Success,
        InvalidArgument,
        OutOfFiles
    };

  public:

    /**
     * Constructor
     */
    FileDescriptor();

    /**
     * Get entry table
     *
     * @param count Maximum number of entries
     *
     * @return Entry table pointer
     */
    Entry * getArray(Size & count);

    /**
     * Assign entry table
     *
     * @param array Pointer to array with file descriptor entries
     * @param count Number of Entry structures in the array
     */
    void setArray(Entry *array,
                  const Size count);

    /**
     * Add new file descriptor entry
     *
     * @param inode Inode number of the file to add
     * @param filesystem Process identifier of the filesystem
     * @param index On output contains the index number for the entry
     *
     * @return Result code
     */
    Result openEntry(const u32 inode,
                     const ProcessID filesystem,
                     Size & index);

    /**
     * Retrieve a file descriptor Entry
     *
     * @param index Index in the array of entries
     *
     * @return Pointer to the Entry on success or ZERO on failure
     */
    Entry * getEntry(const Size index);

    /**
     * Remove file descriptor entry
     *
     * @param index Index in the array of entries
     *
     * @return Result code
     */
    Result closeEntry(const Size index);

  private:

    /** Pointer to array of entries */
    Entry *m_array;

    /** Number of entries in the array */
    Size m_count;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILEDESCRIPTOR_H */

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

#ifndef __LIB_LIBFS_FILESYSTEMCLIENT_H
#define __LIB_LIBFS_FILESYSTEMCLIENT_H

#include <FreeNOS/API/ProcessID.h>
#include <Types.h>
#include <Memory.h>
#include "FileSystem.h"

class FileSystemMessage;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * FileSystemClient provides a simple interface to a FileSystemServer.
 *
 * @see FileSystemServer
 */
class FileSystemClient
{
  public:

    /**
     * Class constructor function.
     *
     * @param pid Optional ProcessID of the target file system. If set to ANY
     *            the cached mounts table will be used to lookup the ProcessID.
     */
    FileSystemClient(const ProcessID pid = ANY);

    /**
     * Create a new file.
     *
     * @param path Path to the file
     * @param type File type of the new file
     * @param mode Initial access permissions
     * @param dev Optional device number specifier
     *
     * @return Result code
     */
    FileSystem::Result createFile(const char *path,
                                  const FileSystem::FileType type,
                                  const FileSystem::FileModes mode,
                                  const DeviceID deviceId) const;

    /**
     * Read a file.
     *
     * @param path Path to the file
     * @param buf Buffer for storing bytes read.
     * @param size On input, number of bytes to read. On output, actual bytes read.
     * @param offset Specifies absolute starting point in bytes to read.
     *
     * @return Result code
     */
    FileSystem::Result readFile(const char *path,
                                void *buf,
                                Size *size,
                                const Size offset) const;

    /**
     * Write a file.
     *
     * @param path Path to the file
     * @param buf Input buffer for bytes to write.
     * @param size On input, number of bytes to write. On output, actual bytes written.
     * @param offset Specifies absolute starting point in bytes to write.
     *
     * @return Result code
     */
    FileSystem::Result writeFile(const char *path,
                                 const void *buf,
                                 Size *size,
                                 const Size offset) const;

    /**
     * Retrieve status of a file.
     *
     * @param path Path to the file
     * @param st Output buffer for the file status
     *
     * @return Result code
     */
    FileSystem::Result statFile(const char *path, FileSystem::FileStat *st) const;

    /**
     * Remove a file from the file system.
     *
     * @param path Path to the file
     *
     * @return Result code
     */
    FileSystem::Result deleteFile(const char *path) const;

  private:

    /**
     * Send an IPC request to the target file system
     *
     * @param path Path to the file
     * @param msg Reference to the FileSystemMessage to send
     *
     * @return Result code
     */
    FileSystem::Result request(const char *path, FileSystemMessage &msg) const;

  private:

    /** ProcessID of the target file system or ANY to lookup in mounts table */
    const ProcessID m_pid;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILESYSTEMCLIENT_H */

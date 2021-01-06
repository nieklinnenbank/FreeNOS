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
#include "FileSystemMount.h"

struct FileSystemMessage;

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
  private:

    /** Maximum number of mounted filesystems. */
    static const Size MaximumFileSystemMounts = 16;

  public:

    /**
     * Class constructor function.
     *
     * @param pid Optional ProcessID of the target file system. If set to ANY
     *            the cached mounts table will be used to lookup the ProcessID.
     */
    FileSystemClient(const ProcessID pid = ANY);

    /**
     * Get current directory String.
     *
     * @return String pointer containing current directory
     */
    const String * getCurrentDirectory() const;

    /**
     * Set new current directory.
     *
     * @param directory String reference to use as current directory.
     */
    void setCurrentDirectory(const String &directory);

    /**
     * Set new current directory.
     *
     * @param directory String to use as current directory.
     *
     * @note If the current directory was already set before, the internal String
     *       object will be overwritten using the value given by the String input.
     */
    void setCurrentDirectory(String *directory);

    /**
     * Create a new file.
     *
     * @param path Path to the file
     * @param type File type of the new file
     * @param mode Initial access permissions
     *
     * @return Result code
     */
    FileSystem::Result createFile(const char *path,
                                  const FileSystem::FileType type,
                                  const FileSystem::FileModes mode) const;

    /**
     * Retrieve status of a file.
     *
     * @param path Path to the file
     * @param st Output buffer for the file status
     *
     * @return Result code
     */
    FileSystem::Result statFile(const char *path,
                                FileSystem::FileStat *st) const;

    /**
     * Open a file
     *
     * @param path Path to the file
     * @param descriptor Outputs the file descriptor number
     *
     * @return Result code
     */
    FileSystem::Result openFile(const char *path,
                                Size & descriptor) const;

    /**
     * Close a file
     *
     * @param descriptor File descriptor number
     *
     * @return Result code
     */
    FileSystem::Result closeFile(const Size descriptor) const;

    /**
     * Read a file.
     *
     * @param descriptor File descriptor number of the file
     * @param buf Buffer for storing bytes read.
     * @param size On input, number of bytes to read. On output, actual bytes read.
     *
     * @return Result code
     */
    FileSystem::Result readFile(const Size descriptor,
                                void *buf,
                                Size *size) const;

    /**
     * Write a file.
     *
     * @param descriptor File descriptor number of the file
     * @param buf Input buffer for bytes to write.
     * @param size On input, number of bytes to write. On output, actual bytes written.
     *
     * @return Result code
     */
    FileSystem::Result writeFile(const Size descriptor,
                                 const void *buf,
                                 Size *size) const;

    /**
     * Remove a file from the file system.
     *
     * @param path Path to the file
     *
     * @return Result code
     */
    FileSystem::Result deleteFile(const char *path) const;

    /**
     * Wait for one or more files to become readable/writable
     *
     * @param filesystemPath Path to the destination mounted filesystem
     * @param waitSet Pointer to a WaitSet array
     * @param count Number of WaitSet entries
     * @param msecTimeout Timeout in milliseconds of the wait or ZERO for infinite wait
     *
     * @return Result code
     */
    FileSystem::Result waitFile(const char *filesystemPath,
                                const FileSystem::WaitSet *waitSet,
                                const Size count,
                                const Size msecTimeout) const;

    /**
     * Mount the current process as a file system on the rootfs.
     *
     * @param mountPath Absolute path for the mount point to use.
     *
     * @return Result code
     */
    FileSystem::Result mountFileSystem(const char *mountPath) const;

    /**
     * Blocking wait for a mounted filesystem
     *
     * @param path Full path of the mounted filesystem
     *
     * @return Result code
     *
     * @note Blocks until a filesystem is mounted on the exact given input path
     */
    FileSystem::Result waitFileSystem(const char *path) const;

    /**
     * Get file system mounts table.
     *
     * @param numberOfMounts Number of entries in the returned array
     *
     * @return FileSystemMount array pointer or NULL on failure
     */
    FileSystemMount * getFileSystems(Size &numberOfMounts) const;

  private:

    /**
     * Send an IPC request to the target file system
     *
     * @param path Path to the file, can be relative or absolute.
     * @param msg Reference to the FileSystemMessage to send
     *
     * @return Result code
     */
    FileSystem::Result request(const char *path, FileSystemMessage &msg) const;

    /**
     * Send an IPC request to the target file system
     *
     * @param pid Process identifier of the target file system.
     * @param msg Reference to the FileSystemMessage to send
     *
     * @return Result code
     */
    FileSystem::Result request(const ProcessID pid, FileSystemMessage &msg) const;

    /**
     * Retrieve the ProcessID of the FileSystemMount for the given path.
     *
     * @param path Path to lookup.
     *
     * @return ProcessID of the FileSystemMount on success and ZERO otherwise.
     */
    ProcessID findMount(const char *path) const;

  private:

    /** FileSystem mounts table */
    static FileSystemMount m_mounts[MaximumFileSystemMounts];

    /** Current directory path is prefixed to relative path inputs */
    static String *m_currentDirectory;

    /** ProcessID of the target file system or ANY to lookup in mounts table */
    const ProcessID m_pid;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILESYSTEMCLIENT_H */

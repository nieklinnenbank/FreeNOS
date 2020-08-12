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

#ifndef __LIB_LIBFS_FILESYSTEMSERVER_H
#define __LIB_LIBFS_FILESYSTEMSERVER_H

#include <ChannelServer.h>
#include <Vector.h>
#include "Directory.h"
#include "Device.h"
#include "File.h"
#include "FileCache.h"
#include "FileSystem.h"
#include "FileSystemPath.h"
#include "FileSystemMessage.h"
#include "FileSystemRequest.h"
#include "FileSystemMount.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Abstract filesystem class.
 */
class FileSystemServer : public ChannelServer<FileSystemServer, FileSystemMessage>
{
  private:

    /** Maximum number of supported file system mount entries */
    static const Size MaximumFileSystemMounts = 32;

  public:

    /**
     * Constructor function.
     *
     * @param p Path to which we are mounted.
     */
    FileSystemServer(const char *path);

    /**
     * Destructor function.
     */
    virtual ~FileSystemServer();

    /**
     * Get mount path
     *
     * @return Mount path of the filesystem
     */
    const char * getMountPath() const;

    /**
     * Get root directory.
     *
     * @return Root directory pointer
     */
    Directory * getRoot();

    /**
     * Mount the FileSystem.
     *
     * This function is responsible for mounting the file system.
     *
     * @return Result code
     */
    FileSystem::Result mount();

    /**
     * Register a new File.
     *
     * @param file File object pointer.
     * @param path The path for the File.
     *
     * @return Result code
     */
    FileSystem::Result registerFile(File *file, const char *path);

    /**
     * Create a new file.
     *
     * @param type Describes the type of file to create.
     * @param deviceID Optionally specifies the device identities to create.
     *
     * @return Pointer to a new File on success or ZERO on failure.
     */
    virtual File * createFile(FileSystem::FileType type, DeviceID deviceID);

    /**
     * Inserts a file into the in-memory filesystem tree.
     *
     * @param file File to insert.
     * @param pathFormat Full path to the file to insert.
     *
     * @return Pointer to the newly created FileCache, or NULL on failure.
     */
    FileCache * insertFileCache(File *file, const char *pathFormat);

    /**
     * Process an incoming filesystem request using a path.
     *
     * This message handler is responsible for processing any
     * kind of FileSystemMessages which have an FileSystemAction using
     * the path field, such as OpenFile.
     *
     * @param msg Incoming request message.
     * @see FileSystemMessage
     * @see FileSystemAction
     */
    void pathHandler(FileSystemMessage *msg);

    /**
     * Process a filesystem mount request message.
     *
     * @param msg FileSystemMessage pointer with the requested mount path
     */
    void mountHandler(FileSystemMessage *msg);

    /**
     * Read the file system mounts table.
     *
     * @param msg FileSystemMessage pointer
     */
    void getFileSystemsHandler(FileSystemMessage *msg);

    /**
     * Retry any pending requests
     *
     * @return True if retry is needed again, false if all requests processed
     */
    virtual bool retryRequests();

  protected:

    /**
     * Process a FileSystemRequest.
     *
     * @return Result code, where RetryAgain indicates the request cannot
     *         be completed yet.
     */
    FileSystem::Result processRequest(FileSystemRequest &req);

    /**
     * Send response for a FileSystemMessage
     *
     * @param msg The FileSystemMessage to send response for
     */
    void sendResponse(FileSystemMessage *msg);

    /**
     * Try to forward the given FileSystemMessage to a mount file system.
     *
     * @param path Path to access in the request
     * @param msg FileSystemMessage pointer
     *
     * @return True if redirected, false otherwise
     */
    bool redirectRequest(const char *path, FileSystemMessage *msg);

    /**
     * Change the filesystem root directory.
     *
     * This function set the root member to the given
     * Directory pointer. Additionally, it inserts '/.' and '/..'
     * references to the file cache.
     *
     * @param newRoot A Directory pointer to set as the new root.
     *
     * @see root
     * @see insertFileCache
     */
    void setRoot(Directory *newRoot);

    /**
     * Retrieve a File from storage.
     *
     * This function is responsible for walking the
     * given FileSystemPath, retrieving each uncached File into
     * the FileCache, and returning a pointer to corresponding FileCache
     * of the last entry in the given path.
     *
     * @param path A path to lookup from storage.
     *
     * @return Pointer to a FileCache on success, ZERO otherwise.
     */
    FileCache * lookupFile(FileSystemPath *path);

    /**
     * Search the cache for an entry.
     *
     * @param path Full path of the file to find.
     *
     * @return Pointer to FileCache object on success, NULL on failure.
     */
    FileCache * findFileCache(char *path);

    /**
     * Search the cache for an entry.
     *
     * @param path Full path of the file to find.
     *
     * @return Pointer to FileCache object on success, NULL on failure.
     */
    FileCache * findFileCache(String *path);

    /**
     * Search the cache for an entry.
     *
     * @param path Full path of the file to find.
     *
     * @return Pointer to FileCache object on success, NULL on failure.
     */
    FileCache * findFileCache(FileSystemPath *p);

    /**
     * Process a cache hit.
     *
     * @param cache FileCache object which has just been referenced.
     *
     * @return FileCache object pointer.
     */
    virtual FileCache * cacheHit(FileCache *cache);

    /**
     * Cleans up the entire file cache (except opened file caches and root).
     *
     * @param cache Input FileCache object. ZERO to clean up all from root.
     */
    void clearFileCache(FileCache *cache = ZERO);

  protected:

    /** Process identifier */
    const ProcessID m_pid;

    /** Root entry of the filesystem tree. */
    FileCache *m_root;

    /** Mount point path. */
    const char *m_mountPath;

    /** Table with mounted file systems (only used by the root file system). */
    FileSystemMount *m_mounts;

    /** Contains ongoing requests */
    List<FileSystemRequest *> *m_requests;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILESYSTEMSERVER_H */

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

#ifndef __LIB_LIBFS_FILESYSTEM_H
#define __LIB_LIBFS_FILESYSTEM_H

#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

namespace FileSystem
{
    /**
     * Actions which may be performed on a filesystem.
     */
    enum Action
    {
        CreateFile = 0,
        ReadFile,
        WriteFile,
        StatFile,
        DeleteFile,
        WaitFile,
        MountFileSystem,
        WaitFileSystem,
        GetFileSystems
    };

    /**
     * Result code for filesystem Actions.
     */
    enum Result
    {
        Success = 0,
        InvalidArgument,
        NotFound,
        RetryAgain,
        IOError,
        PermissionDenied,
        AlreadyExists,
        NotSupported,
        RedirectRequest,
        IpcError,
        TimedOut
    };

    /**
     * All possible filetypes.
     */
    enum FileType
    {
        RegularFile         = 0,
        DirectoryFile       = 1,
        BlockDeviceFile     = 2,
        CharacterDeviceFile = 3,
        SymlinkFile         = 4,
        FIFOFile            = 5,
        SocketFile          = 6,
        UnknownFile         = 7,
    };

    /**
     * File access permissions.
     */
    enum FileMode
    {
        OwnerR   = 0400,
        OwnerW   = 0200,
        OwnerX   = 0100,
        OwnerRW  = 0600,
        OwnerRX  = 0500,
        OwnerRWX = 0700,
        GroupR   = 0040,
        GroupW   = 0020,
        GroupX   = 0010,
        GroupRW  = 0060,
        GroupRX  = 0050,
        GroupRWX = 0070,
        OtherR   = 0004,
        OtherW   = 0002,
        OtherX   = 0001,
        OtherRW  = 0006,
        OtherRX  = 0005,
        OtherRWX = 0007,
    };

    /** Multiple FileMode values combined. */
    typedef u16 FileModes;

    /**
     * Contains file information.
     */
    struct FileStat
    {
        FileType type;      /**@< File type. */
        u32 inode;          /**@< Inode number */
        ProcessID pid;      /**@< Process identifier of filesystem */
        FileModes access;   /**@< File access permission bits. */
        Size size;          /**@< Size of the file in bytes. */
        UserID userID;      /**@< User identity. */
        GroupID groupID;    /**@< Group identity. */
    };

    /**
     * Provides information about an inode
     */
    struct WaitSet
    {
        u32 inode;     /**@< Inode number */
        u16 requested; /**@< Requested status flags of the inode */
        u16 current;   /**@< Indicates the currently active status flags */
    };

    /**
     * WaitSet status flags
     */
    enum WaitStatus
    {
        Readable = (1 << 0), /**@< File can be read without blocking */
        Writable = (1 << 1)  /**@< File can be written without blocking */
    };
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILESYSTEM_H */

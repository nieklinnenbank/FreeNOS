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

/** Number of bits needed to store a FileType. */
#define FILETYPE_BITS   3

/** Masker value for all FileTypes. */
#define FILETYPE_MASK   7

/**
 * Convert from a (host system's) POSIX struct stat into a FileType.
 *
 * @param st struct stat pointer.
 *
 * @return FileType value.
 */
#define FILETYPE_FROM_ST(st) \
({ \
    FileSystem::FileType t = FileSystem::UnknownFile; \
    \
    switch ((st)->st_mode & S_IFMT) \
    { \
        case S_IFBLK:  t = FileSystem::BlockDeviceFile; break; \
        case S_IFCHR:  t = FileSystem::CharacterDeviceFile; break; \
        case S_IFIFO:  t = FileSystem::FIFOFile; break; \
        case S_IFREG:  t = FileSystem::RegularFile; break; \
        case S_IFDIR:  t = FileSystem::DirectoryFile; break; \
        case S_IFLNK:  t = FileSystem::SymlinkFile; break; \
        case S_IFSOCK: t = FileSystem::SocketFile; break; \
        default: break; \
    } \
    t; \
})

/** Number of bits required for all FileModes. */
#define FILEMODE_BITS 9

/** Masker value for all FileMode values. */
#define FILEMODE_MASK 0777

/**
 * Converts an (host system's) POSIX struct st into a FileMode.
 *
 * @param st struct st pointer.
 *
 * @return FileMode value.
 */
#define FILEMODE_FROM_ST(st) \
    (FileSystem::FileMode)((st)->st_mode & FILEMODE_MASK)

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
        MountFileSystem,
        WaitFileSystem,
        GetFileSystems
    };

    /**
     * Result code for filesystem Actions.
     */
    enum Result
    {
        Success          =  0,
        InvalidArgument  = -1,
        NotFound         = -2,
        RetryAgain       = -3,
        IOError          = -4,
        PermissionDenied = -5,
        AlreadyExists    = -6,
        NotSupported     = -7,
        RedirectRequest  = -8,
        IpcError         = -9
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
        FileType type;      /**< File type. */
        FileModes access;   /**< File access permission bits. */
        Size size;          /**< Size of the file in bytes. */
        UserID userID;      /**< User identity. */
        GroupID groupID;    /**< Group identity. */
    };
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILESYSTEM_H */

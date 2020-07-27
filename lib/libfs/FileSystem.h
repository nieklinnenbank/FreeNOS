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
#include "FileMode.h"

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
 * @param st struct stat pointer.
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
        DeleteFile
    };

    /**
     * Result code for filesystem Actions.
     */
    enum Result
    {
        Success,
        InvalidArgument,
        NotFound,
        RetryAgain,
        IOError,
        PermissionDenied,
        AlreadyExists,
        NotSupported
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
     * Contains file information.
     */
    struct FileStat
    {
        FileType type;      /**< File type. */
        FileModes access;   /**< File access permission bits. */
        Size size;          /**< Size of the file in bytes. */
        UserID userID;      /**< User identity. */
        GroupID groupID;    /**< Group identity. */
        DeviceID deviceID;  /**< Device identity. */
    };
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_FILESYSTEM_H */

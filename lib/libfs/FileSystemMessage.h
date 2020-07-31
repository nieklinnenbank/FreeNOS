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

#ifndef __FILESYSTEM_FILE_SYSTEM_MESSAGE_H
#define __FILESYSTEM_FILE_SYSTEM_MESSAGE_H

#include <FreeNOS/System.h>
#include <Types.h>
#include "ChannelMessage.h"
#include "FileSystem.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * FileSystem IPC message.
 */
typedef struct FileSystemMessage : public ChannelMessage
{
    /** Action to perform. */
    FileSystem::Action action;

    /** Result code. */
    FileSystem::Result result;

    /** Points to a buffer for I/O. */
    char *buffer;

    /** Size of the buffer. */
    Size size;

    /** Offset in the file for I/O. */
    Size offset;

    /** Path name of the file. */
    char *path;

    /** User ID and group ID. */
    u16 userID, groupID;

    /** Filetype. */
    FileSystem::FileType filetype;

    /** File mode permissions. */
    FileSystem::FileModes mode;

    /** File Statistics. */
    FileSystem::FileStat *stat;

    /** Device major/minor numbers. */
    DeviceID deviceID;
}
FileSystemMessage;

/**
 * @}
 * @}
 */

#endif /* __FILESYSTEM_FILE_SYSTEM_MESSAGE_H */

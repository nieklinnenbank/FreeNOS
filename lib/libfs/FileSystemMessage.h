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

#include <Types.h>
#include <Timer.h>
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
    FileSystem::Action action;     /**< Action to perform. */
    FileSystem::Result result;     /**< Result code. */
    char *buffer;                  /**< Points to a buffer for I/O. */
    Size size;                     /**< Size of the buffer. */
    Size offset;                   /**< Offset in the file for I/O. */
    u32 inode;                     /**< Inode number of the file */
    FileSystem::FileStat *stat;    /**< File Statistics. */
    Timer::Info timeout;           /**< Timeout value for the action */
    ProcessID pid;                 /**< Process identifier (used for redirection) */
    Size pathMountLength;          /**< Length of the mounted path (used for redirection) */
}
FileSystemMessage;

/**
 * @}
 * @}
 */

#endif /* __FILESYSTEM_FILE_SYSTEM_MESSAGE_H */

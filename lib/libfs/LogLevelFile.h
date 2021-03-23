/*
 * Copyright (C) 2021 Niek Linnenbank
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

#ifndef __LIB_LIBFS_LOGLEVELFILE_H
#define __LIB_LIBFS_LOGLEVELFILE_H

#include <Types.h>
#include "File.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * Provides a File abstraction of the current Log::Level
 */
class LogLevelFile : public File
{
  public:

    /**
     * Default constructor.
     *
     * @param inode Inode number for this File
     */
    LogLevelFile(const u32 inode);

    /**
     * Destructor.
     */
    virtual ~LogLevelFile();

    /**
     * @brief Read bytes from the file.
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
     * Write bytes to the file.
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
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_LOGLEVELFILE_H */

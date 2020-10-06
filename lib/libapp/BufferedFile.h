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

#ifndef __LIB_LIBAPP_BUFFEREDFILE_H
#define __LIB_LIBAPP_BUFFEREDFILE_H

#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libapp
 * @{
 */

/**
 * Provides a buffered abstract interface to a file.
 */
class BufferedFile
{
  public:

    /**
     * Result codes
     */
    enum Result
    {
        Success,
        NotFound,
        IOError,
    };

  public:

    /**
     * Constructor
     */
    BufferedFile(const char *path);

    /**
     * Destructor
     */
    ~BufferedFile();

    /**
     * Get file path
     *
     * @return File path pointer
     */
    const char * path() const;

    /**
     * Get file buffer
     *
     * @return File buffer
     */
    const void * buffer() const;

    /**
     * Get file size
     *
     * @return File size in bytes
     */
    const Size size() const;

    /**
     * Read the file (buffered)
     *
     * @return Result code
     */
    Result read();

    /**
     * Write the file (unbuffered)
     *
     * @note Does not update the internal buffer
     *
     * @param data Input data to write
     * @param size Number of bytes to write
     *
     * @return Result code
     */
    Result write(const void *data, const Size size) const;

  private:

    /** Path to the file */
    const char *m_path;

    /** Stored contents of the file */
    u8 *m_buffer;

    /** Size of the file in bytes */
    Size m_size;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBAPP_BUFFEREDFILE_H */

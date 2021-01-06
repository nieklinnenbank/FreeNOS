/*
 * Copyright (C) 2015 Niek Linnenbank
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

#ifndef __LIB_LIBFS_IOBUFFER_H
#define __LIB_LIBFS_IOBUFFER_H

#include <Types.h>
#include <Macros.h>
#include <Memory.h>
#include "FileSystemMessage.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libfs
 * @{
 */

/**
 * @brief Abstract Input/Output buffer.
 */
class IOBuffer
{
  public:

    /**
     * Default empty constructor
     */
    IOBuffer();

    /**
     * Constructor.
     *
     * @param msg Describes the request being processed.
     */
    IOBuffer(const FileSystemMessage *msg);

    /**
     * Destructor.
     */
    virtual ~IOBuffer();

    /**
     * Get byte count.
     *
     * @return Byte count of the internal buffer.
     */
    Size getCount() const;

    /**
     * Increment byte counter
     *
     * @param bytes Number of bytes to increment
     */
    void addCount(const Size bytes);

    /**
     * Set filesystem message
     *
     * @param msg FileSystemMessage pointer
     *
     * @todo Only allow direct-mapping if the remote buffer size is a multiple of PAGESIZE.
     *       If the size isnt a full page, the rest of the page might contain other data.
     */
    void setMessage(const FileSystemMessage *msg);

    /**
     * Get filesystem message.
     *
     * @return FileSystemMessage pointer.
     */
    const FileSystemMessage * getMessage() const;

    /**
     * Get raw buffer.
     *
     * @return Buffer pointer.
     */
    u8 * getBuffer();

    /**
     * @brief Read bytes from the I/O buffer.
     *
     * @param buffer Copy bytes from the I/O buffer to this memory address.
     * @param size Number of bytes to copy.
     * @param offset The offset inside the I/O buffer to start reading.
     *
     * @return Result code
     */
    FileSystem::Result read(void *buffer,
                            const Size size,
                            const Size offset = ZERO);

    /**
     * Write bytes to the I/O buffer.
     *
     * @param buffer Contains the bytes to write.
     * @param size Number of bytes to write.
     * @param offset The offset inside the I/O buffer to start writing.
     *
     * @return Result code
     */
    FileSystem::Result write(const void *buffer,
                             const Size size,
                             const Size offset = ZERO);

    /**
     * Buffered read bytes from the I/O buffer.
     *
     * @return Result code
     */
    FileSystem::Result bufferedRead();

    /**
     * Buffered write bytes to the I/O buffer.
     *
     * @param buffer Contains the bytes to write.
     * @param size Number of bytes to write.
     *
     * @return Result code
     */
    FileSystem::Result bufferedWrite(const void *buffer,
                                     const Size size);

    /**
     * Flush write buffers.
     *
     * @return Result code
     */
    FileSystem::Result flushWrite();

    /**
     * Byte index operator.
     *
     * @param index Index value
     *
     * @return Byte value at the given index or 0 if index is invalid.
     */
    u8 operator[] (Size index) const;

  private:

    /**
     * @brief Current request being processed.
     *
     * Read() and write() will use fields from the current request
     * to fill in arguments for VMCopy().
     *
     * @see VMCopy
     * @see IOBuffer::read
     * @see IOBuffer::write
     */
    const FileSystemMessage *m_message;

    /** True if using directly memory-mapped memory (unbuffered) */
    bool m_directMapped;

    /** Contains the memory address range of the direct memory mapping */
    Memory::Range m_directMapRange;

    /** Buffer for storing temporary data. */
    u8 *m_buffer;

    /** Buffer size. */
    Size m_size;

    /** Bytes written to the buffer. */
    Size m_count;
};

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBFS_IOBUFFER_H */

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

#ifndef __SERVER_VIDEO_VGA_H
#define __SERVER_VIDEO_VGA_H

#include <FreeNOS/System.h>
#include <DeviceServer.h>
#include <Types.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup video
 * @{
 */

/** VGA physical video memory address. */
#define VGA_PADDR (0xb8000)

/** VGA I/O address port. */
#define VGA_IOADDR 0x3d4

/** VGA I/O data port. */
#define VGA_IODATA 0x3d5

/**
 * Encodes VGA attributes.
 *
 * @param front Front text color.
 * @param back Background color.
 */
#define VGA_ATTR(front,back) \
    (((back & 0xf) << 4) | ((front & 0xf)))

/**
 * Encodes a character for VGA output.
 *
 * @param ch Character to encode.
 * @param front Front text color.
 * @param back Background color.
 */
#define VGA_CHAR(ch,front,back) \
    ((VGA_ATTR(front,back) << 8) | (ch & 0xff))

/**
 * @brief VGA color attributes.
 */
enum Colors
{
    BLACK        = 0,
    BLUE         = 1,
    GREEN        = 2,
    CYAN         = 3,
    RED          = 4,
    MAGENTA      = 5,
    BROWN        = 6,
    LIGHTGREY    = 7,
    DARKGREY     = 8,
    LIGHTBLUE    = 9,
    LIGHTGREEN   = 10,
    LIGHTCYAN    = 11,
    LIGHTRED     = 12,
    LIGHTMAGENTA = 13,
    LIGHTBROWN   = 14,
    WHITE        = 15,
};

/**
 * @brief Video Graphics Array (VGA) support.
 *
 * This class implements a VGA driver. Applications may read and write
 * from the /dev/vga0 device file to retrieve and modify the current screen.
 * Currently the Terminal driver uses the /dev/vga device file to implement
 * the system console in FreeNOS.
 *
 * @see Terminal
 */
class VGA : public Device
{
    public:

    /**
     * @brief Class constructor function.
     *
     * @param inode Inode number
     * @param width Number of characters horizontally.
     * @param height Number of characters vertically.
     */
    VGA(const u32 inode,
        const Size width = 80,
        const Size height = 25);

    /**
     * Initialize the VGA device.
     *
     * First this function maps the VGA video memory
     * into our address space. It clears the screen
     * and finally disables the VGA hardware cursor.
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Read video memory
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
     * Write video memory
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

  private:

    /** @brief VGA video memory address. */
    u16 *vga;

    /** @brief Number of characters horizontally. */
    Size width;

    /** @brief Number of characters vertically. */
    Size height;

    /** Port I/O object. */
    Arch::IO m_io;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_VIDEO_VGA_H */

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

#ifndef __SERVER_PS2_KEYBOARD_H
#define __SERVER_PS2_KEYBOARD_H

#include <FreeNOS/System.h>
#include <Device.h>
#include <Macros.h>
#include <Types.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup ps2
 * @{
 */

/** PS2 Keyboard input port. */
#define PS2_PORT        0x60

/** Interrupt number of the i8042 controller. */
#define PS2_IRQ         1

/** Bit is set in the scancode, if a key is released. */
#define PS2_RELEASE     0x80

/**
 * @brief PS2 Keyboard device driver.
 */
class Keyboard : public Device
{
  public:

    /**
     * Constructor
     *
     * @param inode Inode number
     */
    Keyboard(const u32 inode);

    /**
     * Initialize the device
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Executed when a key state has changed.
     *
     * @param vector Interrupt vector.
     *
     * @return Result code
     */
    virtual FileSystem::Result interrupt(const Size vector);

    /**
     * Read a character from the keyboard
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

  private:

    /**
     * @brief Keyboard map table.
     */
    static const char keymap[0x3a][2];

    /**
     * @brief State of the shift key.
     *
     * Non-zero if pressed, and ZERO otherwise.
     */
    u8 shiftState;

    /** Do we have a byte ready? */
    bool pending;

    /** Port I/O object. */
    Arch::IO m_io;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_PS2_KEYBOARD_H */

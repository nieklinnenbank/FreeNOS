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

#ifndef __TERMINAL_KEYBOARD_H
#define __TERMINAL_KEYBOARD_H

#include <Macros.h>
#include <Types.h>
#include <Error.h>

/** PS2 Keyboard input port. */
#define PS2_PORT        0x60

/** Interrupt number of the i8042 controller. */
#define PS2_IRQ         1

/** Bit is set in the scancode, if a key is released. */
#define PS2_RELEASE     0x80 

/** Size of the keyboard buffer. */
#define KEYBUF		1024

/**
 * PS2 Keyboard functionality.
 */
typedef struct Keyboard
{
    /**
     * Constructor.
     */
    Keyboard();

    /**
     * Reads a character from the keyboard into keyboardBuffer.
     * @return True if we read a valid character, false otherwise.
     * @see keyboardBuffer
     */
    bool flush();

    /** Keyboard mapping. */
    static const char keymap[0x3a][2];

    /** Buffers all read operations. */
    s8 keyboardBuffer[KEYBUF];

    /** Amount of bytes in the keyboardBuffer. */
    Size readBytes;
    
    /** State of the shift key. */
    u8 shiftState;
}
Keyboard;

#endif /* __TERMINAL_KEYBOARD_H */

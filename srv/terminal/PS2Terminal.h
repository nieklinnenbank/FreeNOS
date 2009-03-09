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

#ifndef __TERMINAL_PS2TERMINAL_H
#define __TERMINAL_PS2TERMINAL_H

#include <Macros.h>
#include <Types.h>
#include "Terminal.h"

#include "VGATerminal.h"

/** PS2 Keyboard input port. */
#define PS2_PORT	0x60

/** Interrupt number of the i8042 controller. */
#define PS2_IRQ		1

/** Bit is set in the scancode, if a key is released. */
#define PS2_RELEASE	0x80

/**
 * PS2 keyboard controller (i8042) support.
 */
class PS2Terminal : public Terminal
{
    public:

	/**
	 * Constructor function.
	 */
	PS2Terminal();

	/** 
	 * Read a character from the PS2 keyboard.
	 * @param buffer Buffer to save the byte.
	 * @param size Number of bytes to read.
	 * @return Number of bytes on success and ZERO on failure. 
	 */
	int read(s8 *buffer, Size size);
	
    private:
    
	VGATerminal *vga;
    
	/** Keyboard mapping. */
	static const char keymap[], shiftmap[];
	
	/** Current state of the shift key. */
	u8 shiftState;
};

#endif /* __TERMINAL_PS2TERMINAL_H */

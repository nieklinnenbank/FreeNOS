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

#ifndef __TERMINAL_VGA_H
#define __TERMINAL_VGA_H

#include <Macros.h>
#include <Types.h>
#include <Error.h>
#include <teken.h>

/** VGA physical video memory address. */
#define VGA_PADDR (0xb8000) 

/** VGA virtual video memory address. */
#define VGA_VADDR (0x70000000) 

/** VGA I/O address port. */
#define VGA_IOADDR 0x3d4

/** VGA I/O data port. */
#define VGA_IODATA 0x3d5

/**
 * Encodes VGA attributes.
 * @param front Front text color.
 * @param back Background color.
 */
#define VGA_ATTR(front,back) \
    (((back & 0xf) << 4) | ((front & 0xf)))

/**
 * Encodes a character for VGA output.
 * @param ch Character to encode.
 * @param front Front text color.
 * @param back Background color.
 */
#define VGA_CHAR(ch,front,back) \
    ((VGA_ATTR(front,back) << 8) | (ch & 0xff))

/**
 * VGA color attributes.
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
 * Video Graphics Array (VGA) screen.
 */
class VGA
{
    public:
    
    /**
     * Class constructor.
     * @param videoMem VGA video memory.
     * @param width Width of the VGA screen.
     * @param height Height of the VGA screen.
     */
    VGA(u16 *vgaMem, Size width = 80, Size height = 25);

    /**
     * Class destructor.
     */
    ~VGA();

    /**
     * Retrieve the currently active video buffer.
     * @return Active video buffer.
     */
    u16 * getBuffer()
    {
	return videoMemory;
    }

    /**
     * Get the width of the Terminal.
     * @return Terminal width.
     */
    Size getWidth()
    {
        return width;
    }
	
    /**
     * Get the height of the Terminal.
     * @return Terminal height.
     */
    Size getHeight()
    {
        return height;
    }

    /**
     * Return the last known position of the cursor.
     * @return Last position of the cursor.
     */
    teken_pos_t * getCursor()
    {
        return &cursorPos;
    }

    /**
     * Saved byte and attribute value at cursor position.
     * @return Saved value.
     */
    u16 * getCursorValue()
    {
        return &cursorValue;
    }

    /**
     * Hides the cursor from the VGA screen.
     */
    void hideCursor();
    
    /**
     * Sets the new position of the cursor.
     * @param pos New position coordinates.
     */
    void setCursor(const teken_pos_t *pos);
    
    /**
     * Show the VGA cursor.
     */
    void showCursor();

    protected:

    /** VGA screen size. */
    Size width, height;
	
    /** Saved cursor position. */
    teken_pos_t cursorPos;
	
    /** Saved value at cursor position. */
    u16 cursorValue;

    /** Physical and local (cached) VGA video memory. */
    u16 *vgaMemory, *localMemory;
	
    /** Used instead of videoMemory if not active. */
    u16 *videoMemory;
};

#endif /* __TERMINAL_VGA_H */

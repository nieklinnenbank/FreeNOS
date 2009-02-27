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

#ifndef __TERMINAL_VGATERMINAL_H
#define __TERMINAL_VGATERMINAL_H

#include <Macros.h>
#include <Types.h>
#include "Terminal.h"

/**
 * Video Graphics Array Terminal.
 */
class VGATerminal : public Terminal
{
    public:

	/**
	 * Class constructor.
	 */
	VGATerminal();

	/**
	 * Class destructor.
	 */
	~VGATerminal();

	/**
	 * Clears the entire screen.
	 */
	void clear();

	/**
	 * Write bytes to the VGA memory.
	 * @param buffer Buffer containing bytes to write.
	 * @param size Number of bytes to write.
	 * @return Number of bytes on success and ZERO on failure.
	 */	
	int write(s8 *buffer, Size size);
	
    private:
    
	/**
	 * Move the 'cursor' to the next line.
	 */
	void newline();
    
	/** Video Memory. **/
	u16 *mem;
	
	/** X-coordinate. */
	int x;
	
	/** Y-coordinate. */
	int y;
	
	/** TERMINAL width. */
	int width;
	
	/** TERMINAL height. */
	int height;
};

#endif /* __TERMINAL_VGATERMINAL_H */

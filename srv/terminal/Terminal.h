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

#ifndef __TERMINAL_TERMINAL_H
#define __TERMINAL_TERMINAL_H

#include <Macros.h>
#include <Types.h>
#include <Error.h>
#include <teken.h>
#include <FileSystemMessage.h>
#include "VGA.h"
#include "Keyboard.h"

/**
 * A Terminal is the combination of a VGA screen and PS2 keyboard.
 */
class Terminal : public VGA, public Keyboard
{
    public:

	/**
	 * Class constructor.
	 */
	Terminal();

	/**
	 * Class destructor.
	 */
	~Terminal();

        /**
         * Switches the currently active buffer to VGA.
         * Makes the VGA video memory the currently active buffer for
	 * output operations. The contents of the localMemory buffer are
	 * copied to the vgaMemory buffer.
         * @see videoMemory
	 * @see vgaMemory
         * @see localMemory
         */
	void activate();

	/**
	 * Switches the currently active buffer to local memory.
	 * The contents of the VGA video memory are copies to the
	 * localMemory buffer before switching.
	 * @see videoMemory
	 * @see vgaMemory
	 * @see localMemory
	 */
	void deactivate();

	/**
	 * Read bytes from the Terminal (keyboard buffer).
	 * @param buffer Output buffer.
	 * @param size Number of bytes to read.
	 * @return Number of bytes read.
	 */
	Size read(char *buffer, Size size);

	/**
	 * Write bytes to the Terminal (vga memory).
	 * @param buffer Contains the bytes to write.
	 * @param size Number of bytes to write.
	 */
	void write(char *buffer, Size size);
	
	/**
	 * Retrieve the request being processed.
	 * @return Current request for this Terminal.
	 */
	FileSystemMessage * getRequest()
	{
	    return requestActive ? &request : ZERO;
	}
	
	/**
	 * Sets the request currently being processed.
	 * @param msg Request message.
	 */
	void setRequest(FileSystemMessage *msg)
	{
	    if (msg)
	    {
		request       = msg;
		requestActive = true;
	    }
	    else
		requestActive = false;
	}

    private:

	/** Terminal state. */
	teken_t state;

	/** Terminal function handlers. */
	teken_funcs_t funcs;
	
	/** Process which owns this terminal. */
	ProcessID owner;
	
	/** Request to be processed by the Terminal. */
	FileSystemMessage request;
	
	/** Do we have an active request? */
	bool requestActive;
};

/**
 * Makes a sound (bell).
 * @param term Terminal object pointer.
 */
void bell(Terminal *term);

/**
 * Output a new character.
 * @param term Terminal object pointer.
 * @param pos Terminal position.
 * @param ch Character to output.
 * @param attr Attributes for this character.
 */
void putchar(Terminal *term, const teken_pos_t *pos,
	     teken_char_t ch, const teken_attr_t *attr);
	
/**
 * Sets the Terminal cursor.
 * @param term Terminal object pointer.
 * @param pos Position to put the cursor.
 */
void cursor(Terminal *term, const teken_pos_t *pos);

/**
 * Fills the Terminal buffer with a character.
 * @param term Terminal object pointer.
 * @param rect Indicates where to fill.
 * @param ch Character to be used for filling.
 * @param attr Attributes for this character.
 */
void fill(Terminal *ctx, const teken_rect_t *rect,
	  teken_char_t ch, const teken_attr_t *attr);

/**
 * Copy VGA memory.
 * @param ctx Terminal object pointer.
 * @param rect Source rectagular area with VGA memory buffer.
 * @param pos Position to copy the rect to.
 */
void copy(Terminal *ctx, const teken_rect_t *rect,
	  const teken_pos_t *pos);

/**
 * Set terminal parameters.
 * @param ctx Terminal object pointer.
 * @param key Option key identifier.
 * @param value New value for the option.
 */
void param(Terminal *ctx, int key, int value);

/**
 * ???
 */
void respond(Terminal *ctx, const void *buf, size_t size);

#endif /* __TERMINAL_TERMINAL_H */

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

#include <Types.h>
#include <string.h>
#include "Terminal.h"

Terminal::Terminal()
    : VGA((u16 *) VGA_VADDR), requestActive(false)
{
    teken_pos_t winsz;

    /* Fill in function pointers. */
    funcs.tf_bell    = (tf_bell_t *)    bell;
    funcs.tf_cursor  = (tf_cursor_t *)  cursor;
    funcs.tf_putchar = (tf_putchar_t *) putchar;
    funcs.tf_fill    = (tf_fill_t *)    fill;
    funcs.tf_copy    = (tf_copy_t *)    copy;
    funcs.tf_param   = (tf_param_t *)   param;
    funcs.tf_respond = (tf_respond_t *) respond;

    /* Initialize libteken. */
    teken_init(&state, &funcs, this);
    
    /* Set appropriate terminal sizes. */
    winsz.tp_col = width;
    winsz.tp_row = height;
    teken_set_winsize(&state, &winsz);
}

Terminal::~Terminal()
{
    delete localMemory;
}

void Terminal::activate()
{
    memcpy(vgaMemory, localMemory, width * height * sizeof(u16));
    videoMemory = vgaMemory;
}

Size Terminal::read(char *buffer, Size size)
{
    Size num = readBytes < size ? readBytes : size;

    if (num)
    {
	memcpy(buffer, keyboardBuffer, num);
        readBytes -= num;
    }
    return num;
}

void Terminal::write(char *buffer, Size size)
{
    char cr  = '\r';

    for (Size i = 0; i < size; i++)
    {
	if (*buffer == '\n')
	{
	    teken_input(&state, &cr, 1);
	}
	teken_input(&state, buffer++, 1);
    }
}

void bell(Terminal *term)
{
}

void putchar(Terminal *term, const teken_pos_t *pos,
             teken_char_t ch, const teken_attr_t *attr)
{
    /* Retrieve variables first. */
    u16 *buffer = term->getBuffer();
    Size width  = term->getWidth();

    /* Make sure to don't overwrite cursor. */
    term->hideCursor();

    /* Write the buffer. */
    buffer[pos->tp_col + (pos->tp_row * width)] =
	VGA_CHAR(ch, tekenToVGA[attr->ta_fgcolor], BLACK);

    /* Show cursor again. */
    term->showCursor();
}

void cursor(Terminal *term, const teken_pos_t *pos)
{
    term->hideCursor();
    term->setCursor(pos);
    term->showCursor();
}

void fill(Terminal *term, const teken_rect_t *rect,
          teken_char_t ch, const teken_attr_t *attr)
{
    /* Make sure we don't overwrite the cursor. */
    term->hideCursor();

    /* Fill video memory; loop rows. */
    for (Size row = rect->tr_begin.tp_row;
	            row < rect->tr_end.tp_row; row++)
    {
	/* Loop columns. */
	for (Size col = rect->tr_begin.tp_col;
	                col < rect->tr_end.tp_col; col++)
	{
	    term->getBuffer()[col + (row * term->getWidth())] =
		VGA_CHAR(ch, tekenToVGA[attr->ta_fgcolor], BLACK);
	}
    }
    /* Show cursor again. */
    term->showCursor();
}

void copy(Terminal *term, const teken_rect_t *rect,
          const teken_pos_t *pos)
{
    /* Retrieve variables. */
    u16 *buffer  = term->getBuffer();
    Size width   = term->getWidth();

    /* Calculate sizes. */
    Size numCols = rect->tr_end.tp_col - rect->tr_begin.tp_col;
    Size numRows = rect->tr_end.tp_row - rect->tr_begin.tp_row;

    /* Hide cursor first. */
    term->hideCursor();

    /* Copy video memory. */    
    memcpy(buffer + pos->tp_col + (pos->tp_row * width),
	   buffer + rect->tr_begin.tp_col + (rect->tr_begin.tp_row * width),
	   numCols + (numRows * width) * sizeof(u16));

    /* Show cursor again. */
    term->showCursor();
}

void param(Terminal *term, int key, int value)
{
}

void respond(Terminal *ctx, const void *buf, size_t size)
{
}

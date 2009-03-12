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
#include "VGATerminal.h"

VGATerminal::VGATerminal(u16 *addr)
{
    this->mem    = addr;
    this->width  = 80;
    this->height = 25;
    this->x      = 0;
    this->y      = 0;
}

VGATerminal::~VGATerminal()
{
}

void VGATerminal::clear()
{
    for (int i = 0; i < (width * height); i++)
    {
	mem[i] = 0;
    }
}

int VGATerminal::write(s8 *buffer, Size size)
{
    for (Size i = 0; i < size; i++, buffer++)
    {
	if (x >= width)
	{
	    newline();
	}
	switch (*buffer)
	{
	    case '\n':
		newline();
		break;
	
	    default:
		mem[x++ + (y * width)] = (unsigned char) *buffer | 0x0700;
		break;
	}
    }
    return size;
}

void VGATerminal::newline()
{
    int i;

    if (y >= (height - 1))
    {
	for (i = 0; i < width * (height - 1); i++)
	{
	    mem[i] = mem[i + width];
	}
	for (i = width * (height - 1); i < (width * height); i++)
	{
	    mem[i] = 0;
	}
    }
    else
	y++;

    x = 0;
}

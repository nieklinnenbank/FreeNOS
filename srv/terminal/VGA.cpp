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

#include <API/ProcessCtl.h>
#include <FreeNOS/CPU.h>
#include <Config.h>
#include "VGA.h"

u8 tekenToVGA[] =
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    LIGHTGREY,
    LIGHTGREY,
};

VGA::VGA(u16 *m, Size w, Size h)
    : width(w), height(h), vgaMemory(m)
{
    /* Allocate local memory buffer. */                
    localMemory = new u16[width * height];             
    videoMemory = localMemory;
                                                       
    /* Initialize buffer. */                           
    for (uint i = 0; i < width * height; i++)          
    {                                                  
        videoMemory[i] = VGA_CHAR(' ', LIGHTGREY, BLACK);
    }
    /* Initial saved cursor. */
    cursorPos.tp_row = 0;
    cursorPos.tp_col = 0;
    cursorValue = VGA_CHAR(' ', LIGHTGREY, BLACK);
    
    /* Request CRT I/O ports. */
    ProcessCtl(SELF, AllowIO, VGA_IOADDR);
    ProcessCtl(SELF, AllowIO, VGA_IODATA);
    
    /* Disable hardware cursor. */
    outb(VGA_IOADDR, 0x0a);
    outb(VGA_IODATA, 1 << 5);
}

VGA::~VGA()
{
    delete localMemory;
}

void VGA::hideCursor()
{
    u16 index = cursorPos.tp_col + (cursorPos.tp_row * width);

    /* Restore old attributes. */
    videoMemory[index] &= 0xff;
    videoMemory[index] |= (cursorValue & 0xff00);
}

void VGA::setCursor(const teken_pos_t *pos)
{
    /* Save value. */
    cursorValue = videoMemory[pos->tp_col + (pos->tp_row * width)];
    cursorPos   = *pos;
}

void VGA::showCursor()
{
    u16 index = cursorPos.tp_col + (cursorPos.tp_row * width);

    /* Refresh cursorValue first. */
    setCursor(&cursorPos);

    /* Write cursor. */
    videoMemory[index] &= 0xff;
    videoMemory[index] |= VGA_ATTR(LIGHTGREY, LIGHTGREY) << 8;
}

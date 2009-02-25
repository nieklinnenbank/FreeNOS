/**
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

ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
    . = 0x00100000;
    kernelStart = .;

    .text :
    {
	. = ALIGN(4);
	
	/**
	 * Multiboot header.
	 */
	LONG(0x1BADB002)
	LONG(0x00000003)
	LONG(-(0x1BADB002 + 0x00000003))

	*(.text)
    }

    .data :
    {
	*(.data)

        CTOR_LIST = .;
	KEEP (*(SORT(.ctors.*)))
	KEEP (*(.ctors))
	LONG(0)
        CTOR_END = .;
	
        DTOR_LIST = .;
	KEEP (*(SORT(.dtors.*)))
	KEEP (*(.dtors))
        LONG(0)
        DTOR_END = .;
	. += 4;
	
	initStart = .;
	KEEP (*(SORT(.init*)))
	initEnd   = .;
    }

    .bss :
    {
	*(.bss)
    }
    . += 4;
    kernelEnd = .;
}

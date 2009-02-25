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

#include <api/ProcessCtl.h>
#include <arch/CPU.h>
#include "PS2Terminal.h"

/**
 * Temporary hardcoded keyboard map, from kb.c in SkelixOS.
 * @see http://www.skelix.org
 */
const char PS2Terminal::keymap[0x3a][2] =
{
    /*00*/{0x0, 0x0}, {0x0, 0x0}, {'1', '!'}, {'2', '@'}, 
    /*04*/{'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, 
    /*08*/{'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},
    /*0c*/{'-', '_'}, {'=', '+'}, {'\b','\b'},{'\t','\t'},
    /*10*/{'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'},
    /*14*/{'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'},
    /*18*/{'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'},
    /*1c*/{'\n','\n'},{0x0, 0x0}, {'a', 'A'}, {'s', 'S'},
    /*20*/{'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
    /*24*/{'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'},
    /*28*/{'\'','\"'},{'`', '~'}, {0x0, 0x0}, {'\\','|'}, 
    /*2c*/{'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, 
    /*30*/{'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'},
    /*34*/{'.', '>'}, {'/', '?'}, {0x0, 0x0}, {'*', '*'},
    /*38*/{0x0, 0x0}, {' ', ' '}
};

PS2Terminal::PS2Terminal() : shiftState(ZERO)
{
    ProcessCtl(TERMINAL_PID, AllowIO,  PS2_PORT);
    ProcessCtl(TERMINAL_PID, WatchIRQ, PS2_IRQ);
}

int PS2Terminal::read(s8 *buffer, Size size)
{
    u8 keycode = inb(PS2_PORT);

    /* Update shift state. */
    if (keycode == 0x2a)
    {
	shiftState ^= 1;
	return 0;
    }
    /* Don't do anything on release. */
    if (keycode & PS2_RELEASE)
    {
	return 0;
    }
    /* Write to buffer. */
    buffer[0] = keymap[keycode & 0x7f][shiftState];
    
    /* Success. */
    return 1;
}

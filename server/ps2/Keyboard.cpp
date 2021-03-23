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

#include <FreeNOS/User.h>
#include <Macros.h>
#include "Keyboard.h"

/**
 * Temporary hardcoded keyboard map, from kb.c in SkelixOS.
 * @see http://www.skelix.org
 */
const char Keyboard::keymap[0x3a][2] =
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

Keyboard::Keyboard(const u32 inode)
    : Device(inode, FileSystem::CharacterDeviceFile)
    , shiftState(ZERO)
{
    m_identifier << "keyboard0";
}

FileSystem::Result Keyboard::initialize()
{
    return FileSystem::Success;
}

FileSystem::Result Keyboard::interrupt(const Size vector)
{
    pending = true;
    return FileSystem::Success;
}

FileSystem::Result Keyboard::read(IOBuffer & buffer,
                                  Size & size,
                                  const Size offset)
{
    Size bytes = 0;

    // Do we have any new key events?
    if (pending)
    {
        pending = false;

        // Read byte from the keyboard.
        u8 keycode = m_io.inb(PS2_PORT);

        // Update shift state
        if (keycode == 0x2a || keycode == 0xaa)
        {
            shiftState ^= 1;
        }
        // Don't do anything on release
        else if (!(keycode & PS2_RELEASE) &&
            (keymap[keycode & 0x7f][shiftState]))
        {
            // Write to buffer
            buffer.write((void *) &keymap[keycode & 0x7f][shiftState], 1);
            bytes = 1;
        }
        // Re-enable interrupt
        ProcessCtl(SELF, EnableIRQ, PS2_IRQ);
    }

    if (bytes > 0)
    {
        size = bytes;
        return FileSystem::Success;
    }
    else
    {
        return FileSystem::RetryAgain;
    }
}

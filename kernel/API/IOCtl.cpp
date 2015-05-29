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

#include "IOCtl.h"

Error IOCtlHandler(IOOperation op, Address addr, ulong value)
{
    DEBUG("op =" << op << " addr=" << addr << " value=" << value);

    switch (op)
    {
        case IOByteRead:
            return IO::inb(addr);

        case IOByteWrite:
            IO::outb(addr, value);
            break;

        case IOWordRead:
            return IO::inw(addr);
        
        case IOWordWrite:
            IO::outw(addr, value);
            break;

        case IOLongWrite:
            IO::outl(addr, value);
            break;
    }
    return API::Success;
}

Log & operator << (Log &log, IOOperation op)
{
    switch (op)
    {
        case IOByteRead:  log.write("IOByteRead");  break;
        case IOByteWrite: log.write("IOByteWrite"); break;
        case IOWordRead:  log.write("IOWordRead");  break;
        case IOWordWrite: log.write("IOWordWrite"); break;
        case IOLongWrite: log.write("IOLongWrite"); break;
    }
    return log;
}

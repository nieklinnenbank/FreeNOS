/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include "API.h"

/** Operator to print a Operation to a Log */
Log & operator << (Log &log, Operation op)
{
    switch (op)
    {
        case Create: log.write("Create");   break;
        case Delete: log.write("Delete");   break;
        case Send:   log.write("Send");     break;
        case Receive: log.write("Receive"); break;
        case SendReceive: log.write("SendReceive"); break;
        case Read: log.write("Read");       break;
        case Write: log.write("Write");     break;
    }    
    return log;
}

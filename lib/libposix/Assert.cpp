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

#include <Arch/Memory.h>
#include <API/VMCtl.h>
#include <Assert.h>
#include <Macros.h>
#include <Types.h>
#include <ProcessID.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void __assertFailure(const char *fmt, ...)
{
    va_list args;
    
    /* Output assertion failure message. */
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    /* Terminate immediately. */
    exit(EXIT_FAILURE);
}

int __assertRead(Address addr)
{
    MemoryRange range;

    range.virtualAddress  = addr;
    range.physicalAddress = ZERO;
    range.bytes           = sizeof(Address);
    range.protection      = PAGE_PRESENT | PAGE_USER;

    return VMCtl(SELF, Access, &range);
}

int __assertWrite(Address addr)
{
    MemoryRange range;

    range.virtualAddress  = addr;
    range.physicalAddress = ZERO;
    range.bytes           = sizeof(Address);
    range.protection      = PAGE_PRESENT | PAGE_USER | PAGE_RW;

    return VMCtl(SELF, Access, &range);
}

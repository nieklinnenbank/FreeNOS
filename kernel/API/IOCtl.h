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

#ifndef __API_IOCTL_H
#define __API_IOCTL_H

#include <FreeNOS/System.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/**
 * Available operation to perform using PrivExec().
 * @see PrivExec
 */
typedef enum IOOperation
{
    IOByteRead  = 0,
    IOByteWrite = 1,
    IOWordRead  = 2,
    IOWordWrite = 3,
    IOLongWrite = 4
}
IOOperation;

/** Operator to print a IOOperation to a Log */
Log & operator << (Log &log, IOOperation op);

inline u8 ReadByte(Address addr)
{
    return (u8) trapKernel3(API::IOCtlNumber, IOByteRead, addr, 0);
}

inline Error WriteByte(Address addr, u8 value)
{
    return trapKernel3(API::IOCtlNumber, IOByteWrite, addr, value);
}

inline u16 ReadWord(Address addr)
{
    return (u16) trapKernel3(API::IOCtlNumber, IOWordRead, addr, 0);
}

inline Error WriteWord(Address addr, u16 value)
{
    return trapKernel3(API::IOCtlNumber, IOWordWrite, addr, value);
}

inline Error WriteLong(Address addr, ulong value)
{
    return trapKernel3(API::IOCtlNumber, IOLongWrite, addr, value);
}

/**
 * Kernel handler prototype.
 */
extern Error IOCtlHandler(IOOperation op, Address addr, ulong value);

/**
 * @}
 */

#endif /* __API_PROCESSCTL_H */

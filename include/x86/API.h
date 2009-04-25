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

#ifndef __X86_API_H
#define __X86_API_H

#include <Types.h>
#include <kernel/API.h>

/**  
 * @defgroup x86kernel kernel (x86) 
 * @{  
 */

/** 
 * Perform a kernel trap with 1 argument.
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @return An integer. 
 */
inline ulong trapKernel1(ulong num, ulong arg1)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1));
    return ret;
}

/** 
 * Perform a kernel trap with 3 arguments. 
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX. 
 * @param arg3 Third argument becomes EDX. 
 * @return An integer. 
 */
inline ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
					    "d"(arg3));
    return ret;
}

/** 
 * Perform a kernel trap with 4 arguments. 
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX. 
 * @param arg3 Third argument becomes EDX. 
 * @param arg4 Fourth argument becomes ESI.
 * @return An integer. 
 */
inline ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3,
			 ulong arg4)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
					    "d"(arg3), "S"(arg4));
    return ret;
}

/** 
 * Perform a kernel trap with 5 arguments. 
 * @param num Unique number of the handler to execute. 
 * @param arg1 First argument becomes ECX. 
 * @param arg2 Second argument becomes EBX. 
 * @param arg3 Third argument becomes EDX. 
 * @param arg4 Fourth argument becomes ESI.
 * @param arg5 Fifth argument becomes EDI.
 * @return An integer. 
 */
inline ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3,
			 ulong arg4, ulong arg5)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
				 "d"(arg3), "S"(arg4), "D"(arg5));
    return ret;
}

/**
 * @}
 */

#endif /* __X86_API_H */

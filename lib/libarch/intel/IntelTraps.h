/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __LIB_LIBARCH_INTEL_INTELTRAPS_H
#define __LIB_LIBARCH_INTEL_INTELTRAPS_H

#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_intel
 * @{
 */

/**
 * @name Intel Kernel Traps
 *
 * Intel specific software interrupts.
 * These functions are called by the user program to
 * invoke the kernel APIs, also known as system calls.
 *
 * @{
 */

/**
 * Perform a kernel trap with 1 argument.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument becomes ECX.
 *
 * @return An integer.
 */
inline ulong trapKernel1(ulong num, ulong arg1)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1) : "memory");
    return ret;
}

/**
 * Perform a kernel trap with 2 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument becomes ECX.
 * @param arg2 Second argument becomes EBX.
 *
 * @return An integer.
 */
inline ulong trapKernel2(ulong num, ulong arg1, ulong arg2)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2) : "memory");
    return ret;
}

/**
 * Perform a kernel trap with 3 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument becomes ECX.
 * @param arg2 Second argument becomes EBX.
 * @param arg3 Third argument becomes EDX.
 *
 * @return An integer.
 */
inline ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
                        "d"(arg3) : "memory");
    return ret;
}

/**
 * Perform a kernel trap with 4 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument becomes ECX.
 * @param arg2 Second argument becomes EBX.
 * @param arg3 Third argument becomes EDX.
 * @param arg4 Fourth argument becomes ESI.
 *
 * @return An integer.
 */
inline ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3,
             ulong arg4)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
                        "d"(arg3), "S"(arg4) : "memory");
    return ret;
}

/**
 * Perform a kernel trap with 5 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument becomes ECX.
 * @param arg2 Second argument becomes EBX.
 * @param arg3 Third argument becomes EDX.
 * @param arg4 Fourth argument becomes ESI.
 * @param arg5 Fifth argument becomes EDI.
 *
 * @return An integer.
 */
inline ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3,
             ulong arg4, ulong arg5)
{
    ulong ret;
    asm volatile ("int $0x90" : "=a"(ret) : "a"(num), "c"(arg1), "b"(arg2),
                 "d"(arg3), "S"(arg4), "D"(arg5) : "memory");
    return ret;
}

/**
 * @}
 * @}
 * @}
 * @}
 */

#endif /* __LIB_LIBARCH_INTEL_INTELTRAPS_H */

/*
 * Copyright (C) 2025 Ivan Tan
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

#ifndef __LIB_LIBARCH_ARM64_ARM64TRAPS_H
#define __LIB_LIBARCH_ARM64_ARM64TRAPS_H

#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/**
 * @name ARM64 Kernel Traps
 *
 * ARM64 specific software interrupts.
 * These functions are called by the user program to
 * invoke the kernel APIs, also known as system calls.
 *
 * On ARM64, there are no machine-specific input operand
 * constraints to specify registers for inline assembly.
 * Therefore, we need to use Local Register Variables syntax to
 * guarantee that specific registers are used.
 *
 * @see https://gcc.gnu.org/onlinedocs/gcc/Local-Register-Variables.html#Local-Register-Variables
 *
 * @{
 */

/**
 * Perform a kernel trap with 1 argument.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 *
 * @return An integer.
 */
inline ulong trapKernel1(ulong api, ulong arg1)
{
    register unsigned reg8 asm ("x8") = api;
    register unsigned reg0 asm ("x0") = arg1;

    asm volatile ("svc #0\n"
                : "+r"(reg0)
                : "r"(reg8), "r"(reg0)
                : "memory");
    return reg0;
}

/**
 * Perform a kernel trap with 2 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 *
 * @return An integer
 */
inline ulong trapKernel2(ulong api, ulong arg1, ulong arg2)
{
    register unsigned reg8 asm ("x8") = api;
    register unsigned reg0 asm ("x0") = arg1;
    register unsigned reg1 asm ("x1") = arg2;

    asm volatile ("svc #0\n"
                : "+r"(reg0)
                : "r"(reg8), "r"(reg0), "r"(reg1)
                : "memory");
    return reg0;
}

/**
 * Perform a kernel trap with 3 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 *
 * @return An integer.
 */
inline ulong trapKernel3(ulong api, ulong arg1, ulong arg2, ulong arg3)
{
    register unsigned reg8 asm ("x8") = api;
    register unsigned reg0 asm ("x0") = arg1;
    register unsigned reg1 asm ("x1") = arg2;
    register unsigned reg2 asm ("x2") = arg3;

    asm volatile ("svc #0\n"
                : "+r"(reg0)
                : "r"(reg8), "r"(reg0), "r"(reg1), "r"(reg2)
                : "memory");
    return reg0;
}

/**
 * Perform a kernel trap with 4 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 * @param arg4 Fourth argument
 *
 * @return An integer.
 */
inline ulong trapKernel4(ulong api, ulong arg1, ulong arg2, ulong arg3,
                         ulong arg4)
{
    register unsigned reg8 asm ("x8") = api;
    register unsigned reg0 asm ("x0") = arg1;
    register unsigned reg1 asm ("x1") = arg2;
    register unsigned reg2 asm ("x2") = arg3;
    register unsigned reg3 asm ("x3") = arg4;

    asm volatile ("svc #0\n"
                : "+r"(reg0)
                : "r"(reg8), "r"(reg0), "r"(reg1), "r"(reg2), "r"(reg3)
                : "memory");
    return reg0;
}

/**
 * Perform a kernel trap with 5 arguments.
 *
 * @param api Unique number of the handler to execute.
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 * @param arg4 Fourth argument
 * @param arg5 Fifth argument
 *
 * @return An integer.
 */
inline ulong trapKernel5(ulong api, ulong arg1, ulong arg2, ulong arg3,
                         ulong arg4, ulong arg5)
{
    register unsigned reg8 asm ("x8") = api;
    register unsigned reg0 asm ("x0") = arg1;
    register unsigned reg1 asm ("x1") = arg2;
    register unsigned reg2 asm ("x2") = arg3;
    register unsigned reg3 asm ("x3") = arg4;
    register unsigned reg4 asm ("x4") = arg5;

    asm volatile ("svc #0\n"
                : "+r"(reg0)
                : "r"(reg8), "r"(reg0), "r"(reg1), "r"(reg2), "r"(reg3), "r"(reg4)
                : "memory");
    return reg0;
}

/**
 * @}
 * @}
 * @}
 * @}
 */

#endif /* __LIB_LIBARCH_ARM64_ARM64TRAPS_H */

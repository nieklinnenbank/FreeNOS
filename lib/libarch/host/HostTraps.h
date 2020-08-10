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

#ifndef __LIB_LIBARCH_HOST_HOSTTRAPS_H
#define __LIB_LIBARCH_HOST_HOSTTRAPS_H

#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_host
 * @{
 */

/**
 * @name Host OS simulated traps
 *
 * These functions are called to invoke the FreeNOS kernel from userspace.
 * When executing on the host OS, these functions interact with the host OS
 * using POSIX calls.
 *
 * @{
 */

/**
 * Perform a kernel trap with 1 argument.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument
 *
 * @return An integer.
 */
ulong trapKernel1(ulong num, ulong arg1);

/**
 * Perform a kernel trap with 2 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument.
 * @param arg2 Second argument.
 *
 * @return An integer.
 */
ulong trapKernel2(ulong num, ulong arg1, ulong arg2);

/**
 * Perform a kernel trap with 3 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument.
 * @param arg2 Second argument.
 * @param arg3 Third argument.
 *
 * @return An integer.
 */
ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3);

/**
 * Perform a kernel trap with 4 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument.
 * @param arg2 Second argument.
 * @param arg3 Third argument.
 * @param arg4 Fourth argument.
 *
 * @return An integer.
 */
ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3, ulong arg4);

/**
 * Perform a kernel trap with 5 arguments.
 *
 * @param num Unique number of the handler to execute.
 * @param arg1 First argument.
 * @param arg2 Second argument.
 * @param arg3 Third argument.
 * @param arg4 Fourth argument.
 * @param arg5 Fifth argument.
 *
 * @return An integer.
 */
ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3, ulong arg4, ulong arg5);

/**
 * @}
 */

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIB_LIBARCH_HOST_HOSTTRAPS_H */

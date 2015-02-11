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

#ifndef __ARM_API_H
#define __ARM_API_H

#include <Types.h>

/**
 * @defgroup armkernel kernel (arm) 
 * @{
 */

/** 
 * Perform a kernel trap with 1 argument.
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel1(ulong num, ulong arg1)
{
    return 0;
}

/** 
 * Perform a kernel trap with 3 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel3(ulong num, ulong arg1, ulong arg2, ulong arg3)
{
    return 0;
}

/** 
 * Perform a kernel trap with 4 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer.
 */
inline ulong trapKernel4(ulong num, ulong arg1, ulong arg2, ulong arg3,
			 ulong arg4)
{
    return 0;
}

/** 
 * Perform a kernel trap with 5 arguments. 
 * @param num Unique number of the handler to execute. 
 * @return An integer. 
 */
inline ulong trapKernel5(ulong num, ulong arg1, ulong arg2, ulong arg3,
			 ulong arg4, ulong arg5)
{
    return 0;
}

/**
 * @}
 */

#endif /* __ARM_API_H */

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

#ifndef __KERNEL_API_VMSHARE_H
#define __KERNEL_API_VMSHARE_H

#include <FreeNOS/ProcessShares.h>
#include <Types.h>

/**
 * @addtogroup kernel
 * @{
 *
 * @addtogroup kernelapi
 * @{
 */

/**
 * Prototype for user applications. Creates and removes shared virtual memory mappings.
 *
 * @param pid Remote process.
 * @param op Determines which operation to perform.
 * @param share Pointer to the MemoryShare to use in the operation
 *
 * @return API::Success on success and other API::ErrorCode on failure.
 */
inline API::Result VMShare(const ProcessID pid,
                           const API::Operation op,
                           ProcessShares::MemoryShare *share)
{
    return (API::Result) trapKernel3(API::VMShareNumber, pid, op, (Address) share);
}

/**
 * @}
 */

#ifdef __KERNEL__

/**
 * @addtogroup kernelapi_handler
 * @{
 */

/**
 * Kernel handler prototype. Creates and removes shared virtual memory mappings.
 *
 * @param pid Remote process.
 * @param op Determines which operation to perform.
 * @param share Pointer to the MemoryShare to use in the operation
 *
 * @return API::Success on success and other API::ErrorCode on failure.
 */
extern API::Result VMShareHandler(const ProcessID pid,
                                  const API::Operation op,
                                  ProcessShares::MemoryShare *share);

/**
 * @}
 */

#endif /* __KERNEL__ */

/**
 * @}
 */

#endif /* __KERNEL_API_VMSHARE_H */

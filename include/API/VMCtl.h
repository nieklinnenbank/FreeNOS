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

#ifndef __API_VMCTL_H
#define __API_VMCTL_H

#include <FreeNOS/Process.h>
#include <FreeNOS/API.h>
#include <FreeNOS/Memory.h>
#include <Types.h>

/**  
 * @defgroup kernelapi kernel (API) 
 * @{  
 */

/** SystemCall number for VMCtl(). */
#define VMCTL 5

/**
 * Prototype for user applications. Examines and modifies virtual memory pages.
 * @param action Determines which action to perform.
 * @param proc Remote process.
 * @param paddr Physical address which we map. ZERO to pick a free paddr.
 * @param vaddr Virtual address to map paddr.
 * @param prot Protection bits. Set PAGE_PRESENT to allocate, ~PAGE_PRESENT to release.
 * @return New physical address for of the mapping.
 */
inline Address VMCtl(Action action, ProcessID proc, Address paddr,
		     Address vaddr, ulong prot = PAGE_PRESENT|PAGE_USER|PAGE_RW)
{
    return trapKernel5(VMCTL, action, proc, paddr, vaddr, prot);
}

/**
 * @}
 */

#endif /* __API_VMCTL_H */

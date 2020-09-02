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

#ifndef __LIBARCH_HOST_HOSTCONSTANT_H
#define __LIBARCH_HOST_HOSTCONSTANT_H

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
 * Host CPU Constants
 * @{
 */

/** Host is assumed to be little endian. */
#define CPU_LITTLE_ENDIAN 1

/**
 * @}
 */

/**
 * Host Memory Constants.
 * @{
 */

/** Basic memory page is 4K */
#define PAGESIZE        4096

/** Mask to find the page. */
#define PAGEMASK        0xfffff000

/** Memory address alignment. */
#define MEMALIGN        4

/**
 * @}
 */

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_HOST_HOSTCONSTANT_H */

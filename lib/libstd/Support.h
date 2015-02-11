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

#ifndef __LIBSTD_SUPPORT_H
#define __LIBSTD_SUPPORT_H
#ifndef __ASSEMBLER__

#include "Types.h"
#include "Macros.h"

extern C u32 __aeabi_uidiv(u32 numerator, u32 denominator);

#endif /* __ASSEMBLER__ */
#endif /* __LIBSTD_SUPPORT_H */

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

#ifndef __LIBARCH_SUPPORT_H
#define __LIBARCH_SUPPORT_H

#include <Types.h>
#include <Macros.h>

typedef struct
{
    int quotient;
    int remainder;
}
idiv_return;

typedef struct
{
    uint quotient;
    uint remainder;
}
uidiv_return;

extern C int __aeabi_idiv(int numerator, int denominator);

extern C uint __aeabi_uidiv(uint numerator, uint denominator);

extern C idiv_return __aeabi_idivmod(int numerator, int denominator);

extern C uidiv_return __aeabi_uidivmod(uint numerator, uint denominator);

#endif /* __LIBARCH_SUPPORT_H */

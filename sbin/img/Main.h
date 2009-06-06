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

#ifndef __IMG_MAIN_H
#define __IMG_MAIN_H

#include <FreeNOS/BootImage.h>
#include <string.h>

/**
 * Helper macro for VALUEOF().
 * @see VALUEOF
 */
#define VALUE(x) x

/**
 * Double expand the input argument.
 * @return Double expanded value.
 */
#define VALUEOF(x) VALUE(x)

/**
 * Fills in an BootVariable.
 * @param entry Pointer to an BootVariable.
 * @param var Name of a C preprocessor macro.
 * @see BootVariable
 */
#define VARIABLE(entry,var) \
    strncpy((entry).key,   #var, BOOTIMAGE_KEY); \
    strncpy((entry).value, VALUEOF(var), BOOTIMAGE_VALUE);

#endif /* __IMG_MAIN_H */

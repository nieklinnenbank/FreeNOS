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

#ifndef __LIBC_CTYPE_H
#define __LIBC_CTYPE_H

#include <Macros.h>
#include <sys/types.h>

/** 
 * @defgroup libc libc (ISO C99)
 * @{ 
 */

/**
 * Test for a decimal digit.
 * @param c Input character.
 */
#define isdigit(c) \
    ((c) >= '0' && (c) <= '9')

/**
 * Test for a lowercase letter.
 * @param c Input character.
 */
#define islower(c) \
    ((c) >= 'a' && (c) <= 'z')

/**
 * Test for an uppercase letter.
 * @param c Input character.
 */
#define isupper(c) \
    ((c) >= 'A' && (c) <= 'Z')

/**
 * Test for an alphabetic character.
 * @param c Input character.
 */
#define isalpha(c) \
    (isupper(c) || islower(c))

/**
 * Test for an alphanumeric character.
 * @param c Input character.
 */
#define isalnum(c) \
    (isalpha(c) || isdigit(c))

/**
 * Test for a blank character.
 * @param c Input character.
 */
#define isblank(c) \
    ((c) == ' ' || (c) == '\t')

/**
 * Test for a white-space character.
 * @param c Input character.
 */
#define isspace(c) \
     ((c) == '\v' || (c) == '\f' || \
      (c) == '\r' || (c) == '\n' || isblank(c))

/**
 * Converts the letter c to lowercase.
 * @param c The letter to convert.
 * @return The converted letter, or c if conversion was not possible.
 */
#define tolower(c) \
    (c >= 'A' && c <= 'Z') ? (c + 32) : (c)

/**
 * Converts the letter c to uppercase.
 * @param c The letter to convert.
 * @return The converted letter, or c if conversion was not possible.
 */
#define toupper(c) \
    (c >= 'a' && c <= 'z') ? (c - 32) : (c)

/**
 * @}
 */

#endif /* __LIBC_CTYPE_H */

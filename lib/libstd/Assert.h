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

#ifndef __LIB_LIBSTD_ASSERT_H
#define __LIB_LIBSTD_ASSERT_H

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

#if defined(__ASSERT__)

#include "Log.h"

/**
 * @name Preprocessor assertion
 * @{
 */

/**
 * Verify that a given expression evaluates to true.
 *
 * @param exp Boolean expression.
 */
#define assert(exp) \
    if (!(exp)) \
    { \
        FATAL("*** Assertion `" << QUOTE(#exp) << "' failed ***"); \
    }

/**
 * @}
 */

#else

/**
 * @name Dummy preprocessor macros
 * @{
 */

/**
 * Dummy assertion function which does not perform any check.
 *
 * @param exp Expression is ignored.
 */
#define assert(exp)

/**
 * @}
 */

#endif /* defined(__ASSERT__) */

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBSTD_ASSERT_H */

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

#ifndef __ASSERT_H
#define __ASSERT_H

// TODO: #include "Config.h"
#include "Macros.h"
#include "Types.h"

#if defined(CONFIG_ASSERT) && !defined(HOST)

/**
 * Function which prints a message, and then terminates itself.
 * @param fmt Formatted message.
 * @param ... Argument list.
 */
extern void __assert(const char *fmt, ...);

/**
 * Verify that the given address is readable.
 * @param addr Address to verify.
 */
extern bool __assertRead(Address addr);

/**
 * Verify that the given address is writeable.
 * @param addr Address to verify.
 */
extern bool __assertWrite(Address addr);

/**
 * Verify that a given expression evaluates to true.
 * @param exp Boolean expression.
 */
#define assert(exp) \
    if (!(exp)) \
    { \
	__assert("[%s:%d]: *** Assertion `%s' failed ***\n", __FILE__, __LINE__, #exp); \
    }

/**
 * Presume that the given address is readable.
 * @param addr Address to check.
 */
#define assertRead(addr) \
    assert(__assertRead((Address)addr))

/**
 * Presume that the given address is writeable.
 * @param addr Address to check.
 */
#define assertWrite(addr) \
    assert(__assertWrite((Address)addr))

#else
#define assert(exp) \
    if (!(exp)) \
	for (;;);
#define assertRead(exp)
#define assertWrite(exp)
#endif /* defined(CONFIG_ASSERT) && !defined(HOST) */

#endif /* __ASSERT_H */

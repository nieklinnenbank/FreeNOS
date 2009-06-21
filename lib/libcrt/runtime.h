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

#ifndef __LIBCRT_RUNTIME_H
#define __LIBCRT_RUNTIME_H
#ifndef __ASSEMBLER__

#include <Macros.h>
#include <Init.h>

/** 
 * @defgroup libcrt libcrt (C/C++ runtime)
 * @{ 
 */

/** Setup the dynamic memory heap. */
#define LIBCRT_HEAP	"0"

/** Constructors must be called. */
#define LIBCRT_CTOR	"1"

/** The default initialization level. */
#define LIBCRT_DEFAULT	"2"

/** Start of initialization routines. */
extern Address initStart;

/** Marks the end of all initialization functions. */
extern Address initEnd;

/**
 * C(++) program entry point.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Exit status.
 */
extern C int main(int argc, char **argv);

/** List of constructors. */
extern void (*CTOR_LIST)();

/** List of destructors. */
extern void (*DTOR_LIST)();

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __LIBCRT_RUNTIME_H */

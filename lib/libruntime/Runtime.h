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

#ifndef __LIB_LIBRUNTIME_RUNTIME_H
#define __LIB_LIBRUNTIME_RUNTIME_H

#include <Macros.h>
#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libruntime
 * @{
 */

/** Maximum size of each argument. */
#define ARGV_SIZE  128

/** Number of arguments at maximum. */
#define ARGV_COUNT (PAGESIZE / ARGV_SIZE)

/**
 * Program entry point.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 *
 * @return Exit status.
 */
int main(int argc, char **argv);

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBRUNTIME_RUNTIME_H */

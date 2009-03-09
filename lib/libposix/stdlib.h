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

#ifndef __LIBPOSIX_STDLIB_H
#define __LIBPOSIX_STDLIB_H

#include <Macros.h>
#include "types.h"

/**                                                                                                                                                                                                     
 * @defgroup posix POSIX.1-2008                                                                                                                                                                         
 * @{                                                                                                                                                                                                   
 */ 

/** Successful termination. */
#define EXIT_SUCCESS 0

/** Unsuccessful termination. */
#define EXIT_FAILURE 1

/**
 * The exit function causes normal program termination to occur.
 * @param status If the value of status is zero or EXIT_SUCCESS, an
 *		 implementation defined form of the status successfull
 *		 termination is returned. If the value of status is
 *		 EXIT_FAILURE, an implementation-defined form of the status
 *		 unsuccesfull termination is returned. Otherwise the status
 *		 is implementation-defined.
 * @return The exit function cannot return to its caller.
 */
extern C void exit(int status);

/**
 * @}
 */

#endif /* __LIBPOSIX_STDLIB_H */

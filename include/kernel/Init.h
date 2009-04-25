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

#ifndef __KERNEL_INIT_H
#define __KERNEL_INIT_H

#include <Init.h>
#include <Types.h>

/**
 * @defgroup kernel kernel (generic)
 * @{
 */

/** (Physical) memory must be initialized first. */
#define PMEMORY	"0"

/** (Virtual) memory initialization. */
#define VMEMORY "1"

/** Constructors for C++ objects. */
#define CTOR "2"

/** Register APIHandlers. */
#define API "3"

/** Starts the scheduler. */
#define SCHEDULER "4"

/** Core kernel initialization. */
#define KERNEL "5"

/** Start of initialization routines. */
extern Address initStart;

/** Marks the end of all initialization functions. */
extern Address initEnd;

/**
 * @}
 */

#endif /* __KERNEL_INIT_H */

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

#ifndef __LIBARCH_ARM_STATE_H
#define __LIBARCH_ARM_STATE_H

#include <Types.h>
#include <Macros.h>

/**
 * Switch Core State.
 *
 * This function only needs to save the current Kernel state,
 * and restore the new Kernel state. It will be used by the kernel
 * to preserve its call stack and registers when it wants to switch
 * to a different Process.
 *
 * @param currentStack Address to a pointer of the current stack.
 * @param stackAddr Saved kernel stack pointer.
 */
extern C void switchCoreState(Address *currentStack,
                              Address stackAddr);

/**
 * Full Core State restore.
 *
 * This function is used to restore the full state of the
 * Core. It is used by the kernel as a function to initialize
 * the Process register state when it starts the first time.
 */
extern C void loadCoreState0();

#endif /* __LIBARCH_ARM_STATE_H */

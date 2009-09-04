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

#ifndef __FILESYSTEM_GRUB_MODULE_H
#define __FILESYSTEM_GRUB_MODULE_H

#include <API/SystemInfo.h>
#include <PseudoFile.h>
#include <Types.h>
#include <Error.h>
#include <Config.h>

/**
 * @brief GRUB Boot Module File.
 *
 * This class represents a GRUB boot module as a Regular File.
 */
class GRUBModule : public PseudoFile
{
    public:
    
	/**
	 * @brief Constructor function.
	 *
	 * Loads the GRUB boot module into virtual memory,
	 * using a physical address found in GRUB boot information.
	 *
	 * @param name Name of the boot image loaded by GRUB.
	 */
	GRUBModule(const char *name);
};

#endif /* __FILESYSTEM_GRUB_MODULE_H */

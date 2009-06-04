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

#ifndef __FILESYSTEM_BOOTIMAGE_H
#define __FILESYSTEM_BOOTIMAGE_H

#include <api/SystemInfo.h>
#include <api/VMCtl.h>
#include <arch/Memory.h>
#include <string.h>
#include <Types.h>
#include <Error.h>
#include <Config.h>
#include <string.h>
#include "Storage.h"

/** Virtual address of the loaded boot image. */
#define BOOTIMAGE_VADDR 0xa000f000

/**
 * Uses a GRUB boot module as a filesystem storage provider.
 */
class BootImage : public Storage
{
    public:
    
	/**
	 * Constructor function.
	 * @param m Name of the boot image loaded by GRUB.
	 */
	BootImage(const char *m)
	    : moduleName(m), image((u8 *)BOOTIMAGE_VADDR)
	{
	}

	/**
	 * Loads the boot image into virtual memory.
	 * @return True on success, false otherwise.
	 */
	bool load()
	{
	    SystemInformation info;
	    Address vaddr = BOOTIMAGE_VADDR;
	    
	    /* Search for the boot image. */
	    for (Size i = 0; i < info.moduleCount; i++)
	    {
		if (strcmp(info.modules[i].string, (char *)moduleName) == 0)
		{
		    /* Map bootimage into virtual memory. */
		    for (Address a = info.modules[i].modStart;
		                 a < info.modules[i].modEnd; a += PAGESIZE)
		    {
			VMCtl(Map, SELF, a, vaddr);
			vaddr += PAGESIZE;
		    }
		    imageSize = info.modules[i].modEnd -
				info.modules[i].modStart;
		    return true;
		}
	    }
	    return false;
	}

	/**
	 * Reads data from the boot image.
	 * @param offset Offset to start reading from.
	 * @param buffer Output buffer.
	 * @param size Number of bytes to copied.
	 */
	Error read(u64 offset, u8 *buffer, Size size)
	{
	    memcpy(buffer, image + offset, size);
	    return size;
	}
	
	/**
	 * Writing is not supported.
	 * @param offset Offset to start writing to.
	 * @param buffer Input buffer.
	 * @param size Number of bytes to written.
	 */
	Error write(u64 offset, u8 *buffer, Size size)
	{
	    return ENOTSUP;
	}

	/**
	 * Retrieve maximum storage capacity.
	 * @return Storage capacity.
	 */
	u64 capacity()
	{
	    return imageSize;
	}

    private:
    
	/** Name of the boot image. */
	const char *moduleName;
    
	/** Pointer to the boot image in virtual memory. */
	u8 *image;
	
	/** Size of the boot image. */
	Size imageSize;
};

#endif /* __FILESYSTEM_BOOTIMAGE_H */

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

#ifndef __DEVICE_H
#define __DEVICE_H

#include <Types.h>
#include <Error.h>

/**
 * Represents a device attached to the system.
 */
class Device
{
    public:

        /** 
         * Constructor function. 
         */
	Device()
	{
	}

	/**
	 * Class destructor.
	 */
	virtual ~Device()
	{
	}

	/**
	 * @brief Perform device specific initialization.
	 * @return Error result code.
	 */
	virtual Error initialize()
	{
	    return ESUCCESS;
	}

	/**
	 * Read bytes from the underlying device.
	 * @param buffer Buffer to store bytes to read.
	 * @param size Number of bytes to read.
	 * @param offset Offset in the device.
	 * @return Number of bytes on success and an error code on failure.
	 */
	virtual Error read(s8 *buffer, Size size, Size offset)
	{
	    return ENOTSUP;
	}

	/**
	 * Write bytes to the underlying device.
	 * @param buffer Buffer containing bytes to write.
	 * @param size Number of bytes to write.
	 * @return Number of bytes on success and an error code on failure.
	 */	
	virtual Error write(s8 *buffer, Size size, Size offset)
	{
	    return ENOTSUP;
	}
	
	/**
	 * Called when an interrupt has been triggered for this device.
	 * @param vector Vector number of the interrupt.
	 * @return Error result code.
	 */
	virtual Error interrupt(Size vector)
	{
	    return ESUCCESS;
	}
};

#endif /* __DEVICE_H */

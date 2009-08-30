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

#ifndef __PCI_PCIDEVICE_H
#define __PCI_PCIDEVICE_H

#include <Device.h>
#include <FileSystemPath.h>
#include "PCIServer.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * @brief Represents a Device attached to the PCI bus.
 *
 * This class abstracts a PCI device on the PCI bus. It
 * reads and writes PCI configuration registers from the
 * /dev/pci/$BUS/$SLOT/$FUNC/config file, to access PCI
 * configuration registers directly using normal file I/O.
 *
 * @see PCIServer
 */
class PCIDevice : public Device
{
    public:

	/**
	 * @brief Constructor function.
	 * @param bus PCI bus number, in /dev/pci.
	 * @param slot PCI slot number, in /dev/pci/$BUS.
	 * @param func PCI function number, in /dev/pci/$BUS/$SLOT.
	 */
	PCIDevice(const char *bus, const char *slot,
		  const char *func)
	{
	    char path[PATHLEN];
	    
	    /* Construct full path. */
	    snprintf(path, sizeof(path), "/dev/pci/%s/%s/%s/config",
		     bus, slot, func);
	
	    /* Attempt to open the PCI config file. */
	    this->file = open(path, O_RDWR);
	}

    protected:

	/**
	 * @brief Read a 32-bit register from PCI configuration space.
	 * @param reg Offset in the PCI configuration space.
	 * @return 32-bit integer.
	 */
	u32 readLong(u8 reg)
	{
	    return readRegister(reg, sizeof(u32));
	}

	/**
	 * @brief Read a 16-bit register from PCI configuration space.
	 * @param reg Offset in the PCI configuration space.
	 * @return 16-bit integer.
	 */	
	u16 readWord(u8 reg)
	{
	    return readRegister(reg, sizeof(u16));
	}

	/**
	 * @brief Read a 8-bit register from PCI configuration space.
	 * @param reg Offset in the PCI configuration space.
	 * @return 8-bit integer.
	 */	
	u8 readByte(u8 reg)
	{
	    return readRegister(reg, sizeof(u8));
	}

    private:
    
	/**
	 * @brief Read a register from PCI configuration space in /dev/pci.
	 * @param reg Offset in PCI configuration space.
	 * @param size Size of the register to read. Must be between 1 and 4.
	 * @return Integer value.
	 */
	ulong readRegister(u8 reg, Size size)
	{
	    ulong value = 0;
	    
	    assert(size > 0);
	    assert(size <= sizeof(ulong));
	    
	    /*
	     * Attempt to read value.
	     */
	    if (::lseek(file, reg, SEEK_SET) == 0 &&
	        ::read(file, &value, size) > 0)
	    {
		return value;
	    }
	    else
		return ZERO;
	}
	
	/**
	 * @brief File descriptor for the PCI configuration file found in /dev/pci.
	 */
	int file;
};

#endif /* __PCI_PCIDEVICE_H */

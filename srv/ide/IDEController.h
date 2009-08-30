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

#ifndef __IDE_IDECONTROLLER_H
#define __IDE_IDECONTROLLER_H

#include <PCIDevice.h>

/**
 * @brief Integrated Drive Electronics (IDE) Host Controller Device.
 */
class IDEController : public PCIDevice
{
    public:

	/**
	 * @brief Constructor function.
	 * @param bus PCI bus number, in /dev/pci.
	 * @param slot PCI slot number, in /dev/pci/$BUS.
	 * @param func PCI function number, in /dev/pci/$BUS/$SLOT.
	 */
	IDEController(const char *bus, const char *slot,
		      const char *func);

	/**
	 * @brief Configures the IDE controller.
	 * @return Error result code.
	 */    
	Error initialize();
	
	/**
	 * @brief Process IDE interrupts.
	 * @param vector Interrupt number.
	 * @return Error result code.
	 */
	Error interrupt(Size vector);        
};

#endif /* __IDE_IDECONTROLLER_H */

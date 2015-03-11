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

#ifndef __USB_UHCI_CONTROLLER_H
#define __USB_UHCI_CONTROLLER_H

/**     
 * @defgroup usb USB (Universal Serial Bus)   
 * @{     
 */

#include <PCIDevice.h>

/**
 * @name UHCI Command Register Flags.
 * @{
 */

/** @brief Run or Stop processing USB packets. */
#define UHCI_CMD_RUN		0x01

/** @brief Host Controller Reset. */
#define UHCI_CMD_HOSTRESET	0x02

/** @brief Global Reset. */
#define UHCI_CMD_GLOBRESET	0x04

/** @brief Enter Global Suspend Mode. */
#define UHCI_CMD_SUSPEND	0x08

/** @brief Force Global Resume. */
#define UHCI_CMD_RESUME		0x10

/** @brief Software Debug. */
#define UHCI_CMD_DEBUG		0x20

/** @brief Configure flag. */
#define UHCI_CMD_CONF		0x40

/** @brief Maximum packet size. */
#define UHCI_CMD_MAXP		0x80

/**
 * @}
 */

/**
 * @name UHCI Status Register Flags.
 * @{
 */

/** @brief USB Transaction Interrupt. */
#define UHCI_STATUS_INTR	0x01

/** @brief USB Error Interrupt. */
#define UHCI_STATUS_USBERR	0x02

/** @brief Resume Detect. */
#define UHCI_STATUS_RESUME	0x04

/** @brief Host System Error. */
#define UHCI_STATUS_HOSTERR	0x08

/** @brief Host Controller Process Error. */
#define UHCI_STATUS_TDERR	0x10

/** @brief Host Controller Halted. */
#define UHCI_STATUS_HALTED	0x20

/**
 * @}
 */

/**
 * @name UHCI Interrupt Enable Flags.
 * @{
 */

/** @brief Timeout/CRC Interrupt Enable. */
#define UHCI_INTR_TIMEOUT	0x01

/** @brief Resume Interrupt Enable. */
#define UHCI_INTR_RESUME	0x02

/** @brief Interrupt On Complete Enable. */
#define UHCI_INTR_COMPLETE	0x04

/** @brief Short Packet Interrupt Enable. */
#define UHCI_INTR_SHORT		0x08

/**
 * @}
 */

/**
 * @name UHCI Port Status Flags.
 * @{
 */

/** @brief Current Connect Status. */
#define UHCI_PORT_CONN		0x0001

/** @brief Connect Status Change. */
#define UHCI_PORT_CONN_CHG	0x0002

/** @brief Port Enabled/Disabled. */
#define UHCI_PORT_ENABLE	0x0004

/** @brief Port Enable/Disable Change. */
#define UHCI_PORT_ENABLE_CHG	0x0008

/** @brief Line Status. */
#define UHCI_PORT_LINE		0x0010

/** @brief Resume Detect. */
#define UHCI_PORT_RESUME	0x0040

/** @brief Low Speed Device Attached. */
#define UHCI_PORT_LOWSPEED	0x0100

/** @brief Port Reset. */
#define UHCI_PORT_RESET		0x0200

/** @brief Suspend. */
#define UHCI_PORT_SUSPEND	0x1000

/**
 * @}
 */

/**
 * @brief Universal Host Controller Interface (UHCI) Device.
 */
class UHCIController : public PCIDevice
{
    public:

	/**
	 * @brief Constructor function.
	 * @param bus PCI bus number, in /dev/pci.
	 * @param slot PCI slot number, in /dev/pci/$BUS.
	 * @param func PCI function number, in /dev/pci/$BUS/$SLOT.
	 */
	UHCIController(const char *bus, const char *slot,
		       const char *func);

	/**
	 * @brief Configures the UHCI controller.
	 * @return Error result code.
	 */    
	Error initialize();
	
	/**
	 * @brief Process UHCI interrupts.
	 * @param vector Interrupt number.
	 * @return Error result code.
	 */
	Error interrupt(Size vector);        
};

/**
 * @}
 */

#endif /* __USB_UHCI_CONTROLLER_H */

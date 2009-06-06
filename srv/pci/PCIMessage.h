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

#ifndef __PCI_PCIMESSAGE_H
#define __PCI_PCIMESSAGE_H

#include <API/IPCMessage.h>
#include <Types.h>
#include <Macros.h>
#include <Error.h>
#include "PCIDevice.h"

/**
 * Actions which can be specified in an PCIMessage.
 */
typedef enum PCIAction
{
    GetDevice = 0,
}
MemoryAction;

/**
 * PCI operation message.
 */
typedef struct PCIMessage : public Message
{
    /**
     * Default constructor.
     */
    PCIMessage() : action(GetDevice)
    {
    }

    union
    {
	/** Action to perform. */
        PCIAction action;
	
	/** Result code. */
	Error result;
    };
    
    /** Device information buffer. */
    PCIDevice dev;
}
PCIMessage;

#endif /* __PCI_PCIMESSAGE_H */

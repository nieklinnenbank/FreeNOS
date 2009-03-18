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

#ifndef __DEVICEMESSAGE_H
#define __DEVICEMESSAGE_H

#include <api/IPCMessage.h>
#include <Types.h>
#include <Error.h>

/**
 * Actions which may be performed on a device.
 */
typedef enum DeviceAction
{
    ReadDevice     = 0,
    WriteDevice    = 1,
    ResetDevice    = 2,
    IdentifyDevice = 3,
}
DeviceAction;

/**
 * Device IPC message.
 */
typedef struct DeviceMessage : public Message
{
    /**
     * Default constructor.
     */
    DeviceMessage() : action(ReadDevice)
    {
    }

    /**
     * Assignment operator.
     * @param m DeviceMessage pointer to copy from.
     */
    void operator = (DeviceMessage *m)
    {
	from   = m->from;
	type   = m->type;
	action = m->action;
	buffer = m->buffer;
	size   = m->size;
	offset = m->offset;
    }

    union
    {
	/** Action to perform. */
	DeviceAction action;
    
	/** Result code. */
	Error result;
    };

    /** Points to a buffer for I/O. */
    char *buffer;

    /** Size of the buffer. */
    Size size;
    
    /** Offset in the (Block)Device. */
    Size offset;
    
    /** Unused. */
    ulong unused[3];
}
DeviceMessage;

#endif /* __DEVICEMESSAGE_H */

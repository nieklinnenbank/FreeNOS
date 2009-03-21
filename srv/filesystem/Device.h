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

#ifndef __FILESYSTEM_DEVICE_H
#define __FILESYSTEM_DEVICE_H

#include <Types.h>
#include <Error.h>
#include "FileSystemMessage.h"

/**
 * Abstract device which is represented as a File.
 */
class Device : public File
{
    public:
    
	/**
	 * Constructor function.
	 * @param id Unique identity of the device.
	 * @param t Type of device.
	 */
	Device(DeviceID id, FileType t = CharacterDeviceFile)
	     : File(t), deviceID(id)
	{
	}

        /** 
         * Reads out the buffer. 
	 * @param msg Read request.
         * @return Always zero. The Device will send an IODone
	 *         when it has completed the request.
         */
        Error read(FileSystemMessage *msg)
	{
	    msg->action   = ReadFile;
	    msg->deviceID = deviceID;
	    msg->ipc(deviceID.major, Send, sizeof(*msg));
	    return ZERO;
	}

        /**
         * Retrieve file statistics.
         * @param st Buffer to write statistics to.
         */
        virtual void status(struct stat *st)
        {
	    st->st_dev = deviceID;
	    File::status(st);
        }

    private:

	/** Device ID numbers. */
	DeviceID deviceID;
};

#endif /* __FILESYSTEM_DEVICE_H */

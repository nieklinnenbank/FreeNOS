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

#ifndef __FILESYSTEM_SPECIAL_H
#define __FILESYSTEM_SPECIAL_H

#include <API/VMCopy.h>
#include <Types.h>
#include <Error.h>
#include "File.h"
#include "FileSystemMessage.h"

/**
 * Abstract special device which is represented as a File.
 */
class Special : public File
{
    public:
    
	/**
	 * @brief Constructor function.
	 * @param type Type of device. May be character or block.
	 * @param id Unique identity of the device.
	 * @see CharacterDeviceFile
	 * @see BlockDeviceFile
	 * @see DeviceID
	 */
	Special(FileType type, DeviceID id)
	     : File(type), deviceID(id)
	{
	}

        /** 
         * @brief Attempt to open a file. 
         * @param pid Process Identity to serve us from. May be changed 
         *            to redirect to other servers. 
	 * @param ident Identity number to be put into the FileDescriptor.
         * @return Error code status. 
         */
        Error open(ProcessID *pid, Address *ident)
	{
	    *ident = deviceID.minor;
	    *pid   = deviceID.major;
	    return ESUCCESS;
	}

        /**
         * Retrieve file statistics.
	 * @param msg Describes the status request.
	 * @return Error code status.
         */
        Error status(FileSystemMessage *msg)
        {
	    FileStat st;
	    Error e;
	    
	    /* Fill in the status structure. */
	    st.type     = type;
	    st.access   = access;
            st.size     = size;
            st.userID   = uid;
            st.groupID  = gid;
	    st.deviceID = deviceID;
	    
	    /* Write to remote process' buffer. */
	    if ((e = VMCopy(msg->from, Write, (Address) &st,
			   (Address) msg->stat, sizeof(st))) > 0)
	    {
		return ESUCCESS;
	    }
	    else
	    {
		return e;
	    }
        }

    private:

	/** Device ID numbers. */
	DeviceID deviceID;
};

#endif /* __FILESYSTEM_SPECIAL_H */

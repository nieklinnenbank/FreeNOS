/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <Log.h>
#include "USBMessage.h"
#include "USBTransferFile.h"
#include "USBDescriptor.h"

USBTransferFile::USBTransferFile(USBController *controller)
    : File()
{
    m_access     = FileSystem::OwnerRW;
    m_controller = controller;
}

FileSystem::Result USBTransferFile::write(IOBuffer & buffer,
                                          Size & size,
                                          const Size offset)
{
    USBMessage *usb = (USBMessage *) buffer.getBuffer();
    const FileSystemMessage *msg = buffer.getMessage();

    if (size != sizeof(USBMessage))
    {
        ERROR("invalid size " << size << " != sizeof(USBMessage)");
        return FileSystem::IOError;
    }

    switch (usb->state)
    {
        case USBMessage::Setup:
        case USBMessage::Data:
        case USBMessage::Status:
        {
            Error e = m_controller->transfer(msg, usb);
            if (e < 0)
            {
                return FileSystem::IOError;
            }
            else
            {
                return FileSystem::Success;
            }
        }

        case USBMessage::Success:
            buffer.write((void *)buffer.getBuffer(), sizeof(*usb));
            return FileSystem::Success;

        case USBMessage::Failure:
            return FileSystem::IOError;

        default:
            ERROR("invalid unknown USBMessage state: " << (int) usb->state);
            break;
    }

    return FileSystem::IOError;
}

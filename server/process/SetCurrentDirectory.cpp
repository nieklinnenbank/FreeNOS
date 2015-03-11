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

#include <API/VMCopy.h>
#include <FreeNOS/Process.h> 
#include <Error.h>
#include <ProcessID.h>
#include "FileSystemPath.h"
#include "ProcessServer.h"
#include "ProcessMessage.h"

void ProcessServer::setCurrentDirectory(ProcessMessage *msg)
{
    /* Handle request. */
    msg->result = VMCopy(msg->from, Read,
                        (Address) procs[msg->from]->currentDirectory,
                        (Address) msg->path, PATHLEN);

    /* Mark with ESUCCESS? */
    if (msg->result > 0)
    {
        msg->result = ESUCCESS;
    }
}

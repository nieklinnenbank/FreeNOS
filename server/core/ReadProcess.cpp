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

#include <FreeNOS/API.h>
#include <FreeNOS/Process.h> 
#include "CoreMessage.h"
#include "CoreServer.h"
#include <errno.h>

void CoreServer::readProcessHandler(CoreMessage *msg)
{
    // Copy a specific process only
    if (msg->number)
        VMCopy(msg->from, API::Write, (Address) (procs + msg->number),
              (Address) (msg->buffer), sizeof(UserProcess));

    // Copy the whole process table
    else
        VMCopy(msg->from, API::Write, (Address) procs,
              (Address) (msg->buffer), sizeof(UserProcess) * MAX_PROCS);
    msg->result = ESUCCESS;
}

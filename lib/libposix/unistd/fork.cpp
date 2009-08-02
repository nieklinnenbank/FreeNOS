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

#include <API/IPCMessage.h>
#include <ProcessServer.h>
#include <FileDescriptor.h>
#include <Config.h>
#include "POSIXSupport.h"
#include <errno.h>
#include "unistd.h"

pid_t fork(void)
{
    ProcessMessage msg;
    char key[64];
    
    /* Fill in the message. */
    msg.action = CloneProcess;
    
    /* Ask the process server. */
    IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg));
    
    /*
     * Child must reload file descriptor table.
     */

    /* Format FileDescriptor key. */
    snprintf(key, sizeof(key), "%s%u", FILE_DESCRIPTOR_KEY, getpid());

    /* Then reload the FileDescriptor table. */
    files.load(key, FILE_DESCRIPTOR_MAX);
    
    /* Set errno. */
    errno = msg.result;
    
    /* All done. */
    return msg.result == ESUCCESS ? (pid_t) msg.number : (pid_t) -1;
}

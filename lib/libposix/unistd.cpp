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

#include <api/IPCMessage.h>
#include <ProcessServer.h>
#include <FileSystemMessage.h>
#include <Config.h>
#include <errno.h>
#include "unistd.h"

pid_t getpid()
{
    ProcessMessage msg;
    
    msg.action = GetID;
    IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg));
    
    return msg.number;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    FileSystemMessage fs;

    /* Read the file. */
    fs.fd = fildes;
    fs.readFile((char *) buf, nbyte);

    /* Set error number. */
    errno = fs.result;
    
    /* Success. */
    return errno == ESUCCESS ? fs.size : -1;
}

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    FileSystemMessage fs;
    
    /* Write the file. */
    fs.fd = fildes;
    fs.writeFile((char *) buf, nbyte);
    
    /* Set error number. */
    errno = fs.result;
    
    /* Give the result back. */
    return errno == ESUCCESS ? fs.size : -1;
}

int close(int fildes)
{
    FileSystemMessage msg;
    
    /* Fill the message. */
    msg.action = CloseFile;
    msg.fd     = fildes;
    
    /* Ask VFS. */
    IPCMessage(VFSSRV_PID, SendReceive, &msg, sizeof(msg));
    
    /* Set error number. */
    errno = msg.result;
    
    /* All done. */
    return errno == ESUCCESS ? 0 : -1;
}

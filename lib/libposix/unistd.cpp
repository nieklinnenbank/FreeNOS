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
#include <FileSystemMessage.h>
#include <ExecutableFormat.h>
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
    return errno == ESUCCESS ? fs.size : (ssize_t) -1;
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
    return errno == ESUCCESS ? fs.size : (ssize_t) -1;
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

off_t lseek(int fildes, off_t offset, int whence)
{
    FileSystemMessage fs;
    
    /* Ask for the seek. */
    errno = fs.seekFile(fildes, offset, whence);
    
    /* Done. */
    return errno == ESUCCESS ? 0 : (off_t) -1;
}

int execv(const char *path, const char *argv[])
{
    ExecutableFormat *fmt = ExecutableFormat::find(path);
    return fmt ? 0 : -1;
}

int forkexec(const char *path, const char *argv[])
{
    ProcessMessage msg;
    
    /* We want to spawn a new process. */
    msg.action = SpawnProcess;
    msg.path   = (char *) path;
    
    /* Ask process server. */
    IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg));

    /* Set errno. */
    errno = msg.result;
    
    /* All done. */
    return errno == ESUCCESS ? (int) msg.number : -1;
}

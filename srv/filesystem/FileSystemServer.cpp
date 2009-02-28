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

#include <stdio.h>
#include "FileSystemServer.h"

FileSystemServer::FileSystemServer()
    : IPCServer<FileSystemServer, FileSystemMessage>(this)
{
    addIPCHandler(OpenFile,  &FileSystemServer::doOpenFile);
    addIPCHandler(ReadFile,  &FileSystemServer::doReadFile);
    addIPCHandler(WriteFile, &FileSystemServer::doWriteFile);
}

void FileSystemServer::doOpenFile(FileSystemMessage *msg,
				  FileSystemMessage *reply)
{
    reply->result = EACCESS;
}

void FileSystemServer::doReadFile(FileSystemMessage *msg,
				  FileSystemMessage *reply)
{
    reply->result = EACCESS;
}

void FileSystemServer::doWriteFile(FileSystemMessage *msg,
				   FileSystemMessage *reply)
{
    reply->result = EACCESS;
}

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

#include <FreeNOS/System.h>
#include <Assert.h>
#include <Vector.h>
#include <HashTable.h>
#include <HashIterator.h>
#include "FileSystemClient.h"
#include "FileSystemMount.h"
#include "FileSystemServer.h"

FileSystemServer::FileSystemServer(const char *path)
    : ChannelServer<FileSystemServer, FileSystemMessage>(this)
{
    // Set members
    m_root      = 0;
    m_mountPath = path;
    m_requests  = new List<FileSystemRequest *>();

    // Register message handlers
    addIPCHandler(FileSystem::CreateFile, &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::StatFile,   &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::DeleteFile, &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::ReadFile,   &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::WriteFile,  &FileSystemServer::pathHandler, false);
}

FileSystemServer::~FileSystemServer()
{
    if (m_requests)
        delete m_requests;
}

const char * FileSystemServer::getMountPath() const
{
    return m_mountPath;
}

Directory * FileSystemServer::getRoot()
{
    return (Directory *) m_root->file;
}

FileSystem::Result FileSystemServer::mount()
{
    ProcessID pid = ProcessCtl(SELF, GetPID);
    FileSystemMount mnt;

    // The rootfs server and sysfs server have a fixed mount
    if (pid == ROOTFS_PID || pid == SYSFS_PID)
    {
        return FileSystem::Success;
    }

    // Fill the mount structure
    mnt.procID = pid;
    mnt.options = 0;
    MemoryBlock::copy(mnt.path, m_mountPath, PATHLEN);

    // Write the mounts file in SysFS
    const FileSystemClient sysfs(SYSFS_PID);
    Size size = sizeof(mnt);
    return sysfs.writeFile("/sys/mounts", &mnt, &size, 0);
}

File * FileSystemServer::createFile(FileSystem::FileType type, DeviceID deviceID)
{
    return (File *) ZERO;
}

FileSystem::Result FileSystemServer::registerFile(File *file, const char *path)
{
    // Add to the filesystem cache
    insertFileCache(file, path);

    // Also add to the parent directory
    FileSystemPath p((char *) path);
    Directory *parent;

    if (p.parent())
        parent = (Directory *) findFileCache(**p.parent())->file;
    else
        parent = (Directory *) m_root->file;

    parent->insert(file->getType(), **p.base());
    return FileSystem::Success;
}

void FileSystemServer::pathHandler(FileSystemMessage *msg)
{
    // Prepare request
    FileSystemRequest req(msg);

    // Process the request.
    if (processRequest(req) == FileSystem::RetryAgain)
    {
        FileSystemRequest *reqCopy = new FileSystemRequest(msg);
        assert(reqCopy != NULL);
        m_requests->append(reqCopy);
    }
}

FileSystem::Result FileSystemServer::processRequest(FileSystemRequest &req)
{
    char buf[PATHLEN];
    FileSystemPath path;
    FileCache *cache = ZERO;
    File *file = ZERO;
    Directory *parent;
    FileSystemMessage *msg = req.getMessage();
    Error ret;

    // Copy the file path
    if ((ret = VMCopy(msg->from, API::Read, (Address) buf,
                    (Address) msg->path, PATHLEN)) <= 0)
    {
        ERROR("VMCopy failed: result = " << (int)ret << " from = " << msg->from <<
              " addr = " << (void *) msg->path << " action = " << (int) msg->action);
        msg->type = ChannelMessage::Response;
        msg->result = FileSystem::IOError;
        m_registry->getProducer(msg->from)->write(msg);
        return msg->result;
    }
    DEBUG(m_self << ": path = " << buf << " action = " << msg->action);

    path.parse(buf + String::length(m_mountPath));

    // Do we have this file cached?
    if ((cache = findFileCache(&path)) ||
        (cache = lookupFile(&path)))
    {
        file = cache->file;
    }
    // File not found
    else if (msg->action != FileSystem::CreateFile)
    {
        DEBUG(m_self << ": not found");
        msg->type = ChannelMessage::Response;
        msg->result = FileSystem::NotFound;
        m_registry->getProducer(msg->from)->write(msg);
        ProcessCtl(msg->from, Resume, 0);
        return msg->result;
    }

    // Perform I/O on the file
    switch (msg->action)
    {
        case FileSystem::CreateFile:
            if (cache)
                msg->result = FileSystem::AlreadyExists;
            else
            {
                /* Attempt to create the new file. */
                if ((file = createFile(msg->filetype, msg->deviceID)))
                {
                    insertFileCache(file, **path.full());

                    /* Add directory entry to our parent. */
                    if (path.parent())
                    {
                        parent = (Directory *) findFileCache(**path.parent())->file;
                    }
                    else
                        parent = (Directory *) m_root->file;

                    parent->insert(file->getType(), **path.full());
                    msg->result = FileSystem::Success;
                }
                else
                    msg->result = FileSystem::IOError;
            }
            DEBUG(m_self << ": create = " << (int)msg->result);
            break;

        case FileSystem::DeleteFile:
            if (cache->entries.count() == 0)
            {
                clearFileCache(cache);
                msg->result = FileSystem::Success;
            }
            else
                msg->result = FileSystem::PermissionDenied;
            DEBUG(m_self << ": delete = " << (int)msg->result);
            break;

        case FileSystem::StatFile:
            if (file->status(msg) == FileSystem::Success)
            {
                msg->result = FileSystem::Success;
            }
            else
            {
                msg->result = FileSystem::IOError;
            }
            DEBUG(m_self << ": stat = " << (int)msg->result);
            break;

        case FileSystem::ReadFile: {
            if ((ret = file->read(req.getBuffer(), msg->size, msg->offset)) >= 0)
            {
                msg->size = ret;
                msg->result = FileSystem::Success;

                if (req.getBuffer().getCount())
                    req.getBuffer().flush();
            }
            else if (ret == FileSystem::RetryAgain)
            {
                msg->result = FileSystem::RetryAgain;
            }
            else
            {
                msg->result = FileSystem::IOError;
            }
            DEBUG(m_self << ": read = " << (int)msg->result);
            break;
        }

        case FileSystem::WriteFile: {
            if (!req.getBuffer().getCount())
                req.getBuffer().bufferedRead();

            if ((ret = file->write(req.getBuffer(), msg->size, msg->offset)) >= 0)
            {
                msg->size = ret;
                msg->result = FileSystem::Success;
            }
            else if (ret == FileSystem::RetryAgain)
            {
                msg->result = FileSystem::RetryAgain;
            }
            else
            {
                msg->result = FileSystem::IOError;
            }
            DEBUG(m_self << ": write = " << (int)msg->result);
            break;
        }
    }

    // Only send reply if completed (not RetryAgain)
    if (msg->result != FileSystem::RetryAgain)
    {
        sendResponse(msg);
    }

    return msg->result;
}

void FileSystemServer::sendResponse(FileSystemMessage *msg)
{
    msg->type = ChannelMessage::Response;
    m_registry->getProducer(msg->from)->write(msg);
    ProcessCtl(msg->from, Resume, 0);
}

bool FileSystemServer::retryRequests()
{
    bool restartNeeded = false;

    DEBUG("");

    for (ListIterator<FileSystemRequest *> i(m_requests); i.hasCurrent(); i++)
    {
        FileSystem::Result result = processRequest(*i.current());
        if (result != FileSystem::RetryAgain)
        {
            delete i.current();
            i.remove();
            restartNeeded = true;
        }
    }

    return restartNeeded;
}

void FileSystemServer::setRoot(Directory *newRoot)
{
    m_root = new FileCache(newRoot, "/", ZERO);
    insertFileCache(newRoot, ".");
    insertFileCache(newRoot, "..");
}

FileCache * FileSystemServer::lookupFile(FileSystemPath *path)
{
    List<String *> *entries = path->split();
    FileCache *c = ZERO;
    File *file = ZERO;
    Directory *dir;

    /* Loop the entire path. */
    for (ListIterator<String *> i(entries); i.hasCurrent(); i++)
    {
        /* Start at root? */
        if (!c)
        {
            c = m_root;
        }
        /* Do we have this entry cached already? */
        if (!c->entries.contains(*i.current()))
        {
            /* If this isn't a directory, we cannot perform a lookup. */
            if (c->file->getType() != FileSystem::DirectoryFile)
            {
                return ZERO;
            }
            dir = (Directory *) c->file;

            /* Fetch the file, if possible. */
            if (!(file = dir->lookup(**i.current())))
            {
                return ZERO;
            }
            /* Insert into the FileCache. */
            c = new FileCache(file, **i.current(), c);
            assert(c != NULL);
        }
        /* Move to the next entry. */
        else
            c = (FileCache *) c->entries.value(*i.current());
    }
    /* All done. */
    return c;
}

FileCache * FileSystemServer::insertFileCache(File *file, const char *pathStr)
{
    FileSystemPath path;
    FileCache *parent = ZERO;

    // Interpret the given path
    path.parse(pathStr);

    // Lookup our parent
    if (!(path.parent()))
    {
        parent = m_root;
    }
    else if (!(parent = findFileCache(path.parent())))
    {
        return ZERO;
    }
    /* Create new cache. */
    FileCache *c = new FileCache(file, **path.base(), parent);
    assert(c != NULL);
    return c;
}

FileCache * FileSystemServer::findFileCache(char *path)
{
    FileSystemPath p(path);
    return findFileCache(&p);
}

FileCache * FileSystemServer::findFileCache(String *path)
{
    return path ? findFileCache(**path) : ZERO;
}

FileCache * FileSystemServer::findFileCache(FileSystemPath *p)
{
    List<String *> *entries = p->split();
    FileCache *c = m_root;

    /* Root is treated special. */
    if (!p->parent() && p->length() == 0)
    {
        return m_root;
    }
    /* Loop the entire path. */
    for (ListIterator<String *> i(entries); i.hasCurrent(); i++)
    {
        if (!c->entries.contains(*i.current()))
            return ZERO;

        c = (FileCache *) c->entries.value(*i.current());
    }
    /* Perform cachehit? */
    if (c)
    {
        cacheHit(c);
    }
    /* Return what we got. */
    return c && c->valid ? c : ZERO;
}

FileCache * FileSystemServer::cacheHit(FileCache *cache)
{
    return cache;
}

void FileSystemServer::clearFileCache(FileCache *cache)
{
    /* Start from root? */
    if (!cache)
    {
        cache = m_root;
    }
    /* Mark invalid immediately. */
    else
        cache->valid = false;

    /* Walk all our childs. */
    for (HashIterator<String, FileCache *> i(cache->entries); i.hasCurrent(); i++)
    {
        /* Traverse subtree if it isn't invalidated yet. */
        if (i.current()->valid)
        {
            clearFileCache(i.current());
            i.remove();
        }
    }

    /* Remove the entry itself, if empty. */
    if (!cache->valid && cache->entries.count() == 0)
    {
        /* Remove entry from parent */
        if (cache->parent)
        {
            ((Directory *) cache->parent->file)->remove(*cache->name);
            cache->parent->entries.remove(cache->name);
        }
        delete cache->file;
        delete cache;
    }
}

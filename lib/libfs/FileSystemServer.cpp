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

#include <FreeNOS/User.h>
#include <Assert.h>
#include <Vector.h>
#include <HashTable.h>
#include <HashIterator.h>
#include <DatastoreClient.h>
#include "FileSystemClient.h"
#include "FileSystemMount.h"
#include "FileSystemServer.h"

FileSystemServer::FileSystemServer(const char *path)
    : ChannelServer<FileSystemServer, FileSystemMessage>(this)
    , m_pid(ProcessCtl(SELF, GetPID))
{
    m_root      = 0;
    m_mountPath = path;
    m_requests  = new List<FileSystemRequest *>();
    m_mounts    = ZERO;

    // Register message handlers
    addIPCHandler(FileSystem::CreateFile, &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::StatFile,   &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::DeleteFile, &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::ReadFile,   &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::WriteFile,  &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::MountFileSystem, &FileSystemServer::mountHandler);
    addIPCHandler(FileSystem::WaitFileSystem,  &FileSystemServer::pathHandler, false);
    addIPCHandler(FileSystem::GetFileSystems,  &FileSystemServer::getFileSystemsHandler);
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
    // The rootfs server manages the mounts table. Retrieve it from the datastore.
    if (m_pid == ROOTFS_PID)
    {
        const DatastoreClient datastore;
        const Datastore::Result result =
            datastore.registerBuffer("mounts", &m_mounts, sizeof(FileSystemMount) * MaximumFileSystemMounts);

        if (result != Datastore::Success)
            return FileSystem::IOError;

        assert(m_mounts != NULL);

        // Fill the mounts table
        MemoryBlock::set(m_mounts, 0, sizeof(FileSystemMount) * MaximumFileSystemMounts);
        return FileSystem::Success;
    }
    // Other file systems send a request to root file system to mount.
    else
    {
        const FileSystemClient rootfs(ROOTFS_PID);
        const FileSystem::Result result = rootfs.mountFileSystem(m_mountPath);

        assert(result == FileSystem::Success);
        return result;
    }
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

bool FileSystemServer::redirectRequest(const char *path, FileSystemMessage *msg)
{
    Size savedMountLength = 0;
    FileSystemMount *mnt = ZERO;

    // Search for the longest matching mount
    for (Size i = 0; i < MaximumFileSystemMounts; i++)
    {
        if (m_mounts[i].path[0] != ZERO)
        {
            const String mountStr(m_mounts[i].path, false);
            const Size mountStrLen = mountStr.length();

            if (mountStrLen > savedMountLength && mountStr.compareTo(path, true, mountStrLen) == 0)
            {
                savedMountLength = mountStrLen;
                mnt = &m_mounts[i];
            }
        }
    }

    // If no match was found, no redirect is needed
    if (!mnt)
        return false;

    msg->type = ChannelMessage::Response;

    if (msg->action == FileSystem::WaitFileSystem)
        msg->result = FileSystem::Success;
    else
        msg->result = FileSystem::RedirectRequest;
    msg->pid = mnt->procID;
    msg->pathMountLength = savedMountLength;

    sendResponse(msg);
    return true;
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
        m_registry.getProducer(msg->from)->write(msg);
        return msg->result;
    }
    DEBUG(m_self << ": path = " << buf << " action = " << msg->action);

    // Handle mounted file system paths (will result in redirect messages)
    if (m_pid == ROOTFS_PID && redirectRequest(buf, msg))
    {
        DEBUG(m_self << ": redirect " << buf << " to " << msg->pid);
        return msg->result;
    }

    path.parse(buf + String::length(m_mountPath));

    // Do we have this file cached?
    if ((cache = findFileCache(&path)) ||
        (cache = lookupFile(&path)))
    {
        file = cache->file;
    }
    // File not found
    else if (msg->action != FileSystem::CreateFile && msg->action != FileSystem::WaitFileSystem)
    {
        DEBUG(m_self << ": not found");
        msg->type = ChannelMessage::Response;
        msg->result = FileSystem::NotFound;
        m_registry.getProducer(msg->from)->write(msg);
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

        case FileSystem::WaitFileSystem: {
            // Do nothing here. Once the targeted file system is mounted
            // this function will send a redirect message when called again
            DEBUG(m_self << ": wait for " << buf);
            msg->result = FileSystem::RetryAgain;
            break;
        }

        default: {
            ERROR("unhandled file I/O operation: " << (int)msg->action);
            msg->result = FileSystem::NotSupported;
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
    m_registry.getProducer(msg->from)->write(msg);
    ProcessCtl(msg->from, Resume, 0);
}

void FileSystemServer::mountHandler(FileSystemMessage *msg)
{
    char buf[PATHLEN + 1];

    // Copy the file path
    const API::Result result = VMCopy(msg->from, API::Read, (Address) buf,
                                     (Address) msg->path, PATHLEN);
    if (result <= 0)
    {
        ERROR("failed to copy mount path: result = " << (int) result);
        msg->result = FileSystem::IOError;
        return;
    }

    // Null-terminate
    buf[PATHLEN] = 0;

    // Append to our filesystem mounts table
    for (Size i = 0; i < MaximumFileSystemMounts; i++)
    {
        if (!m_mounts[i].path[0])
        {
            MemoryBlock::copy(m_mounts[i].path, buf, sizeof(m_mounts[i].path));
            m_mounts[i].procID = msg->from;
            m_mounts[i].options = ZERO;
            NOTICE("mounted " << m_mounts[i].path);
            msg->result = FileSystem::Success;
            return;
        }
    }

    // Not space left
    msg->result = FileSystem::IOError;
}

void FileSystemServer::getFileSystemsHandler(FileSystemMessage *msg)
{
    // Copy mounts table to the requesting process
    const Size mountsSize = sizeof(FileSystemMount) * MaximumFileSystemMounts;
    const Size numBytes = msg->size < mountsSize ? msg->size : mountsSize;
    const API::Result result = VMCopy(msg->from, API::Write, (Address) m_mounts,
                                     (Address) msg->buffer, numBytes);
    if (result <= 0)
    {
        ERROR("failed to copy mount table: result = " << (int) result);
        msg->result = FileSystem::IOError;
        return;
    }

    msg->result = FileSystem::Success;
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

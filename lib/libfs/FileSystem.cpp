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
#include <Vector.h>
#include <HashTable.h>
#include <HashIterator.h>
#include <Runtime.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "FileSystem.h"

FileSystem::FileSystem(const char *path)
    : ChannelServer<FileSystem, FileSystemMessage>(this)
{
    // Set members
    m_root      = 0;
    m_mountPath = path;
    m_requests  = new List<FileSystemRequest *>();
        
    // Register message handlers
    addIPCHandler(CreateFile, &FileSystem::pathHandler, false);
    addIPCHandler(StatFile,   &FileSystem::pathHandler, false);
    addIPCHandler(DeleteFile, &FileSystem::pathHandler, false);
    addIPCHandler(ReadFile,   &FileSystem::pathHandler, false);
    addIPCHandler(WriteFile,  &FileSystem::pathHandler, false);
}

FileSystem::~FileSystem()
{
    if (m_requests)
        delete m_requests;
}

const char * FileSystem::getMountPath() const
{
    return m_mountPath;
}

Directory * FileSystem::getRoot()
{
    return (Directory *) m_root->file;
}

Error FileSystem::mount()
{
    pid_t pid = ProcessCtl(SELF, GetPID);
    FileSystemPath path;
    String mountString("/mount");
    String p;
    char buf[16];

    // The rootfs server and /mount server have a fixed mount
    if (pid == ROOTFS_PID || pid == MOUNTFS_PID)
        return ESUCCESS;

    // Set variables
    pid = ProcessCtl(SELF, GetPID);
    mountString << m_mountPath;
    path.parse(*mountString);
    snprintf(buf, sizeof(buf), "%u", pid);
    
    // Make directories for /mount/$PATH
    for (ListIterator<String *> i(*path.split()); i.hasCurrent(); i++)
    {
        String *s = i.current();
        p << "/" << **s;

        // TODO: avoid libposix dependency on mkdir().
        // TODO: implement client interface functions in libfs for doing things like mkdir() open() read() write() etc
        // TODO: then use those client interface functions in libposix, and here too.
        if (i.hasNext())
            mkdir(*p, S_IWUSR | S_IRUSR);
    }
    // write our PID to /mount/$PATH
    creat(*mountString, S_IRUSR|S_IWUSR);
    int fd = open(*mountString, O_WRONLY);
    write(fd, buf, strlen(buf));
    close(fd);

    // Done
    return ESUCCESS;
}

File * FileSystem::createFile(FileType type, DeviceID deviceID)
{
    return (File *) ZERO;
}

Error FileSystem::registerFile(File *file, const char *path, ...)
{
    va_list args;
    Error r;

    va_start(args, path);
    r = registerFile(file, path, args);
    va_end(args);

    return r;
}

Error FileSystem::registerFile(File *file, const char *path, va_list args)
{
    char buf[PATHLEN];

    // Add to the filesystem cache
    vsnprintf(buf, sizeof(buf), path, args);
    insertFileCache(file, path, args);

    // Also add to the parent directory
    FileSystemPath p((char *)buf);
    Directory *parent;

    if (p.parent())
        parent = (Directory *) findFileCache(**p.parent())->file;
    else
        parent = (Directory *) m_root->file;

    parent->insert(file->getType(), **p.base());
    return ESUCCESS;
}

void FileSystem::pathHandler(FileSystemMessage *msg)
{
    // Copy the request
    FileSystemRequest *req = new FileSystemRequest(msg);

    // Process the request.
    if (processRequest(req) == EAGAIN)
        m_requests->append(req);
    else
        delete req;
}

Error FileSystem::processRequest(FileSystemRequest *req)
{
    char buf[PATHLEN];
    FileSystemPath path;
    FileCache *cache = ZERO; 
    File *file = ZERO;
    Directory *parent;
    FileSystemMessage *msg = req->getMessage();
    Error ret;
    
    // Copy the file path
    if ((msg->result = VMCopy(msg->from, API::Read, (Address) buf,
                    (Address) msg->path, PATHLEN)) <= 0)
    {
        ERROR("path missing: result = " << (int)msg->result << " from = " << msg->from <<
              " addr = " << (uint)msg->path << " action = " << (int) msg->action << " stat = " << (int) msg->stat);
        msg->type = ChannelMessage::Response;
        msg->result = EACCES;
        m_registry->getProducer(msg->from)->write(msg);
        return msg->result;
    }
    DEBUG(m_self << ": path = " << buf << " action = " << msg->action);

    path.parse(buf + strlen(m_mountPath));

    // Do we have this file cached?
    if ((cache = findFileCache(&path)) ||
        (cache = lookupFile(&path)))
    {
        file = cache->file;
    }
    // File not found
    else if (msg->action != CreateFile)
    {
        DEBUG(m_self << ": not found");
        msg->type = ChannelMessage::Response;
        msg->result = ENOENT;
        m_registry->getProducer(msg->from)->write(msg);
#warning put this in ChannelClient somehow
        ProcessCtl(msg->from, Resume, 0);
        return msg->result;
    }

    // Perform I/O on the file
    switch (msg->action)
    {
        case CreateFile:
            if (cache)
                msg->result = EEXIST;
            else
            {
                /* Attempt to create the new file. */
                if ((file = createFile(msg->filetype, msg->deviceID)))
                {
                    const char *p = **path.full();
                    insertFileCache(file, "%s", p);
            
                    /* Add directory entry to our parent. */
                    if (path.parent())
                    {
                        parent = (Directory *) findFileCache(**path.parent())->file;
                    }
                    else
                        parent = (Directory *) m_root->file;

                    parent->insert(file->getType(), **path.full());
                    msg->result = ESUCCESS;
                }
                else
                    msg->result = EIO;
            }
            DEBUG(m_self << ": create = " << (int)msg->result);
            break;

        case DeleteFile:
            if (cache->entries.count() == 0)
            {
                clearFileCache(cache);
                msg->result = ESUCCESS;
            }
            else
                msg->result = ENOTEMPTY;
            DEBUG(m_self << ": delete = " << (int)msg->result);
            break;

        case StatFile:
            msg->result = file->status(msg);
            DEBUG(m_self << ": stat = " << (int)msg->result);
            break;

        case ReadFile:
            {
                msg->result = file->read(req->getBuffer(), msg->size, msg->offset);
                if (req->getBuffer().getCount())
                    req->getBuffer().flush();
            }
            DEBUG(m_self << ": read = " << (int)msg->result);
            break;
        
        case WriteFile:
            {
                if (!req->getBuffer().getCount())
                    req->getBuffer().bufferedRead();

                msg->result = file->write(req->getBuffer(), msg->size, msg->offset);
            }
            DEBUG(m_self << ": write = " << (int)msg->result);
            break;
    }
    ret = msg->result;
//    msg->result &= 0xffffff; // (msg->result & ~ERESTART);

    // Only send reply if completed (not EAGAIN)
    if (msg->result != EAGAIN)
    {
        msg->type = ChannelMessage::Response;
        m_registry->getProducer(msg->from)->write(msg);
        ProcessCtl(msg->from, Resume, 0);
    }
    return ret;
}

void FileSystem::timeout()
{
    DEBUG("");

    while (retryRequests());
}

bool FileSystem::retryRequests()
{
    DEBUG("");
    bool restartNeeded = false;

    for (ListIterator<FileSystemRequest *> i(m_requests); i.hasCurrent(); i++)
    {
        Error ret = processRequest(i.current());
        if (ret != EAGAIN)
        {
            delete i.current();
            i.remove();
            restartNeeded = true;
        }
        /*Error set = ret < 0 ? -ret : ret;
        
        if (set & (ERESTART))
        {
            DEBUG("ERESTART set: " << (int)ret << " : " << (int)set);
            restartNeeded = true;
        }*/
    }
    DEBUG("done");
    return restartNeeded;
}

void FileSystem::setRoot(Directory *newRoot)
{
    m_root = new FileCache(newRoot, "/", ZERO);
    insertFileCache(newRoot, ".");
    insertFileCache(newRoot, "..");
}

FileCache * FileSystem::lookupFile(FileSystemPath *path)
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
            if (c->file->getType() != DirectoryFile)
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
        }
        /* Move to the next entry. */
        else
            c = (FileCache *) c->entries.value(*i.current());
    }
    /* All done. */
    return c;
}

FileCache * FileSystem::insertFileCache(File *file, const char *pathFormat, ...)
{
    va_list args;

    va_start(args, pathFormat);
    FileCache *c = insertFileCache(file, pathFormat, args);
    va_end(args);

    return c;
}

FileCache * FileSystem::insertFileCache(File *file, const char *pathFormat, va_list args)
{
    char pathStr[PATHLEN];
    FileSystemPath path;
    FileCache *parent = ZERO;
        
    // Format the path first
    vsnprintf(pathStr, sizeof(pathStr), pathFormat, args);

    /* Interpret the given path. */
    path.parse(pathStr);
        
    /* Lookup our parent. */
    if (!(path.parent()))
    {
        parent = m_root;
    }
    else if (!(parent = findFileCache(path.parent())))
    {
        return ZERO;
    }
    /* Create new cache. */
    return new FileCache(file, **path.base(), parent);
}

FileCache * FileSystem::findFileCache(char *path)
{
    FileSystemPath p(path);
    return findFileCache(&p);
}

FileCache * FileSystem::findFileCache(String *path)
{
    return path ? findFileCache(**path) : ZERO;
}

FileCache * FileSystem::findFileCache(FileSystemPath *p)
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

FileCache * FileSystem::cacheHit(FileCache *cache)
{
    return cache;
}

void FileSystem::clearFileCache(FileCache *cache)
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

            /* May we remove reference to this entry? */
            if (i.current()->file->getOpenCount() == 0)    
            {
                i.remove();
            }
        }
    }

    /* Remove the entry itself, if empty. */
    if (!cache->valid && cache->entries.count() == 0 &&
         cache->file->getOpenCount() == 0)
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

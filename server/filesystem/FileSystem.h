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

#ifndef __FILESYSTEM_FILESYSTEM_H
#define __FILESYSTEM_FILESYSTEM_H

#include <FreeNOS/API.h>
#include <IPCServer.h>
#include <Vector.h>
#include <HashTable.h>
#include <HashIterator.h>
#include <Runtime.h>
#include "Directory.h"
#include "File.h"
#include "FileSystemPath.h"
#include "FileSystemMessage.h"
#include "FileSystemMount.h"
#include "FileDescriptor.h"
#include <unistd.h>
#include <stdlib.h>

/**
 * Cached in-memory file.
 */
typedef struct FileCache
{
    /**
     * @brief Constructor function.
     *
     * @param f File to insert into the cache.
     * @param name Entry name of the File in the parent, if any.
     * @param p Our parent. ZERO if we have no parent.
     */
    FileCache(File *f, const char *n, FileCache *p)
            : file(f), valid(true), parent(p)
    {
        name = n;

        if (p && p != this)
        {
            p->entries.insert(name, this);
        }
    }
    
    /**
     * Comparision operator.
     * @param fc Instance to compare us with.
     * @return True if equal, false otherwise.
     */
    bool operator == (FileCache *fc)
    {
        return file == fc->file;
    }

    /** File pointer. */
    File *file;

    /** Our name */
    String name;
    
    /** Contains childs. */
    HashTable<String, FileCache *> entries;

    /** Is this entry still valid?. */
    bool valid;
    
    /** Parent */
    FileCache *parent;
}
FileCache;

/**
 * Abstract filesystem class.
 */
class FileSystem : public IPCServer<FileSystem, FileSystemMessage>
{
    public:

    /**
     * Constructor function.
     * @param p Path to which we are mounted.
     */
    FileSystem(const char *path)
        : IPCServer<FileSystem, FileSystemMessage>(this),
          root(ZERO), mountPath(path)
    {
        /* Register message handlers. */
        addIPCHandler(CreateFile, &FileSystem::pathHandler);
        addIPCHandler(StatFile,   &FileSystem::pathHandler);
        addIPCHandler(DeleteFile, &FileSystem::pathHandler);
        addIPCHandler(ReadFile,   &FileSystem::pathHandler);
        addIPCHandler(WriteFile,  &FileSystem::pathHandler);
    }
    
    /**
     * Destructor function.
     */
    virtual ~FileSystem()
    {
    }

    /**
     * @brief Mount the FileSystem.
     *
     * This function is responsible for mounting the
     * FileSystem. This happends by creating a new entry
     * in the FileSystemMounts table, which is a Shared object.
     *
     * @param background Set to true to run as a background process (default).
     * @return True if mounted successfully, false otherwise.
     */
    bool mount(bool background = true)
    {
        // TODO: forkexec() ourselves in the background first, if requested.
        // TODO: Mount ourselves. Send a message to the coreserver for it.
        return true;
    }

    /**
     * @brief Create a new file.
     * @param type Describes the type of file to create.
     * @param deviceID Optionally specifies the device identities to create.
     * @return Pointer to a new File on success or ZERO on failure.
     */
    virtual File * createFile(FileType type, DeviceID deviceID)
    {
        return (File *) ZERO;
    }

    /**
     * @brief Process an incoming filesystem request using a path.
     *
     * This message handler is responsible for processing any
     * kind of FileSystemMessages which have an FileSystemAction using
     * the path field, such as OpenFile.
     *
     * @param msg Incoming request message.
     * @see FileSystemMessage
     * @see FileSystemAction
     */
    void pathHandler(FileSystemMessage *msg)
    {
        FileSystemPath path;
        FileCache *cache = ZERO; 
        File *file = ZERO;
        Directory *parent;
        IOBuffer io(msg);
        char buf[PATHLEN];
    
        // Copy the file path
        if ((msg->result = VMCopy(msg->from, API::Read, (Address) buf,
                         (Address) msg->path, PATHLEN)) <= 0)
        {
            return;
        }
        path.parse(buf + strlen(mountPath));

        // Do we have this file cached?
        if ((cache = findFileCache(&path)) ||
            (cache = lookupFile(&path)))
        {
            file = cache->file;
        }
        // File not found
        else if (msg->action != CreateFile)
        {
            msg->result = ENOENT;
            return;
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
                    insertFileCache(file, "%s", **path.full());
                
                    /* Add directory entry to our parent. */
                    if (path.parent())
                    {
                        parent = (Directory *) findFileCache(**path.parent())->file;
                    }
                    else
                        parent = (Directory *) root->file;

                    parent->insert(file->getType(), **path.full());
                    msg->result = ESUCCESS;
                }
                else
                    msg->result = EIO;
            }
            break;

        case DeleteFile:
            if (cache->entries.count() == 0)
            {
                clearFileCache(cache);
                msg->result = ESUCCESS;
            }
            else
                msg->result = ENOTEMPTY;
            break;

        case StatFile:
            msg->result = file->status(msg);
            break;

        case ReadFile:
            msg->result = file->read(&io, msg->size, msg->offset);
            break;
        
        case WriteFile:
            msg->result = file->write(&io, msg->size, msg->offset);
            break;
        }
    }
    
  protected:

    /**
     * @brief Change the filesystem root directory.
     *
     * This function set the root member to the given
     * Directory pointer. Additionally, it inserts '/.' and '/..'
     * references to the file cache.
     *
     * @param newRoot A Directory pointer to set as the new root.
     *
     * @see root
     * @see insertFileCache
     */
    void setRoot(Directory *newRoot)
    {
        root = new FileCache(newRoot, "/", ZERO);
        insertFileCache(newRoot, ".");
        insertFileCache(newRoot, "..");
    }

    /**
     * @brief Retrieve a File from storage.
     *
     * This function is responsible for walking the
     * given FileSystemPath, retrieving each uncached File into
     * the FileCache, and returning a pointer to corresponding FileCache
     * of the last entry in the given path.
     *
     * @param path A path to lookup from storage.
     * @return Pointer to a FileCache on success, ZERO otherwise.
     */
    FileCache * lookupFile(FileSystemPath *path)
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
                c = root;
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

    /**
     * Inserts a file into the in-memory filesystem tree.
     * @param file File to insert.
     * @param pathFormat Formatted full path to the file to insert.
     * @param ... Argument list.
     * @return Pointer to the newly created FileCache, or NULL on failure.
     */
    FileCache * insertFileCache(File *file, char *pathFormat, ...)
    {
        char pathStr[PATHLEN];
        FileSystemPath path;
        FileCache *parent = ZERO;
        va_list args;
        
        /* Format the path first. */
        va_start(args, pathFormat);
        vsnprintf(pathStr, sizeof(pathStr), pathFormat, args);
        va_end(args);
        
        /* Interpret the given path. */
        path.parse(pathStr);
        
        /* Lookup our parent. */
        if (!(path.parent()))
        {
            parent = root;
        }
        else if (!(parent = findFileCache(path.parent())))
        {
            return ZERO;
        }
        /* Create new cache. */
        return new FileCache(file, **path.base(), parent);
    }

    /**
     * Search the cache for an entry.
     * @param path Full path of the file to find.
     * @return Pointer to FileCache object on success, NULL on failure.
     */
    FileCache * findFileCache(char *path)
    {
        FileSystemPath p(path);
        return findFileCache(&p);
    }

    /**
     * Search the cache for an entry.
     * @param path Full path of the file to find.
     * @return Pointer to FileCache object on success, NULL on failure.
     */
    FileCache * findFileCache(String *path)
    {
        return path ? findFileCache(**path) : ZERO;
    }

    /**
     * Search the cache for an entry.
     * @param path Full path of the file to find.
     * @return Pointer to FileCache object on success, NULL on failure.
     */
    FileCache * findFileCache(FileSystemPath *p)
    {
        List<String *> *entries = p->split();
        FileCache *c = root;

        /* Root is treated special. */
        if (!p->parent() && p->length() == 0)
        {
            return root;
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

    /**
     * Process a cache hit.
     * @param cache FileCache object which has just been referenced.
     * @return FileCache object pointer.
     */
    virtual FileCache * cacheHit(FileCache *cache)
    {
        return cache;
    }

    /**
     * Cleans up the entire file cache (except opened file caches and root).
     * @param cache Input FileCache object. ZERO to clean up all from root.
     */
    void clearFileCache(FileCache *cache = ZERO)
    {
        /* Start from root? */
        if (!cache)
        {
            cache = root;
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
    
    /** Root entry of the filesystem tree. */
    FileCache *root;
    
    /** Mount point. */
    const char *mountPath;

  private:

    /** Log object */
    Log *m_log;
};

#endif /* __FILESYSTEM_FILESYSTEM_H */

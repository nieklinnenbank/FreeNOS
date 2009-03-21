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

#include <api/IPCMessage.h>
#include <api/VMCopy.h>
#include <IPCServer.h>
#include <Config.h>
#include <HashTable.h>
#include <HashIterator.h>
#include "Directory.h"
#include "File.h"
#include "FileSystemPath.h"
#include "FileSystemMessage.h"

/** Maximum length of a filesystem path. */
#define PATHLEN 64

/**
 * Cached in-memory file.
 */
typedef struct FileCache
{
    /**
     * Constructor function.
     * @param path Full path of the new file to insert.
     * @param f File to insert into the cache.
     * @param p Our parent. ZERO if we have no parent.
     */
    FileCache(FileSystemPath *path, File *f, FileCache *p)
	: file(f), count(ZERO)
    {
	entries.insert(new String("."),  this);
	entries.insert(new String(".."), p ? p : this);
	
	if (p && p != this)
	{
	    p->entries.insert(new String(path->base()), this);
	    p->dir->addEntry(**path->base(), f->getType());
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
    
    union
    {
	/** File pointer. */
        File *file;
    
	/** Directory pointer. */
	Directory *dir;
    };
    
    /** Contains parent, ourselves, and childs. */
    HashTable<String, FileCache> entries;
    
    /** Number of times opened. */
    Size count;
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
	 * @param path Path to which we are mounted.
	 */
	FileSystem(const char *path)
	    : IPCServer<FileSystem, FileSystemMessage>(this), root(ZERO)
	{
	    FileSystemMessage msg;
	    
	    /* Register message handlers. */
	    addIPCHandler(CreateFile, &FileSystem::createFileHandler);
	    addIPCHandler(OpenFile,   &FileSystem::openFileHandler);
	    addIPCHandler(ReadFile,   &FileSystem::readWriteFileHandler, false);
	    addIPCHandler(WriteFile,  &FileSystem::readWriteFileHandler, false);
	    addIPCHandler(CloseFile,  &FileSystem::closeFileHandler);
	    addIPCHandler(StatFile,   &FileSystem::statFileHandler);
	    addIPCHandler(IODone,     &FileSystem::ioDoneHandler, false);
	    
	    /* Mount ourselves. */
	    msg.action = Mount;
	    msg.buffer = (char *) path;
	    
	    /* Request VFS mount. */
	    IPCMessage(VFSSRV_PID, SendReceive, &msg, sizeof(msg));
	}
    
	/**
	 * Destructor function.
	 */
	virtual ~FileSystem()
	{
	}

	/**
	 * Load a file corresponding to the given path from underlying storage.
	 * @param path Full path to the file to load.
	 * @return Pointer to FileCache object if the file exists, or ZERO otherwise.
	 */
	virtual FileCache * lookupFile(FileSystemPath *path)
	{
	    return (FileCache *) ZERO;
	}

	/**
	 * Create a new file.
	 * @param msg Incoming message.
	 * @param reply Response message.
	 */
	virtual void createFileHandler(FileSystemMessage *msg,
				       FileSystemMessage *reply)
	{
	    reply->result = ENOSUPPORT;
	}

	/**
         * Attempt to open a file.
	 * @param msg Incoming message.
	 * @param reply Response message.
         */    
	virtual void openFileHandler(FileSystemMessage *msg,
				     FileSystemMessage *reply)
	{
	    FileSystemPath path;
	    FileCache *entry;
	    char buf[PATHLEN];

	    /* Copy the path first. */
	    if (VMCopy(msg->from, Read, (Address) buf,
				        (Address) msg->buffer, PATHLEN) <= 0)
	    {
		reply->result = EACCESS;
		return;
	    }
	    /* Parse the path. */
	    path.parse(buf);
	    
	    /* Do we have this file cached? */
	    if ((entry = findFileCache(&path)) || (entry = lookupFile(&path)))
	    {
		entry->count++;
		reply->result = ESUCCESS;
		reply->ident  = (Address) entry;
	    }
	    else
		reply->result = ENOSUCH;
	}

	/**
         * Read or writes an opened file.
	 * @param msg Incoming message.
	 * @param reply Response message.
         */    
	void readWriteFileHandler(FileSystemMessage *msg,
			          FileSystemMessage *reply)
	{
	    FileCache *fc = (FileCache *) msg->ident;
	    Error result;
	    
	    /* Perform I/O on the file. */
	    switch (msg->action)
	    {
		case ReadFile:
		    msg->result = fc->file->read(msg);
		    break;
		
		case WriteFile:
		default:
		    msg->result = fc->file->write(msg);
		    break;
	    }
	    /* Did the operation succeed already? */
	    if (msg->result != EWAIT)
	    {
		if (msg->result >= 0)
		{
		    msg->size   = msg->result;
		    msg->result = ESUCCESS;
		}
		ioDoneHandler(msg, reply);
	    }
	}

	/**
	 * Closes a file.
	 * @param msg Incoming message.
	 * @param reply Response message.
	 */
	virtual void closeFileHandler(FileSystemMessage *msg,
				      FileSystemMessage *reply)
	{
	    FileCache *file = (FileCache *) msg->ident;
	    
	    /* Decrement count. */
	    file->count--;
	}

	/**
	 * Retrieve file statistics.
	 * @param msg Incoming message.
	 * @param reply Response message.
	 */
	void statFileHandler(FileSystemMessage *msg,
			     FileSystemMessage *reply)
	{
	    FileSystemPath path;
	    FileCache *entry;
	    struct stat st;
	    char buf[PATHLEN];
	    
            /* Copy the path first. */
            if (VMCopy(msg->from, Read, (Address) buf,
                                        (Address) msg->buffer, PATHLEN) <= 0)
            {
                reply->result = EACCESS;
                return;
            }
            /* Parse the path. */
            path.parse(buf);
                                      
            /* Do we have this file cached? */
            if ((entry = findFileCache(&path)) || (entry = lookupFile(&path)))
	    {
		/* Retrieve file status. */
        	entry->file->status(&st);
	    
	        /* Copy to remote process. */
                VMCopy(msg->procID, Write, (Address) &st,
                                           (Address) msg->stat, sizeof(st));
		/* Done. */
		reply->result = ESUCCESS;
	    }
	    else
		reply->result = ENOSUCH;
	}

	/**
	 * Allows devices to inform the filesystem that an I/O operation has completed.
	 * @param msg Incoming message.
	 * @param reply Response message.
	 */
	void ioDoneHandler(FileSystemMessage *msg,
			   FileSystemMessage *reply)
	{
	     msg->ioDone(VFSSRV_PID, msg->procID, msg->size, msg->result);
	}

    protected:

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
	    return new FileCache(&path, file, parent);
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
	    List<String> *entries = p->split();
	    FileCache *c = root;

	    /* Root is treated special. */
	    if (!p->parent() && p->length() == 0)
	    {
		return root;
	    }
	    /* Loop the entire path. */
	    for (ListIterator<String> i(entries); i.hasNext(); i++)
	    {
		if (!(c = c->entries[i.current()]))
		{
		    break;
		}
	    }
	    /* Perform cachehit? */
	    if (c)
	    {
		cacheHit(c);
	    }
	    /* Return what we got. */
	    return c;
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
	    /* May we clear this entry? Mark it, if so. */
	    if (cache->count == 0 && cache != root)
	    {
		cache->count = (Size) -1;
	    }
	    /* Walk all our childs. */
	    for (HashIterator<String, FileCache> i(&cache->entries); i.hasNext(); i++)
	    {
		/* May we free this entry? */
		if (i.current()->count == 0 && i.current() != root)
		{
		    clearFileCache(i.current());
		    cache->entries.remove(i.key(), true);
		}
	    }
	    /* Remove the entry itself. */
	    if (cache->count == (Size) -1)
	    {
		delete cache->file;
		delete cache;
	    }
	}
    
	/** Root entry of the filesystem tree. */
	FileCache *root;
};

#endif /* __FILESYSTEM_FILESYSTEM_H */

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

#include <API/IPCMessage.h>
#include <API/VMCopy.h>
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
	: file(f), valid(true)
    {
	if (p && p != this)
	{
	    file->incrementRefCount();
	    p->entries.insert(new String(path->base()), this);
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
    
    /** Contains parent, ourselves, and childs. */
    HashTable<String, FileCache> entries;
    
    /** Is this entry still valid?. */
    bool valid;
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
	    : IPCServer<FileSystem, FileSystemMessage>(this), root(ZERO), mountPath((char*)path)
	{
	    FileSystemMessage msg;
	    
	    /* Register message handlers. */
	    addIPCHandler(CreateFile, &FileSystem::ioHandler, false);
	    addIPCHandler(OpenFile,   &FileSystem::ioHandler, false);
	    addIPCHandler(ReadFile,   &FileSystem::ioHandler, false);
	    addIPCHandler(WriteFile,  &FileSystem::ioHandler, false);
	    addIPCHandler(CloseFile,  &FileSystem::ioHandler, false);
	    addIPCHandler(StatFile,   &FileSystem::ioHandler, false);
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
	 * Create a new file.
	 * @param msg Describes the file to create.
	 * @param path Full path to the file to create.
	 */
	virtual Error createFile(FileSystemMessage *msg,
				 FileSystemPath *path)
	{
	    return ENOTSUP;
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
         * Perform Input/Output on a file.
	 * @param msg Incoming message.
         */    
	void ioHandler(FileSystemMessage *msg)
	{
	    FileSystemPath path;
	    FileCache *fc = ZERO; 
	    File *f = ZERO;
	    char buf[PATHLEN];
	    Error result;

	    /*
	     * Find the file, either in cache, storage or via
	     * the message itself. 
	     */
	    switch (msg->action)
	    {
		case CreateFile:
		case OpenFile:
		case StatFile:

		    /* Copy the path first. */
		    if (VMCopy(msg->procID, Read, (Address) buf,
			      (Address) msg->buffer + strlen(mountPath), PATHLEN) <= 0)
		    {
			msg->error(EACCES, IODone);
			return;
		    }
		    else
		    {
			/* Parse the path. */
		        path.parse(buf);

			/* No need to lookup caches for creation. */	    
			if (msg->action == CreateFile)
			{
			    break;
			}
			/* Do we have this file cached? */
		    	if ((fc = findFileCache(&path)) || (fc = lookupFile(&path)))
			{
			    f = fc->file;
			    f->incrementRefCount();
			    msg->ident = (Address) f;
			}
			else
			{
			    msg->error(ENOENT, IODone);
			    return;
			}
		    }
		    break;
		
		default:

		    /* Simply use the message identity. */
		    f = (File *) msg->ident;
	    }
	    /* Perform I/O on the file. */
	    switch (msg->action)
	    {
		case CreateFile:
		    msg->result = createFile(msg, &path);
		    break;
		
		case OpenFile:
		    msg->result = f->open(msg);
		    break;

		case StatFile:
		    msg->result = f->status(msg);
		    break;

		case ReadFile:
		    msg->result = f->read(msg);
		    break;
		
		case WriteFile:
		    msg->result = f->write(msg);
		    break;
		
		case CloseFile:
	    	    msg->result = ESUCCESS;	// TODO: for devices, we must inform them!!!
						// TODO: incrementReferenceCount() here aswell.
		    break;

		default:
		    msg->error(ENOTSUP);
		    break;
	    }
	    /* Did the operation complete already? */
	    if (msg->result != EAGAIN)
	    {
		ioDoneHandler(msg);
	    }
	}

	/**
	 * Allows devices to inform the filesystem that an I/O operation has completed.
	 * @param msg Incoming message.
	 */
	void ioDoneHandler(FileSystemMessage *msg)
	{
	    File *f = (File *) msg->ident;
	
	    if (msg->result == ESUCCESS)
	    {
		switch (msg->savedAction)
	        {
		    case OpenFile:
			f->incrementOpenCount();
		
		    case StatFile:
			f->decrementRefCount();
			break;

		    case CloseFile:
			f->decrementOpenCount();
			// TODO: decrementReferenceCount()
			break;
		
		    default:
			;
		}
	    }
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
	    for (HashIterator<String, FileCache> i(&cache->entries); i.hasNext(); i++)
	    {
		/* Traverse subtree if it isn't invalidated yet. */
		if (i.current()->valid)
		{
		    clearFileCache(i.current());

		    /* May we remove reference to this entry? */
		    if (i.current()->file->getRefCount()  >= 1 &&
		        i.current()->file->getOpenCount() == 0)
		    
		    {
			i.current()->file->decrementRefCount();
			cache->entries.remove(i.key(), true);
		    }
		}
	    }
	    /* Remove the entry itself, if empty. */
	    if (!cache->valid && cache->file->getRefCount() == 0 &&
		 cache->entries.count() == 0 && cache->file->getOpenCount() == 0)
	    {
		delete cache->file;
		delete cache;
	    }
	}
    
	/** Root entry of the filesystem tree. */
	FileCache *root;
	
	/** Mount point. */
	char *mountPath;
};

#endif /* __FILESYSTEM_FILESYSTEM_H */

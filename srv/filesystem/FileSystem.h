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
     * @param f File to insert into the cache.
     */
    FileCache(File *f, FileSystemPath *p)
	: file(f), path(p), parent(ZERO)
    {
    }
    
    /** File pointer. */
    File *file;
    
    /** Full path to us. */
    FileSystemPath *path;
    
    /** Our parent entry. */
    FileCache *parent;
    
    /** Possible childs. */
    List<FileCache> childs;
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
	    addIPCHandler(OpenFile, &FileSystem::openFileHandler);
	    addIPCHandler(ReadFile, &FileSystem::readFileHandler);
	    
	    /* Mount ourselves. */
	    msg.action = Mount;
	    msg.buffer = (char *) path;
	    
	    /* Request VFS mount. */
	    IPCMessage(VFSSRV_PID, SendReceive, &msg);
	}
    
	/**
	 * Destructor function.
	 */
	virtual ~FileSystem()
	{
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
	    if ((entry = cache[path.full()]))			// TODO: allow the filesystem too lookup it on "storage" (provide virtual lookupFile()).
	    {
		reply->result = ESUCCESS;
		reply->ident  = (Address) entry;
	    }
	    else
		reply->result = ENOSUCH;
	}

	/**
         * Read an opened file.
	 * @param msg Incoming message.
	 * @param reply Response message.
         */    
	void readFileHandler(FileSystemMessage *msg,
			     FileSystemMessage *reply)
			    					// TODO: VMCtl() the page into our memory, instead of using a local buffer...
	{
	    FileCache *fc = (FileCache *) msg->ident;
	    u8 buf[1024];
	    Size bufSize  = sizeof(buf) > msg->size ? msg->size : sizeof(buf);
	    Error result;
	    
	    /* Let the file read into our buffer. */
	    if ((result = fc->file->read(buf, bufSize, msg->offset)) > 0)
	    {
		/* Recalculate number of bytes. */
		result = result > bufSize ? bufSize : result;
	    
		/* Copy to remote process. */
	        VMCopy(msg->procID, Write, (Address) buf,
				           (Address) msg->buffer, result);
		reply->result = ESUCCESS;
		reply->size   = result;
	    }
	    else
	    {
		reply->result = result;
		reply->size   = 0;
	    }
	}

    protected:

	/**
	 * Inserts a file into the in-memory filesystem tree.
	 * @param path Full path to the file to insert.
	 * @param file File to insert.
	 * @return Pointer to the newly created FileCache, or NULL on failure.
	 */
	FileCache * insertFileCache(char *path, File *file)
	{
	    FileSystemPath *p = new FileSystemPath(path);
	    FileCache *entry  = (FileCache *) ZERO;
	    FileCache *parent = findFileCache(p->parent());
	    
    	    /* Create new cache. */
	    entry = new FileCache(file, p);
	    entry->parent = parent;
    	    cache.insert(p->full(), entry);
		
	    /* Add it to parent. */
	    if (parent)
		parent->childs.insertTail(entry);

	    return entry;
	}

	/**
	 * Search the cache for an entry.
	 * @param path Full path of the file to find.
	 * @return Pointer to FileCache object on success, NULL on failure.
	 */
	FileCache * findFileCache(char *path)
	{
    	    FileSystemPath p(path);
	    return cache[p.full()];
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
    
	/** Root entry. */
	FileCache *root;
	
	/** Quick path -> FileCache mapping. */
	HashTable<String, FileCache> cache;
};

#endif /* __FILESYSTEM_FILESYSTEM_H */

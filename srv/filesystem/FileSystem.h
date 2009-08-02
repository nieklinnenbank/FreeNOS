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
#include <UserProcess.h>
#include <Config.h>
#include <Shared.h>
#include <Array.h>
#include <HashTable.h>
#include <HashIterator.h>
#include <POSIXSupport.h>
#include "Directory.h"
#include "File.h"
#include "FileSystemPath.h"
#include "FileSystemMessage.h"
#include "FileSystemMount.h"
#include "FileDescriptor.h"
#include <unistd.h>

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
	    : IPCServer<FileSystem, FileSystemMessage>(this),
	      root(ZERO), mountPath(path)
	{
	    /* Register message handlers. */
	    addIPCHandler(CreateFile, &FileSystem::pathHandler);
	    addIPCHandler(OpenFile,   &FileSystem::pathHandler);
	    addIPCHandler(StatFile,   &FileSystem::pathHandler);	    
	    addIPCHandler(ReadFile,   &FileSystem::fileDescriptorHandler);
	    addIPCHandler(WriteFile,  &FileSystem::fileDescriptorHandler);
	    addIPCHandler(CloseFile,  &FileSystem::fileDescriptorHandler);
	    addIPCHandler(SeekFile,   &FileSystem::fileDescriptorHandler);

	    /* Load shared tables. */
	    procs.load(USER_PROCESS_KEY, MAX_PROCS);
	    mounts.load(FILE_SYSTEM_MOUNT_KEY, MAX_MOUNTS);
	    files = new Array<Shared<FileDescriptor> >(MAX_PROCS);
	    
	    /* Mount ourselves. */
	    for (Size i = 0; i < MAX_MOUNTS; i++)
	    {
		if (!mounts[i]->path[0] || !strcmp(mounts[i]->path, path))
		{
		    strlcpy(mounts[i]->path, path, PATHLEN);
		    mounts[i]->procID  = getpid();
		    mounts[i]->options = ZERO;
		    break;
		}
	    }
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
	    File *file;
	    ProcessID pid;
	    Address ident;
	    char buf[PATHLEN], tmp[PATHLEN];
    
	    /*
	     * Attempt to copy the input path first.
	     */
	    if ((msg->result = VMCopy(msg->from, Read, (Address) buf,
			    	     (Address) msg->buffer, PATHLEN)) <= 0)
	    {
		return;
	    }
	    /* Is the path relative? */
	    if (buf[0] != '/')
	    {
		/* Reconstruct path. */
    		snprintf(tmp, sizeof(tmp), "%s/%s",
			 procs[msg->from]->currentDirectory, buf);
		path.parse(tmp);
	    }
	    else
		path.parse(buf + strlen(mountPath));

	    /*
	     * Do we have this file cached?
	     */
	    if ((cache = findFileCache(&path)) ||
	        (cache = lookupFile(&path)))
	    {
		file = cache->file;
	    }
	    /* Sorry, no such file! */
	    else if (msg->action != CreateFile)
	    {
		msg->result = ENOENT;
		return;
	    }			
	    /* Perform I/O on the file. */
	    switch (msg->action)
	    {
		case CreateFile:
		    if (cache)
			msg->result = EEXIST;
		    else
			msg->result = createFile(msg, &path);
		    break;
		
		case OpenFile:
		    
		    /*
		     * Attempt to open the file.
		     */
		    pid   = getpid();
		    ident = (Address) file;
		    msg->result = cache->file->open(msg, &pid, &ident);

		    /* Create a FileDescriptor on success. */
		    if (msg->result == ESUCCESS)
		    {
        		msg->fd = insertFileDescriptor(msg->from, pid, ident);
		    }
		    break;

		case StatFile:
		default:
		    msg->result = file->status(msg);
		    break;
	    }
	}

	/**
         * @brief Process an incoming filesystem request using a FileDescriptor.
	 *
	 * This message handler is responsible for processing any
	 * kind of FileSystemMessages which have an FileSystemAction using
	 * a FileDescriptor, such as ReadFile.
	 *
	 * @param msg Incoming request message.
	 * @see FileSystemMessage
	 * @see FileSystemAction
	 * @see FileDescriptor
         */    
	void fileDescriptorHandler(FileSystemMessage *msg)
	{
	    FileDescriptor *fd;
	    File *file = ZERO;

	    /*
	     * Obtain the FileDescriptor.
	     */
	    if (!(fd = getFileDescriptor(files, msg->from, msg->fd)))
	    {
		msg->result = EBADF;
		return;
	    }
	    /* Copy FileDescriptor properties. */
	    if (msg->action != SeekFile)
    	    {
            	msg->offset = fd->position;
		file = (File *) fd->identifier;
    	    }
	    /* Perform I/O on the file. */
	    switch (msg->action)
	    {
		case ReadFile:
		    msg->result   = file->read(msg);
		    fd->position += msg->size;
		    break;
		
		case WriteFile:
		    msg->result = file->write(msg);
		    fd->position += msg->size;
		    break;

		case CloseFile:
		    msg->result = file->close(msg);
		    memset(getFileDescriptor(files, msg->from, msg->fd), 0,
			   sizeof(FileDescriptor));
		    break;

		case SeekFile:
		default:
		    fd->position = msg->offset;
		    msg->result  = ESUCCESS;
		    break;
	    }
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
		    if (i.current()->file->getOpenCount() == 0)
		    
		    {
			cache->entries.remove(i.key(), true);
		    }
		}
	    }
	    /* Remove the entry itself, if empty. */
	    if (!cache->valid && cache->entries.count() == 0 &&
	         cache->file->getOpenCount() == 0)
	    {
		delete cache->file;
		delete cache;
	    }
	}
    
	/** Root entry of the filesystem tree. */
	FileCache *root;
	
	/** Mount point. */
	const char *mountPath;

        /** Mounted filesystems. */
        Shared<FileSystemMount> mounts;
        
        /** User process table. */
        Shared<UserProcess> procs;

        /** Per-process File descriptors. */
        Array<Shared<FileDescriptor> > *files;

    private:
    	
	/** 
         * Fills a new FileDescriptor entry. 
         * @param procID Process Identity to insert the FileDescriptor for. 
         * @param mount Process identity of the filesystem server. 
         * @param ident Unique identifier for the file. 
         * @return FileDescriptor index number. 
         */
        int insertFileDescriptor(ProcessID procID, ProcessID mount,
                                 Address ident)
	{
	    Shared<FileDescriptor> *fds = getFileDescriptors(files, procID);
    
	    /* Loop the FileDescriptor table. */
	    for (Size i = 0; i < FILE_DESCRIPTOR_MAX; i++)
	    {
    		/* Use the entry, if the mount field is ZERO. */
    		if (!fds->get(i)->mount)
    		{
        	    fds->get(i)->mount      = mount;
        	    fds->get(i)->identifier = ident;
        	    fds->get(i)->position   = ZERO;
        	    return i;
    		}
	    }
	    /* Maximum number of FileDescriptors reached. */
	    return -1;
	}
};

#endif /* __FILESYSTEM_FILESYSTEM_H */

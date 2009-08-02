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

#ifndef __FILESYSTEM_DIRECTORY_H
#define __FILESYSTEM_DIRECTORY_H

#include <API/VMCopy.h>
#include <List.h>
#include "File.h"

/** Maximum length of a filename. */
#define DIRENT_LEN	64

/**
 * Describes an entry inside a Directory.
 */
typedef struct Dirent
{
    /** Name of the file. */
    char name[DIRENT_LEN];
    
    /** Type of file. */
    FileType type;
    
    /**
     * Compares this Dirent with another Dirent instance.
     * @param dir Instance to compare with.
     * @return True if equal, false otherwise.
     */
    bool operator == (struct Dirent *dir)
    {
	return strcmp(name, dir->name) == 0;
    }
}
Dirent;

/**
 * In-memory directory object.
 */
class Directory : public File
{
    public:
    
	/**
	 * Constructor function.
	 */
	Directory() : File(DirectoryFile)
	{
	}

	/**
	 * Destructor function.
	 */
	~Directory()
	{
	    entries.clear(true);
	}
    
	/**
	 * Read directory entries.
	 * @param msg Read request.
	 * @return Number of bytes read on success, Error on failure.
	 */
	virtual Error read(FileSystemMessage *msg)
	{
	    Size bytes = 0;
	    Dirent *dent = (Dirent *) msg->buffer;
	    Error e;
	
	    /* Loop our list of Dirents. */
	    for (ListIterator<Dirent> i(&entries); i.hasNext(); i++)
	    {
		/* Can we read another entry? */
		if (bytes + sizeof(Dirent) <= msg->size)
		{
		    if ((e = VMCopy(msg->from, Write, (Address) i.current(),
					    	      (Address) (dent++), sizeof(Dirent))) < 0)
		    {
			return e;
		    }
		    bytes += e;
		}
		else break;
	    }
	    /* Report results. */
	    msg->size = bytes;
	    return ESUCCESS;
	}

	/**
	 * Adds a directory entry.
	 * @param name Name of the entry to add.
	 * @param type File type.
	 * @note Entry names must be unique within the same Dirent.
	 */
	virtual void insertEntry(char *name, FileType type)
	{
	    if (!getEntry(name))
	    {
	        Dirent *d = new Dirent;
	        strlcpy(d->name, name, DIRENT_LEN);
	        d->type = type;
		entries.insertTail(d);
		size += sizeof(*d);
	    }
	}
	
	/**
	 * Retrieve a directory entry by it's name.
	 * @param name Name of the entry to get.
	 * @return Direct pointer on success, ZERO otherwise.
	 */
	Dirent * getEntry(char *name)
	{
	    for (ListIterator<Dirent> i(&entries); i.hasNext(); i++)
	    {
		if (strcmp(i.current()->name, name) == 0)
		{
		    return i.current();
		}
	    }
	    return (Dirent *) ZERO;
	}

	/**
	 * Removes an entry.
	 * @param name Name of the entry to remove.
	 */	
	virtual void removeEntry(char *name)
	{
	    for (ListIterator<Dirent> i(&entries); i.hasNext(); i++)
	    {
		if (strcmp(i.current()->name, name) == 0)
		{
		    entries.remove(i.current());
		    delete i.current();
		    size -= sizeof(Dirent);
		    return;
		}
	    }
	}
	
	/**
	 * Clears the list of entries.
	 */
	void clear()
	{
	    entries.clear(true);
	}

    private:

	/** List of directory entries. */
	List<Dirent> entries;
};

#endif /* __FILESYSTEM_DIRECTORY_H */

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

#ifndef __FILESYSTEM_FILESYSTEMPATH_H
#define __FILESYSTEM_FILESYSTEMPATH_H

#include <List.h>
#include <ListIterator.h>
#include <Types.h>
#include <String.h>

/** The default FileSystemPath separator. */
#define DEFAULT_SEPARATOR '/'

/**
 * Simple filesystem path parser.
 */
class FileSystemPath
{
    public:

	/**
	 * Empty constructor.
	 */
	FileSystemPath()
	{
	}
    
	/**
	 * Constructor.
	 * @param path The input path to parse.
	 * @param separator Pathname separator.
	 */
	FileSystemPath(char *path, char separator = DEFAULT_SEPARATOR)
	{
	    parse(path, separator);
	}

	/**
	 * Constructor using a String.
	 * @param s String containing the path to parse.
	 * @param separator Pathname separator.
	 */
	FileSystemPath(String *s, char separator = DEFAULT_SEPARATOR)
	{
	    parse(**s, separator);
	}

	/**
	 * Destructor.
	 */
	~FileSystemPath()
	{
	    path.clear(true);
	    fullpath.clear(true);
	}

	/**
	 * Parses a given character string as the path.
	 * @param p Path to parse.
	 * @param separator Pathname separator.
	 */
	void parse(char *p, char separator = DEFAULT_SEPARATOR)
	{
	    const char *saved = p, *cur = p;

	    /* Save parameters. */
	    this->length    = strlen(p);
	    this->separator = separator;

	    /* Insert the root for absolute paths. */
	    if (*cur == separator)
	    {
		path.insertTail(new String(&separator, 1));
		fullpath.insertTail(new String(&separator, 1));
		saved++, cur++;
	    }
	    /* Loop the entire path. */
	    while (*cur)
	    {
		if (*cur == separator || cur == p + length - 1)
		{
		    path.insertTail(new String(saved, (cur - saved) + 1));
		    fullpath.insertTail(new String(p, (cur - p) + 1));
		    saved = cur + 1;
		}
		cur++;
	    }
	}

	/**
	 * Retrieve the (absolute) pathname of our parent.
	 * @param absolute Include full path to the parent or only it's base.
	 * @return Pathname of our parent.
	 */
	String * parent(bool absolute = true)
	{
	    ListIterator<String> i;
	    String *ret = ZERO;
    
	    /* Loop path. */
	    for (absolute ? i.reset(&fullpath) : i.reset(&path); i.hasNext(); i++)
	    {
		if (( absolute && i.current() == fullpath.tail()) ||
		    (!absolute && i.current() == path.tail()))
		{
		    break;
		}
		ret = i.current();
	    }
	    return ret;
	}

	/**
	 * The name of the last element in the path.
	 * @return Name of the base.
	 */
	String * base()
	{
	    return path.tail();
	}

	/**
	 * Get the full path as a String instance.
	 * @return String instance.
	 */
	String * full()
	{
	    return fullpath.tail();
	}

	/**
	 * Returns a List of seperate path elements.
	 * @param absolute Include full path of each element or it's base.
	 * @return Pointer to a List.
	 */
	List<String> * split(bool absolute = true)
	{
	    return absolute ? &fullpath : &path;
	}

    private:

	/** The path split in pieces. */
	List<String> path;

	/** Each piece also contains all parents. */
	List<String> fullpath;

	/** Full length of the given path. */
	Size length;
	
	/** Separator character. */
	char separator;
};

#endif /* __FILESYSTEM_FILESYSTEMPATH_H */

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

#define SEPARATOR(x) \
    (this->separator == (x))

#define EOL(s,x) \
    ((x) == (s) + length - 1)

/**
 * Simple filesystem path parser.
 */
class FileSystemPath
{
    public:

	/**
	 * Empty constructor.
	 */
	FileSystemPath() : fullPath(ZERO), parentPath(ZERO)
	{
	}
    
	/**
	 * Constructor.
	 * @param path The input path to parse.
	 * @param separator Pathname separator.
	 */
	FileSystemPath(char *path, char separator = DEFAULT_SEPARATOR)
	    : fullPath(ZERO), fullLength(ZERO), parentPath(ZERO)
	{
	    parse(path, separator);
	}

	/**
	 * Constructor using a String.
	 * @param s String containing the path to parse.
	 * @param separator Pathname separator.
	 */
	FileSystemPath(String *s, char separator = DEFAULT_SEPARATOR)
	    : fullPath(ZERO), fullLength(ZERO), parentPath(ZERO)
	{
	    parse(**s, separator);
	}

	/**
	 * Destructor.
	 */
	~FileSystemPath()
	{
	    path.clear(true);
	    if (parentPath) delete parentPath;
	    if (fullPath) delete fullPath;
	}

	/**
	 * Parses a given character string as the path.
	 * @param p Path to parse.
	 * @param separator Pathname separator.
	 */
	void parse(char *p, char sep = DEFAULT_SEPARATOR)
	{
	    const char *saved = ZERO, *savedParent = ZERO, *savedTmp = ZERO;
	    const char *cur   = p;
	    Size size, savedParentSz;

	    /* Skip heading separators. */
	    while (*cur && *cur == sep) cur++;

	    /* Save parameters. */
	    p          = (char *) cur;
	    saved      = cur;
	    separator  = sep;
	    fullLength = strlen((char *)cur);
	    fullPath   = new String(cur);

	    /* Loop the entire path. */
	    while (*cur)
	    {
		if (*cur == separator || cur == p + fullLength - 1)
		{
		    if (cur == saved)
			size = 1;
		    else
			size = (cur - saved);
			
		    path.insertTail(new String(saved, size));
		    savedParent   = savedTmp;
		    savedParentSz = (savedTmp - savedParent) + 1;
		    savedTmp      = saved;
		    saved         = cur + 1;
		}
		cur++;
	    }
	    /* Create parent, if any. */
	    if (savedParent) parentPath = new String(savedParent, savedParentSz);
	}

	/**
	 * Retrieve the full path of our parent.
	 * @return Path of our parent.
	 */
	String * parent()
	{
	    return parentPath;
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
	    return fullPath;
	}

	/**
	 * Returns a List of seperate path elements.
	 * @return Pointer to a List.
	 */
	List<String> * split()
	{
	    return &path;
	}

	/**
	 * Get Length of our full path.
	 * @return Length.
	 */
	Size length()
	{
	    return fullLength;
	}

    private:

	/** The path split in pieces. */
	List<String> path;

	/** Full input path. */
	String *fullPath;

	/** Full length of the given path. */
	Size fullLength;
	
	/** Full path to our parent. */
	String *parentPath;

	/** Separator character. */
	char separator;
};

#endif /* __FILESYSTEM_FILESYSTEMPATH_H */

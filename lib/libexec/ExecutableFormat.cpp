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

#include <Macros.h>
#include <Error.h>
#include <List.h>
#include <ListIterator.h>
#include "ExecutableFormat.h"
#include <sys/stat.h>
#include <errno.h>

/** List of known executable formats. */
List<FormatDetector> ExecutableFormat::formats;

ExecutableFormat::ExecutableFormat(const char *p) : path(p)
{
}

ExecutableFormat::~ExecutableFormat()
{
}

ExecutableFormat * ExecutableFormat::find(const char *path)
{
    ExecutableFormat *fmt = ZERO;
    struct stat st;

    /* Must be an existing, regular, executable file. */
    if (stat(path, &st) != -1)
    {
        if (!S_ISREG(st.st_mode))
        {
            errno = EINVAL;
            return ZERO;
        }
    }
    else
        return ZERO;

    /* Search for the corresponding executable format. */
    for (ListIterator<FormatDetector> i(&formats); i.hasNext(); i++)
    {
	if ((fmt = (i.current())(path)))
	{
	    return fmt;
	}
    }
    errno = ENOEXEC;
    return ZERO;
}

/**
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

#include <Version.h>
#include <string.h>
#include "utsname.h"

int uname(struct utsname *name)
{
    /* Fill in the struct. */
    strlcpy(name->sysname, "FreeNOS", UTSBUF);
    strlcpy(name->nodename, "localhost", UTSBUF);
    strlcpy(name->release, RELEASE, UTSBUF);
    strlcpy(name->version, COMPILER DATETIME, UTSBUF);
    strlcpy(name->machine, ARCH, UTSBUF);

    /* Success. */
    return 0;
}

/*
 * Copyright (C) 2015 Niek Linnenbank
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

#ifndef __HOST__
#include <FreeNOS/System.h>
#endif /* __HOST__ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "StdioLog.h"

StdioLog::StdioLog() : Log()
{
}

void StdioLog::write(const char *str)
{
    ::write(1, str, strlen(str));
}

void StdioLog::terminate() const
{
#ifndef __HOST__
    PrivExec(Panic);
#endif /* __HOST__ */
    ::exit(EXIT_FAILURE);
}

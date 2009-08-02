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
#include <API/ProcessCtl.h>
#include <Macros.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    const char *av[] = { "/bin/sh/sh", "/etc/rc", ZERO };

    /*
     * TODO: give up all priviledges.
     */

    /* Execute the run commands file. */
    forkexec("/bin/sh/sh", av);

    /* Exit immediately. */
    return EXIT_SUCCESS;
}

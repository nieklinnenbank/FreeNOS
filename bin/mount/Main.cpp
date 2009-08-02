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

#include <FileSystemMount.h>
#include <Shared.h>
#include <Types.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    Shared<FileSystemMount> mounts(FILE_SYSTEM_MOUNT_KEY, MAX_MOUNTS);

    /* Print out. */
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
        if (mounts[i]->path[0])
            printf("%s : PID %u\r\n", mounts[i]->path,
				      mounts[i]->procID);
    }
    /* Success. */
    return EXIT_SUCCESS;
}

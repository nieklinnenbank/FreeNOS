/*
 * Copyright (C) 2020 Niek Linnenbank
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

#include <FreeNOS/User.h>
#include "stdlib.h"
#include "errno.h"
#include "signal.h"

extern C int kill(pid_t pid, int sig)
{
    switch (sig)
    {
        case SIGTERM:
        case SIGKILL:
        {
            const API::Result result = ProcessCtl(pid, KillPID, EXIT_FAILURE);
            if (result == API::Success)
            {
                return 0;
            }
            else
            {
                errno = EIO;
                return -1;
            }
        }

        default:
            break;
    }

    // Not supported
    errno = ENOTSUP;
    return -1;
}

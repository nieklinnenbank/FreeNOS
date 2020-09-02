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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <Log.h>
#include "Shell.h"
#include "TimeCommand.h"

TimeCommand::TimeCommand(Shell *shell)
    : ShellCommand("time", 1)
    , m_shell(shell)
{
    m_help = "Measure the execution time of a program";
}

static int local_printtimediff(const struct timeval *t1,
                               const struct timeval *t2)
{
    u64 usec1 = (t1->tv_sec * 1000000) + (t1->tv_usec);
    u64 usec2 = (t2->tv_sec * 1000000) + (t2->tv_usec);

    // Print time measured
    printf("%us %uusec",
            (uint) ((usec2 - usec1) / 1000000),
            (uint) ((usec2 - usec1) % 1000000));
    return 0;
}

int TimeCommand::execute(const Size nparams, const char **params)
{
    struct timeval t1, t2;
    struct timezone zone;

    // Get timestamp before
    gettimeofday(&t1, &zone);

    // Run command
    int result = m_shell->executeInput(nparams, params, false);
    if (result != EXIT_SUCCESS)
    {
        return result;
    }

    // Get timestamp after
    gettimeofday(&t2, &zone);

    // Print time measured
    printf("\r\nTime: ");
    local_printtimediff(&t1, &t2);
    printf("\r\n");
    return result;
}

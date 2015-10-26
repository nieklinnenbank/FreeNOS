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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "ExternalTest.h"

ExternalTest::ExternalTest(const char *name)
    : TestInstance(name)
{
    // TODO: perhaps argc/argv?
}

TestResult ExternalTest::run()
{
    int status;

#ifdef __HOST__
    char tmp[255];

    snprintf(tmp, sizeof(tmp), "%s -n", *m_name);
    status = system(tmp);

    if (WIFEXITED(status))
        status = WEXITSTATUS(status);
#else
    const char *argv[3] = { *m_name, "-n", 0 };

    pid_t pid = forkexec(*m_name, (const char **) argv);
    waitpid(pid, &status, 0);
#endif

    return status == 0 ? OK : FAIL;
}

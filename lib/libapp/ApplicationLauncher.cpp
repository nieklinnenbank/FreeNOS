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

#include <Log.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "ApplicationLauncher.h"

ApplicationLauncher::ApplicationLauncher(const char *path,
                                         const char **argv)
    : m_path(path)
    , m_argv(argv)
    , m_pid(0)
    , m_exitCode(0)
{
}

const ProcessID ApplicationLauncher::getPid() const
{
    return m_pid;
}

const int ApplicationLauncher::getExitCode() const
{
    return WEXITSTATUS(m_exitCode);
}

ApplicationLauncher::Result ApplicationLauncher::exec()
{
    int pid;

#ifdef __HOST__
    struct stat buf;

    if (stat(*m_path, &buf) != 0)
    {
        DEBUG("stat failed for " << *m_path << ": " << strerror(errno));
        return NotFound;
    }

    pid = fork();
    switch (pid)
    {
        case 0:
            execve(*m_path, (char * const *)m_argv, (char * const *)NULL);
            ERROR("execve failed for " << *m_path << ": " << strerror(errno));
            exit(1);
            break;

        case -1:
            ERROR("fork failed for " << *m_path << ": " << strerror(errno));
            break;

        default:
            break;
    }
#else
    pid = forkexec(*m_path, m_argv);
    if (pid == -1)
    {
        DEBUG("forkexec failed for " << *m_path << ": " << strerror(errno));
        return IOError;
    }
#endif

    m_pid = pid;
    return Success;
}

ApplicationLauncher::Result ApplicationLauncher::terminate() const
{
    if (m_pid == 0)
    {
        return InvalidArgument;
    }

    if (::kill(m_pid, SIGTERM) == 0)
    {
        return Success;
    }
    else
    {
        return IOError;
    }
}

ApplicationLauncher::Result ApplicationLauncher::wait()
{
    if (m_pid == 0)
    {
        return InvalidArgument;
    }

    if (waitpid(m_pid, &m_exitCode, 0) == (pid_t) m_pid)
    {
        return Success;
    }
    else if (errno == ESRCH)
    {
        return NotFound;
    }
    else
    {
        return IOError;
    }
}

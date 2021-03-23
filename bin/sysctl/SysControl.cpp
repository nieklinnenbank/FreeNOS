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
#include <Log.h>
#include <RecoveryClient.h>
#include "SysControl.h"

SysControl::SysControl(int argc, char **argv)
    : POSIXApplication(argc, argv)
{
    parser().setDescription("Control program for various system services");
    parser().registerPositional("PID", "List of processes to target", 0);
    parser().registerFlag('s', "stop", "Stop the given process(es)");
    parser().registerFlag('c', "continue", "Continue executing the given process(es)");
    parser().registerFlag('r', "restart", "Restart the given process(es) via recovery server");
}

SysControl::~SysControl()
{
}

SysControl::Result SysControl::exec()
{
    const Vector<Argument *> & positionals = arguments().getPositionals();

    // Loop positional arguments (list of process identifiers)
    for (Size i = 0; i < positionals.count(); i++)
    {
        const ProcessID pid = positionals[i]->getValue().toLong();

        if (arguments().get("stop") != ZERO)
        {
            const Result r = stopProcess(pid);
            if (r != Success)
            {
                return r;
            }
        }
        else if (arguments().get("continue") != ZERO)
        {
            const Result r = resumeProcess(pid);
            if (r != Success)
            {
                return r;
            }
        }
        else if (arguments().get("restart") != ZERO)
        {
            const Result r = restartProcess(pid);
            if (r != Success)
            {
                return r;
            }
        }
        else
        {
            ERROR("no operation specified for PID: " << pid);
            return InvalidArgument;
        }
    }

    return Success;
}

SysControl::Result SysControl::stopProcess(const ProcessID pid) const
{
    DEBUG("pid = " << pid);

    const API::Result result = ProcessCtl(pid, Stop);
    if (result != API::Success)
    {
        ERROR("failed to stop PID " << pid << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

SysControl::Result SysControl::resumeProcess(const ProcessID pid) const
{
    DEBUG("pid = " << pid);

    const API::Result result = ProcessCtl(pid, Resume);
    if (result != API::Success)
    {
        ERROR("failed to resume PID " << pid << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

SysControl::Result SysControl::restartProcess(const ProcessID pid) const
{
    DEBUG("pid = " << pid);

    const RecoveryClient recovery;
    const Recovery::Result result = recovery.restartProcess(pid);
    if (result != Recovery::Success)
    {
        ERROR("failed to restart PID " << pid << ": result = " << (int) result);
        return IOError;
    }

    return Success;
}

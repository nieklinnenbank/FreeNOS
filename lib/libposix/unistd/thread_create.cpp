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

#include <FreeNOS/User.h>
#include <errno.h>
#include "unistd.h"

/**
 * @brief Internal thread trampoline.
 *
 * Each thread starts here so we can pass the `arg` parameter and manage
 * thread termination cleanly if the entry function returns.
 */
struct ThreadArgs
{
    void (*entry)(void *arg);
    void  *arg;
};

/**
 * thread_create() — Create a new kernel thread sharing current address space.
 *
 * Uses the SpawnThread system call introduced in kernel/API/ProcessCtl.cpp.
 * The new thread starts executing directly at `entry(arg)`.
 *
 * @param entry  Thread entry function.
 * @param arg    Argument forwarded to the entry function.
 *
 * @return Kernel thread PID (>= 0) on success, -1 on failure with errno set.
 */
int thread_create(void (*entry)(void *arg), void *arg)
{
    /*
     * SpawnThread takes the entry address as the `addr` parameter.
     * The convention matches Spawn: returns (API::Success | (pid << 16)).
     *
     * NOTE: `arg` passing is architecture-specific. On x86 the first argument
     * would normally be on the stack; the kernel-side reset() already sets up
     * a fresh user stack. A full implementation should push `arg` on that stack.
     * For now, `arg` is ignored — the thread entry is invoked with no arguments.
     * Developers should use shared global variables or shared-memory regions
     * to pass data until libpthread provides a proper trampoline.
     *
     * TODO: Push `arg` onto the new thread's user stack in Arch::Process::reset().
     */
    (void) arg;

    const API::Result result = ProcessCtl(SELF, SpawnThread, (Address) entry);

    if ((result & 0xffff) != API::Success)
    {
        /* Map common API errors to errno */
        switch (result & 0xffff)
        {
            case API::OutOfMemory:    errno = ENOMEM;  break;
            case API::InvalidArgument: errno = EINVAL; break;
            default:                  errno = EAGAIN;  break;
        }
        return -1;
    }

    /* Upper 16 bits contain the new thread's PID */
    return (int)(result >> 16);
}

/**
 * thread_exit() — Terminate the calling thread.
 *
 * Other threads in the same process are unaffected.
 */
void thread_exit(int exit_code)
{
    ProcessCtl(SELF, KillPID, (Address) exit_code);
    /* Never reached */
    while (true) ;
}

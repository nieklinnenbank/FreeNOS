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

#include <TestCase.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>
#include <unistd.h>
#include <sys/wait.h>

TestCase(RunMpiPing)
{
    const char *prog = "/bin/mpiping";
    const char *args[] = { prog, (const char *)NULL };
    int status;
    int pid;

    // Start the MPI ping program
    pid = forkexec(prog, args);
    testAssert(pid != -1);
    testAssert(pid > 0);

    // Wait for it to terminate. Exit status must be zero (success)
    pid_t p = waitpid(pid, &status, 0);
    testAssert(p == (pid_t) pid);
    testAssert(status == 0);

    // Done
    return OK;
}

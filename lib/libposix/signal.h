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

#ifndef __LIB_LIBPOSIX_SIGNAL_H
#define __LIB_LIBPOSIX_SIGNAL_H

#include <sys/types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libposix
 * @{
 */

/**
 * @name ISO C99 Signals
 * @{
 *
 * @note This list based on the bits/signum-generic.h file in glibc.
 */

/** Interactive attention signal. */
#define SIGINT      2

/** Illegal instruction. */
#define SIGILL      4

/** Abnormal termination. */
#define SIGABRT     6

/** Erroneous arithmetic operation. */
#define SIGFPE      8

/** Invalid access to storage. */
#define SIGSEGV    11

/** Termination request. */
#define SIGTERM    15

/**
 * @}
 */

/**
 * @name POSIX.1-2008 Signals
 * @{
 *
 * @note This list based on the bits/signum-generic.h file in glibc.
 */

/** Hangup. */
#define SIGHUP      1

/** Quit. */
#define SIGQUIT     3

/** Trace/breakpoint trap. */
#define SIGTRAP     5

/** Killed. */
#define SIGKILL     9

/** Bus error. */
#define SIGBUS     10

/** Bad system call. */
#define SIGSYS     12

/** Broken pipe. */
#define SIGPIPE    13

/** Alarm clock. */
#define SIGALRM    14

/** Urgent data is available at a socket. */
#define SIGURG     16

/** Stop, unblockable. */
#define SIGSTOP    17

/** Keyboard stop. */
#define SIGTSTP    18

/** Continue. */
#define SIGCONT    19

/** Child terminated or stopped. */
#define SIGCHLD    20

/** Background read from control terminal. */
#define SIGTTIN    21

/** Background write to control terminal. */
#define SIGTTOU    22

/** Pollable event occurred (System V). */
#define SIGPOLL    23

/** CPU time limit exceeded. */
#define SIGXCPU    24

/** File size limit exceeded. */
#define SIGXFSZ    25

/** Virtual timer expired. */
#define SIGVTALRM  26

/** Profiling timer expired. */
#define SIGPROF    27

/** User-defined signal 1. */
#define SIGUSR1    30

/** User-defined signal 2. */
#define SIGUSR2    31

/**
 * @}
 */

/**
 * @name Signal Functions
 * @{
 */

/**
 * @brief Send a signal to a process or a group of processes
 *
 * For a process to have permission to send a signal to a process designated by pid,
 * unless the sending process has appropriate privileges, the real or effective user
 * ID of the sending process shall match the real or saved set-user-ID of the receiving process.
 *
 * If pid is greater than 0, sig shall be sent to the process whose process ID is equal to pid.
 * If pid is 0, sig shall be sent to all processes (excluding an unspecified set of system processes)
 * whose process group ID is equal to the process group ID of the sender, and for which the process
 * has permission to send a signal.
 * If pid is -1, sig shall be sent to all processes (excluding an unspecified set of system processes)
 * for which the process has permission to send that signal.
 * If pid is negative, but not -1, sig shall be sent to all processes (excluding an unspecified set of
 * system processes) whose process group ID is equal to the
 * absolute value of pid, and for which the process has permission to send a signal.
 * If the value of pid causes sig to be generated for the sending process, and if sig is not blocked for
 * the calling thread and if no other thread has sig unblocked or is waiting in a sigwait() function for
 * sig, either sig or at least one pending unblocked signal shall be delivered to the sending thread
 * before kill() returns.
 *
 * The user ID tests described above shall not be applied when sending SIGCONT to a process that is a
 * member of the same session as the sending process.
 *
 * @param pid  The kill() function shall send a signal to a process or a group
 *             of processes specified by pid. 
 * @param sig  The signal to be sent is specified by sig and is either one from the
 *             list given in <signal.h> or 0. If sig is 0 (the null signal), error
 *             checking is performed but no signal is actually sent. The null signal
 *             can be used to check the validity of pid.
 *
 * @return Upon successful completion, 0 shall be returned.
 *         Otherwise, -1 shall be returned and errno set to indicate the error.
 *
 */
extern C int kill(pid_t pid, int sig);

/**
 * @}
 */

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBPOSIX_SIGNAL_H */

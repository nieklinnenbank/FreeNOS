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

#ifndef __LIBPOSIX_UNISTD_H
#define __LIBPOSIX_UNISTD_H

#include <Macros.h>
#include <stdio.h>
#include "sys/types.h"

/**                                                                                                                                                                                                     
 * @defgroup libposix libposix (POSIX.1-2008)
 * @{
 */ 

/**
 * Integer value indicating version of this standard (C-language binding)
 * to which the implementation conforms. For implementations conforming to
 * POSIX.1-2008, the value shall be 200809L.
 */
#define _POSIX_VERSION 200809L

/**
 * Integer value indicating version of the Shell and Utilities volume of
 * POSIX.1 to which the implementation conforms. For implementations
 * conforming to POSIX.1-2008, the value shall be 200809L.
 */
#define _POSIX2_VERSION 200809L

/**
 * Get the process ID.
 * The getpid() function shall return the process ID of the calling process.
 * @return The getpid() function shall always be successful and no return
 *         value is reserved to indicate an error.
 */
extern C pid_t getpid();

/**
 * Get parent process ID.
 *
 * @return The parent process ID.
 */
extern C pid_t getppid();

/**
 * Read from a file
 * @param fildes The read() function shall attempt to read nbyte bytes from the file
 *               associated with the open file descriptor, fildes, into the buffer
 *               pointed to by buf.
 * @param buf The read() function shall attempt to read nbyte bytes from the file
 *            associated with the open file descriptor, fildes, into the buffer
 *            pointed to by buf.
 * @param nbyte If the value of nbyte is greater than {SSIZE_MAX}, the result is implementation-defined.
 * @return Upon successful completion, these functions shall return a non-negative integer
 *         indicating the number of bytes actually read. Otherwise, the functions shall
 *         return -1 and set errno to indicate the error.
 */
extern C ssize_t read(int fildes, void *buf, size_t nbyte);

/**
 * @brief Write on a file.
 *
 * The write() function shall attempt to write nbyte bytes from
 * the buffer pointed to by buf to the file associated with the
 * open file descriptor, fildes.
 *
 * @param fildes File descriptor.
 * @param buf Input buffer.
 * @param nbyte Maximum number of bytes to write.
 * @return Upon successful completion, these functions shall return the number
 *         of bytes actually written to the file associated with fildes. This
 *         number shall never be greater than nbyte. Otherwise, -1 shall be
 *         returned and errno set to indicate the error.
 */
extern C ssize_t write(int fildes, const void *buf, size_t nbyte);

/**
 * Close a file descriptor
 * @param fildes The close() function shall deallocate the file descriptor indicated by fildes.
 * @return Upon successful completion, 0 shall be returned; otherwise, -1 shall
 *         be returned and errno set to indicate the error.
 *
 */
extern C int close(int fildes);

/**
 * @brief Move the read/write file offset.
 *
 * The lseek() function shall set the file offset for the open file
 * description associated with the file descriptor fildes, as follows:
 *
 * If whence is SEEK_SET, the file offset shall be set to offset bytes.
 * If whence is SEEK_CUR, the file offset shall be set to its current location plus offset.
 * If whence is SEEK_END, the file offset shall be set to the size of the file plus offset.
 *
 * @param fildes File descriptor.
 * @param offset New file offset.
 * @param whence Determines how to modify the file offset pointer.
 * @return Upon successful completion, the resulting offset, as measured in bytes
 *         from the beginning of the file, shall be returned. Otherwise, (off_t)-1
 *         shall be returned, errno shall be set to indicate the error, and the file
 *         offset shall remain unchanged.
 */
extern C off_t lseek(int fildes, off_t offset, int whence);

/**
 * @brief Execute a file.
 *
 * The exec family of functions shall replace the current process image with a
 * new process image. The new image shall be constructed from a regular,
 * executable file called the new process image file. There shall be no return
 * from a successful exec, because the calling process image is overlaid by the
 * new process image.
 *
 * @param path File to execute.
 * @param argv The argument argv is an array of character pointers to
 *             null-terminated strings. The application shall ensure that the
 *             last member of this array is a null pointer. These strings shall
 *             constitute the argument list available to the new process image.
 *             The value in argv[0] should point to a filename that is associated
 *             with the process being started by one of the exec functions.
 * @return If one of the exec functions returns to the calling process image,
 *         an error has occurred; the return value shall be -1, and errno shall
 *         be set to indicate the error.
 *
 */
extern C int execv(const char *path, const char *argv[]);

/**
 * @brief Create a new process.
 *
 * The fork() function shall create a new process. The new process (child process)
 * shall be an exact copy of the calling process (parent process) except as
 * detailed below:
 *
 *  - The child process shall have a unique process ID.
 *  - The child process ID also shall not match any active process group ID.
 *  - The child process shall have a different parent process ID, which shall
 *    be the process ID of the calling process.
 *  - The child process shall have its own copy of the parent's file descriptors.
 *    Each of the child's file descriptors shall refer to the same open file
 *    description with the corresponding file descriptor of the parent.
 *  - The child process shall have its own copy of the parent's open directory
 *    streams. Each open directory stream in the child process may share directory
 *    stream positioning with the corresponding directory stream of the parent.
 *  - The child process shall have its own copy of the parent's message catalog descriptors.
 *  - The child process values of tms_utime, tms_stime, tms_cutime, and tms_cstime
 *    shall be set to 0.
 *  - The time left until an alarm clock signal shall be reset to zero, and the
 *    alarm, if any, shall be canceled; see alarm.
 *  - File locks set by the parent process shall not be inherited by the child process.
 *  - The set of signals pending for the child process shall be initialized to the empty set.
 *  - Interval timers shall be reset in the child process.
 *  - Any semaphores that are open in the parent process shall also be open
 *  - in the child process.
 *  - The child process shall not inherit any address space memory locks established
 *    by the parent process via calls to mlockall() or mlock().
 *  - Memory mappings created in the parent shall be retained in the child process.
 *    MAP_PRIVATE mappings inherited from the parent shall also be MAP_PRIVATE mappings
 *    in the child, and any modifications to the data in these mappings made by the parent
 *    prior to calling fork() shall be visible to the child. Any modifications to the data
 *    in MAP_PRIVATE mappings made by the parent after fork() returns shall be visible only
 *    to the parent. Modifications to the data in MAP_PRIVATE mappings made by the child
 *    shall be visible only to the child.
 *  - For the SCHED_FIFO and SCHED_RR scheduling policies, the child process shall inherit
 *    the policy and priority settings of the parent process during a fork() function. For
 *    other scheduling policies, the policy and priority settings on fork() are implementation-defined.
 *  - Per-process timers created by the parent shall not be inherited by the child process.
 *  - The child process shall have its own copy of the message queue descriptors of the parent.
 *    Each of the message descriptors of the child shall refer to the same open message queue
 *    description as the corresponding message descriptor of the parent.
 *  - No asynchronous input or asynchronous output operations shall be inherited by the
 *    child process. Any use of asynchronous control blocks created by the parent produces
 *    undefined behavior.
 *  - A process shall be created with a single thread. If a multi-threaded process calls fork(),
 *    the new process shall contain a replica of the calling thread and its entire address space,
 *    possibly including the states of mutexes and other resources. Consequently, to avoid errors,
 *    the child process may only execute async-signal-safe operations until such time as one of the
 *    exec functions is called. Fork handlers may be established by means of the pthread_atfork()
 *    function in order to maintain application invariants across fork() calls.
 *  - When the application calls fork() from a signal handler and any of the fork
 *    handlers registered by pthread_atfork() calls a function that is not
 *    async-signal-safe, the behavior is undefined.
 *  - If the Trace option and the Trace Inherit option are both supported:
 *    If the calling process was being traced in a trace stream that had its
 *    inheritance policy set to POSIX_TRACE_INHERITED, the child process shall
 *    be traced into that trace stream, and the child process shall inherit the
 *    parent's mapping of trace event names to trace event type identifiers. If
 *    the trace stream in which the calling process was being traced had its
 *    inheritance policy set to POSIX_TRACE_CLOSE_FOR_CHILD, the child process
 *    shall not be traced into that trace stream. The inheritance policy is set by
 *    a call to the posix_trace_attr_setinherited() function.
 *  - If the Trace option is supported, but the Trace Inherit option is not supported:
 *    The child process shall not be traced into any of the trace streams of
 *    its parent process.
 *  - If the Trace option is supported, the child process of a trace controller
 *    process shall not control the trace streams controlled by its parent process.
 *  - The initial value of the CPU-time clock of the child process shall be set to zero.
 *  - The initial value of the CPU-time clock of the single thread of the child process
 *    shall be set to zero.
 *
 * All other process characteristics defined by POSIX.1-2008 shall be the same in the
 * parent and child processes. The inheritance of process characteristics not defined
 * by POSIX.1-2008 is unspecified by POSIX.1-2008.
 * After fork(), both the parent and the child processes shall be capable of executing
 * independently before either one terminates.
 *
 * @return Upon successful completion, fork() shall return 0 to the child process and
 *         shall return the process ID of the child process to the parent process. Both
 *         processes shall continue to execute from the fork() function. Otherwise, -1
 *         shall be returned to the parent process, no child process shall be created,
 *         and errno shall be set to indicate the error.
 */
extern C pid_t fork(void);

/**
 * @brief Create a new process and execute program.
 * @param path File to execute.
 * @param argv Argument list pointer.
 * @return New process ID on success and -1 on failure. Errno is set with the
 *         appropriate error code on failure.
 */
extern C int forkexec(const char *path, const char *argv[]);

/**
 * @brief Create a new process using in-memory image.
 */
extern C int spawn(Address program, Size programSize, const char *command);

/**
 * @brief Get name of current host.
 *
 * The gethostname() function shall return the standard host name for the
 * current machine. The namelen argument shall specify the size of the
 * array pointed to by the name argument. The returned name shall be
 * null-terminated, except that if namelen is an insufficient length to
 * hold the host name, then the returned name shall be truncated and it
 * is unspecified whether the returned name is null-terminated.
 * Host names are limited to {HOST_NAME_MAX} bytes.
 * 
 * @param name Output buffer.
 * @param namelen Size of the output buffer.
 * @return Upon successful completion, 0 shall be returned; otherwise,
 *         -1 shall be returned.
 */
extern C int gethostname(char *name, size_t namelen);

/**
 * @brief Get the pathname of the current working directory.
 *
 * The getcwd() function shall place an absolute pathname of the
 * current working directory in the array pointed to by buf, and
 * return buf. The pathname shall contain no components that are
 * dot or dot-dot, or are symbolic links.
 *
 * @param buf Output buffer, to store the current directory.
 * @param size Size of the output buffer.
 * @return Upon successful completion, getcwd() shall return
 *         the buf argument. Otherwise, getcwd() shall return
 *         a null pointer and set errno to indicate the error.
 *         The contents of the array pointed to by buf are then undefined.
 */
extern C char *getcwd(char *buf, size_t size);

/**
 * @brief Change working directory.
 *
 * The chdir() function shall cause the directory named by the
 * pathname pointed to by the path argument to become the current
 * working directory; that is, the starting point for path searches
 * for pathnames not beginning with '/' .
 *
 * @param path Path to the directory to use as working directory.
 * @return Upon successful completion, 0 shall be returned. Otherwise,
 *         -1 shall be returned, the current working directory shall
 *         remain unchanged, and errno shall be set to indicate the error.
 */
extern C int chdir(const char *path);

/**
 * @brief Remove a file from the filesystem.
 * @param path Parh to the file to remove.
 * @return Zero on success or -1 on failure with errno set.
 */
extern C int unlink(const char *path);

/**
 * Sleep for the specified number of seconds.
 *
 * @param seconds Number of seconds to sleep
 * @return Zero on success or number of seconds left when interrupted.
 */
extern C unsigned int sleep(unsigned int seconds);

/**
 * @}
 */

#endif /* __LIBPOSIX_UNISTD_H */

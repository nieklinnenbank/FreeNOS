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

#include <api/IPCMessage.h>
#include <arch/Process.h>
#include <MemoryServer.h>
#include <FileSystemMessage.h>
#include <VirtualFileSystem.h>
#include <Config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fcntl.h>
#include "Shell.h"

Shell::Shell()
{
    printf("Shell(): PID %u\n", getpid());
}

int Shell::run()
{
    char *cmd;

    /* Read commands. */    
    while (true)
    {
	prompt();
	cmd = getCommand();

	if (strcmp(cmd, "ps") == 0)
	    ps();
	else if (strcmp(cmd, "uname") == 0)
	    doUname();
	else if (strcmp(cmd, "memstat") == 0)
	    memstat();
	else if (strcmp(cmd, "mount") == 0)
	    mount();
	else if (strncmp(cmd, "cat ", 3) == 0)
	    cat(cmd + 4);
	else if (strncmp(cmd, "ls", 2) == 0)
	    ls(cmd + 3);
	else if (strcmp(cmd, "help") == 0)
	    help();
	else
	    printf("Command not found: '%s'\n", cmd);
    }
    return 0;
}

char * Shell::getCommand()
{
    static char line[1024];
    Size total = 0;

    /* Read a line. */
    while (total < sizeof(line))
    {
        /* Read a character. */
	getc(line + total);
	
	/* End of line reached? */
	if (line[total] != '\r' && line[total] != '\n')
	    printf("%c", line[total++]);
	else
	{
	    printf("\n"); break;
	}
    }
    line[total] = ZERO;
    return line;
}

void Shell::prompt()
{
    printf("# ");
}

void Shell::ps()
{
    DIR *d;
    struct dirent *dent;
    
    /* Attempt to open the directory. */
    if (!(d = opendir("/proc/")))
    {
	printf("Failed to open '/proc/': %s\n",
		strerror(errno));
	return;
    }
    printf("PID STATUS CMD\n");
    
    /* Read directory. */
    while ((dent = readdir(d)))
    {
	printf("%s ", dent->d_name);
	catFmt("/proc/%s/status",  dent->d_name);
	printf(" ");
	catFmt("/proc/%s/cmdline", dent->d_name);
	printf("\n");
    }
    /* Close it. */
    closedir(d);
}

void Shell::doUname()
{
    struct utsname info;
    
    if (uname(&info) >= 0)
    {
	printf("%s %s %s %s %s\n",
		info.sysname,
		info.nodename,
		info.release,
		info.version,
		info.machine);
    }
}

void Shell::memstat()
{
    MemoryMessage msg;
    
    /* Query stats. */
    msg.action = MemoryUsage;
    
    /* Ask memory server for memory stats. */
    IPCMessage(MEMSRV_PID, SendReceive, &msg);
    
    /* Print it. */
    printf("Total:     %u KB\n"
	   "Available: %u KB\n",
	   msg.bytes / 1024, msg.bytesFree / 1024);
}

void Shell::mount()
{
    FileSystemMessage msg;
    FileSystemMount mounts[MAX_MOUNTS];
    
    /* Ask filesystem for active mounts. */
    msg.action = MountInfo;
    msg.buffer = (char *) &mounts;
    msg.size   = sizeof(mounts);
    
    /* Trap. */
    IPCMessage(VFSSRV_PID, SendReceive, &msg);
    
    /* Print out. */
    for (Size i = 0; i < MAX_MOUNTS; i++)
    {
	if (mounts[i].path[0])
	    printf("%s\n", mounts[i].path);
    }
}

void Shell::catFmt(char *fmt, ...)
{
    char path[128];
    va_list args;
    
    /* Format the path. */
    va_start(args, fmt);
    vsnprintf(path, sizeof(path), fmt, args);
    va_end(args);
    
    /* Invoke cat. */
    cat(path);
}

void Shell::cat(char *file)
{
    char buf[1024];
    int fd;
    
    /* Clear buffer. */
    memset(buf, 0, sizeof(buf));
    
    /* Attempt to open the file first. */
    if ((fd = open(file, ZERO)) < 0)
    {
	printf("Failed to open '%s': %s\n",
	        file, strerror(errno));
	return;
    }
    /* Read contents. */
    if (read(fd, buf, sizeof(buf)) < 0)
    {
	printf("Failed to read '%s': %s\n",
		file, strerror(errno));
	close(fd);
	return;
    }
    /* Success! Print out results. */
    printf("%s", buf);
    close(fd);
}

void Shell::ls(char *dir)
{
    DIR *d;
    struct dirent *dent;
    
    /* Attempt to open the directory. */
    if (!(d = opendir(dir)))
    {
	printf("Failed to open '%s': %s\n",
		dir, strerror(errno));
	return;
    }
    /* Read directory. */
    while ((dent = readdir(d)))
    {
	printf("%s ", dent->d_name);
    }
    printf("\n");
    /* Close it. */
    closedir(d);
}

void Shell::help()
{
    printf("ps         - Output list of Processes\n"
	   "uname      - Print UNIX name\n"
	   "memstat    - Memory statistics\n"
	   "mount      - Shows active mount information\n"
	   "cat <file> - Print contents of a file\n"
	   "ls <dir>   - List directory contents\n"
	   "help       - This message\n");
}

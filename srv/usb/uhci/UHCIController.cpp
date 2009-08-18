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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char path[256];
    int fd;
    u8 irq;

    /*
     * Verify command-line arguments.
     */
    if (argc < 4)
    {
	return EXIT_SUCCESS;
    }
    /* Open the system log. */
    openlog("USB", LOG_PID | LOG_CONS, LOG_USER);
    
    /* Construct path to read PCI interrupt. */
    snprintf(path, sizeof(path), "/dev/pci/%s/%s/%s/interrupt",
	     argv[1], argv[2], argv[3]);
    
    /* Attempt to open the file. */
    if ((fd = open(path, O_RDONLY)) < 0)
    {
	syslog(LOG_ERR, "failed to open() `%s': %s",
	       path, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Read out PCI information. */
    if ((read(fd, &irq, sizeof(irq))) < 0)
    {
	syslog(LOG_ERR, "failed to read() `%s': %s",
	       path, strerror(errno));
	return EXIT_FAILURE;
    }
    syslog(LOG_INFO, "UHCI Host Controller at IRQ %d", irq);

    /* All done. */
    close(fd);
    closelog();
    return EXIT_SUCCESS;
}

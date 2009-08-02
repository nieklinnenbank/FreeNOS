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

#include <API/IPCMessage.h>
#include <FileSystem.h>
#include <FileSystemPath.h>
#include <ProcessMessage.h>
#include <Config.h>
#include <Macros.h>
#include <String.h>
#include <List.h>
#include <ListIterator.h>
#include "ChangeDirCommand.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int ChangeDirCommand::execute(Size nparams, char **params)
{
    ProcessMessage msg;
    FileSystemPath path;
    String *last = ZERO;
    List<String> lst;
    char cwd[PATHLEN], buf[PATHLEN];
    struct stat st;

    /* First stat the file. */
    if (stat(params[0], &st) != 0)
    {
	printf("Failed to stat() `%s': %s\r\n",
		params[0], strerror(errno));
	return errno;
    }
    /* Must be a directory. */
    if (!S_ISDIR(st.st_mode))
    {
	printf("`%s' is not a directory\r\n",
		params[0]);
	return -1;
    }
    /* What's the current working dir? */
    getcwd(cwd, PATHLEN);
    
    /* Relative or absolute? */
    if (params[0][0] != '/')
    {
	snprintf(buf, sizeof(buf), "%s/%s", cwd, params[0]);
	path.parse(buf);
	memset(buf, 0, sizeof(buf));
	
	/* Process '..' */
	for (ListIterator<String> i(path.split()); i.hasNext(); i++)
	{
	    if ((**i.current())[0] != '.')
	    {
		lst.insertTail(i.current());
		last = i.current();
	    }
	    else if ((**i.current())[1] == '.' && last)
	    {
		lst.remove(last);
	    }
	}
	/* Construct final path. */
	for (ListIterator<String> i(&lst); i.hasNext(); i++)
	{
	    strcat(buf, "/");
	    strcat(buf, **i.current());
	}
	msg.path = buf;
    }
    else
	msg.path = params[0];
    
    /* Fall back to slash? */
    if (!buf[0])
    {
	strcpy(buf, "/");
    }
    /* Fill the message. */
    msg.action = SetCurrentDirectory;
    
    /* Ask the process server. */
    IPCMessage(PROCSRV_PID, SendReceive, &msg, sizeof(msg));

    /* Done. */
    return msg.result;
}

INITOBJ(ChangeDirCommand, cdCmd, LIBCRT_DEFAULT)

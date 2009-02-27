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

#include "ProcessServer.h"

ProcessServer::ProcessServer()
{
}
	
int ProcessServer::run()
{
    ProcessMessage msg, reply;

    /* Enter loop. */
    while (true)
    {
	/* Now wait for a message. */
	IPCMessage(ZERO, Receive, &msg);
	
	/* Handle incoming request. */
	switch (msg.action)
	{
	    case GetID:
		doGetID(&msg, &reply);
		break;

	    default:
		continue;
	}
	/* Send reply. */
	IPCMessage(msg.from, Send, &reply);
    }
    /* Satify compiler. */
    return 0;
}

void ProcessServer::doGetID(ProcessMessage *msg, ProcessMessage *reply)
{
    reply->number = msg->from;
    reply->action = ProcessOK;
}

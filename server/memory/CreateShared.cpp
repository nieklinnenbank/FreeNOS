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

#include <API/VMCopy.h>
#include "MemoryServer.h"
#include "MemoryMessage.h"

void MemoryServer::createShared(MemoryMessage *msg)
{
    char key[256];
    Error num = msg->keyLength < sizeof(key) ?
	        msg->keyLength : sizeof(key);
    
    /* Obtain key. */
    if ((msg->result = VMCopy(msg->from, Read, (Address) key,
			     (Address) msg->key, num)) != num)
    {
	return;
    }
    /* Null-terminate. */
    key[num] = ZERO;
    
    /* Insert shared mapping. */
    insertShared(msg->from, key, msg->bytes, msg, &msg->created);
}

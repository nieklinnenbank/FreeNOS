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

#ifndef __SERVER_CORE_USERPROCESS_H
#define __SERVER_CORE_USERPROCESS_H
        
/**  
 * @defgroup core CoreServer
 * @{    
 */

#ifndef __KERNEL__

#include <FreeNOS/Process.h>
#include <FileSystemPath.h>
#include <Types.h>

#endif /* __KERNEL__ */

/** Maximum length of a command (as saved in the user process table). */
#define COMMANDLEN PATHLEN

/** Virtual memory address of the array of arguments for new processes. */
#define ARGV_ADDR  0x9ffff000

/** Maximum size of each argument. */
#define ARGV_SIZE  128

/** Number of arguments at maximum. */
#define ARGV_COUNT (PAGESIZE / ARGV_SIZE)

#ifndef __KERNEL__

/**
 * Userlevel process information.
 */
typedef struct UserProcess
{
    /** Command string. */
    char command[COMMANDLEN];

    /** User identity. */
    UserID userID;
    
    /** Group identity. */
    GroupID groupID;
    
    /** Process state. */
    Process::State state;

    /** Parent Process ID. */
    ProcessID parent;
    
    /** Waits for exit of this Process. */
    ProcessID waitProcessID;
}
UserProcess;

#endif /* __KERNEL__ */

/**
 * @}
 */

#endif /* __SERVER_CORE_USERPROCESS_H */

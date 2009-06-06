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

#ifndef __KERNEL_PROCESS_H
#define __KERNEL_PROCESS_H
#ifndef __ASSEMBLER__

#include <FreeNOS/Interrupt.h>
#include <Types.h>
#include <Vector.h>
#include <List.h>
#include "Scheduler.h"

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/** Maximum number of processes. */
#define MAX_PROCS 1024

/** @see IPCMessage.h. */
class UserMessage;

/**
 * Enumeration of possible state in which a Process can be.
 */
typedef enum ProcessState
{
    Running   = 0,
    Ready     = 1,
    Stopped   = 2,
    Sleeping  = 3,
}
ProcessState;

/**
 * Represents a process which may run on the host.
 */
class Process
{
    public:
    
	/**
	 * Constructor function.
	 * @param entry Initial program counter value.
	 */
	Process(Address entry);
    
	/**
	 * Destructor function.
	 */
	virtual ~Process();

	/**
	 * Retrieve our ID number.
	 * @return Process Identification number.
	 */
	ProcessID getID()
	{
	    return pid;
	}
	
	/**
	 * Retrieves the current state.
	 * @return Current status of the Process.
	 */
	ProcessState getState()
	{
	    return status;
	}

	/**
	 * Puts the Process in a new state.
	 * @param st New state of the Process.
	 */
	void setState(ProcessState st)
	{
	    status = st;
	}

	/**
	 * Request this Process to be awoken by the Scheduler.
	 */
	void wakeup()
	{
	    ulong t = irq_disable();
	    wakeups.insertTail(this);
	    irq_restore(t);
	}

	/**
	 * Fetch the list of processes waiting to be woken up.
	 * @return Pointer to the List of Proccess to be awoken.
	 */
	static List<Process> * getWakeups()
	{
	    return &wakeups;
	}

	/**
	 * Retrieve the list of Messages for this Process.
	 * @return Pointer to the message queue.
	 */
	List<UserMessage> * getMessages()
	{
	    return &messages;
	}

	/**
	 * Retrieve the process table.
	 * @return Pointer to the process table.
	 */
	Vector<Process> * getProcessTable()
	{
	    return &procs;
	}

	/**
	 * Compare two Processes.
	 * @param p Other Process to compare us with.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (Process *p);

	/**
	 * Retrieve a Process by it's ID.
	 * @param id ProcessID number.
	 * @return Pointer to the appropriate process or ZERO if not found.
	 */
	static ArchProcess * byID(ProcessID id);	

	/**
	 * (Dis)allow access to an I/O port.
	 * @param port I/O port to (dis)allow.
	 * @param enabled Allow or disallow.
	 */
	virtual void IOPort(u16 port, bool enabled) = 0;

	/**
	 * Allow the Process to run on the CPU.
	 */
	virtual void execute() = 0;

    private:
    
	/** Current process status. */
	ProcessState status;
	
	/** Unique ID number. */
	ProcessID pid;
	
	/** Incoming messages. */
	List<UserMessage> messages;

	/** Processes waiting to be woken up. */
	static List<Process> wakeups;
	
	/** All known Processes. */
	static Vector<Process> procs;
};

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __KERNEL_PROCESS_H */

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

#include <Types.h>
#include <Array.h>
#include <List.h>

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
        ProcessID getID();
        
        /**
         * Retrieves the current state.
         * @return Current status of the Process.
         */
        ProcessState getState();

        /**
         * Puts the Process in a new state.
         * @param st New state of the Process.
         */
        void setState(ProcessState st);

        /**
         * Request this Process to be awoken by the Scheduler.
         */
        void wakeup();

        /**
         * Fetch the list of processes waiting to be woken up.
         * @return Pointer to the List of Proccess to be awoken.
         */
        static List<Process> * getWakeups();

        /**
         * Retrieve the list of Messages for this Process.
         * @return Pointer to the message queue.
         */
        List<UserMessage> * getMessages();

        /**
         * Retrieve the process table.
         * @return Pointer to the process table.
         */
        Array<Process> * getProcessTable();

        /**
         * Compare two processes.
         * @param p Process to compare with.
         * @return True if equal, false otherwise.
         */
        bool operator == (Process *p);

        /**
         * Retrieve a Process by it's ID.
         * @param id ProcessID number.
         * @return Pointer to the appropriate process or ZERO if not found.
         */
        static Process * byID(ProcessID id);        

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
        static Array<Process> procs;
};

/**
 * @}
 */

#endif /* __ASSEMBLER__ */
#endif /* __KERNEL_PROCESS_H */

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
#include <Macros.h>

/** Process Identifier */
typedef u16 ProcessID;

#ifdef CPP

#include <Array.h>
#include <List.h>

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/** @see IPCMessage.h. */
class UserMessage;
class ProcessScheduler;

/**
 * Represents a process which may run on the host.
 */
class Process
{
  public:

    enum State
    {
        Running,
        Ready,
        Stopped,
        Sleeping
    };    
    
    /**
     * Constructor function.
     * @param entry Initial program counter value.
     */
    Process(ProcessID id, Address entry);
    
    /**
     * Destructor function.
     */
    virtual ~Process();

    /**
     * Retrieve our ID number.
     * @return Process Identification number.
     */
    ProcessID getID() const;
        
    /**
     * Retrieves the current state.
     * @return Current status of the Process.
     */
    State getState() const;

    /**
     * Puts the Process in a new state.
     * @param st New state of the Process.
     */
    void setState(State st);

    /**
     * Retrieve the list of Messages for this Process.
     * @return Pointer to the message queue.
     */
    List<UserMessage> * getMessages();

    /**
     * Compare two processes.
     * @param p Process to compare with.
     * @return True if equal, false otherwise.
     */
    bool operator == (Process *proc);

    /**
     * Allow the Process to run on the CPU.
     */
    virtual void execute() = 0;

private:

    /** Process Identifier */
    const ProcessID m_id;
    
    /** Current process status. */
    State m_state;
        
    /** Incoming messages. */
    List<UserMessage> m_messages;
};

/**
 * @}
 */

#endif /* CPP */
#endif /* __ASSEMBLER__ */
#endif /* __KERNEL_PROCESS_H */

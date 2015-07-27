/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <Types.h>
#include <Macros.h>
#include <List.h>

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/** @see IPCMessage.h. */
struct Message;

/** Virtual memory address of the array of arguments for new processes. */
#define ARGV_ADDR  0x9ffff000

/** Maximum size of each argument. */
// TODO: ARGV_ADDR fixed address here is wrong. Put it in Arch::Memory::range(). Or perhaps put it on the stack?
#define ARGV_SIZE  128

/** Number of arguments at maximum. */
#define ARGV_COUNT (PAGESIZE / ARGV_SIZE)

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
        Sleeping,
        Waiting
    };    
    
    /**
     * Constructor function.
     *
     * @param id Process Identifier
     * @param entry Initial program counter value.
     * @param privileged If true, the process has unlimited access to hardware.
     */
    Process(ProcessID id, Address entry, bool privileged);
    
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
     * Retrieve our parent ID.
     * @return Process ID of our parent.
     */
    ProcessID getParent() const;

    /**
     * Get Wait ID.
     */
    ProcessID getWait() const;

    /**
     * Retrieves the current state.
     * @return Current status of the Process.
     */
    State getState() const;

    /*
     * Get the address of our page directory.
     * @return Page directory address.
     */
    Address getPageDirectory() const;

    /**
     * Get the address of the user stack.
     *
     * @return User stack address.
     */
    Address getUserStack() const;

    /**
     * Get the address of the kernel stack.
     *
     * @return Kernel stack address.
     */
    Address getKernelStack() const;

    /**
     * Get privilege.
     *
     * @return Privilege of the Process.
     */
    bool isPrivileged() const;

    /**
     * Puts the Process in a new state.
     * @param st New state of the Process.
     */
    void setState(State st);

    /**
     * Set parent process ID.
     */
    void setParent(ProcessID id);

    /**
     * Set Wait ID.
     */
    void setWait(ProcessID id);

    /**
     * Set page directory address.
     *
     * @param addr New page directory address.
     */
    void setPageDirectory(Address addr);

    /**
     * Sets the address of the user stack.
     *
     * @param addr New stack address.
     */
    void setUserStack(Address addr);

    /**
     * Set the kernel stack address.
     *
     * @param addr New kernel stack address.
     */
    void setKernelStack(Address addr);

    /**
     * Retrieve the list of Messages for this Process.
     * @return Pointer to the message queue.
     */
    List<Message *> * getMessages();

    /**
     * Compare two processes.
     * @param p Process to compare with.
     * @return True if equal, false otherwise.
     */
    bool operator == (Process *proc);

    /**
     * Allow the Process to run on the CPU.
     *
     * @param previous The previous Process which ran on the CPU. ZERO if none.
     */
    virtual void execute(Process *previous) = 0;

  protected:

    /** Process Identifier */
    const ProcessID m_id;

    /** Parent process */
    ProcessID m_parent;

    /** Current process status. */
    State m_state;

    /** Waits for exit of this Process. */
    ProcessID m_waitId;

    /** Privilege level */
    bool m_privileged;

    /** Incoming messages. */
    List<Message *> m_messages;

    /** Page directory. */
    Address m_pageDirectory;

    /** User stack address. */
    Address m_userStack;

    /** Kernel stack address. */
    Address m_kernelStack;
};

/**
 * @}
 */

#endif /* __KERNEL_PROCESS_H */

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

#ifndef __API_IPCMESSAGE_H
#define __API_IPCMESSAGE_H

#include <arch/Process.h>
#include <arch/API.h>
#include <Macros.h>
#include <Types.h>
#include <Error.h>
#include <Config.h>

/** SystemCall number for IPCMessage(). */
#define IPCMESSAGE 1

/**
 * Forward declaration.
 * @see Message
 */
class Message;

/**
 * Prototype for user applications.
 * @param proc Remote process to send/receive from.
 * @param action Either Send or Receive.
 * @param msg Message buffer.
 * @return Instance of a SystemInfo object.
 */
inline int IPCMessage(ProcessID proc, Action action, Message *msg)
{
    return trapKernel3(IPCMESSAGE, proc, action, (ulong) msg);
}


/**
 * Determines the type a Message can be.
 */
typedef enum MessageType
{
    IPCType   = 0,
    IRQType   = 1,
    FaultType = 2,
}
MessageType;

/**
 * Inter Process Communication (IPC) message.
 * Derived classes must have 6 ulong data members.
 */
class Message
{
    public:
    
	/**
	 * Default constructor.
	 */
	Message() : from(ZERO), type(IPCType)
	{
	}
    
	/**
	 * Constructor function.
	 * @param t Message type.
	 * @param p ProcessID value.
	 */
	Message(MessageType t, ProcessID p) : from(p), type(t)
	{
	}

	/**
	 * Copy constructor function.
	 */
	Message(Message *m) : from(m->from), type(m->type)
	{
	}
	
	/**
	 * Perform IPC operation to a given process.
	 * @param pid Process to IPC to/from.
	 * @param action Determines the action to perform.
	 */
	Error ipc(ProcessID pid, Action action = SendReceive)
	{
	    return IPCMessage(pid, action, this);
	}

	/** At minimum, we must know the origin. */
	ProcessID from;
	
	/** Message type. */
	MessageType type;
};

/**
 * Message send by any usermode process.
 */
class UserMessage : public Message
{
    public:
    
	/**
	 * Default constructor function.
	 */
	UserMessage() : Message(IPCType, ZERO), d1(0), d2(0), d3(0),
						d4(0), d5(0), d6(0)
	{
	}
	
	/**
	 * Copy constructor.
	 * @param u UserMessage instance pointer.
	 */
	UserMessage(UserMessage *u) : Message(u), d1(u->d1), d2(u->d2), d3(u->d3),
						  d4(u->d4), d5(u->d5), d6(u->d6)
	{
	}

	/**
	 * Comparision operator.
	 * @param u UserMessage instance pionter to compare with.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (UserMessage *u)
	{
	    return d1 == u->d1 && d2 == u->d2 && d3 == u->d3 &&
		   d4 == u->d4 && d5 == u->d5 && d6 == u->d6;
	}

	/** User messages have exactly six ulong's as data. */
        ulong d1, d2, d3, d4, d5, d6;
};

/**
 * Send by the kernel, when an IRQ has been received.
 */
class InterruptMessage : public Message
{
    public:
    
	/**
	 * Default constructor function.
	 */
	InterruptMessage(ulong v) :
	    Message(IRQType, KERNEL_PID), vector(v)
	{
	}

	/** Interrupt vector. */
	ulong vector;
	
	/** Not used. */
	ulong unused[5];
};


#endif /* __API_IPCMESSAGE_H */

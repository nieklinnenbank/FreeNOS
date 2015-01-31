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

#include <FreeNOS/Scheduler.h>
#include <FreeNOS/API.h>
#include <Macros.h>
#include <Types.h>
#include <Error.h>
#include <ProcessID.h>
#include <MemoryBlock.h>

/** 
 * @defgroup kernelapi kernel (API)
 * @{ 
 */

/** SystemCall number for IPCMessage(). */
#define IPCMESSAGE 1

/** Maximum size of an Message, in bytes. */
#define MAX_MESSAGE_SIZE 64

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
 * @param sz Size of message.
 * @return Zero on success and error code on failure.
 */
inline Error IPCMessage(ProcessID proc, Operation action, Message *msg, Size sz)
{
    return trapKernel4(IPCMESSAGE, proc, action, (ulong) msg, sz);
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
	 * @param sz Size of message.
	 */
	Error ipc(ProcessID pid, Operation action, Size sz)
	{
	    return IPCMessage(pid, action, this, sz);
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
	 * @param u Input  message.
	 * @param sz Size of the message.
	 */
	UserMessage(Message *u, Size sz) : Message(u), size(sz)
	{
	    data = new s8[size];
	    MemoryBlock::copy(data, u, size);
	}

	/**
	 * Destructor function.
	 */
	~UserMessage()
	{
	    delete data;
	}

	/**
	 * Comparision operator.
	 * @param u UserMessage instance pionter to compare with.
	 * @return True if equal, false otherwise.
	 */
	bool operator == (UserMessage *u)
	{
	    return data == u->data && size == u->size;
	}

	/** User data. */
	s8 *data;
	
	/** Size of user data. */
	Size size;
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
};

/**
 * @}
 */

#endif /* __API_IPCMESSAGE_H */

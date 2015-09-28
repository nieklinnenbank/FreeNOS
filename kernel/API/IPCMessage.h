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

#ifndef __API_IPCMESSAGE_H
#define __API_IPCMESSAGE_H

#include <FreeNOS/API.h>
#include <FreeNOS/System.h>
#include <Macros.h>
#include <Types.h>
#include <MemoryBlock.h>
#include "ProcessID.h"

/** 
 * @defgroup kernelapi kernel (API)
 * @{ 
 */

/** Maximum size of an Message in bytes. */
#define MAX_MESSAGE_SIZE 64

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
 */
typedef struct Message
{
    /** At minimum, we must know the origin. */
    ProcessID from;
    
    /** Message type. */
    MessageType type;
}
Message;

/**
 * Send by the kernel, when an IRQ has been received.
 */
typedef struct InterruptMessage : public Message
{
    /** Interrupt vector. */
    ulong vector;
}
InterruptMessage;

typedef struct UserMessage : public Message
{
    u8 data[MAX_MESSAGE_SIZE - sizeof(ProcessID) - sizeof(MessageType)];
}
UserMessage;

/**
 * Prototype for user applications.
 * @param proc Remote process to send/receive from.
 * @param action Either Send or Receive.
 * @param msg Message buffer.
 * @param sz Size of message.
 * @return Zero on success and error code on failure.
 */
inline Error IPCMessage(ProcessID proc, API::Operation action, Message *msg, Size sz)
{
    return trapKernel4(API::IPCMessageNumber, proc, action, (ulong) msg, sz);
}

/**
 * Prototype for the kernel handler implementation.
 */
extern Error IPCMessageHandler(ProcessID id, API::Operation action, Message *msg, Size size);

/**
 * @}
 */

#endif /* __API_IPCMESSAGE_H */

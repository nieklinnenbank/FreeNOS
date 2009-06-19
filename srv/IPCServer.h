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

#ifndef __IPCSERVER_H
#define __IPCSERVER_H

#include <API/IPCMessage.h>
#include <Error.h>
#include <Config.h>

/**
 * Message handler function (dummy) container.
 */
template <class Func> struct MessageHandler
{
    /**
     * Constructor function.
     * @param f Function to execute.
     * @param r Send a reply?
     */
    MessageHandler(Func f, bool r) : exec(f), sendReply(r)
    {}
    
    /** Handler function. */
    Func exec;
    
    /** Whether to send a reply or not. */
    bool sendReply;
};

/**
 * Template class which serves incoming messages, using MessageHandlers.
 * @param MsgType Type of Message to serve.
 */
template <class Base, class MsgType> class IPCServer
{
    /** Member function pointer inside Base, to handle IPC messages. */
    typedef void (Base::*IPCHandlerFunction)(MsgType *);
    
    /** Member function pointer inside Base, to handle IRQ messages. */
    typedef void (Base::*IRQHandlerFunction)(InterruptMessage *);

    public:

        /**
         * Constructor function.
	 * @param num Number of message handlers to support.
         */
        IPCServer(Base *inst, Size num = 32)
	    : sendReply(true), instance(inst)
        {
	    ipcHandlers = new Array<MessageHandler<IPCHandlerFunction> >(num);
	    irqHandlers = new Array<MessageHandler<IRQHandlerFunction> >(num);
	}

	/**
	 * Destructor function.
	 */
	~IPCServer()
	{
	    delete ipcHandlers;
	    delete irqHandlers;
	}

        /**
         * Enters an infinite loop, serving incoming terminal requests.
         * @return Never.
         */
        int run()
	{
	    MsgType msg;
	    InterruptMessage *imsg = (InterruptMessage *) &msg;

    	    /* Enter loop. */
	    while (true)
	    {
		/* Reset. */
		sendReply = true;
	    
		/* Now wait for a message. */
		IPCMessage(ANY, Receive, &msg, sizeof(MsgType));

		/* Handle the message. */
		switch (msg.type)
		{			
		    case IPCType:
			if ((*ipcHandlers)[msg.action])
			{
			    sendReply =  (*ipcHandlers)[msg.action]->sendReply;
			    (instance->*((*ipcHandlers)[msg.action])->exec) (&msg);
			}
			break;

		    case FaultType:
			break;

		    case IRQType:
			if ((*irqHandlers)[imsg->vector])
			{
			    sendReply = false;
			    (instance->*((*irqHandlers)[imsg->vector])->exec) (imsg);
			}
			
		    default:
			continue;
		}
		/* Send Reply. */
		if (sendReply)
		{
		    IPCMessage(msg.from, Send, &msg, sizeof(MsgType));
		}
	    }
    	    /* Satify compiler. */
	    return 0;
	}
    
	/**
	 * Register a new IPC message action handler.
	 * @param slot Action value to trigger h.
	 * @param h Handler to execute.
	 * @param r Does the handler need to send a reply (per default) ?
	 */
	void addIPCHandler(Size slot, IPCHandlerFunction h, bool sendReply = true)
	{
	    ipcHandlers->insert(slot, new MessageHandler<IPCHandlerFunction>(h, sendReply));
	}
	
	/**
	 * Register a new IRQ message vector handler
	 * @param slot Array value to trigger h.
	 * @param h Handler to execute.
	 */
	void addIRQHandler(Size slot, IRQHandlerFunction h)
	{
	    irqHandlers->insert(slot, new MessageHandler<IRQHandlerFunction>(h, false));
	}

    protected:

	/** Should we send a reply message? */
	bool sendReply;

    private:
    
	/** IPC handler functions. */
	Array<MessageHandler<IPCHandlerFunction> > *ipcHandlers;
	
	/** IRQ handler functions. */
	Array<MessageHandler<IRQHandlerFunction> > *irqHandlers;
	
	/** Server object instance. */
	Base *instance;
};

#endif /* __IPCSERVER_H */

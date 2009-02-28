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

#include <api/IPCMessage.h>
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
     */
    MessageHandler(Func f) : exec(f)
    {}
    
    /** Handler function. */
    Func exec;
};

/**
 * Template class which serves incoming messages, using MessageHandlers.
 * @param MsgType Type of Message to serve.
 */
template <class Base, class MsgType> class IPCServer
{
    /** Member function pointer inside Base, to handle IPC messages. */
    typedef void (Base::*IPCHandlerFunction)(MsgType *, MsgType *);
    
    /** Member function pointer inside Base, to handle IRQ messages. */
    typedef void (Base::*IRQHandlerFunction)(InterruptMessage *);

    public:

        /**
         * Constructor function.
	 * @param num Number of message handlers to support.
         */
        IPCServer(Base *inst, Size num = 32)
        {
	    ipcHandlers = new Vector<MessageHandler<IPCHandlerFunction> >(num);
	    irqHandlers = new Vector<MessageHandler<IRQHandlerFunction> >(num);
	    instance    = inst;
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
	    MsgType msg, reply;
	    InterruptMessage *imsg = (InterruptMessage *) &msg;

    	    /* Enter loop. */
	    while (true)
	    {
		/* Now wait for a message. */
		IPCMessage(ANY, Receive, &msg);

		/* Handle the message. */
		switch (msg.type)
		{			
		    case IPCType:
			if ((*ipcHandlers)[msg.action])
			    (instance->*((*ipcHandlers)[msg.action])->exec) (&msg, &reply);
			else
			    reply.result = ENOSUPPORT;
			break;

		    case FaultType:
			break;

		    case IRQType:
			if ((*irqHandlers)[imsg->vector])
			    (instance->*((*irqHandlers)[imsg->vector])->exec) (imsg);

		    default:
			continue;
		}
		/* Send Reply. */
		IPCMessage(msg.from, Send, &reply);
	    }
    	    /* Satify compiler. */
	    return 0;
	}
    
	/**
	 * Register a new IPC message action handler.
	 * @param slot Action value to trigger h.
	 * @param h Handler to execute.
	 */
	void addIPCHandler(Size slot, IPCHandlerFunction h)
	{
	    ipcHandlers->insert(slot, new MessageHandler<IPCHandlerFunction>(h));
	}
	
	/**
	 * Register a new IRQ message vector handler
	 * @param slot Vector value to trigger h.
	 * @param h Handler to execute.
	 */
	void addIRQHandler(Size slot, IRQHandlerFunction h)
	{
	    irqHandlers->insert(slot, new MessageHandler<IRQHandlerFunction>(h));
	}

    private:
    
	/** IPC handler functions. */
	Vector<MessageHandler<IPCHandlerFunction> > *ipcHandlers;
	
	/** IRQ handler functions. */
	Vector<MessageHandler<IRQHandlerFunction> > *irqHandlers;
	
	/** Server object instance. */
	Base *instance;
};

#endif /* __IPCSERVER_H */

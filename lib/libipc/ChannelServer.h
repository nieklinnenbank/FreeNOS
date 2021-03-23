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

#ifndef __LIBIPC_CHANNELSERVER_H
#define __LIBIPC_CHANNELSERVER_H

#include <FreeNOS/User.h>
#include <FreeNOS/ProcessManager.h>
#include <FreeNOS/ProcessEvent.h>
#include <FreeNOS/ProcessShares.h>
#include <HashIterator.h>
#include <Timer.h>
#include <Vector.h>
#include "MemoryChannel.h"
#include "ChannelClient.h"
#include "ChannelRegistry.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libipc
 * @{
 */

/**
 * Message handler function (dummy) container.
 */
template <class Func> struct MessageHandler
{
    /**
     * Constructor function.
     *
     * @param func Function to execute.
     * @param reply True to send a reply for this message
     */
    MessageHandler(const Func func, const bool reply)
        : exec(func)
        , sendReply(reply)
    {
    }

    const bool operator == (const struct MessageHandler<Func> & h) const
    {
        return false;
    }

    const bool operator != (const struct MessageHandler<Func> & h) const
    {
        return false;
    }

    /** Handler function. */
    const Func exec;

    /** Whether to send a reply or not. */
    const bool sendReply;
};

/**
 * Template class which serves incoming messages from Channels using MessageHandlers.
 *
 * @param MsgType Type of Message to serve.
 */
template <class Base, class MsgType> class ChannelServer
{
  private:

    /** Maximum number of IPC/IRQ handlers. */
    static const Size MaximumHandlerCount = 255u;

  protected:

    /** Member function pointer inside Base, to handle IPC messages. */
    typedef void (Base::*IPCHandlerFunction)(MsgType *);

    /** Member function pointer inside Base, to handle interrupts. */
    typedef void (Base::*IRQHandlerFunction)(Size);

  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        InvalidArgument,
        InvalidSize,
        IOError,
    };

  public:

    /**
     * Constructor function.
     */
    ChannelServer(Base *inst)
        : m_instance(inst)
        , m_client(ChannelClient::instance())
        , m_registry(m_client->getRegistry())
        , m_kernelEvent(Channel::Consumer, sizeof(ProcessEvent))
        , m_ipcHandlers()
        , m_irqHandlers()
    {
        m_self = ProcessCtl(SELF, GetPID, 0);

        // Reset timeout values
        m_expiry.frequency = 0;
        m_expiry.ticks = 0;

        // Setup kernel event channel
        const SystemInformation info;
        ProcessShares::MemoryShare share;
        share.pid    = KERNEL_PID;
        share.coreId = info.coreId;
        share.tagId  = 0;

        if (VMShare(SELF, API::Read, &share) != API::Success)
        {
            FATAL("failed to get kernel event channel");
        }
        else
        {
            m_kernelEvent.setVirtual(share.range.virt,
                                     share.range.virt + PAGESIZE, false);
        }

        // Try to recover channels after a restart
        recoverChannels();
    }

    /**
     * Destructor function.
     */
    virtual ~ChannelServer()
    {
        m_ipcHandlers.deleteAll();
        m_irqHandlers.deleteAll();
    }

    /**
     * Enters an infinite loop, serving incoming requests
     *
     * @return Never.
     */
    int run()
    {
        // Enter loop
        while (true)
        {
            processAll();
            sleepUntilWakeup();
        }

        // Satify compiler
        return 0;
    }

    /**
     * Set a sleep timeout
     *
     * @param msec Milliseconds to sleep (approximately)
     */
    void setTimeout(const uint msec)
    {
        DEBUG("msec = " << msec);

        if (ProcessCtl(SELF, InfoTimer, (Address) &m_time) != API::Success)
        {
            ERROR("failed to retrieve system timer info");
            return;
        }

        const Size msecPerTick = 1000 / m_time.frequency;
        m_expiry.frequency = m_time.frequency;
        m_expiry.ticks     = m_time.ticks + ((msec / msecPerTick) + 1);
    }

  protected:

    /**
     * Register a new IPC message action handler.
     *
     * @param slot Action value to trigger h.
     * @param h Handler to execute.
     * @param sendReply True if the handler needs to send a reply
     */
    void addIPCHandler(const Size slot, IPCHandlerFunction h, const bool sendReply = true)
    {
        m_ipcHandlers.insertAt(slot, new MessageHandler<IPCHandlerFunction>(h, sendReply));
    }

    /**
     * Register a new IRQ message vector handler
     *
     * @param slot Vector value to trigger h.
     * @param h Handler to execute.
     */
    void addIRQHandler(const Size slot, IRQHandlerFunction h)
    {
        m_irqHandlers.insertAt(slot, new MessageHandler<IRQHandlerFunction>(h, false));
    }

    /**
     * Called when sleep timeout is reached
     */
    virtual void timeout()
    {
        DEBUG("");

        retryAllRequests();
    }

    /**
     * Retry any pending requests
     *
     * @return True if retry is needed again, false if all requests processed
     */
    virtual bool retryRequests()
    {
        return false;
    }

    /**
     * Called whenever another Process is terminated
     *
     * @param pid ProcessID of the terminating process
     */
    virtual void onProcessTerminated(const ProcessID pid)
    {
    }

    /**
     * Keep retrying requests until all served
     */
    void retryAllRequests()
    {
        while (m_instance->retryRequests())
            ;
    }

  private:

    /**
     * Process all current events and channels.
     */
    inline void processAll()
    {
        // Process kernel events
        readKernelEvents();

        // Process user messages
        readChannels();

        // Retry requests until all served (EAGAIN or return value)
        retryAllRequests();
    }

    /**
     * Let this process sleep until more events are raised.
     */
    inline void sleepUntilWakeup()
    {
        // Sleep with timeout or return in case the process is
        // woken up by an external (wakeup) interrupt.
        DEBUG("EnterSleep");
        Address expiry = 0;

        if (m_expiry.frequency)
            expiry = (Address) &m_expiry;

        const Error r = ProcessCtl(SELF, EnterSleep, expiry, (Address) (m_expiry.frequency ? &m_time : 0));
        DEBUG("EnterSleep returned: " << (int)r);

        // Check for sleep timeout
        if (m_expiry.frequency)
        {
            if (ProcessCtl(SELF, InfoTimer, (Address) &m_time) != API::Success)
            {
                ERROR("failed to retrieve system timer");
            }
            else if (m_expiry.ticks < m_time.ticks)
            {
                m_expiry.frequency = 0;
                timeout();
            }
        }
    }

    /**
     * Accept new channel connection.
     *
     * @param pid ProcessID
     * @param range Memory range of shared mapping
     * @param hardReset True if the channel contents should be reset to initial state.
     *
     * @return Result code
     */
    Result accept(const ProcessID pid,
                  const Memory::Range range,
                  const bool hardReset = true)
    {
        Address prodAddr, consAddr;

        // ProcessID's determine where the producer/consumer is placed
        if (m_self < pid)
        {
            prodAddr = range.virt;
            consAddr = range.virt + (PAGESIZE * 2);
        }
        else
        {
            prodAddr = range.virt + (PAGESIZE * 2);
            consAddr = range.virt;
        }

        // Create consumer
        if (!m_registry.getConsumer(pid))
        {
            MemoryChannel *consumer = new MemoryChannel(Channel::Consumer, sizeof(MsgType));
            assert(consumer != NULL);
            consumer->setVirtual(consAddr, consAddr + PAGESIZE, hardReset);
            m_registry.registerConsumer(pid, consumer);
        }

        // Create producer
        if (!m_registry.getProducer(pid))
        {
            MemoryChannel *producer = new MemoryChannel(Channel::Producer, sizeof(MsgType));
            assert(producer != NULL);
            producer->setVirtual(prodAddr,
                                 prodAddr + PAGESIZE,
                                 hardReset);
            m_registry.registerProducer(pid, producer);
        }

        // Done
        return Success;
    }

    /**
     * Read existing shares to recover MemoryChannels after restart.
     */
    void recoverChannels()
    {
        const SystemInformation info;

        for (ProcessID i = 0; i < MAX_PROCS; i++)
        {
            if (i != m_self && i != KERNEL_PID)
            {
                ProcessShares::MemoryShare share;
                share.pid    = i;
                share.coreId = info.coreId;
                share.tagId  = 0;

                const API::Result result = VMShare(SELF, API::Read, &share);
                if (result == API::Success)
                {
                    const Result r = accept(i, share.range, false);
                    if (r != Success)
                    {
                        ERROR("failed to recover share for PID " << i << ": " << (int)r);
                    }
                }
            }
        }
    }

    /**
     * Read and process kernel events.
     *
     * @return Result code.
     */
    Result readKernelEvents()
    {
        ProcessEvent event;
        ChannelRegistry::Result result = ChannelRegistry::Success;

        // Try to read a message on the kernel event channel
        while (m_kernelEvent.read(&event) == Channel::Success)
        {
            DEBUG(m_self << ": got kernel event: " << (int) event.type);

            switch (event.type)
            {
                case ShareCreated:
                {
                    DEBUG(m_self << ": share created for PID: " << event.share.pid);
                    accept(event.share.pid, event.share.range);
                    break;
                }
                case InterruptEvent:
                {
                    DEBUG(m_self << ": interrupt: " << event.number);

                    const MessageHandler<IRQHandlerFunction> *h = m_irqHandlers.get(event.number);
                    if (h)
                    {
                        (m_instance->*h->exec) (event.number);
                    }
                    else
                    {
                        ERROR(m_self << ": unhandled IRQ raised: " << event.number);
                    }
                    break;
                }
                case ProcessTerminated:
                {
                    DEBUG(m_self << ": process terminated: PID " << event.number);
                    result = m_registry.unregisterConsumer(event.number);
                    if (result != ChannelRegistry::Success)
                    {
                        ERROR("failed to unregister consumer for PID " <<
                               event.number << ": " << (int)result);
                    }

                    result = m_registry.unregisterProducer(event.number);
                    if (result != ChannelRegistry::Success)
                    {
                        ERROR("failed to unregister producer for PID " <<
                               event.number << ": " << (int)result);
                    }

                    // cleanup the VMShare area now for that process
                    const API::Result shareResult = VMShare(event.number, API::Delete, ZERO);
                    if (shareResult != API::Success)
                    {
                        ERROR("failed to remove shares with VMShare for PID " <<
                               event.number << ": " << (int)shareResult);
                    }

                    onProcessTerminated(event.number);
                    break;
                }
                default:
                    WARNING(m_self << ": unknown event.type: "  << event.type);
                    break;
            }
        }
        return Success;
    }

    /**
     * Read each Channel for messages.
     *
     * @return Result code
     */
    Result readChannels()
    {
        MsgType msg;

        // Try to receive message on each consumer channel
        for (HashIterator<ProcessID, Channel *> i(m_registry.getConsumers()); i.hasCurrent(); i++)
        {
            Channel *ch = i.current();
            DEBUG(m_self << ": trying to receive from PID " << i.key());

            // Read all messages in the consumer channel
            while (ch->read(&msg) == Channel::Success)
            {
                DEBUG(m_self << ": received message");
                msg.from = i.key();

                // Is the message a response from earlier client request?
                if (msg.type == ChannelMessage::Response)
                {
                    if (m_client->processResponse(msg.from, &msg) != ChannelClient::Success)
                    {
                        ERROR(m_self << ": failed to process client response from PID " <<
                               msg.from << " with identifier " << msg.identifier);
                    }
                }
                // Message is a request to us
                else
                {
                    const MessageHandler<IPCHandlerFunction> *h = m_ipcHandlers.get(msg.action);
                    if (h)
                    {
                        (m_instance->*h->exec) (&msg);

                        // Send reply
                        if (h->sendReply)
                        {
                            Channel *ch = m_registry.getProducer(i.key());
                            if (!ch)
                            {
                                ERROR(m_self << ": no producer channel found for PID: " << i.key());
                            }
                            else if (ch->write(&msg) != Channel::Success)
                            {
                                ERROR(m_self << ": failed to send reply message to PID: " << i.key());
                            }
                            else
                                ProcessCtl(i.key(), Wakeup, 0);
                        }
                    }
                    else
                    {
                        ERROR(m_self << ": invalid action " << (int)msg.action << " from PID " << i.key());
                    }
                }
            }
        }
        return Success;
    }

  protected:

    /** Server object instance. */
    Base *m_instance;

    /** Client for sending replies */
    ChannelClient *m_client;

    /** Contains registered channels */
    ChannelRegistry &m_registry;

    /** Kernel event channel */
    MemoryChannel m_kernelEvent;

    /** IPC handler functions. */
    Index<MessageHandler<IPCHandlerFunction>, MaximumHandlerCount> m_ipcHandlers;

    /** IRQ handler functions. */
    Index<MessageHandler<IRQHandlerFunction>, MaximumHandlerCount> m_irqHandlers;

    /** ProcessID of ourselves */
    ProcessID m_self;

    /** System timer value */
    Timer::Info m_time;

    /** System timer expiration value */
    Timer::Info m_expiry;
};

/**
 * @}
 * @}
 */

#endif /* __LIBIPC_CHANNELSERVER_H */

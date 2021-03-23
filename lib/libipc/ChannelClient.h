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

#ifndef __LIBIPC_CHANNELCLIENT_H
#define __LIBIPC_CHANNELCLIENT_H

#include <Singleton.h>
#include <Callback.h>
#include <Index.h>
#include "ChannelRegistry.h"
#include "Channel.h"
#include "ChannelMessage.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libipc
 * @{
 */

/**
 * Client for using Channels on the local processor.
 *
 * @todo Intra-core communication is not yet supported by this client.
 *       For intra-core channels, the local CoreServer should be contacted
 *       which communicates with the remote CoreServer to properly setup
 *       the channel between cores.
 */
class ChannelClient : public StrictSingleton<ChannelClient>
{
  private:

    /** Maximum number of concurrent outgoing requests. */
    static const Size MaximumRequests = 32u;

    /** Maximum number of retries for establishing new connection. */
    static const Size MaxConnectRetries = 16u;

    /**
     * Holds an outgoing request
     */
    typedef struct Request
    {
        bool active;
        ProcessID pid;
        ChannelMessage *message;
        CallbackFunction *callback;

        const bool operator == (const struct Request & req) const
        {
            return req.message == message && req.callback == callback;
        }

        const bool operator != (const struct Request & req) const
        {
            return req.message != message || req.callback != callback;
        }
    }
    Request;

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
        OutOfMemory,
        NotFound
    };

  public:

    /**
     * Constructor.
     */
    ChannelClient();

    /**
     * Destructor
     */
    virtual ~ChannelClient();

    /**
     * Get channel registry.
     *
     * @return ChannelRegistry object reference
     */
    ChannelRegistry & getRegistry();

    /**
     * Initialize the ChannelClient.
     *
     * @return Result code
     */
    virtual Result initialize();

    /**
     * Connect to a process.
     *
     * This function creates a producer and consumer Channel
     * to the given process and registers it with the ChannelRegistry.
     *
     * @param pid ProcessID for the process to connect to.
     * @param msgSize Message size to use.
     *
     * @return Result code
     */
    virtual Result connect(const ProcessID pid, const Size msgSize);

    /**
     * Try to receive message from any channel.
     *
     * @param buffer Message buffer for output
     * @param msgSize Message size to use.
     * @param pid ProcessID for output
     *
     * @return Result code
     */
    virtual Result receiveAny(void *buffer, const Size msgSize, ProcessID *pid);

    /**
     * Send asynchronous request message
     *
     * The client assigns an internal request identifier
     * for the message and ensures that the callback will be
     * called when a response messages is received.
     *
     * @param pid ProcessID to send the message to
     * @param buffer Points to message to send
     * @param msgSize Message size to use.
     * @param callback Called when response message is received
     *
     * @return Result code
     */
    virtual Result sendRequest(const ProcessID pid,
                               void *buffer,
                               const Size msgSize,
                               CallbackFunction *callback);

    /**
     * Process a response message
     *
     * @param pid ProcessID from which we receive the message
     * @param msg Message which is received
     *
     * @return Result code
     */
    virtual Result processResponse(const ProcessID pid,
                                   ChannelMessage *msg);

    /**
     * Synchronous receive from one process.
     *
     * @param buffer Message buffer for output
     * @param msgSize Message size to use.
     * @param pid ProcessID for the channel
     *
     * @return Result code
     */
    virtual Result syncReceiveFrom(void *buffer, const Size msgSize, const ProcessID pid);

    /**
     * Synchronous send to one process.
     *
     * @param buffer Message buffer to send
     * @param msgSize Message size to use.
     * @param pid ProcessID for the channel
     *
     * @return Result code
     */
    virtual Result syncSendTo(const void *buffer, const Size msgSize, const ProcessID pid);

    /**
     * Synchronous send and receive to/from one process.
     *
     * @param buffer Message buffer to send/receive
     * @param msgSize Message size to use.
     * @param pid ProcessID for the channel
     *
     * @return Result code
     */
    virtual Result syncSendReceive(void *buffer, const Size msgSize, const ProcessID pid);

  private:

    /**
     * Get consumer for a process.
     *
     * @param pid ProcessID of the process
     * @param msgSize Message size to use.
     *
     * @return Channel object if found or ZERO otherwise.
     */
    Channel * findConsumer(const ProcessID pid, const Size msgSize);

    /**
     * Get producer for a process.
     *
     * @param pid ProcessID of the process
     * @param msgSize Message size to use.
     *
     * @return Channel object if found or ZERO otherwise.
     */
    Channel * findProducer(const ProcessID pid, const Size msgSize);

  private:

    /** Contains registered channels */
    ChannelRegistry m_registry;

    /** Contains ongoing requests */
    Index<Request, MaximumRequests> m_requests;

    /** Current Process ID */
    const ProcessID m_pid;
};

/**
 * @}
 * @}
 */

#endif /* __LIBIPC_CHANNELCLIENT_H */

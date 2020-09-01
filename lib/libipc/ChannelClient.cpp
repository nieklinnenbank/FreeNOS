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

#include <FreeNOS/User.h>
#include <HashIterator.h>
#include "ChannelClient.h"
#include "MemoryChannel.h"

ChannelClient::ChannelClient()
    : StrictSingleton<ChannelClient>()
{
}

ChannelClient::~ChannelClient()
{
}

ChannelRegistry & ChannelClient::getRegistry()
{
    return m_registry;
}

ChannelClient::Result ChannelClient::initialize()
{
    return Success;
}

ChannelClient::Result ChannelClient::connect(const ProcessID pid, const Size messageSize)
{
    Address prodAddr, consAddr;
    const SystemInformation info;

    // Allocate consumer
    MemoryChannel *cons = new MemoryChannel(Channel::Consumer, messageSize);
    if (!cons)
    {
        return OutOfMemory;
    }

    // Allocate producer
    MemoryChannel *prod = new MemoryChannel(Channel::Producer, messageSize);
    if (!prod)
    {
        delete cons;
        return OutOfMemory;
    }

    // Call VMShare to create shared memory mapping for MemoryChannel.
    ProcessShares::MemoryShare share;
    share.pid    = pid;
    share.coreId = info.coreId;
    share.tagId  = 0;
    share.range.size = PAGESIZE * 4;
    share.range.virt = 0;
    share.range.phys = 0;
    share.range.access = Memory::User | Memory::Readable | Memory::Writable;

    // Create shared memory mapping
    const Error r = VMShare(pid, API::Create, &share);
    switch (r)
    {
        case API::Success:
        {
            prodAddr = share.range.virt;
            consAddr = share.range.virt + (PAGESIZE * 2);
            break;
        }
        case API::AlreadyExists:
        {
            prodAddr = share.range.virt + (PAGESIZE * 2);
            consAddr = share.range.virt;
            break;
        }
        default:
        {
            return IOError;
        }
    }

    // Setup producer memory address
    if (prod->setVirtual(prodAddr, prodAddr + PAGESIZE) != MemoryChannel::Success)
    {
        delete prod;
        delete cons;
        return IOError;
    }

    // Setup consumer memory address
    if (cons->setVirtual(consAddr, consAddr + PAGESIZE) != MemoryChannel::Success)
    {
        delete prod;
        delete cons;
        return IOError;
    }

    // Register channels
    m_registry.registerConsumer(pid, cons);
    m_registry.registerProducer(pid, prod);
    return Success;
}

ChannelClient::Result ChannelClient::receiveAny(void *buffer, const Size msgSize, ProcessID *pid)
{
    assert(msgSize > 0);

    for (HashIterator<ProcessID, Channel *> i(m_registry.getConsumers()); i.hasCurrent(); i++)
    {
        if (i.current()->read(buffer) == Channel::Success)
        {
            *pid = i.key();
            return Success;
        }
    }
    return NotFound;
}

ChannelClient::Result ChannelClient::sendRequest(const ProcessID pid,
                                                 void *buffer,
                                                 const Size msgSize,
                                                 CallbackFunction *callback)
{
    Request *req = 0;
    Size identifier = 0;
    Channel *ch = findProducer(pid, msgSize);
    if (!ch)
        return NotFound;

    // Find request object
    for (Size i = 0; i < m_requests.count(); i++)
    {
        req = m_requests.get(i);
        if (!req->active)
        {
            identifier = i;
            break;
        }
    }

    // Allocate new request object if none available
    if (!req || req->active)
    {
        req = new Request;
        assert(req != NULL);
        req->message = (ChannelMessage *) new u8[ch->getMessageSize()];
        assert(req->message != NULL);

        if (!m_requests.insert(identifier, req))
        {
            return OutOfMemory;
        }
    }

    // Fill request object
    MemoryBlock::copy(req->message, buffer, ch->getMessageSize());
    req->pid = pid;
    req->message->identifier = identifier;
    req->message->type = ChannelMessage::Request;
    req->callback = callback;
    req->active = true;

    DEBUG("sending request with id = " << req->message->identifier << " to PID " << pid);

    // Try to send the message
    if (ch->write(req->message) != Channel::Success)
    {
        req->active = false;
        return IOError;
    }

    // Wakeup the receiver
    ProcessCtl(pid, Resume, 0);
    return Success;
}

ChannelClient::Result ChannelClient::processResponse(const ProcessID pid,
                                                     ChannelMessage *msg)
{
    const Size count = m_requests.count();

    for (Size i = 0; i < count; i++)
    {
        Request *req = m_requests.get(i);

        if (req->active &&
            req->pid == pid &&
            req->message->identifier == msg->identifier)
        {
            req->callback->execute(msg);
            req->active = false;
            return Success;
        }
    }
    return NotFound;
}


Channel * ChannelClient::findConsumer(const ProcessID pid, const Size msgSize)
{
    Channel *ch = m_registry.getConsumer(pid);
    if (ch)
        return ch;

    // Try to connect
    if (connect(pid, msgSize) != Success)
        return ZERO;

    return m_registry.getConsumer(pid);
}

Channel * ChannelClient::findProducer(const ProcessID pid, const Size msgSize)
{
    Channel *ch = m_registry.getProducer(pid);
    if (ch)
        return ch;

    // Try to connect
    if (connect(pid, msgSize) != Success)
        return ZERO;

    return m_registry.getProducer(pid);
}

ChannelClient::Result ChannelClient::syncReceiveFrom(void *buffer, const Size msgSize, const ProcessID pid)
{
    Channel *ch = findConsumer(pid, msgSize);
    if (!ch)
        return NotFound;

    while (ch->read(buffer) != Channel::Success)
        ProcessCtl(SELF, EnterSleep, 0);

    return Success;
}

ChannelClient::Result ChannelClient::syncSendTo(const void *buffer, const Size msgSize, const ProcessID pid)
{
    Channel *ch = findProducer(pid, msgSize);
    if (!ch)
        return NotFound;

    while (true)
    {
        switch (ch->write(buffer))
        {
            case Channel::Success:
                ProcessCtl(pid, Resume, 0);
                return Success;

            case Channel::ChannelFull:
                ProcessCtl(pid, Resume, 0);
                break;

            default:
                return IOError;
        }
        ProcessCtl(SELF, Schedule, 0);
    }
    return IOError;
}

ChannelClient::Result ChannelClient::syncSendReceive(void *buffer, const Size msgSize, const ProcessID pid)
{
    const Result result = syncSendTo(buffer, msgSize, pid);
    if (result != Success)
        return result;

    return syncReceiveFrom(buffer, msgSize, pid);
}

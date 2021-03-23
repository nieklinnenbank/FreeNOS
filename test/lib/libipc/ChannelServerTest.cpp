/*
 * Copyright (C) 2020 Niek Linnenbank
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

#include <TestCase.h>
#include <FreeNOS/System.h>
#include <FreeNOS/ProcessManager.h>
#include <TestRunner.h>
#include <TestInt.h>
#include <TestMain.h>
#include <Log.h>
#include <ChannelMessage.h>
#include <ChannelServer.h>

class DummyMessage : public ChannelMessage
{
  public:
    u32 action;
    u32 value;
    u32 result;
};

class DummyServer : public ChannelServer<DummyServer, DummyMessage>
{
  public:
    static const Size DummyIrqVector = 67U;
    static const u32 DummyIpcAction  = 12U;
    static const u32 DummyIpcResult  = 112233U;

  public:
    DummyServer()
        : ChannelServer<DummyServer, DummyMessage>(this)
        , m_kernelProducer(Channel::Producer, sizeof(ProcessEvent))
        , m_irqValue(0)
        , m_irqCount(0)
        , m_msgValue(0)
        , m_msgCount(0)
    {
        addIRQHandler(DummyIrqVector, &DummyServer::irqHandler);
        addIPCHandler(DummyIpcAction, &DummyServer::ipcHandler);

        MemoryBlock::set(m_kernelPages, 0, sizeof(m_kernelPages));
        m_kernelProducer.setVirtual((Address) m_kernelPages, ((Address) m_kernelPages) + PAGESIZE);
        m_kernelEvent.setVirtual((Address) m_kernelPages, ((Address) m_kernelPages) + PAGESIZE);
    }

    void irqHandler(Size irq)
    {
        m_irqValue = irq;
        m_irqCount++;
    }

    void ipcHandler(DummyMessage *msg)
    {
        m_msgValue = msg->value;
        m_msgCount++;
        msg->result = DummyIpcResult;
    }

    static u8 m_kernelPages[PAGESIZE * 2u];
    MemoryChannel m_kernelProducer;
    Size m_irqValue;
    Size m_irqCount;
    u32  m_msgValue;
    Size m_msgCount;
};

u8 DummyServer::m_kernelPages[PAGESIZE * 2u];

TestCase(ChannelServerConstruct)
{
    DummyServer server;

    // Validate members
    testAssert(server.m_self != 0);
    testAssert(server.m_instance == &server);
    testAssert(&server.m_registry == &ChannelClient::instance()->getRegistry());

    return OK;
}

TestCase(ChannelServerRecoverChannels)
{
    SystemInformation info;
    DummyServer server;

    // Prepare channel. This test simulates the client-side of both channels.
    static u8 pages[PAGESIZE * 4];
    MemoryChannel prod(Channel::Producer, sizeof(DummyMessage));
    MemoryChannel cons(Channel::Consumer, sizeof(DummyMessage));

    // Determine producer/consumer pages by the PIDs
    // Note that this test simulates a client of the server.
    Address prodAddr, consAddr;
    if (SELF <= server.m_self)
    {
        prodAddr = (Address) pages;
        consAddr = ((Address) pages) + (PAGESIZE * 2);
    }
    else
    {
        prodAddr = ((Address) pages) + (PAGESIZE * 2);
        consAddr = (Address) pages;
    }

    // Apply channel pages
    prod.setVirtual(prodAddr, prodAddr + PAGESIZE);
    cons.setVirtual(consAddr, consAddr + PAGESIZE);

    // Setup a pending message in the request channel
    DummyMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.from   = SELF;
    msg.action = DummyServer::DummyIpcAction;
    msg.value  = 12345U;
    msg.result = 0;
    testAssert(prod.write(&msg) == MemoryChannel::Success);

    // Invoke accept with soft-reset, just like recoverChannels would do
    ProcessShares::MemoryShare share;
    share.pid    = SELF;
    share.coreId = info.coreId;
    share.tagId  = 0;
    share.range.virt = (Address) pages;
    share.range.size = sizeof(pages);
    testAssert(server.accept(SELF, share.range, false) == DummyServer::Success);

    // See if the pending message is processed correctly (i.e. recovered after the soft-reset)
    server.readChannels();
    testAssert(server.m_msgCount == 1);
    testAssert(server.m_irqCount == 0);

    // See if the response can be received properly
    testAssert(cons.read(&msg) == MemoryChannel::Success);
    testAssert(msg.result == DummyServer::DummyIpcResult);
    testAssert(server.m_msgValue == 12345U);

    return OK;
}

TestCase(ChannelServerReadChannels)
{
    DummyServer server;
    const ProcessID pid = MAX_PROCS + 1234u;

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // No channels created, thus no handlers called
    server.readChannels();
    testAssert(server.m_msgCount == 0);
    testAssert(server.m_irqCount == 0);

    // Create client channels
    static u8 pages[PAGESIZE * 4];
    MemoryChannel clientProducer(Channel::Producer, sizeof(DummyMessage));
    MemoryChannel clientConsumer(Channel::Consumer, sizeof(DummyMessage));

    // Determine producer/consumer pages by the PIDs
    // Note that this test simulates a client of the server.
    Address prodAddr, consAddr;
    if (pid <= server.m_self)
    {
        prodAddr = (Address) pages;
        consAddr = ((Address) pages) + (PAGESIZE * 2);
    }
    else
    {
        prodAddr = ((Address) pages) + (PAGESIZE * 2);
        consAddr = (Address) pages;
    }

    // Assign memory pages
    MemoryBlock::set(pages, 0, sizeof(pages));
    clientProducer.setVirtual(prodAddr, prodAddr + PAGESIZE);
    clientConsumer.setVirtual(consAddr, consAddr + PAGESIZE);

    // Raise event with a newly created share
    ProcessEvent event;
    event.type = ShareCreated;
    event.share.pid = pid;
    event.share.range.virt = (Address) &pages;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);
    testAssert(server.m_msgCount == 0);
    testAssert(server.m_irqCount == 0);

    // Process all events. This should accept the incoming channels, but no new messages.
    server.processAll();
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) != ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) != ZERO);
    testAssert(server.m_msgCount == 0);
    testAssert(server.m_irqCount == 0);

    // Send a message with an invalid action
    DummyMessage msg;
    msg.type   = ChannelMessage::Request;
    msg.from   = pid;
    msg.action = 123U;
    msg.value  = 12345U;
    msg.result = 0;
    testAssert(clientProducer.write(&msg) == MemoryChannel::Success);

    // Process the message. No handlers must be called (invalid action)
    server.processAll();
    testAssert(server.m_msgCount == 0);
    testAssert(server.m_irqCount == 0);

    // Send valid message
    msg.action = DummyServer::DummyIpcAction;
    testAssert(clientProducer.write(&msg) == MemoryChannel::Success);

    // Process the message. IPC handler must be called once
    server.processAll();
    testAssert(server.m_msgCount == 1);
    testAssert(server.m_msgValue == msg.value);
    testAssert(server.m_irqCount == 0);

    // Verify that a response message was send with proper result
    testAssert(clientConsumer.read(&msg) == MemoryChannel::Success);
    testAssert(msg.action == DummyServer::DummyIpcAction);
    testAssert(msg.result == DummyServer::DummyIpcResult);

    // Only one response message must be send
    testAssert(clientConsumer.read(&msg) == MemoryChannel::NotFound);

    // Raise event of process being terminated
    event.type   = ProcessTerminated;
    event.number = pid;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the event
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) == ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) == ZERO);

    return OK;
}

TestCase(ChannelServerShareCreated)
{
    DummyServer server;
    const ProcessID pid = MAX_PROCS + 1234u;
    const Address addr  = 0x12340000;

    // Determine producer/consumer pages by the PIDs
    Address prodAddr, consAddr;
    if (server.m_self < pid)
    {
        prodAddr = addr;
        consAddr = addr + (PAGESIZE * 2);
    }
    else
    {
        prodAddr = addr + (PAGESIZE * 2);
        consAddr = addr;
    }

    // Raise event with a newly created share
    ProcessEvent event;
    event.type = ShareCreated;
    event.share.pid = pid;
    event.share.range.virt = addr;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the new channel
    server.readKernelEvents();

    // Verify consumer channel creation
    MemoryChannel *cons = (MemoryChannel *) ChannelClient::instance()->getRegistry().getConsumer(pid);
    testAssert(cons != ZERO);
    testAssert(cons->m_data.m_base == consAddr);
    testAssert(cons->m_feedback.m_base == consAddr + PAGESIZE);

    // Verify producer channel creation
    MemoryChannel *prod = (MemoryChannel *) ChannelClient::instance()->getRegistry().getProducer(pid);
    testAssert(prod != ZERO);
    testAssert(prod->m_data.m_base == prodAddr);
    testAssert(prod->m_feedback.m_base == prodAddr + PAGESIZE);

    // Raise event of process being terminated
    event.type   = ProcessTerminated;
    event.number = pid;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the event
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) == ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) == ZERO);

    return OK;
}

TestCase(ChannelServerInterruptEvent)
{
    DummyServer server;

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Without any events, no IRQs should be raised
    server.processAll();
    testAssert(server.m_irqCount == 0);

    // Raise an IRQ which is not registered
    ProcessEvent event;
    event.type = InterruptEvent;
    event.number = 251;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the unregistered IRQ
    server.processAll();
    testAssert(server.m_irqCount == 0);

    // Now raise the registered IRQ once.
    event.number = DummyServer::DummyIrqVector;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the registered IRQ. This must invoke the IRQ callback once.
    server.processAll();
    testAssert(server.m_irqCount == 1);
    testAssert(server.m_irqValue == DummyServer::DummyIrqVector);

    // Raise the registered IRQ multiple times. This will enqueue them.
    for (Size i = 0; i < 10U; i++)
    {
        testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
        testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);
    }

    // For each queued IRQ event, the handler must be invoked.
    server.processAll();
    testAssert(server.m_irqCount == 11);
    testAssert(server.m_irqValue == DummyServer::DummyIrqVector);

    return OK;
}

TestCase(ChannelServerProcessTerminated)
{
    DummyServer server;

    // Raise event with a newly created share
    const ProcessID pid = MAX_PROCS + 1234u;
    const Address addr  = 0x12340000;
    ProcessEvent event;
    event.type = ShareCreated;
    event.share.pid = pid;
    event.share.range.virt = addr;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the new channel
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) != ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) != ZERO);

    // Raise event of process being terminated
    event.type   = ProcessTerminated;
    event.number = pid;
    testAssert(server.m_kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(server.m_kernelProducer.flush() == MemoryChannel::Success);

    // Process the event
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) == ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) == ZERO);

    return OK;
}

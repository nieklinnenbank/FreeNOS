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
    DummyServer(const Address kernelChannelAddr)
        : ChannelServer<DummyServer, DummyMessage>(this)
        , m_irqValue(0)
        , m_irqCount(0)
        , m_msgValue(0)
        , m_msgCount(0)
    {
        addIRQHandler(DummyIrqVector, &DummyServer::irqHandler);
        addIPCHandler(DummyIpcAction, &DummyServer::ipcHandler);
        m_kernelEvent.setVirtual(kernelChannelAddr, kernelChannelAddr + PAGESIZE);
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

    Size m_irqValue;
    Size m_irqCount;
    u32  m_msgValue;
    Size m_msgCount;
};

static Address prepareKernelChannel(MemoryChannel *producer)
{
    static u8 kernelPages[PAGESIZE * 2];

    MemoryBlock::set(kernelPages, 0, sizeof(kernelPages));
    producer->setVirtual((Address) kernelPages, ((Address) kernelPages) + PAGESIZE);

    return (Address) &kernelPages;
}

TestCase(ChannelServerConstruct)
{
    MemoryChannel kernelProducer(Channel::Producer, sizeof(ProcessEvent));
    DummyServer server(prepareKernelChannel(&kernelProducer));

    // Validate members
    testAssert(server.m_self != 0);
    testAssert(server.m_instance == &server);
    testAssert(&server.m_registry == &ChannelClient::instance()->getRegistry());

    return OK;
}

TestCase(ChannelServerReadChannels)
{
    MemoryChannel kernelProducer(Channel::Producer, sizeof(ProcessEvent));
    DummyServer server(prepareKernelChannel(&kernelProducer));

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // No channels created, thus no handlers called
    server.readChannels();
    testAssert(server.m_msgCount == 0);
    testAssert(server.m_irqCount == 0);

    // Create client channels
    static u8 clientPages[PAGESIZE * 4];
    MemoryChannel clientProducer(Channel::Producer, sizeof(DummyMessage));
    MemoryChannel clientConsumer(Channel::Consumer, sizeof(DummyMessage));

    // Assign memory pages
    MemoryBlock::set(clientPages, 0, sizeof(clientPages));
    clientProducer.setVirtual((Address) clientPages, (Address) clientPages + PAGESIZE);
    clientConsumer.setVirtual((Address) clientPages + (PAGESIZE * 2),
                              (Address) clientPages + (PAGESIZE * 3));

    // Raise event with a newly created share
    const ProcessID pid = MAX_PROCS + 1234u;
    ProcessEvent event;
    event.type = ShareCreated;
    event.share.pid = pid;
    event.share.range.virt = (Address) &clientPages;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);
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
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the event
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) == ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) == ZERO);

    return OK;
}

TestCase(ChannelServerShareCreated)
{
    MemoryChannel kernelProducer(Channel::Producer, sizeof(ProcessEvent));
    DummyServer server(prepareKernelChannel(&kernelProducer));

    // Raise event with a newly created share
    const ProcessID pid = MAX_PROCS + 1234u;
    const Address addr  = 0x12340000;
    ProcessEvent event;
    event.type = ShareCreated;
    event.share.pid = pid;
    event.share.range.virt = addr;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the new channel
    server.readKernelEvents();

    // Verify consumer channel creation
    MemoryChannel *cons = (MemoryChannel *) ChannelClient::instance()->getRegistry().getConsumer(pid);
    testAssert(cons != ZERO);
    testAssert(cons->m_data.m_base == addr);
    testAssert(cons->m_feedback.m_base == addr + PAGESIZE);

    // Verify producer channel creation
    MemoryChannel *prod = (MemoryChannel *) ChannelClient::instance()->getRegistry().getProducer(pid);
    testAssert(prod != ZERO);
    testAssert(prod->m_data.m_base == addr + (PAGESIZE * 2));
    testAssert(prod->m_feedback.m_base == addr + (PAGESIZE * 3));

    // Raise event of process being terminated
    event.type   = ProcessTerminated;
    event.number = pid;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the event
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) == ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) == ZERO);

    return OK;
}

TestCase(ChannelServerInterruptEvent)
{
    MemoryChannel kernelProducer(Channel::Producer, sizeof(ProcessEvent));
    DummyServer server(prepareKernelChannel(&kernelProducer));

    // Mask error output
    Log::instance()->setMinimumLogLevel(Log::Critical);

    // Without any events, no IRQs should be raised
    server.processAll();
    testAssert(server.m_irqCount == 0);

    // Raise an IRQ which is not registered
    ProcessEvent event;
    event.type = InterruptEvent;
    event.number = 251;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the unregistered IRQ
    server.processAll();
    testAssert(server.m_irqCount == 0);

    // Now raise the registered IRQ once.
    event.number = DummyServer::DummyIrqVector;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the registered IRQ. This must invoke the IRQ callback once.
    server.processAll();
    testAssert(server.m_irqCount == 1);
    testAssert(server.m_irqValue == DummyServer::DummyIrqVector);

    // Raise the registered IRQ multiple times. This will enqueue them.
    for (Size i = 0; i < 10U; i++)
    {
        testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
        testAssert(kernelProducer.flush() == MemoryChannel::Success);
    }

    // For each queued IRQ event, the handler must be invoked.
    server.processAll();
    testAssert(server.m_irqCount == 11);
    testAssert(server.m_irqValue == DummyServer::DummyIrqVector);

    return OK;
}

TestCase(ChannelServerProcessTerminated)
{
    MemoryChannel kernelProducer(Channel::Producer, sizeof(ProcessEvent));
    DummyServer server(prepareKernelChannel(&kernelProducer));

    // Raise event with a newly created share
    const ProcessID pid = MAX_PROCS + 1234u;
    const Address addr  = 0x12340000;
    ProcessEvent event;
    event.type = ShareCreated;
    event.share.pid = pid;
    event.share.range.virt = addr;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the new channel
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) != ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) != ZERO);

    // Raise event of process being terminated
    event.type   = ProcessTerminated;
    event.number = pid;
    testAssert(kernelProducer.write(&event) == MemoryChannel::Success);
    testAssert(kernelProducer.flush() == MemoryChannel::Success);

    // Process the event
    server.readKernelEvents();
    testAssert(ChannelClient::instance()->getRegistry().getProducer(pid) == ZERO);
    testAssert(ChannelClient::instance()->getRegistry().getConsumer(pid) == ZERO);

    return OK;
}

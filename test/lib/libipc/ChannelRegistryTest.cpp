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

#include <TestRunner.h>
#include <TestInt.h>
#include <TestCase.h>
#include <TestMain.h>
#include <Types.h>
#include <HashIterator.h>
#include <Channel.h>
#include <ChannelRegistry.h>

TestCase(ChannelRegistryConstruct)
{
    ChannelRegistry reg;

    testAssert(reg.getConsumers().count() == 0);
    testAssert(reg.getProducers().count() == 0);

    return OK;
}

TestCase(ChannelRegistryConsumer)
{
    ChannelRegistry reg;
    Channel *cons1 = new Channel(Channel::Consumer, sizeof(u32));
    Channel *cons2 = new Channel(Channel::Consumer, sizeof(u32));

    // Initially we should be empty
    testAssert(reg.getConsumers().count() == 0);
    testAssert(reg.getConsumer(1) == NULL);

    // Add consumers
    testAssert(reg.registerConsumer(1, cons1) == ChannelRegistry::Success);
    testAssert(reg.getConsumer(1) == cons1);
    testAssert(reg.registerConsumer(2, cons2) == ChannelRegistry::Success);
    testAssert(reg.getConsumer(2) == cons2);
    testAssert(reg.getConsumer(3) == NULL);

    // We should not have touched any producers
    testAssert(reg.getProducers().count() == 0);

    // Iterate over the consumers
    HashTable<ProcessID, Channel *> & getConsumers();
    for (HashIterator<ProcessID, Channel *> it(reg.getConsumers()); it.hasCurrent(); it++)
    {
        testAssert(it.key() == 1 || it.key() == 2);
        testAssert(it.current() == cons1 || it.current() == cons2);
    }

    // Try to remove non-existing consumer
    testAssert(reg.unregisterConsumer(3) == ChannelRegistry::NotFound);
    testAssert(reg.getConsumer(1) == cons1);
    testAssert(reg.getConsumer(2) == cons2);

    // Only remove first consumer
    testAssert(reg.unregisterConsumer(1) == ChannelRegistry::Success);
    testAssert(reg.getConsumer(1) == NULL);
    testAssert(reg.getConsumer(2) == cons2);

    // Remove the second consumer
    testAssert(reg.unregisterConsumer(2) == ChannelRegistry::Success);
    testAssert(reg.getConsumer(1) == NULL);
    testAssert(reg.getConsumer(2) == NULL);

    // Now we should be empty
    testAssert(reg.getConsumers().count() == 0);
    testAssert(reg.getProducers().count() == 0);

    return OK;
}

TestCase(ChannelRegistryProducer)
{
    ChannelRegistry reg;
    Channel *prod1 = new Channel(Channel::Producer, sizeof(u32));
    Channel *prod2 = new Channel(Channel::Producer, sizeof(u32));

    // Initially we should be empty
    testAssert(reg.getProducers().count() == 0);
    testAssert(reg.getProducer(1) == NULL);

    // Add producers
    testAssert(reg.registerProducer(1, prod1) == ChannelRegistry::Success);
    testAssert(reg.getProducer(1) == prod1);
    testAssert(reg.registerProducer(2, prod2) == ChannelRegistry::Success);
    testAssert(reg.getProducer(2) == prod2);
    testAssert(reg.getProducer(3) == NULL);

    // We should not have touched any consumers
    testAssert(reg.getConsumers().count() == 0);

    // Iterate over the producers
    HashTable<ProcessID, Channel *> & getProducers();
    for (HashIterator<ProcessID, Channel *> it(reg.getProducers()); it.hasCurrent(); it++)
    {
        testAssert(it.key() == 1 || it.key() == 2);
        testAssert(it.current() == prod1 || it.current() == prod2);
    }

    // Try to remove non-existing producer
    testAssert(reg.unregisterProducer(3) == ChannelRegistry::NotFound);
    testAssert(reg.getProducer(1) == prod1);
    testAssert(reg.getProducer(2) == prod2);

    // Only remove first producer
    testAssert(reg.unregisterProducer(1) == ChannelRegistry::Success);
    testAssert(reg.getProducer(1) == NULL);
    testAssert(reg.getProducer(2) == prod2);

    // Remove the second producer
    testAssert(reg.unregisterProducer(2) == ChannelRegistry::Success);
    testAssert(reg.getProducer(1) == NULL);
    testAssert(reg.getProducer(2) == NULL);

    // Now we should be empty
    testAssert(reg.getProducers().count() == 0);
    testAssert(reg.getProducers().count() == 0);

    return OK;
}

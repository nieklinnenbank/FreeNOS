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

#include <HashIterator.h>
#include "Channel.h"
#include "ChannelRegistry.h"

ChannelRegistry::ChannelRegistry()
{
}

ChannelRegistry::~ChannelRegistry()
{
    for (HashIterator<ProcessID, Channel *> i(m_consumer); i.hasCurrent(); i++)
        delete i.current();

    for (HashIterator<ProcessID, Channel *> i(m_producer); i.hasCurrent(); i++)
        delete i.current();
}

Channel * ChannelRegistry::getConsumer(const ProcessID pid)
{
    Channel * const *ch = m_consumer.get(pid);
    if (ch)
        return *ch;
    else
        return ZERO;
}

Channel * ChannelRegistry::getProducer(const ProcessID pid)
{
    Channel * const *ch = m_producer.get(pid);
    if (ch)
        return *ch;
    else
        return ZERO;
}

HashTable<ProcessID, Channel *> & ChannelRegistry::getConsumers()
{
    return m_consumer;
}

HashTable<ProcessID, Channel *> & ChannelRegistry::getProducers()
{
    return m_producer;
}

ChannelRegistry::Result ChannelRegistry::registerConsumer(
    const ProcessID pid,
    Channel *channel)
{
    m_consumer.insert(pid, channel);
    return Success;
}

ChannelRegistry::Result ChannelRegistry::registerProducer(
    const ProcessID pid,
    Channel *channel)
{
    m_producer.insert(pid, channel);
    return Success;
}

ChannelRegistry::Result ChannelRegistry::unregisterConsumer(const ProcessID pid)
{
    Channel *ch = getConsumer(pid);
    if (ch)
        delete ch;

    if (m_consumer.remove(pid) > 0)
        return Success;
    else
        return NotFound;
}

ChannelRegistry::Result ChannelRegistry::unregisterProducer(const ProcessID pid)
{
    Channel *ch = getProducer(pid);
    if (ch)
        delete ch;

    if (m_producer.remove(pid) > 0)
        return Success;
    else
        return NotFound;
}

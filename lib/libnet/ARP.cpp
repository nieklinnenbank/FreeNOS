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

#include <ByteOrder.h>
#include "NetworkClient.h"
#include "NetworkServer.h"
#include "NetworkDevice.h"
#include "ARP.h"
#include "ARPSocket.h"

ARP::ARP(NetworkServer &server,
         NetworkDevice &device,
         NetworkProtocol &parent)
    : NetworkProtocol(server, device, parent)
{
    m_ip = 0;
}

ARP::~ARP()
{
    for (HashIterator<IPV4::Address, ARPCache *> i(m_cache); i.hasCurrent(); i++)
    {
        delete i.current();
    }
}

FileSystem::Result ARP::initialize()
{
    m_sock = new ARPSocket(m_server.getNextInode(), this);
    m_server.registerDirectory(this, "/arp");
    m_server.registerFile(m_sock, "/arp/socket");

    return FileSystem::Success;
}

void ARP::setIP(::IPV4 *ip)
{
    m_ip = ip;
}

ARP::ARPCache * ARP::getCacheEntry(const IPV4::Address ipAddr)
{
    ARPCache **entry = (ARPCache **) m_cache.get(ipAddr);
    if (entry)
        return (*entry);
    else
        return insertCacheEntry(ipAddr);
}

ARP::ARPCache * ARP::insertCacheEntry(const IPV4::Address ipAddr)
{
    ARPCache *entry = new ARPCache;
    MemoryBlock::set(entry, 0, sizeof(*entry));
    entry->valid  = false;

    // Insert to the ARP cache
    m_cache.insert(ipAddr, entry);
    return entry;
}

void ARP::updateCacheEntry(const IPV4::Address ipAddr,
                           const Ethernet::Address *ethAddr)
{
    ARPCache *entry = getCacheEntry(ipAddr);
    if (entry)
    {
        entry->valid = true;
        MemoryBlock::copy(&entry->ethAddr, ethAddr, sizeof(Ethernet::Address));
    }
}

FileSystem::Result ARP::lookupAddress(const IPV4::Address *ipAddr,
                                      Ethernet::Address *ethAddr)
{
    DEBUG("");

    // See if we have the IP cached
    const ARPCache *entry = getCacheEntry(*ipAddr);
    if (entry && entry->valid)
    {
        MemoryBlock::copy(ethAddr, &entry->ethAddr, sizeof(Ethernet::Address));
        return FileSystem::Success;
    }

    // Is this a broadcast address?
    if (*ipAddr == 0xffffffff)
    {
        MemoryBlock::set(ethAddr, 0xff, sizeof(Ethernet::Address));
        return FileSystem::Success;
    }

    // See if timeout has expired for re-transmission
    m_kernelTimer.tick();
    Timer::Info inf;
    m_kernelTimer.getCurrent(&inf);
    DEBUG("entry->time.ticks = " << entry->time.ticks <<
          " entry->time.freq = " << entry->time.frequency <<
          " kernelTimer.ticks = " << inf.ticks <<
          " kernelTimer.freq = " << inf.frequency);

    if (!entry->time.ticks || m_kernelTimer.isExpired(entry->time))
    {
        DEBUG("entry timeout: re-transmitting");

        // Send an ARP request
        const FileSystem::Result result = sendRequest(*ipAddr);
        if (result != FileSystem::Success && result != FileSystem::RetryAgain)
        {
            ERROR("failed to send request: result = " << (int) result);
            return result;
        }
    }

    // Make sure we are called again in about 500msec (or earlier)
    m_server.setTimeout(500);
    return FileSystem::RetryAgain;
}

FileSystem::Result ARP::sendRequest(const IPV4::Address address)
{
    DEBUG("address = " << *IPV4::toString(address));

    // Get cache entry
    ARPCache *entry = getCacheEntry(address);
    if (!entry)
    {
        return FileSystem::NotFound;
    }

    // Update the cache entry administration
    entry->valid = false;
    entry->retryCount++;
    if (entry->retryCount > MaxRetries)
    {
        entry->retryCount = 0;
        return FileSystem::NotFound;
    }
    m_kernelTimer.tick();
    m_kernelTimer.getCurrent(&entry->time, 500);

    // Destination is broadcast ethernet address
    Ethernet::Address destAddr;
    MemoryBlock::set(&destAddr, 0xff, sizeof(destAddr));

    // Get a fresh ethernet packet
    NetworkQueue::Packet *pkt;
    const FileSystem::Result result = m_parent.getTransmitPacket(&pkt, &destAddr, sizeof(destAddr),
                                                                 NetworkProtocol::ARP, sizeof(Header));
    if (result != FileSystem::Success)
    {
        if (result != FileSystem::RetryAgain)
        {
            ERROR("failed to get transmit packet: result = " << (int) result);
        }
        return result;
    }

    Ethernet::Header *ether = (Ethernet::Header *) (pkt->data + pkt->size - sizeof(Ethernet::Header));
    ARP::Header *arp = (ARP::Header *) (pkt->data + pkt->size);
    pkt->size += sizeof(ARP::Header);

    // Fill the ARP packet
    writeBe16(&arp->hardwareType, ARP::Ethernet);
    writeBe16(&arp->protocolType, ARP::IPV4);
    writeBe16(&arp->operation, ARP::Request);
    arp->hardwareLength = sizeof(Ethernet::Address);
    arp->protocolLength = sizeof(IPV4::Address);

    // Get our current IP
    IPV4::Address ipaddr;
    m_ip->getAddress(&ipaddr);

    // Fill source and destinations
    MemoryBlock::copy(&arp->etherSender, &ether->source, sizeof(Ethernet::Address));
    MemoryBlock::copy(&arp->etherTarget, &ether->destination, sizeof(Ethernet::Address));
    writeBe32(&arp->ipSender, ipaddr);
    writeBe32(&arp->ipTarget, address);

    // Send the packet using the network device
    return m_device.transmit(pkt);
}

FileSystem::Result ARP::sendReply(const Ethernet::Address *ethAddr, const IPV4::Address ipAddr)
{
    DEBUG("");

    if (!m_ip)
    {
        return FileSystem::InvalidArgument;
    }

    // Get a fresh ethernet packet
    NetworkQueue::Packet *pkt;
    const FileSystem::Result result = m_parent.getTransmitPacket(&pkt, ethAddr, sizeof(*ethAddr),
                                                                 NetworkProtocol::ARP, sizeof(Header));
    if (result != FileSystem::Success)
    {
        if (result != FileSystem::RetryAgain)
        {
            ERROR("failed to get transmit packet: result = " << (int) result);
        }
        return result;
    }

    IPV4::Address myip;
    m_ip->getAddress(&myip);

    // Fill the ARP packet
    Ethernet::Header *ether = (Ethernet::Header *)(pkt->data + pkt->size - sizeof(Ethernet::Header));
    ARP::Header *arp = (ARP::Header *)(pkt->data + pkt->size);
    pkt->size += sizeof(ARP::Header);

    DEBUG("eth: source=" << ether->source << " dest=" << ether->destination);

    // ARP packet
    writeBe16(&arp->hardwareType, ARP::Ethernet);
    writeBe16(&arp->protocolType, ARP::IPV4);
    writeBe16(&arp->operation, ARP::Reply);
    arp->hardwareLength = sizeof(Ethernet::Address);
    arp->protocolLength = sizeof(IPV4::Address);

    // Fill source and destinations
    MemoryBlock::copy(&arp->etherSender, &ether->source, sizeof(Ethernet::Address));
    MemoryBlock::copy(&arp->etherTarget, &ether->destination, sizeof(Ethernet::Address));
    writeBe32(&arp->ipSender, myip);
    writeBe32(&arp->ipTarget, ipAddr);

    // Send the packet using the network device
    return m_device.transmit(pkt);
}

FileSystem::Result ARP::process(const NetworkQueue::Packet *pkt, const Size offset)
{
    const Ethernet::Header *ether = (const Ethernet::Header *) (pkt->data + offset - sizeof(Ethernet::Header));
    const Header *arp = (const Header *) (pkt->data + offset);
    const u16 operation = readBe16(&arp->operation);
    const u32 ipTarget  = readBe32(&arp->ipTarget);
    const u32 ipSender  = readBe32(&arp->ipSender);
    IPV4::Address ipAddr;

    m_ip->getAddress(&ipAddr);

    DEBUG("target = " << *IPV4::toString(ipTarget) << " sender = " << *IPV4::toString(ipSender) <<
          " ipAddr = " << *IPV4::toString(ipAddr) << " operation = " << operation <<
          " etherSender = " << arp->etherSender << " etherTarget = " << arp->etherTarget);

    switch (operation)
    {
        case Request:
            // Only send reply if the request is for our IP
            if (ipTarget == ipAddr)
            {
                updateCacheEntry(ipSender, &arp->etherSender);
                return sendReply(&ether->source, ipSender);
            }
            return FileSystem::Success;

        case Reply: {
            updateCacheEntry(ipSender, &arp->etherSender);
            return m_sock->process(pkt);
        }
    }

    // Unknown ARP operation
    return FileSystem::InvalidArgument;
}

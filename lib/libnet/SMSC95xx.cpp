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

#include <FreeNOS/System.h>
#include <Log.h>
#include "SMSC95xx.h"

SMSC95xx::SMSC95xx(u8 deviceId, const char *usbPath)
    : USBDevice(deviceId, usbPath)
{
    DEBUG("");

    m_packetSize = 1500 + TransmitCommandSize;
    m_packet     = new u8[m_packetSize];
    m_value      = new u32;
}

Error SMSC95xx::initialize()
{
    DEBUG("");

    Error r = USBDevice::initialize();
    if (r != ESUCCESS)
    {
        ERROR("failed to initialize USBDevice");
        return r;
    }
    DEBUG("setting MAC");

    // Set MAC address
    MACAddress a;
    a.addr[0] = 0x00;
    a.addr[1] = 0x11;
    a.addr[2] = 0x22;
    a.addr[3] = 0x33;
    a.addr[4] = 0x44;
    a.addr[5] = 0x55;
    setMACAddress(a);

    // Submit receive transfers
    // TODO
    // TODO: we have blocking synchronous I/O now
    // TODO: modify the controller such that it will write US on transfer completion?

    // Enable RX/TX bits on hardware
    write(HardwareConfig, read(HardwareConfig) | MultipleEther | BulkIn | BCE);

#define SMSC9512_HS_USB_PKT_SIZE 512
#define SMSC9512_DEFAULT_HS_BURST_CAP_SIZE (16 * 1024 + 5 * SMSC9512_HS_USB_PKT_SIZE)
    write(BurstCap, SMSC9512_DEFAULT_HS_BURST_CAP_SIZE / SMSC9512_HS_USB_PKT_SIZE);

    write(MACControl, read(MACControl) | MACTransmit | MACReceive);
    write(TransmitConfig, TransmitOn);

    // Done
    return ESUCCESS;
}

Error SMSC95xx::read(IOBuffer & buffer, Size size, Size offset)
{
    return ENOTSUP;
}

Error SMSC95xx::write(IOBuffer & buffer, Size size, Size offset)
{
    DEBUG("size = " << size);

    // Check packet size
    if (size > m_packetSize - TransmitCommandSize)
    {
        ERROR("packet size too large: " << size);
        return ERANGE;
    }

    // Fill command word A
    u32 tx_cmd_a = size | TxCommandAFirstSeg | TxCommandALastSeg;
    m_packet[0] = (tx_cmd_a >> 0)  & 0xff;
    m_packet[1] = (tx_cmd_a >> 8)  & 0xff;
    m_packet[2] = (tx_cmd_a >> 16) & 0xff;
    m_packet[3] = (tx_cmd_a >> 24) & 0xff;

    // Fill command word B
    u32 tx_cmd_b = size;
    m_packet[4] = (tx_cmd_b >> 0)  & 0xff;
    m_packet[5] = (tx_cmd_b >> 8)  & 0xff;
    m_packet[6] = (tx_cmd_b >> 16) & 0xff;
    m_packet[7] = (tx_cmd_b >> 24) & 0xff;

    // Insert payload
    MemoryBlock::copy(m_packet + TransmitCommandSize, buffer.getBuffer(), size);

    // Flush L1 cache
    VMCtl(SELF, CacheClean, 0);

    // Start bulk transfer
    transfer(USBTransfer::Bulk,
             USBTransfer::Out,
             2,
             m_packet,
             size + TransmitCommandSize);

    DEBUG("done");
    return size;
}

Error SMSC95xx::getMACAddress(SMSC95xx::MACAddress *address)
{
    DEBUG("");

    u32 low  = read(MACAddrLow);
    u32 high = read(MACAddrHigh);

    address->addr[0] = (low >> 0) & 0xff;
    address->addr[1] = (low >> 8) & 0xff;
    address->addr[2] = (low >> 16) & 0xff;
    address->addr[3] = (low >> 24) & 0xff;
    address->addr[4] = (high >> 0) & 0xff;
    address->addr[5] = (high >> 8) & 0xff;
    return ESUCCESS;
}

Error SMSC95xx::setMACAddress(MACAddress address)
{
    DEBUG("");

    write(MACAddrLow, address.addr[0] |
                      (address.addr[1] << 8) |
                      (address.addr[2] << 16) |
                      (address.addr[3] << 24));
    write(MACAddrHigh, address.addr[4] |
                      (address.addr[5] << 8));
    return ESUCCESS;
}

u32 SMSC95xx::read(SMSC95xx::Register reg)
{
    Error r = controlMessage(ReadRegister,
                             USBTransfer::In,
                             USBTransfer::Vendor,
                             USBTransfer::Device,
                             0, (u16)reg, m_value, sizeof(*m_value));

    if (r != ESUCCESS)
    {
        ERROR("failed to read SMSC95xx register at offset " <<
               (uint) reg);
        return 0;
    }
    return (*m_value);
}

void SMSC95xx::write(SMSC95xx::Register reg, u32 value)
{
    *m_value = value;
    Error r = controlMessage(WriteRegister,
                             USBTransfer::Out,
                             USBTransfer::Vendor,
                             USBTransfer::Device,
                             0, (u16)reg, m_value, sizeof(*m_value));
    if (r != ESUCCESS)
    {
        ERROR("failed to write SMSC95xx register at offset " <<
               (uint) reg << " with value " << *m_value);
    }
}

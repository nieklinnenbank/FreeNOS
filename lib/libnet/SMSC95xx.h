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

#ifndef __LIBNET_SMSC95XX_H
#define __LIBNET_SMSC95XX_H

#include <USBDevice.h>

/**
 * SMSC95xx USB-based Ethernet controller.
 */
class SMSC95xx : public USBDevice
{
  private:

    /** vendor ID in the USB device descriptor for this device */
    static const u16 vendorId = 0x0424;

    /** product ID in the USB device descriptor for this device */
    static const u16 productId = 0xec00;

    /**
     * Hardware registers.
     */
    enum Register
    {
        DeviceID        = 0x00,
        InterruptStatus = 0x08,
        ReceiveConfig   = 0x0c,
        TransmitConfig  = 0x10,
        HardwareConfig  = 0x14,
        ReceiveFIFOInfo = 0x18,
        PowerControl    = 0x20,
        LEDConfig       = 0x24,
        GPIOConfig      = 0x28,
        FlowControl     = 0x2c,
        EEPromCommand   = 0x30,
        EEPromData      = 0x34,
        BurstCap        = 0x38,
        GPIOWake        = 0x64,
        MACControl      = 0x100,
        MACAddrHigh     = 0x104,
        MACAddrLow      = 0x108,
        MIIControl      = 0x114
    };

    /**
     * InterruptStatus register flags.
     */
    enum InterruptStatusFlags
    {
        TransferStop    = 0x00020000,
        ReceiveStop     = 0x00010000,
        PhyInterrupt    = 0x00008000
    };

    /**
     * TransmitConfig register flags.
     */
    enum TransmitConfigFlags
    {
        TransmitOn      = 0x00000004,
        TransmitOff     = 0x00000002,
        TransmitFlush   = 0x00000001
    };

    /**
     * HardwareConfig register flags.
     */
    enum HardwareConfigFlags
    {
        BulkIn          = 0x00001000,
        MultipleEther   = 0x00000020,
        BCE             = 0x00000002
    };

    /**
     * MACControl register flags.
     */
    enum MACControlFlags
    {
        MACTransmit     = 0x00000008,
        MACReceive      = 0x00000004
    };

    /**
     * Various vendor-specific USB control message requests.
     */
    enum USBRequest
    {
        WriteRegister = 0xa0,
        ReadRegister  = 0xa1,
        GetStatus     = 0xa2
    };

    /**
     * Transmit Command Words.
     */
    enum TransmitCommand
    {
        TxCommandADataOffset = 0x001F0000,
        TxCommandAFirstSeg   = 0x00002000,
        TxCommandALastSeg    = 0x00001000,
        TxCommandABufSize    = 0x000007FF,
        TxCommandBChecksum   = 0x00004000,
        TxCommandBCRC        = 0x00002000,
        TxCommandBPadding    = 0x00001000,
        TxCommandBLength     = 0x000007FF
    };

    /** Transmit Command Word size. */
    static const Size TransmitCommandSize = 8;

  public:

    /**
     * Represents a MAC Ethernet address.
     */
    typedef struct MACAddress
    {
        u8 addr[6];
    }
    MACAddress;

    /**
     * Constructor
     */
    SMSC95xx(u8 deviceId, const char *usbPath = "/usb");

    /**
     * Initialize the controller.
     *
     * @return Result code
     */
    virtual Error initialize();

    /**
     * @brief Read bytes from the file.
     *
     * @param buffer Input/Output buffer to output bytes to.
     * @param size Number of bytes to read, at maximum.
     * @param offset Offset inside the file to start reading.
     * @return Number of bytes read on success, Error on failure.
     */
    virtual Error read(IOBuffer & buffer, Size size, Size offset);

    /**
     * Write bytes to ethernet.
     *
     * @param buffer Input/Output buffer to input bytes from.
     * @param size Number of bytes to write, at maximum.
     * @param offset Offset inside the file to start writing.
     * @return Number of bytes written on success, Error on failure.
     */
    virtual Error write(IOBuffer & buffer, Size size, Size offset);

  private:

    /**
     * Get MAC address.
     */
    Error getMACAddress(MACAddress *address);

    /**
     * Set MAC address.
     */
    Error setMACAddress(MACAddress address);

    /**
     * Read a SMSC95xx register.
     */
    u32 read(Register reg);

    /**
     * Write a SMSC95xx register.
     */
    void write(Register reg, u32 value);

    /** Packet buffer */
    u8 *m_packet;

    /** Packet buffer size */
    Size m_packetSize;

    /** Word aligned temporary value */
    u32 *m_value;
};

#endif /* __LIBNET_SMSC95XX_H */

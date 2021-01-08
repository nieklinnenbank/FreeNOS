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

#ifndef __SERVER_NETWORK_SUN8I_SUN8IEMAC_H
#define __SERVER_NETWORK_SUN8I_SUN8IEMAC_H

#include <FreeNOS/System.h>
#include <Types.h>
#include <Index.h>
#include <Queue.h>
#include <NetworkDevice.h>
#include <Ethernet.h>
#include <arm/sunxi/SunxiClockControl.h>
#include <arm/sunxi/SunxiSystemControl.h>

/**
 * @addtogroup server
 * @{
 *
 * @addtogroup sun8iemac
 * @{
 */

/**
 * Network device of the Allwinner Sun8i family System-on-Chips.
 */
class Sun8iEmac : public NetworkDevice
{
  public:

    /** Interrupt number for this device on a sun8i family SoC */
    static const Size InterruptNumber = 114;

  private:

    /** Physical memory address of the device memory mapped registers. */
    static const Address MemoryAddress = 0x01c30000;

    /** Fixed address of the PHY on the MDIO bus */
    static const u8 PhyMdioAddress = 0x1;

    /** Maximum number of polling reads for MII-busy flag */
    static const Size MaximumMiiPoll = 100000;

    /** Maximum number of polling reset iterations */
    static const Size MaximumResetPoll = 100000;

    /**
     * Hardware registers
     */
    enum Registers
    {
        BasicCtl0        = 0x0000, /**@< Basic Control 0 */
        BasicCtl1        = 0x0004, /**@< Basic Control 1 */
        IntStatus        = 0x0008, /**@< Interrupt Status */
        IntEnable        = 0x000C, /**@< Interrupt Enable */
        TransmitCtl0     = 0x0010, /**@< Transmit Control 0 */
        TransmitCtl1     = 0x0014, /**@< Transmit Control 1 */
        TransmitFlowCtl  = 0x001C, /**@< Transmit Flow Control */
        TransmitDescList = 0x0020, /**@< Transmit Descriptor List Address */
        ReceiveCtl0      = 0x0024, /**@< Receive Control 0 */
        ReceiveCtl1      = 0x0028, /**@< Receive Control 1 */
        ReceiveDescList  = 0x0034, /**@< Receive Descriptor List Address */
        ReceiveFrmFilter = 0x0038, /**@< Receive Frame Filter */
        ReceiveHash0     = 0x0040, /**@< Receive Hash Table 0 */
        ReceiveHash1     = 0x0044, /**@< Receive Hash Table 1 */
        MiiCmd           = 0x0048, /**@< Management Interface Command */
        MiiData          = 0x004C, /**@< Management Interface Data */
        AddrHigh         = 0x0050, /**@< MAC Address High */
        AddrLow          = 0x0054, /**@< MAC Address Low */
        TransmitStatus   = 0x00B0, /**@< Transmit DMA Status */
        TransmitCurDesc  = 0x00B4, /**@< Transmit Current Descriptor */
        TransmitCurBuf   = 0x00B8, /**@< Transmit Current Buffer */
        ReceiveStatus    = 0x00C0, /**@< Receive DMA Status */
        ReceiveCurDesc   = 0x00C4, /**@< Receive Current Descriptor */
        ReceiveCurBuf    = 0x00C8, /**@< Receive Current Buffer */
        RgmiiStatus      = 0x00D0, /**@< RGMII Status */
    };

    /**
     * Flags for the Basic Control 0 register
     */
    enum BasicCtl0Flags
    {
        BasicCtl0FullDup   = (1 << 0),
        BasicCtl0SpeedMask = ((1 << 2) | (1 << 3)),
        BasicCtl0Speed100  = (0x3 << 2)
    };

    /**
     * Flags for the Basic Control 1 register
     */
    enum BasicCtl1Flags
    {
        BasicCtl1Reset      = (1 << 0),
        BasicCtl1BurstShift = 24
    };

    /**
     * Flags for the Interrupt Status register
     */
    enum IntStatusFlags
    {
        IntStatusReceive  = (1 << 8),
        IntStatusTransmit = (1 << 0)
    };

    /**
     * Flags for the Interrupt Enable register
     */
    enum IntEnableFlags
    {
        IntEnableReceive  = (1 << 8),
        IntEnableTransmit = (1 << 0)
    };

    /**
     * Flags for Media-Independent-Interface (MII) Command register
     */
    enum MiiCmdFlags
    {
        MiiCmdPhyAddrShift = 12,
        MiiCmdPhyAddrMask  = 0xf000,
        MiiCmdPhyRegShift  = 4,
        MiiCmdPhyRegMask   = 0xf0,
        MiiCmdPhyCsrDiv128 = 0x3,
        MiiCmdPhyCsrShift  = 20,
        MiiCmdPhyWrite     = (1 << 1),
        MiiCmdPhyBusy      = (1 << 0)
    };

    /**
     * Registers provided by the PHY on the Media-Independent-Interface (MII)
     */
    enum MiiRegisters
    {
        MiiRegControl = (0x0), /**@< Control */
        MiiRegStatus  = (0x1), /**@< Status */
        MiiRegIdHigh  = (0x2), /**@< Identifier High */
        MiiRegIdLow   = (0x3), /**@< Identifier Low */
        MiiRegAdv     = (0x4), /**@< Advertised abilities */
        MiiRegLink    = (0x5), /**@< Link partner abilities */
    };

    /**
     * Flags in the Mii Control register
     */
    enum MiiControlFlags
    {
        MiiControlFullDuplex  = 0x0100,
        MiiControlAutoRestart = 0x0200,
        MiiControlAutoEnable  = 0x1000,
        MiiControlSpeed100    = 0x2000,
        MiiControlReset       = 0x8000,
    };

    /**
     * Flags in the Mii Status register
     */
    enum MiiStatusFlags
    {
        MiiStatusExt          = 0x0001,
        MiiStatusLink         = 0x0004,
        MiiStatusAutoCap      = 0x0008,
        MiiStatusAutoCompl    = 0x0020
    };

    /**
     * Flags in the Mii Advertised abilities register
     */
    enum MiiAdvFlags
    {
        MiiAdvSpeed10Full  = 0x0040,
        MiiAdvSpeed100Full = 0x0100
    };

    /**
     * Flags for the Transmit Control 0 register
     */
    enum TransmitCtl0Flags
    {
        TransmitCtl0Enable = (1 << 31)
    };

    /**
     * Flags for the Transmit Control 1 register
     */
    enum TransmitCtl1Flags
    {
        TransmitCtl1DmaStart  = (1 << 31),
        TransmitCtl1DmaEnable = (1 << 30),
        TransmitCtl1NextFrame = (1 << 2),
        TransmitCtl1FullFrame = (1 << 1)
    };

    /**
     * Flags for the Receive Control 0 register
     */
    enum ReceiveCtl0Flags
    {
        ReceiveCtl0Enable = (1 << 31)
    };

    /**
     * Flags for the Receive Control 1 register
     */
    enum ReceiveCtl1Flags
    {
        ReceiveCtl1DmaStart   = (1 << 31),
        ReceiveCtl1DmaEnable  = (1 << 30),
        ReceiveCtl1ErrorFrame = (1 << 3),
        ReceiveCtl1UnderFrame = (1 << 2),
        ReceiveCtl1FullFrame  = (1 << 1)
    };

    /**
     * Transmit/receive frame descriptor
     */
    struct FrameDescriptor
    {
        u32 status;
        u32 bufsize;
        u32 bufaddr;
        u32 next;
    };

    /**
     * Flags for frame descriptors
     */
    enum FrameDescriptorFlags
    {
        FrameDescriptorCtl   = (1 << 31),
        TransmitDescRaiseInt = (1 << 31),
        TransmitDescFirst    = (1 << 29),
        TransmitDescLast     = (1 << 30),
        TransmitDescChained  = (1 << 24),
        ReceiveDescLast      = (1 << 8),
        ReceiveDescFrmShift  = 16,
        ReceiveDescFrmMask   = (0x3fff)
    };

  public:

    /**
     * Constructor
     *
     * @param inode Inode number
     * @param server NetworkServer reference
     */
    Sun8iEmac(const u32 inode,
              NetworkServer &server);

    /**
     * Destructor
     */
    virtual ~Sun8iEmac();

    /**
     * Initialize the device
     *
     * @return Result code
     */
    virtual FileSystem::Result initialize();

    /**
     * Read ethernet address.
     *
     * @param address Ethernet address reference for output
     *
     * @return Result code
     */
    virtual FileSystem::Result getAddress(Ethernet::Address *address);

    /**
     * Set ethernet address
     *
     * @param address New ethernet address to set
     *
     * @return Result code
     */
    virtual FileSystem::Result setAddress(const Ethernet::Address *address);

    /**
     * Called when an interrupt has been triggered for this device.
     *
     * @param vector Vector number of the interrupt.
     *
     * @return Result code
     */
    virtual FileSystem::Result interrupt(const Size vector);

    /**
     * Add a network packet to the transmit queue
     *
     * @param pkt Network packet buffer
     *
     * @return Result code
     */
    virtual FileSystem::Result transmit(NetworkQueue::Packet *pkt);

    /**
     * Start DMA processing
     *
     * @return Result code
     */
    virtual FileSystem::Result startDMA();

  private:

    /**
     * Receive packets.
     *
     * @return Result code
     */
    FileSystem::Result receive();

    /**
     * Wait until the PHY comes out of busy state.
     *
     * @return True if PHY is ready, false if still busy
     */
    bool miiBusyWait() const;

    /**
     * Read a Media-Independent-Interface (MII) register on the PHY.
     *
     * @param phyAddr Address of the PHY on the MDIO bus
     * @param regAddr MII-register to read
     *
     * @return Value of the register
     */
    u32 miiRead(const u8 phyAddr,
                const u8 regAddr);

    /**
     * Write a Media-Independent-Interface (MII) register on the PHY.
     *
     * @param phyAddr Address of the PHY on the MDIO bus
     * @param regAddr MII-register to write
     * @param data Value for the register
     */
    void miiWrite(const u8 phyAddr,
                  const u8 regAddr,
                  const u32 data);

    /**
     * Configure the PHY connected to the MAC controller.
     *
     * @return Result code
     */
    FileSystem::Result configPhy();

    /**
     * Reset the PHY connected to the MAC controller.
     *
     * @return Result code
     */
    FileSystem::Result resetPhy();

    /**
     * Reset receive control functions
     *
     * @return Result code
     */
    FileSystem::Result resetReceive();

    /**
     * Reset transmit control functions
     *
     * @return Result code
     */
    FileSystem::Result resetTransmit();

    /**
     * Reset the controller.
     *
     * @return Result code
     */
    FileSystem::Result reset();

    /**
     * Print diagnostic information about the receive queue (RX)
     */
    void printRx();

    /**
     * Print diagnostic information about the transmit queue (TX)
     */
    void printTx();

  private:

    /** Memory mapped registers */
    Arch::IO m_io;

    /** Clock Control Unit */
    SunxiClockControl m_ccu;

    /** System Control Unit */
    SunxiSystemControl m_syscon;

    /** Memory range for receive descriptors */
    Memory::Range m_receiveDescRange;

    /** List of pointers to receive descriptors */
    Index<FrameDescriptor, NetworkQueue::MaxPackets> m_receiveDesc;

    /** List of pointers to receive packets */
    Index<NetworkQueue::Packet, NetworkQueue::MaxPackets> m_receivePackets;

    /** Current receive packet index */
    Size m_receiveIndex;

    /** Memory range for transmit descriptors */
    Memory::Range m_transmitDescRange;

    /** List of pointers to transmit descriptors */
    Index<FrameDescriptor, NetworkQueue::MaxPackets> m_transmitDesc;

    /** List of pointers to packets pending transmission */
    Queue<NetworkQueue::Packet *, NetworkQueue::MaxPackets> m_transmitPending;

    /** List of pointers to packets that the driver has submitted for transmission */
    Queue<NetworkQueue::Packet *, NetworkQueue::MaxPackets> m_transmitPackets;

    /** Current transmit packet index */
    Size m_transmitIndex;
};

/**
 * @}
 * @}
 */

#endif /* __SERVER_NETWORK_SUN8I_SUN8IEMAC_H */

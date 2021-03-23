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

#ifndef __LIBIPC_MEMORYCHANNEL_H
#define __LIBIPC_MEMORYCHANNEL_H

#include <FreeNOS/System.h>
#include <Types.h>
#include "Channel.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libipc
 * @{
 */

/**
 * Unidirectional point-to-point channel using shared memory.
 *
 * Implemented by using two separated memory pages.
 * The data page is for the consumer in which it only reads
 * the incoming data payloads. The producer writes payloads
 * to the data page. The feedback page is written only by the
 * consumer, where it stores the feedback information from its
 * consumption.
 */
class MemoryChannel : public Channel
{
  private:

    /**
     * Defines in-memory ring header
     */
    typedef struct RingHead
    {
        /** Index where the ring buffer starts. */
        Size index;
    }
    RingHead;

  public:

    /**
     * Constructor
     *
     * @param mode Channel mode is either a producer or consumer
     * @param messageSize Size of each individual message in bytes
     */
    MemoryChannel(const Mode mode, const Size messageSize);

    /**
     * Destructor.
     */
    virtual ~MemoryChannel();

    /**
     * Set memory pages by virtual address.
     *
     * This function assumes that the given virtual addresses
     * are already mapped into the associated address space.
     *
     * @param data Virtual memory address of the data page.
     *             Read/Write for the producer, Read-only for the consumer.
     * @param feedback Virtual memory address of the feedback page.
     *        Read/write for the consumer, read-only for the producer.
     * @param hardReset Perform a hard reset after setting pages.
     *
     * @return Result code.
     */
    Result setVirtual(const Address data,
                      const Address feedback,
                      const bool hardReset = true);

    /**
     * Set memory pages by physical address.
     *
     * This function maps the given physical addresses
     * into the current address space using IO::map.
     *
     * @param data Physical memory address of the data page.
     *             Read/Write for the producer, Read-only for the consumer.
     * @param feedback Physical memory address of the feedback page.
     *        Read/write for the consumer, read-only for the producer.
     * @param hardReset Perform a hard reset after setting pages.
     *
     * @return Result code.
     */
    Result setPhysical(const Address data,
                       const Address feedback,
                       const bool hardReset = true);

    /**
     * Unmap memory pages from virtual address space
     *
     * @return Result code
     */
    Result unmap();

    /**
     * Read a message.
     *
     * @param buffer Output buffer for the message.
     *
     * @return Result code.
     */
    virtual Result read(void *buffer);

    /**
     * Write a message.
     *
     * @param buffer Input buffer for the message.
     *
     * @return Result code.
     */
    virtual Result write(const void *buffer);

    /**
     * Flush message buffers.
     *
     * Ensures that all messages are written through caches.
     *
     * @return Result code.
     */
    virtual Result flush();

    bool operator == (const MemoryChannel & ch) const
    {
        return false;
    }

    bool operator != (const MemoryChannel & ch) const
    {
        return false;
    }

  private:

    /**
     * Reset to initial state.
     *
     * @param hardReset True to always start using the channel from
     *                  beginning of data/feedback pages. False for soft-reset
     *                  to read the new value of m_head back from the data page.
     * @return Result code.
     */
    Result reset(const bool hardReset);

    /**
     * Flush memory page.
     *
     * @param page Memory page to flush
     *
     * @return Result code.
     */
    Result flushPage(const Address page) const;

  private:

    /** Maximum number of messages that can be stored. */
    const Size m_maximumMessages;

    /** The data page */
    Arch::IO m_data;

    /** The feedback page */
    Arch::IO m_feedback;

    /** Local RingHead. */
    RingHead m_head;
};

/**
 * @}
 * @}
 */

#endif /* __LIBIPC_MEMORYCHANNEL_H */

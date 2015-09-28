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

#include <Types.h>
#include "Channel.h"

/**
 * Unidirectional point-to-point channel using shared memory.
 *
 * Implemented by using two separated memory pages.
 * The data page is for the consumer in which it only reads
 * the incoming data payloads. The producer writes payloads
 * to the data page. The feedback page is written only by the
 * consumer, where it stores the feedback information from its
 * consumption, such as the total bytes read and status.
 */
class MemoryChannel : public Channel
{
  private:

    typedef struct RingHead
    {
        /** Index where the ring buffer starts. */
        Size index;
    }
    RingHead;

  public:

    /**
     * Constructor
     */
    MemoryChannel();

    /**
     * Set data page address.
     *
     * @param addr Physical memory address of the data page.
     *             Read/Write for the producer, Read-only for the consumer.
     * @return Result code.
     */
    Result setData(Address addr);

    /**
     * Set feedback page address.
     *
     * @param addr Physical memory address of the feedback page.
     *             Read/Write for the consumer, Read-only for the producer.
     * @return Result code.
     */
    Result setFeedback(Address addr);

    /**
     * Read a message.
     *
     * @param buffer Output buffer for the message.
     * @return Result code.
     */
    virtual Result read(void *buffer);

    /**
     * Write a message.
     *
     * @param buffer Input buffer for the message.
     * @return Result code.
     */
    virtual Result write(void *buffer);

    bool operator == (const MemoryChannel & ch) const
    {
        return false;
    }

    bool operator != (const MemoryChannel & ch) const
    {
        return false;
    }

  private:

    /** The data page */
    Arch::IO m_data;

    /** The feedback page */
    Arch::IO m_feedback;

    /** Local RingHead. */
    RingHead m_head;
};

#endif /* __LIBIPC_MEMORYCHANNEL_H */


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

#ifndef __LIBIPC_CHANNEL_H
#define __LIBIPC_CHANNEL_H

#include <Types.h>

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libipc
 * @{
 */

/**
 * Unidirectional point-to-point messaging channel.
 */
class Channel
{
  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        InvalidArgument,
        InvalidMode,
        InvalidSize,
        IOError,
        ChannelFull,
        NotFound,
        NotSupported
    };

    /**
     * Channel modes.
     */
    enum Mode
    {
        Producer,
        Consumer
    };

  public:

    /**
     * Constructor.
     *
     * @param mode Channel mode is either a producer or consumer
     * @param messageSize Size of each individual message in bytes
     */
    Channel(const Mode mode, const Size messageSize);

    /**
     * Destructor.
     */
    virtual ~Channel();

    /**
     * Get message size
     *
     * @return Message size
     */
    const Size getMessageSize() const;

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

  protected:

    /** Channel mode. */
    const Mode m_mode;

    /** Message size. */
    const Size m_messageSize;
};

/**
 * @}
 * @}
 */

#endif /* __LIBIPC_CHANNEL_H */

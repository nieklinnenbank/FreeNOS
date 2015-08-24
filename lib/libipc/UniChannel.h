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

/**
 * Unidirectional channel using shared memory.
 *
 * Implemented by using two separated memory pages.
 * The data page is for the consumer in which it only reads
 * the incoming data payloads. The producer writes payloads
 * to the data page. The feedback page is written only by the
 * consumer, where it stores the feedback information from its
 * consumption, such as the total bytes read and status.
 */
class UniChannel
{
  public:

    /**
     * Constructor
     *
     * @param data Physical memory address of the data page.
     * @param feedback Physical memory address of the feedback page.
     */
    UniChannel(Address data,
               Address feedback);

  private:

    /** Pointer to the data page */
    Address *m_data;

    /** Pointer to the feedback page */
    Address *m_feedback;
};

#endif /* __LIBIPC_CHANNEL_H */

/*
 * Copyright (C) 2009 Niek Linnenbank
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

#ifndef __HASHITERATOR_H
#define __HASHITERATOR_H

#include "Macros.h"
#include "Iterator.h"
#include "Container.h"
#include "ListIterator.h"
#include "HashTable.h"
#include "Assert.h"

/**
 * Iterate through a HashTable.
 */
template <class K, class V> class HashIterator // : public Iterator<V>
{
  public:

    /**
     * Class constructor.
     */
    HashIterator(HashTable<K, V> *hash)
        : m_hash(*hash)
    {
        assertRead(hash);

        m_listIter = ZERO;
        m_index    = 0;
    }

    /**
     * Class constructor.
     *
     * @param h Reference to the List to iterate.
     */
    HashIterator(HashTable<K, V> &hash)
        : m_hash(hash)
    {
        assertRead(hash);

        m_listIter = ZERO;
        m_index    = 0;
    }

    /**
     * Destructor.
     */
    ~HashIterator()
    {
        if (m_listIter) delete m_listIter;
    }

    /**
     * Reset the iterator.
     */
    virtual void reset()
    {
        m_index = 0;
        if (m_listIter)
            delete m_listIter;
        m_listIter = ZERO;
    }

    /**
     * Get current item.
     * @return Current item.
     */
    //virtual V & current()
    virtual V * current()
    {
        return m_listIter->current()->value;
    }

    /**
     * Retrieve key of the current item.
     * @return Current key.
     */
    //virtual K & key()
    virtual K * key()
    {
        return m_listIter->current()->key;
    }

    /**
     * Check if there is more on the HashTable to iterate.
     * @return true if more items, false if not.
     */
    virtual bool hasCurrent() const
    {
        return m_listIter && m_listIter->hasNext();
    }

    /**
     * Check if there is any next item.
     */
    virtual bool hasNext() const
    {
        // TODO: fix this
        return false;
    }

    /**
     * Fetch the next item.
     * @return Pointer to the next item.
     */
    //virtual V & next()
    virtual V * next()
    {
        HashBucket<K, V> *n = ZERO;
        List<HashBucket<K, V> *> *lst = ZERO;

        // Look for next item on the List, otherwise find next List
        if (!m_listIter || !(n = m_listIter->next()))
        {
            while (++m_index < m_hash.size() - 1)
            {
                if ((lst = &(m_hash.map()[m_index])) && lst->head())
                {
                    if (m_listIter)
                        delete m_listIter;

                    m_listIter = new ListIterator<HashBucket<K, V> *>(lst);
                    n = m_listIter->current();
                    break;
                }
            }
        }
        // Next item
        return n->value;
    }

    /**
     * Increment operator
     */
    virtual void operator++(int num)
    {
        // TODO: this is wrong. must obey current vs. next. See ListIterator/Iterator
        next();
    }

  private:

    /** Points to the HashTable to iterate. */
    HashTable<K, V> & m_hash;

    /** Current list of HashBuckets. */
    ListIterator<HashBucket<K, V> *> *m_listIter;

    /** Current index in the HashTable. */
    Size m_index;
};

#endif /* __HASHITERATOR_H */

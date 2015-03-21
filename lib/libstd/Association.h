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

#ifndef __LIBSTD_ASSOCIATION_H
#define __LIBSTD_ASSOCIATION_H

#include "Container.h"
#include "Comparable.h"
#include "Types.h"
#include "Macros.h"
#include "List.h"
#include "ListIterator.h"

/**
 * Associations are containers that provide a mapping of keys to values.
 */
template <class K, class V> class Association : public Container, public Comparable<Assocation<K,V> >
{
  public:

    /**
     * Inserts the given item to the Assocation.
     * If an item exists for the given key, its value will be replaced.
     *
     * @param position The position to insert the item.
     * @param item The item to insert
     * @return bool Whether inserting the item at the given position succeeded.
     */
    virtual bool insert(const K & key, const V & item)
    {
        return false;
    }

    /**
     *
     */
    virtual bool append(const K & key, const V & item)
    {
        return false;
    }

    /**
     * Removes all items associated with the given key.
     *
     * @param key The key to remove items for.
     * @return Number of items removed associated with the key.
     */
    virtual int remove(const K & key)
    {
        return 0;
    }

    /**
     * Removes all items from the Association
     */
    virtual void clear()
    {
        for (ListIterator<K> i(keys()); i.hasNext(); i++)
        {
            remove(i.current());
        }
    }

    /**
     * Retrieve all keys inside the Association.
     *
     * @return A List of keys.
     */
    virtual const List<K> keys() const = 0;

    /**
     * Retrieve all values inside the Association.
     *
     * @return A List of values.
     */
    virtual const List<V> values(const K & key) const = 0;

    /**
     * Returns the item at the given position.
     *
     * @param position The position of the item to get.
     * @return Pointer to the item at the given position or ZERO if no item available.
     */
    virtual const V * value(const K & key) const = 0;

    /**
     * Returns a reference to the item at the given position.
     * Note that this function does not perform bounds checking.
     * Position must be a valid index.
     *
     * @param position Valid index inside this array.
     * @return Reference to the item at the given position
     */
    virtual const V & value(const K & key) const = 0;

    /**
     * Return value at the given position.
     * If position is not within bounds of this array,
     * this function will return a default constructed T.
     *
     * @param position Index inside this array.
     * @return T at the given position or default constructed T.
     */
    virtual const V value(const K & key) const = 0;

    /**
     * Compare this Container to another Container.
     */
    virtual int compareTo(const Association<T> &a) const
    {
        Size sz = size();
        Size cnt = count();

        // Size must be equal
        if (t.size() != sz)
            return t.size() - sz;

        // Count must be equal
        if (t.count() != cnt)
            return t.count() - cnt;

        // All elements must be equal
        /*
        for (Size s = 0; s < cnt; s++)
        {
            if (at(s) != t.at(s))
            {
                return s + 1;
            }
        }*/
#warning please use key and values here
        return 0;
    }


#warning please provide better operators using Key instead

    /**
     * Returns the item at the given position in the Array.
     *
     * @param i The index of the item to return.
     * @return the Item at position i.
     */
    const V & operator [] (int i) const
    {
        return at(i);
    }

    /**
     * Returns the item at the given position in the Array.
     *
     * @param i The index of the item to return.
     * @return the Item at position i.
     */
    const V & operator [] (Size i) const
    {
        return at(i);
    }

    /**
     * Returns the item at the given position in the Array.
     *
     * @param i The index of the item to return.
     * @return the Item at position i.
     */
    T & operator [] (int i)
    {
        return (T &) at(i);
    }

    /**
     * Returns the item at the given position in the Array.
     *
     * @param i The index of the item to return.
     * @return the Item at position i.
     */
    T & operator [] (Size i)
    {
        return (T &) at(i);
    }
};

#endif /* __LIBSTD_ASSOCIATION_H */

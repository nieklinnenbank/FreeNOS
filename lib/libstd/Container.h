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

#ifndef __LIBSTD_CONTAINER_H
#define __LIBSTD_CONTAINER_H

#include "Comparable.h"
#include "Types.h"
#include "Macros.h"

/**
 * Containers provide access to stored items.
 */
template <class T> class Container : public Comparable<Container<T> >
{
  public:

    /**
     * Adds the given item to the Container, if possible.
     *
     * @param item The item to add to the Container.
     * @return Position of the item in the Container or -1 on failure.
     */
    virtual int put(const T & item)
    {
        return -1;
    }

    /**
     * Inserts the given item at the given position.
     * If an item exists at the given position, it will be replaced by the given item.
     *
     * @param position The position to insert the item.
     * @param item The item to insert
     * @return bool Whether inserting the item at the given position succeeded.
     */
    virtual bool put(Size position, const T & item)
    {
        return false;
    }

    /**
     * Fill the container with the given value.
     *
     * @param value New value to fill the Container.
     */
    virtual void fill(T value)
    {
        Size s = size();

        for (Size i = 0; i < s; i++)
            put(i, value);
    }

    /**
     * Removes the item at the given position.
     *
     * @param position The position of the item to remove.
     * @return bool Whether removing the item succeeded.
     */
    virtual bool remove(Size position)
    {
        return false;
    }

    /**
     * Removes all items from the Container.
     */
    virtual void clear()
    {
        Size s = size();

        for (Size i = 0; i < s; i++)
            remove(i);
    }

    /**
     * Returns the item at the given position.
     *
     * @param position The position of the item to get.
     * @return Pointer to the item at the given position or ZERO if no item available.
     */
    virtual const T * get(Size position) const = 0;

    /**
     * Returns a reference to the item at the given position.
     * Note that this function does not perform bounds checking.
     * Position must be a valid index.
     *
     * @param position Valid index inside this array.
     * @return Reference to the item at the given position
     */
    virtual const T & at(Size position) const = 0;

    /**
     * Return value at the given position.
     * If position is not within bounds of this array,
     * this function will return a default constructed T.
     *
     * @param position Index inside this array.
     * @return T at the given position or default constructed T.
     */
    virtual const T value(Size position) const
    {
        const T *t = get(position);

        if (!t)
            return T();
        else
            return (*t);
    }

    /**
     * Returns the maximum size of this Container.
     *
     * @return size The maximum size of this Container.
     */
    virtual Size size() const = 0;

    /**
     * Returns the number of items inside the Container.
     * @return Number of items inside the Container.
     */
    virtual Size count() const
    {
        return size();
    }

    /**
     * Compare this Container to another Container.
     */
    virtual int compareTo(const Container<T> &t) const
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
        for (Size s = 0; s < cnt; s++)
        {
            if (at(s) != t.at(s))
            {
                return s + 1;
            }
        }
        return 0;
    }

    /**
     * Returns the item at the given position in the Array.
     *
     * @param i The index of the item to return.
     * @return the Item at position i.
     */
    const T & operator [] (int i) const
    {
        return at(i);
    }

    /**
     * Returns the item at the given position in the Array.
     *
     * @param i The index of the item to return.
     * @return the Item at position i.
     */
    const T & operator [] (Size i) const
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

#endif /* __LIBSTD_CONTAINER_H */

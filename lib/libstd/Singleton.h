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

#ifndef __LIB_LIBSTD_SINGLETON_H
#define __LIB_LIBSTD_SINGLETON_H

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

/**
 * Singleton design pattern: only one instance is allowed.
 *
 * The StrictSingleton has the following constraints:
 *
 * - only one instance allowed
 * - the instance cannot be overwritten
 * - the instance is created by the StrictSingleton on first use (lazy-instantiation)
 * - user-class must provide a default constructor without parameters
 */
template <class T> class StrictSingleton
{
  public:

    /**
     * Retrieve the instance.
     *
     * Note that we use a static initializer here, which makes use
     * of the __cxa_guard_acquire and __cxa_guard_release functions
     * in order to guarantee the object is only initialized once.
     *
     * @see __cxa_guard_acquire
     * @see __cxa_guard_release
     */
    static inline T * instance()
    {
        static T obj;
        return &obj;
    }
};

/**
 * Singleton design pattern: only one instance is allowed.
 *
 * The WeakSingleton follows weaker rules than the StrictSingleton:
 *
 * - only one instance allowed
 * - the instance can be overwritten
 * - the instance may be ZERO at any time
 */
template <class T> class WeakSingleton
{
  public:

    /**
     * Constructor
     *
     * @param obj New instance of T.
     */
    WeakSingleton<T>(T *obj)
    {
        m_instance = obj;
    }

    /**
     * Retrieve the instance
     */
    static inline T * instance()
    {
        return m_instance;
    }

  private:

    /** One and only instance. */
    static T *m_instance;
};

/* Initialize the static member obj. */
template <class T> T* WeakSingleton<T>::m_instance = 0;

/**
 * @}
 * @}
 */

#endif /* __LIB_LIBSTD_SINGLETON_H */

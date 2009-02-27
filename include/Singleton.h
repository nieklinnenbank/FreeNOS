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

#ifndef __SINGLETON_H
#define __SINGLETON_H

/**
 * Singleton design pattern; only one instance is allowed.
 */
template <class T> class Singleton
{
    public:

        /**
         * Create an instance of T.
         * @return Pointer to T.
         */
        static T * instance()
        {
	    if (!Singleton<T>::obj)
	    {
		Singleton<T>::obj = new T();
	    }
	    return Singleton<T>::obj;
	}
	
    private:

	/** One and only instance. */    
	static T *obj;
};

/* Initialize the static member obj. */
template <class T> T* Singleton<T>::obj = 0;

#endif /* __SINGLETON_H */

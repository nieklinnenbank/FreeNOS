/*
 * Copyright (C) 2009 Coen Bijlsma
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

#ifndef __MAP_H
#define __MAP_H

#include "Macros.h"
#include "Types.h"
#include "Assert.h"
#include "Vector.h"
#include "Comparable.h"

template <class K = Comparable<class K>,
	  class V = Comparable<class V> > class Map
{
    public:

	/**
	 * Constructor.
	 */
	Map()
	{
	}
	
	/**
	 * Destructor.
	 */
	virtual ~Map() {}
	
	/**
	 * Removes all of the mappings from this map.
	 */
	void clear()
	{
	    for(Size size = 0; size < _keys.count(); size++ )
	    {
		_keys[size] = (K*) NULL;
		_values[size] = (V*) NULL;
	    }
	}
	
	/**
	 * @brief Returns a shallow copy of this Map.
	 *        The keys and values themselves are not cloned.
	 * @return A shallow clone of this Map.
	 */
	Map<K, V> clone()
	{
	    Map<K, V> clone;
	    
	    for(Size index = 0; index < _keys.count(); index++)
	    {
		clone.put( _keys[index], _values[index] );
	    }
	    return clone;
	}
	
	/**
	 * Returns true if this Map contains a mapping for the given key.
	 * @return bool Whether this Map contains a mapping the given key.
	 */
	bool containsKey(K* key, bool strict = false)
	{
	    for( Size index = 0; index < _keys.count(); index++ )
	    {
		if( ! strict )
		{
		    if( _keys[index]->equals(key) )
		    {
			return true;
		    }
		} else {
		    if( _keys[index] == key )
		    {
			return true;
		    }
		}
	    }
	    return false;
	}
	
	/**
	 * Returns true if this Map maps one ore more keys to the given value.
	 * @return bool Whether this Map maps >= 1 key(s) to the given value.
	 */
	bool containsValue(V* value, bool strict = false)
	{
	    for( Size index = 0; index < _values.count(); index++ )
	    {
		if( ! strict )
		{
		    if( _values[index]->equals(value) )
		    {
			return true;
		    }
		} else {
		    if( _values[index] == value )
		    {
			return true;
		    }
		}
	    }
	    return false;
	}
	
	/**
	 * Get a shallow clone of the Vector containing the keys in this Map.
	 */
	Vector<K> keys()
	{
    	    return _keys.clone();
	}
	
	/**
	 * Compares Map instances.
	 * @param map Map instance.
	 * @return True if equal, false otherwise.
	 */
	bool equals(Map<K, V> map)
	{
	    if( *map == this )
	    {
		return true;
	    }
	    if( map.size() != this->size() )
	    {
		return false;
	    }
		
	    for( Size index = 0; index < _keys.count(); index++ )
	    {
		if( ! _keys[index]->equals(map.keys()[index])
		 || ! _values[index]->equals(map.values()[index]) )
		{
		    return false;
		}
	    }
	    return true;
	}
	
	/**
	 * Get value to which the specified key is mapped, or NULL if not mapped.
	 * @param key Pointer to the key to lookup.
	 * @return The value for the given key, or NULL if not found.
	 */
	V* get(K* key)
	{
    	    int index = _getKeyIndex(key);
		
	    if( index != -1 )
	    {
		return *(_values[(Size)index]);
	    }
	    return (V*) NULL;
	}
	
	/**
	 * Returns the hash code value for this Map.
	 * This is currently not implemented
	 */
	int hashCode()
	{
	    return 0;
	}
	
	/**
	 * Returns true if this Map contains no key-value mappings.
	 */
	bool isEmpty()
	{
	    return (_keys.count() == 0);
	}
	
	/**
	 * Associates the specified value with the specified key in this map.
	 */
	V* put(K* key, V* value)
	{
	    int index = _getKeyIndex(key);
		
	    if( index == -1 )
	    {
		_keys.insert(key);
		_values.insert(value);
	    } else {
		_values.insert( (Size)index, value);
	    }
	    return value;
	}
	
	/**
	 * Removes the mapping for a key from this Map if it is present.
	 */
	V* remove(K* key)
	{
	    int index = _getKeyIndex(key);
		
	    if( index != -1 )
	    {
		_keys.remove( (Size)index );
		V* value = _values[index];
		_values.remove( (Size)index );
		return value;
	    }
	    return (V*) NULL;
	}
	
	/**
	 * Returns the number of key-value mappings in this Map.
	 */
	Size size()
	{
	    return _keys.size();
	}
	
	/**
	 * Returns a shallow clone if the Vector that contains the values.
	 */
	Vector<V> values()
	{
	    return _values.clone();
	}

    private:

	/** Known keys in the Map. */
	Vector<K> _keys;
	
	/** Values in the Map. */
	Vector<V> _values;
	
	int _getKeyIndex(K* key)
	{
	    for( Size index = 0; index < _keys.count(); index++ )
	    {
		if( _keys[index]->equals(key) )
		{
	    	    return (int)index;
		}
	    }
	    return -1;
	}
};

#endif /* __MAP_H */

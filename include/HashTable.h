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

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#include "Types.h"
#include "Macros.h"
#include "List.h"
#include "ListIterator.h"
#include "HashFunction.h"
#include "Comparable.h"
#include "Assert.h"

/** Default size of the HashTable internal table. */
#define DEFAULT_SIZE	64

/**
 * Describes a bucket in the HashTable, for collition avoidance.
 */
template <class Key, class Value> class HashBucket
{
    public:
    
	/**
	 * Constructor.
	 * @param k Key to use.
	 * @param v Value of the bucket.
	 */
	HashBucket(Key *k, Value *v)
	{
	    assert(k != ZERO);
	    assert(v != ZERO);
	    key = k;
	    value = v;
	}

	/**
	 * Comparision operator.
	 * @param b HashBucket instance to compare us with.
	 */
	bool operator == (HashBucket *b)
	{
	    return key == b->key && value == b->value;
	}

	/** Unique key. */
	Key *key;
	
	/** Value of this bucket. */
	Value *value;
};

/**
 * Efficient key -> value lookups.
 */
template <class Key, class Value> class HashTable
{
    public:
    
	/**
	 * Class constructor.
	 * @param hash Hash function to use.
	 * @param sz Initial size of the internal table.
	 */
	HashTable(Size (*hash)(Comparable<Key> *, Size) = FNVHash, Size sz = DEFAULT_SIZE)
	{
	    assert(hash != ZERO);
	    assert(sz > 0);

	    _size  = sz;	
	    _count = ZERO;
	    _map   = new List<HashBucket<Key,Value> >[sz];
	    _hash  = hash;
	}
	
	/**
	 * Insert a new item.
	 * @param k Associated key.
	 * @param v New item to add.
	 */
	void insert(Key *k, Value *v)
	{
	    assertRead(k);
	    assertRead(v);
	    _map[_hash(k,_size)].insertTail(new HashBucket<Key,Value>(k,v));
	    _count++;
	}
	
	/**
	 * Remove an item.
	 * @param k Associated key.
	 * @param deleteKey Perform an delete() on the key if found.
	 * @param deleteValue Perform an delete() on value if found.
	 */
	void remove(Key *k, bool deleteKey   = false,
			    bool deleteValue = false)
	{
	    HashBucket<Key,Value> *b;
	    
	    assertRead(k);
	    
	    if ((b = findBucket(k)))
	    {
		_map[_hash(k,_size)].remove(b);
		
		if (deleteKey) delete b->key;
		if (deleteValue) delete b->value;
		delete b;
		_count--;
	    }
	}

	/**
	 * Get the size of the HashTable.
	 * @return Size of the internal array.
	 */
	Size size() const
	{
	    return _size;
	}

	/**
	 * Get the number of filled buckets.
	 * @return Number of items in the HashTable.
	 */
	Size count() const
	{
	    return _count;
	}

	
	/**
	 * Fetch the internal array.
	 * @return Pointer to the internal array.
	 */
	List<HashBucket<Key, Value> > * map() const
	{
	    return _map;
	}

	/**
	 * Lookup the value for the given key.
	 * @param k Key to find a value for.
	 * @return Value on success and ZERO otherwise.
	 */
        Value * operator [] (Key *k)
	{
	    HashBucket<Key,Value> *b;
	    
	    assertRead(k);
	    
	    return (b = findBucket(k)) ? b->value : ZERO;
	}
    
    private:

	/**
	 * Find the corresponding HashBucket for a given key.
	 * @param k Key for which we find a bucket.
	 * @return Pointer to the bucket on success, ZERO otherwise.
	 */
	HashBucket<Key,Value> * findBucket(Key * &k)
	{
	    assertRead(k);
	
	    for (ListIterator<HashBucket<Key,Value> > i(&_map[_hash(k,_size)]); i.hasNext(); i++)
	    {
		if (k->equals(i.current()->key))
		{
		    return i.current();
		}
	    }
	    return ZERO;
	}

	/** Internal array. */
	List<HashBucket<Key,Value> > *_map;
	
	/** Size of the internal array. */
	Size _size;
	
	/** Number of filled buckets. */
	Size _count;
	
	/** Hash function. */
	Size (*_hash)(Comparable<Key> *, Size);
};

#endif /* __HASHTABLE_H */

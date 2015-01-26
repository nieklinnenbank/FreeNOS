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
#include "ListIterator.h"
#include "HashTable.h"
#include "Assert.h"

/**
 * Iterate through a HashTable.
 */
template <class Key, class Value> class HashIterator
    : public Iterator<Value, HashTable<Key, Value> *>
{
    public:

	/**
	 * Empty constructor.
	 */
	HashIterator()
	    : hash(ZERO), listIter(ZERO), index(-1)
	{
	}

	/**
	 * Class constructor.
	 * @param h Points to the HashTable to iterate.
	 */
	HashIterator(HashTable<Key, Value> *h)
	    : hash(h), listIter(ZERO), index(-1)
	{
	    assertRead(h);
	    reset(h);
	}
	
	/**
	 * Class constructor.
	 * @param h Reference to the List to iterate.
	 */
	HashIterator(HashTable<Key, Value> &h)
	    : hash(&h), listIter(ZERO), index(-1)
	{
	    assertRead(&h);
	    reset(&h);
	}
	
	/**
	 * Destructor.
	 */
	~HashIterator()
	{
	    if (listIter) delete listIter;
	}

	/**
	 * Reset the iterator.
	 * @param h Points to the HashTable to iterate.
	 */
	void reset(HashTable<Key, Value> *h)
	{
	    assertRead(h);
	    index    = ZERO;
	    if (listIter) delete listIter;
	    listIter = ZERO;
	    next();
	}

	/**
	 * Get current item.
	 * @return Current item.
	 */
	Value * current()
	{
	    return listIter ? listIter->current()->value : ZERO;
	}
	
	/**
	 * Retrieve key of the current item.
	 * @return Current key.
	 */
	Key * key()
	{
	    return listIter ? listIter->current()->key : ZERO;
	}
	
	/**
	 * Check if there is more on the HashTable to iterate.
	 * @return true if more items, false if not.
	 */
	bool hasNext() const
	{
	    return listIter ? listIter->hasNext() : ZERO;
	}
	
	/**
	 * Fetch the next item.
	 * @return Pointer to the next item.
	 */
	Value * next()
	{
	    HashBucket<Key, Value> *n = ZERO;
	    List<HashBucket<Key, Value> > *lst = ZERO;
	    
	    /* Look for next item on the List, otherwise find next List. */
	    if (!listIter || !(n = listIter->next()))
	    {
		while (index++ < hash->size() - 1)
		{
		    if ((lst = &(hash->map()[index])) && lst->head())
		    {
			if (listIter)
			    delete listIter;
			
			listIter = new ListIterator<HashBucket<Key, Value> >(lst);
			n = listIter->current();
			break;
		    }
		}
	    }
	    /* Next item, if any. */
	    return n ? n->value : ZERO;
	}
	
	/**
	 * Post increment operator.
	 */
	void operator++(int n)
	{
	    next();
	}
	
	private:
	
	    /** Points to the HashTable to iterate. */
	    HashTable<Key, Value> *hash;
	    
	    /** Current list of HashBuckets. */
	    ListIterator<HashBucket<Key, Value> > *listIter;
	    
	    /** Current index in the HashTable. */
	    Size index;
};

#endif /* __HASHITERATOR_H */

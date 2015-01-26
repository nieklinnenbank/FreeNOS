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

#ifndef __SORTEDLIST_H
#define __SORTEDLIST_H

#include "Assert.h"
#include "Comparable.h"

/**
 * Represents an item in the SortedList. The data in a SortedListNode
 * cannot be (U*)0.
 */
template <class U> class SortedListNode
{
    public:
    
        /**
         * Class constructor.
         * @param t Data value.
         * @param p Previous SortedListNode.
         * @param n Next SortedListNode.
         */
        SortedListNode(Comparable<U> *t
            , SortedListNode<U>* p
            , SortedListNode<U>* n)
            : data(t), prev(p), next(n)
        {
            assertRead(t);
            assert(t);
        }
        
        /** User data. */
        U* data;
        
        /** Previous and next node. */
        SortedListNode<U>* prev;
        SortedListNode<U>* next;
};

/**
 * Represents a sorted list. Every item in a SortedList must implement
 * the functions of the Comparable class as to sort the items in the list.
 */
template <class T> class SortedList
{
    public:
    
    /**
     * Class constructor.
     * Creates an empty list without a headnode and
     * a node count of zero.
     */
    SortedList() : headNode(0), nodeCount(0)
    {
    }
    
    /**
     * Class destructor.
     * Deletes the nodes but not the data contained in the nodes.
     */
    ~SortedList()
    {
        while (headNode)
        {
            SortedListNode<T> *tmp = headNode;
            headNode = headNode->next;
            delete tmp;
        }
    }
    
    /**
     * Inserts the given item in the list. If the given item
     * is (T*)0, then assertion fails. If not, a new SortedListNode<T>*
     * is created and added to the list in it's designated position.
     * If the same data already exists in the list, the newly created node
     * is not inserted in the list and deleted whilst preserving the data 
     * in the node.
     * If the head node is empty, the newly created SortedListNode becomes
     * the nead node and the function returns.
     * If the head node already exists, the middle of the list is determined
     * by calling get( (Size) (nodeCount / 2)) and the second insert is called.
     * This saves up to twice the time needed to insert a node because at most
     * half of the list has to be searched and compared.
     *
     * @param t The data to insert to the list.
     * @see SortedList::insert(SortedListNode<T>* t, SortedListNode<T>* from)
     */
    void insert(Comparable<T> *t)
    {
        assertRead(t);
        
        /* Create a new node from the given data. */
        SortedListNode<T>* n 
            = new SortedListNode<T>(t
                , (SortedListNode<T>*) 0
                , (SortedListNode<T>*) 0);
        
        if(!headNode)
        {
            /* headNode == 0, so the list is still empty. */
            headNode = n;
            nodeCount++;
        } else {
            /*
             * The list is filled. That means we have to
             * retrieve the node in the middle
             */
            SortedListNode<T>* middle 
                = get( (Size)(nodeCount / 2) );
            insert(n, middle);
        }
        
    }
    
    /**
     * Removes the node that contains the given data from the list.
     * Note that only the node is deleted, not the data contained in it.
     * This function runs in O(n).
     * @param t The data to remove from the list.
     * @see http://en.wikipedia.org/wiki/Big_O_notation
     */
    void remove(Comparable <T> *t)
    {
        if(!t)
        {
            return;
        }
        
        SortedListNode<T>* tmp = headNode;
        
        while(tmp)
        {
            if( tmp->data == t )
            {
                SortedListNode<T>* p = tmp->prev;
                SortedListNode<T>* n = tmp->next;
                
                if(p && n)
                {
                    n->prev = p;
                    p->next = n;
                } else if(!p) {
                    n->prev = p;
                } else {
                    p->next = n;
                }
                delete tmp;
                return;
            }
            tmp = tmp->next;
        }
    }
    
    /**
     * Removes the given node from the list. The data contained in the node
     * is preserved. This function runs in O(n).
     * @param n The node to be removed from the list.
     * @see http://en.wikipedia.org/wiki/Big_O_notation
     */
    void remove(SortedListNode<T>* n)
    {
        remove(n->data);
    }
    
    /**
     * Returns the head node of the list.
     * @return The head node if this list.
     */
    SortedListNode<T>* head() const
    {
        return headNode;
    }
    
    private:
    
        /** Head of the SortedList. */
        SortedListNode<T>* headNode;
        
        /** Number of items currently in the SortedList. */
        Size nodeCount;
    
        /**
         * Returns the SortedListNode at the nth position.
         * This function runs in O(n).
         * @param n The position of the node to get.
         * @return The node at position n or (SortedListNode<Comparable<T> >*)0
         * if it doesn't exist.
         * @see http://en.wikipedia.org/wiki/Big_O_notation
         */
        SortedListNode<T>* get(Size n)
        {
            if( n >= nodeCount )
            {
                return (SortedListNode<T>*)0;
            }
            
            SortedListNode<T>* item = headNode;
            
            for(Size s = 0; s < n && item; s++)
            {
                item = item->next;
            }
            
            return item;
        }
        
        /**
         * Inserts the SortedListNode in the SortedList, but starts comparing
         * as of the given position. This prevents unnecessary searches through
         * the SortedList and can save up to half the SortedList's length.
         * This function runs at a minimum speed of O(n/2).
         * @param t The SortedListNode to add.
         * @param pos The position to start the insert.
         * @see http://en.wikipedia.org/wiki/Big_O_notation
         */
        void insert(SortedListNode<T>* t, 
            SortedListNode<T>* from)
        {
            
            if(!from)
            {
                /* The `from' node doesn't exist. TODO: exit(EXIT_FAILURE)*/
                return;
            }
            T* tc = t->data;
            T* fc = from->data;
            int result = tc->compareTo(fc);
            
            if( result == 0 )
            {
                /* The nodes contain the same data, so quit */
                return;
            } else if( result < 0 )
            {
                /* t->data is less then from->data so search the first half */
                SortedListNode<T>* tmp = headNode;
                
                do
                {
                    if( tmp->data->compareTo(tc) > 0 )
                    {
                        /* Insert t between tmp->prev and tmp. */
                        SortedListNode<T>* p = tmp->prev;
                        if(p)
                        {
                            p->next = t;
                            t->prev = p;
                            t->next = tmp;
                            tmp->prev = t;
                        } else {
                            t->next = tmp;
                            t->prev = (SortedListNode<T>*)0;
                            tmp->prev = t;
                            headNode = t;
                        }
                        break;
                    }
                    
                    tmp = tmp->next;
                } 
                while( tmp->prev != from );
                nodeCount++;
                return;
            }
            
            /* t->data is greater then from->data so search the last half */
            SortedListNode<T>* tmp = from;
            
            do
            {
            
                if(!tmp->next)
                {
                    tmp->next = t;
                    t->prev = tmp;
                    break;
                } else if( tmp->data->compareTo(tc) > 0 )
                {
                    /* Insert t between tmp->prev and tmp. */
                    SortedListNode<T>* p = tmp->prev;
                    if(p)
                    {
                        p->next = t;
                        t->prev = p;
                        t->next = tmp;
                        tmp->prev = t;
                    } else {
                        t->next = tmp;
                        tmp->prev = t;
                        headNode = t;
                    }
                    break;
                 }
                
                tmp = tmp->next;
            } 
            while( tmp );
            nodeCount++;
            return;
        }
};

#endif /* __SORTEDLIST_H */

// ----------------------------------------------------------------------------
// Least Recently Used cache backed by a hash table
// (unordered_map) and a fixed-size circular queue.
//
// License:
//  Public Domain.
//  Feel free to copy, distribute, and modify this file as you see fit.
// ----------------------------------------------------------------------------

#pragma once
#ifndef lru_h__
#define lru_h__

#include "compat.h"

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <utility>

#include <array>
#include <unordered_map>

// ----------------------------------------------------------------------------

enum ResetFlags : uint8_t
{
    RF_NOTHING,
    RF_INIT = 1,
    RF_FREE = 2,
    RF_INIT_AND_FREE = RF_INIT | RF_FREE,
};

// Simple fixed-size circular queue. Once the capacity is full it starts
// popping the front of the queue to make room for new items pushed at the end.
template <typename T, ssize_t Capacity, ResetFlags ResetItems = RF_INIT> class CircularQueue final
{
private:
    T * m_items = nullptr;
    int m_head;
    int m_tail;
    int m_count;

public:
#ifdef USE_MIMALLOC
    CircularQueue() { m_items = (T *)mi_calloc(Capacity, sizeof(T)); clear(); }
    ~CircularQueue() { mi_free(m_items); }
#else
    CircularQueue() { m_items = (T *)std::calloc(Capacity, sizeof(T)); clear(); }
    ~CircularQueue() { std::free(m_items); }
#endif
    void clear()
    {
        m_head  = -1;
        m_tail  = -1;
        m_count =  0;

        if (ResetItems & RF_FREE)
            for (int i = 0; i < Capacity; i++)
                DO_FREE_AND_NULL(m_items[i]);

        if (ResetItems & RF_INIT)
            for (int i = 0; i < Capacity; i++)
                m_items[i] = T {};
    }

    void pushBack(const T & item)
    {
        if (m_head == (Capacity - 1))
            m_head = -1;
        else if (m_count < Capacity)
            ++m_count;
        if ((++m_head == m_tail) | (m_tail == -1))
        {
            if ((m_tail != -1) & ((ResetItems & RF_FREE) == RF_FREE))
                Xfree(m_items[m_tail]);
            m_tail = (m_tail + 1) % Capacity;
        }
        m_items[m_head] = item;
    }

    void popFront()
    {
        Bassert(!isEmpty());

        if (ResetItems & RF_FREE)
            Xfree(m_items[m_tail]);

        if (ResetItems & RF_INIT)
            m_items[m_tail] = T {};

        if (--m_count == 0)
            m_head = m_tail = -1;
        else
        {
            Bassert(m_tail != m_head);
            m_tail = (m_tail + 1) % Capacity;
        }
    }

    FORCE_INLINE T* begin()        const { return m_items;  }
    FORCE_INLINE T* end()          const { return m_items + m_count;  }
    FORCE_INLINE int size()        const { return m_count;  }
    FORCE_INLINE int capacity()    const { return Capacity; }
    FORCE_INLINE int frontIndex()  const { return m_tail;   }
    FORCE_INLINE int backIndex()   const { return m_head;   }

    FORCE_INLINE bool isEmpty()    const { return (m_count == 0); }
    FORCE_INLINE bool isFull()     const { return (m_count == Capacity); }

    FORCE_INLINE       T & front()       { Bassert(!isEmpty()); return m_items[m_tail]; }
    FORCE_INLINE const T & front() const { Bassert(!isEmpty()); return m_items[m_tail]; }

    FORCE_INLINE       T & back()        { Bassert(!isEmpty()); return m_items[m_head]; }
    FORCE_INLINE const T & back()  const { Bassert(!isEmpty()); return m_items[m_head]; }

    FORCE_INLINE       T & operator[](size_t index)       { return m_items[index]; }
    FORCE_INLINE const T & operator[](size_t index) const { return m_items[index]; }
};

// ----------------------------------------------------------------------------

// Simple fixed-size cache that keeps track of the
// Most Recently Used (MRU) and Least Recently Used (LRU)
// items in it. Once the cache is full, the LRU item
// is removed to make room for the new insertion, which
// becomes the MRU item. The cache uses a hash-table and
// a circular queue under the hood.
template <typename K,      // Key type
          typename V,      // Value type
          int CacheSize,   // Size of the cache in items
          int HTPrimeSize  // CacheSize rounded up to a prime, for the hash-table
          >
class LruCache final
{
private:
    std::unordered_map<K, V>    m_cache; // Size capped to CacheSize
    CircularQueue<K, CacheSize> m_lruQ;  // back=MRU, front=LRU

public:
    using Pair = std::pair<K, V>;
    LruCache() : m_cache { HTPrimeSize } {}

    V * access(const K & key)
    {
        auto iter = m_cache.find(key);
        if (iter == m_cache.end())
            return nullptr;
        m_lruQ.pushBack(iter->first);
        return &iter->second;
    }

    bool insert(const K & key, const V & value, Pair * outOptEvictedEntry = nullptr)
    {
        Bassert(access(key) == nullptr); // No duplicate keys

        bool entryEvicted = false;
        if (m_cache.size() == CacheSize)
        {
            // Make room for a new item by removing the oldest cached.
            // Need to call in a loop because the LRU queue might have
            // multiple references to the same item, which might have
            // already been removed from the cache by a previous insert.
            for (;;)
            {
                auto iter = m_cache.find(m_lruQ.front());
                m_lruQ.popFront();

                if (iter != m_cache.end())
                {
                    if (outOptEvictedEntry)
                        *outOptEvictedEntry = *iter;
                    m_cache.erase(iter);
                    entryEvicted = true;
                    break;
                }
            }
            Bassert(m_cache.size() == CacheSize - 1);
        }

        m_cache.insert({ key, value });
        m_lruQ.pushBack(key);
        return entryEvicted;
    }

    void clear()
    {
        m_cache.clear();
        m_lruQ.clear();
    }

    int size() const { return (int)m_cache.size(); }

    bool isEmpty() const
    {
        return m_cache.empty();
    }

    Pair mru() const
    {
        Bassert(!isEmpty());
        return *m_cache.find(m_lruQ.back());
    }

    Pair lru() const
    {
        Bassert(!isEmpty());
        return *m_cache.find(m_lruQ.front());
    }

    int copyContents(std::array<Pair, CacheSize> & dest) const
    {
        int n = 0;
        for (const Pair & p : m_cache)
            dest[n++] = p;
        return n;
    }
};
#endif // lru_h__

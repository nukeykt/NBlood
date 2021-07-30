#include "compat.h"
#ifndef atomiclist_h__
#define atomiclist_h__

// ------------------------------------------------------------------
// AtomicSListNode:
// ------------------------------------------------------------------

template<typename T>
struct AtomicSListNode
{
    T * m_next = nullptr;
};

// ------------------------------------------------------------------
// AtomicSList64:
// ------------------------------------------------------------------

// Atomic, but *not* A-B-A safe.
// We can concurrently push and pop, but only as long
// as the incoming pointer is guaranteed to be unique.
// Still covers a lot of usage cases though, such as
// concurrently building up a list that will later be
// consumed by a single thread.
template<typename T>
class AtomicSList64 final
{
public:

    AtomicSList64()
        : m_head{ nullptr }
    { }

    FORCE_INLINE void push(T * node)
    {
        T * currHead = m_head.load(std::memory_order_relaxed);
        do {
            node->m_next = currHead;
        } while (!m_head.compare_exchange_weak(currHead, node,
                                               std::memory_order_release,
                                               std::memory_order_relaxed));
    }

    FORCE_INLINE T * pop()
    {
        T * currHead = m_head.load(std::memory_order_relaxed);
        while (currHead != nullptr)
        {
            if (m_head.compare_exchange_weak(currHead, currHead->m_next,
                                             std::memory_order_release,
                                             std::memory_order_acquire))
            {
                break;
            }
        }
        return currHead;
    }

    // These are non-atomic.
    FORCE_INLINE T * first() const
    {
        return m_head.load(std::memory_order_relaxed);
    }
    FORCE_INLINE bool isEmpty() const
    {
        return first() == nullptr;
    }

    // Syntactic sugar to iterate the list.
    FORCE_INLINE static const T * next(const T * curr) { return curr->m_next; }
    FORCE_INLINE static       T * next(      T * curr) { return curr->m_next; }

    // Not copyable.
    AtomicSList64(const AtomicSList64 &) = delete;
    AtomicSList64 & operator = (const AtomicSList64 &) = delete;

private:

    std::atomic<T *> m_head;
};
#endif // atomiclist_h__

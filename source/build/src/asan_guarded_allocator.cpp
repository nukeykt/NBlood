
#if __SANITIZE_ADDRESS__ == 1

#include "compat.h"
#include "mimalloc-override.h"
#include "smmalloc.h"

#include <unordered_map>

# include "sanitizer/asan_interface.h"

#define MAX_GUARD_SIZE 64

// Adds a footer and header block to the allocation,
// as big as the block requested by the user, up to a maximum
// of 128 bytes. The footer and header are kept poisoned, acting
// as guard pages around the addressable block.

struct AllocRecord
{
    void *headerStart;
    void *userStart;

    size_t userSize;
    size_t alignment;
};

std::unordered_map<void const *, AllocRecord> m_allocs;

bool IsValidPtr(void const *ptr) { return (m_allocs.find(ptr) != m_allocs.end()); }

using GuardedAllocator = sm::GenericAllocator;
GuardedAllocator::TInstance GuardedAllocator::Invalid() { return nullptr; }

bool GuardedAllocator::IsValid(TInstance instance)
{
    SMMALLOC_UNUSED(instance);
    return true;
}

GuardedAllocator::TInstance GuardedAllocator::Create() { return nullptr; }
void GuardedAllocator::Destroy(GuardedAllocator::TInstance instance)
{
    SMMALLOC_UNUSED(instance);
}

static FORCE_INLINE size_t AlignSize(size_t const bytesCount, size_t const alignment)
{
    size_t const alignedSize = (bytesCount + alignment - 1) & ~(alignment - 1);
    Bassert((alignedSize % alignment) == 0);
    return alignedSize + alignment;
}


static FORCE_INLINE size_t GetPaddedSize(size_t const bytesCount)
{
    return min(bytesCount * 3, bytesCount + (MAX_GUARD_SIZE<<1));
}

void GuardedAllocator::Free(GuardedAllocator::TInstance instance, void* p)
{
    SMMALLOC_UNUSED(instance);
    auto iter = m_allocs.find(p);
    if (iter == m_allocs.end())
    {
        return;
    }

    size_t const bytes    = iter->second.userSize;
    size_t const realSize = AlignSize(GetPaddedSize(bytes), iter->second.alignment);
    void *       block    = iter->second.headerStart;

    ASAN_UNPOISON_MEMORY_REGION(block, realSize);
    mi_free(block);

    m_allocs.erase(iter);
}

static FORCE_INLINE void *AlignPtr(void const *ptr, size_t const alignment)
{
    uintptr_t alignedPtr = (uintptr_t(ptr) + alignment - 1) & ~(alignment - 1);
    auto      userPtr    = (void *)(alignedPtr);
    Bassert((uintptr_t(userPtr) % alignment) == 0);
    return userPtr;
}

static FORCE_INLINE void *AdjustUserPtr(void *headerStart, size_t const bytesCount, size_t const padding, size_t const alignment)
{
    void *userPtr = AlignPtr((uint8_t *)(headerStart) + padding, alignment);
    ASAN_UNPOISON_MEMORY_REGION(userPtr, bytesCount);
    return userPtr;
}

void* GuardedAllocator::Alloc(GuardedAllocator::TInstance instance, size_t bytesCount, size_t alignment)
{
    SMMALLOC_UNUSED(instance);
    size_t const paddedSize = GetPaddedSize(bytesCount);
    size_t const realSize = AlignSize(paddedSize, alignment);

    // Poison the whole range:
    void *block = mi_malloc(realSize);
    ASAN_POISON_MEMORY_REGION(block, realSize);

    AllocRecord ar;
    ar.headerStart = block;
    ar.userStart   = AdjustUserPtr(block, bytesCount, (paddedSize - bytesCount) >> 1, alignment); // Will unpoison the user portion
    ar.userSize    = bytesCount;
    ar.alignment   = alignment;

    auto iter = m_allocs.emplace(ar.userStart, ar);
    Bassert(iter.second == true);
    return ar.userStart;
}

void* GuardedAllocator::Realloc(GuardedAllocator::TInstance instance, void* p, size_t bytesCount, size_t alignment)
{
    SMMALLOC_UNUSED(instance);
    void *newp = GuardedAllocator::Alloc(instance, bytesCount, alignment);
    Bmemcpy(newp, p, min(bytesCount, GetUsableSpace(instance, p)));
    Free(instance, p);
    return newp;
}

size_t GuardedAllocator::GetUsableSpace(GuardedAllocator::TInstance instance, void* p)
{
    SMMALLOC_UNUSED(instance);
    auto iter = m_allocs.find(p);
    return (iter != m_allocs.end() ? iter->second.userSize : 0);
}
#endif // __SANITIZE_ADDRESS__

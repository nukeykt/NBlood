#include "compat.h"
#include "mutex.h"

int32_t mutex_init(mutex_t *mutex)
{
#if SDL_MAJOR_VERSION >= 2
    // SDL2's spinlock is most preferable
    *mutex = 0;
    return 0;
#elif defined _WIN32
    // Windows' "Critical Section" is preferable to "Mutex" -- within a single process only
    // https://preshing.com/20111124/always-use-a-lightweight-mutex/
    InitializeCriticalSection(mutex);
    return 0;
#elif SDL_MAJOR_VERSION == 1
    if (mutex)
    {
        *mutex = SDL_CreateMutex();
        if (*mutex != NULL)
            return 0;
    }
    return -1;
#else
    return -1;
#endif
}

void mutex_destroy(mutex_t *mutex)
{
#if SDL_MAJOR_VERSION >= 2
    *mutex = 0;
#elif defined _WIN32
    DeleteCriticalSection(mutex);
#elif SDL_MAJOR_VERSION == 1
    if (mutex)
    {
        SDL_DestroyMutex(*mutex);
        *mutex = nullptr;
    }
#endif
}

void mutex_lock(mutex_t *mutex)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_AtomicLock(mutex);
#elif defined _WIN32
    EnterCriticalSection(mutex);
#elif SDL_MAJOR_VERSION == 1
    SDL_LockMutex(*mutex);
#endif
}

void mutex_unlock(mutex_t *mutex)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_AtomicUnlock(mutex);
#elif defined _WIN32
    LeaveCriticalSection(mutex);
#elif SDL_MAJOR_VERSION == 1
    SDL_UnlockMutex(*mutex);
#endif
}

void mutex_try(mutex_t *mutex)
{
#if SDL_MAJOR_VERSION >= 2
    SDL_AtomicTryLock(mutex);
#elif defined _WIN32
    TryEnterCriticalSection(mutex);
#elif SDL_MAJOR_VERSION == 1
    SDL_TryLockMutex(*mutex);
#endif
}

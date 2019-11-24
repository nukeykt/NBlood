#include "compat.h"

#ifdef _WIN32
# define NEED_PROCESS_H
# include "windows_inc.h"
#endif

#include "mutex.h"

int32_t mutex_init(mutex_t *mutex)
{
#ifdef RENDERTYPEWIN
    *mutex = CreateMutex(0, FALSE, 0);
    return (*mutex == 0);
#elif SDL_MAJOR_VERSION == 1
    if (mutex)
    {
        *mutex = SDL_CreateMutex();
        if (*mutex != NULL)
            return 0;
    }
    return -1;
#else
    *mutex = 0;
    return 0;
#endif
}

void mutex_lock(mutex_t *mutex)
{
#ifdef RENDERTYPEWIN
    WaitForSingleObject(*mutex, INFINITE);
#elif SDL_MAJOR_VERSION == 1
    SDL_LockMutex(*mutex);
#else
    SDL_AtomicLock(mutex);
#endif
}

void mutex_unlock(mutex_t *mutex)
{
#ifdef RENDERTYPEWIN
    ReleaseMutex(*mutex);
#elif SDL_MAJOR_VERSION == 1
    SDL_UnlockMutex(*mutex);
#else
    SDL_AtomicUnlock(mutex);
#endif
}

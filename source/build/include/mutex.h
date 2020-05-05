#ifndef mutex_h_
#define mutex_h_

/* Mutual exclusion mechanism wrappers for the different platforms */

#ifdef RENDERTYPESDL
# define SDL_MAIN_HANDLED
# include "sdl_inc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if SDL_MAJOR_VERSION >= 2
typedef SDL_SpinLock mutex_t;
#elif defined _WIN32
# include "windows_inc.h"
typedef CRITICAL_SECTION mutex_t;
#elif SDL_MAJOR_VERSION == 1
typedef SDL_mutex * mutex_t;
#else
# error No mutex implementation provided.
#endif

extern int32_t mutex_init(mutex_t *mutex);
extern void mutex_destroy(mutex_t *mutex);
extern void mutex_lock(mutex_t *mutex);
extern void mutex_unlock(mutex_t *mutex);
extern void mutex_try(mutex_t *mutex);


#ifdef __cplusplus
}
#endif

#endif

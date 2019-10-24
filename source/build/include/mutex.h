#ifndef mutex_h_
#define mutex_h_

/* Mutual exclusion mechanism wrappers for the different platforms */

#ifdef RENDERTYPEWIN
# include "windows_inc.h"
#else
# define SDL_MAIN_HANDLED
# include "sdl_inc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RENDERTYPEWIN
typedef HANDLE mutex_t;
#elif SDL_MAJOR_VERSION == 1
typedef SDL_mutex * mutex_t;
#else
typedef SDL_SpinLock mutex_t;
#endif

extern int32_t mutex_init(mutex_t *mutex);
extern void mutex_lock(mutex_t *mutex);
extern void mutex_unlock(mutex_t *mutex);


#ifdef __cplusplus
}
#endif

#endif

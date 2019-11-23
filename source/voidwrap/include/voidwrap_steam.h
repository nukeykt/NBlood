#ifndef VOIDWRAP_STEAM_H_
#define VOIDWRAP_STEAM_H_

#include "voidwrap.h"

#if defined VOIDWRAP_ISEXPORTING || !defined VOIDWRAP_RUNTIMELINK

#ifdef __cplusplus
extern "C" {
#endif

VOIDWRAP_API bool Voidwrap_Steam_Init(void);
VOIDWRAP_API void Voidwrap_Steam_Shutdown(void);
VOIDWRAP_API void Voidwrap_Steam_RunCallbacks(void);

VOIDWRAP_API void Voidwrap_Steam_UnlockAchievement(char const * id);
VOIDWRAP_API void Voidwrap_Steam_SetStat(char const * id, int32_t value);
VOIDWRAP_API void Voidwrap_Steam_ResetStats(void);

#ifdef VWDEBUG
VOIDWRAP_API void Voidwrap_Steam_SetCallback_PrintDebug(VW_VOID_CONSTCHARPTR function);
#endif

#ifdef VWSCREENSHOT
VOIDWRAP_API bool Voidwrap_Steam_SendScreenshot(char * filepath, int32_t width, int32_t height);
VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotRequested(VW_VOID function);
#if 0
VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotReady(VW_VOID_INT32 function);
#endif
#endif

#ifdef __cplusplus
}
#endif

#else

static VW_BOOL Voidwrap_Steam_Init;
static VW_VOID Voidwrap_Steam_Shutdown;
static VW_VOID Voidwrap_Steam_RunCallbacks;
static VW_VOID Voidwrap_Steam_ResetStats;

static VW_VOID_CONSTCHARPTR       Voidwrap_Steam_UnlockAchievement;
static VW_VOID_CONSTCHARPTR_INT32 Voidwrap_Steam_SetStat;

#ifdef VWDEBUG
static VW_SETCALLBACK_VOID_CONSTCHARPTR Voidwrap_Steam_SetCallback_PrintDebug;
#endif

#ifdef VWSCREENSHOT
static VW_BOOL_SCREENSHOT Voidwrap_Steam_SendScreenshot;
static VW_SETCALLBACK_VOID Voidwrap_Steam_SetCallback_ScreenshotRequested;
#if 0
static VW_SETCALLBACK_VOID_INT32 Voidwrap_Steam_SetCallback_ScreenshotReady;
#endif
#endif

#endif

#endif

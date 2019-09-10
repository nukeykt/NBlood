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

#ifdef VWDEBUG
VOIDWRAP_API void Voidwrap_Steam_SetCallback_PrintDebug(VW_CALLBACK_CHARPTR function);
#endif

#ifdef VWSCREENSHOT
VOIDWRAP_API bool Voidwrap_Steam_SendScreenshot(char * filepath, int32_t width, int32_t height);
VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotRequested(VW_CALLBACK_NOPARAM function);
#if 0
VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotReady(VW_CALLBACK_INT32 function);
#endif
#endif

#ifdef VWCONTROLLER
VOIDWRAP_API int32_t Voidwrap_Steam_GetConnectedControllers(void);
#endif

#ifdef __cplusplus
}
#endif

#else

static VW_BOOL Voidwrap_Steam_Init;
static VW_VOID Voidwrap_Steam_Shutdown;
static VW_VOID Voidwrap_Steam_RunCallbacks;

#ifdef VWDEBUG
static VW_SETCALLBACK_CHARPTR Voidwrap_Steam_SetCallback_PrintDebug;
#endif

#ifdef VWSCREENSHOT
static VW_BOOL_SCREENSHOT Voidwrap_Steam_SendScreenshot;
static VW_SETCALLBACK_NOPARAM Voidwrap_Steam_SetCallback_ScreenshotRequested;
#if 0
static VW_SETCALLBACK_INT32 Voidwrap_Steam_SetCallback_ScreenshotReady;
#endif
#endif

#ifdef VWCONTROLLER
static VW_INT32 Voidwrap_Steam_GetConnectedControllers;
#endif

#endif

#endif

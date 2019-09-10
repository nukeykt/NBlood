// Voidwrap - A Steam API wrapper.

#define VOIDWRAP_ISEXPORTING
#include "voidwrap_steam.h"
#include "steam_api.h"
#include "compat.h"

static CSteamID SteamID;
static uint64_t SteamID64;
static uint64_t AppID;


static VW_CALLBACK_CHARPTR Callback_PrintDebug;

static void PrintDebug(const char * fmt, ...)
{
    if (Callback_PrintDebug == nullptr)
    {
        return;
    }

    static char tmpstr[8192];
    va_list va;

    va_start(va, fmt);
    Bvsnprintf(tmpstr, sizeof(tmpstr), fmt, va);
    va_end(va);

    Callback_PrintDebug(tmpstr);
}

static void SteamAPIWarningMessageHook(int nSeverity, const char * pchDebugText)
{
    if (Callback_PrintDebug == nullptr)
    {
        return;
    }

    Callback_PrintDebug(pchDebugText);
}

#ifdef VWDEBUG
VOIDWRAP_API void Voidwrap_Steam_SetCallback_PrintDebug(VW_CALLBACK_CHARPTR function)
{
    Callback_PrintDebug = function;
}
#endif


#ifdef VWSCREENSHOT
class SteamScreenshotHandler
{
private:
    STEAM_CALLBACK(SteamScreenshotHandler, screenshotRequested, ScreenshotRequested_t);
    STEAM_CALLBACK(SteamScreenshotHandler, screenshotReady, ScreenshotReady_t);
};

static SteamScreenshotHandler * ScreenHandler;
static VW_CALLBACK_NOPARAM Callback_ScreenshotRequested;
// static VW_CALLBACK_INT32 Callback_ScreenshotReady;

VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotRequested(VW_CALLBACK_NOPARAM function)
{
    Callback_ScreenshotRequested = function;
}

#if 0
VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotReady(VW_CALLBACK_INT32 function)
{
    Callback_ScreenshotReady = function;
}
#endif

VOIDWRAP_API bool Voidwrap_Steam_SendScreenshot(char * filepath, int32_t width, int32_t height)
{
    if (INVALID_SCREENSHOT_HANDLE == SteamScreenshots()->AddScreenshotToLibrary(filepath, nullptr, width, height))
    {
        return false;
    }

    return true;
}

void SteamScreenshotHandler::screenshotRequested(ScreenshotRequested_t * pCallback)
{
    if (Callback_ScreenshotRequested != nullptr)
    {
        PrintDebug("SteamScreenshotHandler::screenshotRequested executed.");
        Callback_ScreenshotRequested();
    }
}

void SteamScreenshotHandler::screenshotReady(ScreenshotReady_t * pCallback)
{
    PrintDebug("SteamScreenshotHandler::screenshotReady executed. Result: %d", pCallback->m_eResult);

#if 0
    if (Callback_ScreenshotReady != nullptr)
    {
        Callback_ScreenshotReady(pCallback->m_eResult);
    }
#endif
}
#endif


#ifdef VWCONTROLLER
static int32_t NumControllerHandles;
static ControllerHandle_t * ControllerHandles;

VOIDWRAP_API int32_t Voidwrap_Steam_GetConnectedControllers()
{
    SteamController()->RunFrame(); // poll for any queued controller events
    NumControllerHandles = SteamController()->GetConnectedControllers(ControllerHandles);
    return NumControllerHandles;
}
#endif


VOIDWRAP_API bool Voidwrap_Steam_Init()
{
    if (!SteamAPI_Init())
    {
        SteamAPI_Shutdown();
        return false;
    }

    SteamUtils()->SetWarningMessageHook(&SteamAPIWarningMessageHook);

    SteamID = SteamUser()->GetSteamID();
    SteamID64 = SteamID.ConvertToUint64();
    AppID = SteamUtils()->GetAppID();

#if 0
    PrintDebug("AppID is %llu", AppID);
    if (SteamUtils()->IsOverlayEnabled()) { PrintDebug("Overlay is enabled."); }
#endif

#ifdef VWSCREENSHOT
    SteamScreenshots()->HookScreenshots(true);
    if (SteamScreenshots()->IsScreenshotsHooked()) { PrintDebug("Screenshots hooked."); }
    ScreenHandler = new SteamScreenshotHandler();
#endif

#ifdef VWCONTROLLER
    if (SteamController()->Init()) { PrintDebug("Controller API init succeeded."); }
    ControllerHandles = new ControllerHandle_t[STEAM_CONTROLLER_MAX_COUNT];
#endif

    SteamAPI_RunCallbacks();

    return true;
}

VOIDWRAP_API void Voidwrap_Steam_Shutdown()
{
    SteamAPI_Shutdown();
}

VOIDWRAP_API void Voidwrap_Steam_RunCallbacks()
{
    SteamAPI_RunCallbacks();
}

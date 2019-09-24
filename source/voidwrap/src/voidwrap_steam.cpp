// Voidwrap - A Steam API wrapper.

#define VOIDWRAP_ISEXPORTING
#include "voidwrap_steam.h"
#include "steam_api.h"
#include "compat.h"

static uint64_t AppID;
static VW_VOID_CONSTCHARPTR Callback_PrintDebug;

static void PrintDebug(const char * fmt, ...)
{
    if (Callback_PrintDebug == nullptr)
        return;

    static char tmpstr[8192];
    va_list va;

    va_start(va, fmt);
    Bvsnprintf(tmpstr, sizeof(tmpstr), fmt, va);
    va_end(va);

    Callback_PrintDebug(tmpstr);
}

static void SteamAPIWarningMessageHook(int nSeverity, const char * pchDebugText)
{
    UNREFERENCED_PARAMETER(nSeverity);

    if (Callback_PrintDebug == nullptr)
        return;

    Callback_PrintDebug(pchDebugText);
}

#ifdef VWDEBUG
VOIDWRAP_API void Voidwrap_Steam_SetCallback_PrintDebug(VW_VOID_CONSTCHARPTR function)
{
    Callback_PrintDebug = function;
}
#endif


class SteamStatsAndAchievementsHandler
{
public:
    SteamStatsAndAchievementsHandler()
        : m_pSteamUserStats{SteamUserStats()}
    { }

    void SetAchievement(char const * id);
    void SetStat(char const * id, int32_t value);
    void ResetStats();

    STEAM_CALLBACK(SteamStatsAndAchievementsHandler, OnUserStatsReceived, UserStatsReceived_t);
    STEAM_CALLBACK(SteamStatsAndAchievementsHandler, OnUserStatsStored, UserStatsStored_t);

    void Process();

private:
    ISteamUserStats * m_pSteamUserStats;

    bool m_bRequestedStats{};
    bool m_bStatsValid{};

    bool m_bStoreStats{};
};

void SteamStatsAndAchievementsHandler::SetAchievement(char const * id)
{
    if (nullptr == m_pSteamUserStats)
        return;

    m_pSteamUserStats->SetAchievement(id);

    m_bStoreStats = true;
}

void SteamStatsAndAchievementsHandler::SetStat(char const * id, int32_t value)
{
    if (nullptr == m_pSteamUserStats)
        return;

    m_pSteamUserStats->SetStat(id, value);

    m_bStoreStats = true;
}

void SteamStatsAndAchievementsHandler::ResetStats()
{
    if (nullptr == m_pSteamUserStats)
        return;

    m_pSteamUserStats->ResetAllStats(true);
    m_bStatsValid     = false;
    m_bRequestedStats = false;
}

void SteamStatsAndAchievementsHandler::OnUserStatsReceived(UserStatsReceived_t * pCallback)
{
    if (nullptr == m_pSteamUserStats || AppID != pCallback->m_nGameID)
        return;

    if (k_EResultOK == pCallback->m_eResult)
        m_bStatsValid = true;
    else
        PrintDebug("RequestStats - failed, %d\n", pCallback->m_eResult);
}

void SteamStatsAndAchievementsHandler::OnUserStatsStored(UserStatsStored_t * pCallback)
{
    if (AppID != pCallback->m_nGameID)
        return;

    if (k_EResultInvalidParam == pCallback->m_eResult)
        PrintDebug("StoreStats - some failed to validate\n");
    else
        PrintDebug("StoreStats - failed, %d\n", pCallback->m_eResult);
}

void SteamStatsAndAchievementsHandler::Process()
{
    if (nullptr == m_pSteamUserStats)
        return;

    if (!m_bRequestedStats)
    {
        m_bRequestedStats = m_pSteamUserStats->RequestCurrentStats();
        return;
    }

    if (!m_bStatsValid || !m_bStoreStats)
        return;

    m_bStoreStats = !m_pSteamUserStats->StoreStats();
}

static SteamStatsAndAchievementsHandler * StatsAndAchievementsHandler;

VOIDWRAP_API void Voidwrap_Steam_UnlockAchievement(char const * id)
{
    if (nullptr == StatsAndAchievementsHandler)
        return;

    StatsAndAchievementsHandler->SetAchievement(id);
}

VOIDWRAP_API void Voidwrap_Steam_SetStat(char const * id, int32_t value)
{
    if (nullptr == StatsAndAchievementsHandler)
        return;

    StatsAndAchievementsHandler->SetStat(id, value);
}

VOIDWRAP_API void Voidwrap_Steam_ResetStats()
{
    if (nullptr == StatsAndAchievementsHandler)
        return;

    StatsAndAchievementsHandler->ResetStats();
}

#ifdef VWSCREENSHOT
class SteamScreenshotHandler
{
private:
    STEAM_CALLBACK(SteamScreenshotHandler, screenshotRequested, ScreenshotRequested_t);
    STEAM_CALLBACK(SteamScreenshotHandler, screenshotReady, ScreenshotReady_t);
};

static SteamScreenshotHandler * ScreenHandler;
static VW_VOID Callback_ScreenshotRequested;
// static VW_VOID_INT32 Callback_ScreenshotReady;

VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotRequested(VW_VOID function)
{
    Callback_ScreenshotRequested = function;
}

#if 0
VOIDWRAP_API void Voidwrap_Steam_SetCallback_ScreenshotReady(VW_VOID_INT32 function)
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


VOIDWRAP_API bool Voidwrap_Steam_Init()
{
    if (!SteamAPI_Init())
    {
        SteamAPI_Shutdown();
        return false;
    }

    SteamUtils()->SetWarningMessageHook(&SteamAPIWarningMessageHook);
    SteamUtils()->SetOverlayNotificationPosition(k_EPositionTopRight);
    AppID = SteamUtils()->GetAppID();

#if 0
    PrintDebug("AppID is %llu", AppID);
    if (SteamUtils()->IsOverlayEnabled()) { PrintDebug("Overlay is enabled."); }
#endif

    StatsAndAchievementsHandler = new SteamStatsAndAchievementsHandler{};

#ifdef VWSCREENSHOT
    SteamScreenshots()->HookScreenshots(true);
    if (SteamScreenshots()->IsScreenshotsHooked()) { PrintDebug("Screenshots hooked."); }
    ScreenHandler = new SteamScreenshotHandler();
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
    StatsAndAchievementsHandler->Process();

    SteamAPI_RunCallbacks();
}

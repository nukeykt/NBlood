// Windows layer-independent code
// (c) EDuke32 developers and contributors. All rights reserved. ;)

#include "winbits.h"

#include "baselayer.h"
#include "build.h"
#include "cache1d.h"
#include "compat.h"
#include "editor.h"
#include "osd.h"
#include "renderlayer.h"

#include <avrt.h>
#include <mmsystem.h>
#include <winnls.h>
#include <winternl.h>
#include <winnt.h>

#include <system_error>

#ifdef BITNESS64
# define EBACKTRACEDLL "ebacktrace1-64.dll"
#else
# define EBACKTRACEDLL "ebacktrace1.dll"
#endif

int32_t    win_boostpriority = 2;
char       win_silentvideomodeswitch;
static int win_silentfocuschange;

static HANDLE  g_singleInstanceSemaphore = nullptr;
static int32_t win_togglecomposition;
static int32_t win_systemtimermode = 1;
static int32_t win_performancemode;

static OSVERSIONINFOEX osv;
#if 0
static FARPROC ntdll_wine_get_version;
#endif
static char const *enUSLayoutString = "00000409";

DWM_TIMING_INFO timingInfo;

static HMODULE hPOWRPROF;
static GUID *systemPowerSchemeGUID;

typedef DWORD(WINAPI *PFNPOWERGETACTIVESCHEME)(HKEY, GUID **);
typedef DWORD(WINAPI *PFNPOWERSETACTIVESCHEME)(HKEY, CONST GUID *);

static PFNPOWERGETACTIVESCHEME powrprof_PowerGetActiveScheme;
static PFNPOWERSETACTIVESCHEME powrprof_PowerSetActiveScheme;

static HMODULE hAVRT;
static HANDLE hMMTHREAD;
static HMODULE hNTDLL;

typedef HANDLE(WINAPI *PFNAVSETMMTHREADCHARACTERISTICS)(LPCSTR, LPDWORD);
typedef BOOL(WINAPI* PFNAVREVERTMMTHREADCHARACTERISTICS)(HANDLE);
typedef BOOL(WINAPI* PFNAVSETMMTHREADPRIORITY)(HANDLE, AVRT_PRIORITY);

static PFNAVSETMMTHREADCHARACTERISTICS    avrt_AvSetMmThreadCharacteristics;
static PFNAVREVERTMMTHREADCHARACTERISTICS avrt_AvRevertMmThreadCharacteristics;
static PFNAVSETMMTHREADPRIORITY           avrt_AvSetMmThreadPriority;

typedef NTSTATUS(NTAPI* PFNSETTIMERRESOLUTION)(ULONG, BOOLEAN, PULONG);
typedef NTSTATUS(NTAPI* PFNQUERYTIMERRESOLUTION)(PULONG, PULONG, PULONG);

static PFNQUERYTIMERRESOLUTION ntdll_NtQueryTimerResolution;
static PFNSETTIMERRESOLUTION   ntdll_NtSetTimerResolution;

// convert Windows' NTSTATUS codes to regular Win32 error codes so we can print error strings if any of our bullshit undocumented API usage fails
DWORD windowsConvertNTSTATUS(LONG ntstatus)
{
    DWORD oldError = GetLastError();
    OVERLAPPED o = {};
    o.Internal = ntstatus;

    DWORD br;
    GetOverlappedResult(NULL, &o, &br, FALSE);

    DWORD result = GetLastError();
    SetLastError(oldError);

    return result;
}

void windowsSetupTimer(bool const appHasFocus)
{
#if 0
    if (ntdll_wine_get_version)
        return;
#endif

    TIMECAPS timeCaps;
    MMRESULT result = timeGetDevCaps(&timeCaps, sizeof(TIMECAPS));

    if (result != MMSYSERR_NOERROR)
        LOG_F(ERROR, "timeGetDevCaps() failed: MMRESULT: %d", result);

    static UINT timePeriod;

    if (timePeriod && (result = timeEndPeriod(timePeriod)) != TIMERR_NOERROR)
        LOG_F(ERROR, "timeEndPeriod(%d) failed: MMRESULT: %d", timePeriod, result);

    timePeriod = 0;

    NTSTATUS status;
    static ULONG timePeriodNT;

    if (timePeriodNT && !NT_SUCCESS(status = ntdll_NtSetTimerResolution(timePeriodNT, FALSE, &timePeriodNT)))
    {
        LOG_F(ERROR, "NtSetTimerResolution(%ld) unset failed: NTSTATUS: 0x%08x (%s)", timePeriodNT, (unsigned)status, windowsGetErrorMessage(windowsConvertNTSTATUS(status)));
        win_systemtimermode = 0;
    }

    timePeriodNT = 0;

    if (!appHasFocus)
    {
        DLOG_IF_F(INFO, !win_silentfocuschange, "System timer resolution requests cleared");
        return;
    }

#if defined RENDERTYPESDL && SDL_MAJOR_VERSION >= 2
    int const onBattery = (SDL_GetPowerInfo(NULL, NULL) == SDL_POWERSTATE_ON_BATTERY);
#else
    static constexpr int const onBattery = 0;
#endif

    if (!onBattery && hNTDLL && win_systemtimermode == 1)
    {
        ULONG minRes, maxRes, actualRes;

        if (NT_SUCCESS(status = ntdll_NtQueryTimerResolution(&minRes, &maxRes, &actualRes)))
        {
            if (NT_SUCCESS(status = ntdll_NtSetTimerResolution(maxRes, TRUE, &actualRes)))
            {
                timePeriodNT = actualRes;
                LOG_IF_F(INFO, !win_silentfocuschange, "Set %.1fms timer resolution", actualRes * (1.0 / 10000.0));
                return;
            }
            else
            {
                LOG_F(ERROR, "NtSetTimerResolution(%ld) failed: NTSTATUS: 0x%08x (%s)", maxRes, (unsigned)status, windowsGetErrorMessage(windowsConvertNTSTATUS(status)));
                win_systemtimermode = 0;
            }
        }
        else
        {
            LOG_F(ERROR, "NtQueryTimerResolution() failed: NTSTATUS: 0x%08x (%s)", (unsigned)status, windowsGetErrorMessage(windowsConvertNTSTATUS(status)));
            win_systemtimermode = 0;
        }
    }

    if (!timePeriodNT)
    {
        int const newPeriod = clamp(1u << onBattery, timeCaps.wPeriodMin, timeCaps.wPeriodMax);

        if ((result = timeBeginPeriod(newPeriod)) != TIMERR_NOERROR)
            LOG_F(ERROR, "timeBeginPeriod(%d) failed: MMRESULT: %d", newPeriod, result);
        else
        {
            timePeriod = newPeriod;
            LOG_IF_F(INFO, !win_silentfocuschange, "Set %u.0ms timer resolution", newPeriod);
            return;
        }
    }

    LOG_F(ERROR, "Unable to configure system timer!");
}

//
// CheckWinVersion() -- check to see what version of Windows we happen to be running under
//
BOOL windowsGetVersion(void)
{
#if 0
    HMODULE hNTDLL = GetModuleHandle("ntdll.dll");

    if (hNTDLL)
        ntdll_wine_get_version = GetProcAddress(hNTDLL, "wine_get_version");
#endif

    ZeroMemory(&osv, sizeof(osv));
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (GetVersionEx((LPOSVERSIONINFOA)&osv)) return TRUE;

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx((LPOSVERSIONINFOA)&osv)) return TRUE;

    return FALSE;
}

static void windowsPrintVersion(void)
{
    const char *ver = "";

    switch (osv.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_WINDOWS:
            if (osv.dwMinorVersion < 10)
                ver = "95";
            else if (osv.dwMinorVersion < 90)
                ver = "98";
            else
                ver = "ME";
            break;

        case VER_PLATFORM_WIN32_NT:
            switch (osv.dwMajorVersion)
            {
                case 5:
                    switch (osv.dwMinorVersion)
                    {
                        case 0: ver = "2000"; break;
                        case 1: ver = "XP"; break;
                        case 2: ver = osv.wProductType == VER_NT_WORKSTATION ? "XP x64" : "Server 2003"; break;
                    }
                    break;

                case 6:
                    {
                        static const char *client[] = { "Vista", "7", "8", "8.1" };
                        static const char *server[] = { "Server 2008", "Server 2008 R2", "Server 2012", "Server 2012 R2" };
                        ver = ((osv.wProductType == VER_NT_WORKSTATION) ? client : server)[osv.dwMinorVersion % ARRAY_SIZE(client)];
                    }
                    break;

                case 10:
                    switch (osv.wProductType)
                    {
                        case VER_NT_WORKSTATION: ver = "10"; break;
                        default: ver = "Server"; break;
                    }
                    break;
            }
            break;
    }

    char *buf = (char *)Xcalloc(1, 256);
    int len;

#if 0
    if (ntdll_wine_get_version)
        len = Bsprintf(buf, "Wine %s, identifying as Windows %s", (char *)ntdll_wine_get_version(), ver);
    else
#endif
    {
        len = Bsprintf(buf, "Windows %s", ver);

        if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT || osv.dwMajorVersion < 6)
        {
            Bstrcat(buf, " (UNSUPPORTED)");
            len = Bstrlen(buf);
        }
    }

    // service packs
    if (osv.szCSDVersion[0])
    {
        buf[len] = 32;
        Bstrcat(&buf[len], osv.szCSDVersion);
    }

    LOG_F(INFO, "OS: %s (build %lu.%lu.%lu)", buf, osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber);
    Xfree(buf);
}

//
// win_checkinstance() -- looks for another instance of a Build app
//
int windowsCheckAlreadyRunning(void)
{
    if (!g_singleInstanceSemaphore) return 1;
    return (WaitForSingleObject(g_singleInstanceSemaphore,0) != WAIT_TIMEOUT);
}


typedef void (*dllSetString)(const char*);

//
// win_open(), win_init(), win_setvideomode(), win_close() -- shared code
//
int windowsPreInit(void)
{
    if (!windowsGetVersion())
    {
        windowsShowError("This version of Windows is not supported.");
        return -1;
    }

    windowsGetSystemKeyboardLayout();
    windowsGetSystemKeyboardLayoutName();

#ifdef DEBUGGINGAIDS
    HMODULE ebacktrace = LoadLibraryA(EBACKTRACEDLL);
    if (ebacktrace)
    {
        dllSetString SetTechnicalName = (dllSetString)(void(*))GetProcAddress(ebacktrace, "SetTechnicalName");
        dllSetString SetProperName = (dllSetString)(void(*))GetProcAddress(ebacktrace, "SetProperName");

        if (SetTechnicalName)
            SetTechnicalName(AppTechnicalName);

        if (SetProperName)
            SetProperName(AppProperName);
    }
#endif

    g_singleInstanceSemaphore = CreateSemaphore(NULL, 1,1, WindowClass);

    return 0;
}

static int osdcmd_win_systemtimermode(osdcmdptr_t parm)
{
    int const r = osdcmd_cvar_set(parm);

    if (r != OSDCMD_OK)
        return r;

    windowsSetupTimer(true);

    return OSDCMD_OK;
}

void windowsPlatformInit(void)
{
    static osdcvardata_t cvars_win[] = {
        { "win_togglecomposition", "disables Windows Vista/7 DWM composition", (void *)&win_togglecomposition, CVAR_BOOL, 0, 1 },

        { "win_boostpriority",
          "Windows process priority class:\n"
          "   0: do not alter process priority\n"
          "   1: ABOVE NORMAL when game has focus, IDLE when interacting with other programs"
          "   2: HIGH when game has focus, BELOW NORMAL when interacting with other programs\n",
          (void *)&win_boostpriority, CVAR_INT, 0, 2 },

        { "win_performancemode",
          "Windows performance mode:\n"
          "   0: do not alter performance mode\n"
          "   1: use HIGH PERFORMANCE power plan when game has focus",
          (void *)&win_performancemode, CVAR_BOOL, 0, 1 },

    };

    static osdcvardata_t win_timer_cvar = { "win_systemtimermode",
                                            "Windows timer interrupt resolution:\n"
                                            "   0: 1.0ms\n"
                                            "   1: 0.5ms low-latency"
#if defined RENDERTYPESDL && SDL_MAJOR_VERSION >= 2
                                            "\nThis option has no effect when running on battery power.",
#else
                                            ,
#endif
                                            (void *)&win_systemtimermode, CVAR_BOOL, 0, 1 };

    OSD_RegisterCvar(&win_timer_cvar, osdcmd_win_systemtimermode);

    for (int i=0; i<ARRAY_SSIZE(cvars_win); i++)
        OSD_RegisterCvar(&cvars_win[i], osdcmd_cvar_set);

    windowsPrintVersion();
    //windowsSetupTimer(0);

    if (osv.dwMajorVersion >= 6)
    {
        if (!hPOWRPROF && (hPOWRPROF = GetModuleHandle("powrprof.dll")))
        {
            powrprof_PowerGetActiveScheme = (PFNPOWERGETACTIVESCHEME)(void(*))GetProcAddress(hPOWRPROF, "PowerGetActiveScheme");
            powrprof_PowerSetActiveScheme = (PFNPOWERSETACTIVESCHEME)(void(*))GetProcAddress(hPOWRPROF, "PowerSetActiveScheme");

            if (powrprof_PowerGetActiveScheme == nullptr || powrprof_PowerSetActiveScheme == nullptr)
            {
                LOG_F(ERROR, "PowerGetActiveScheme or PowerSetActiveScheme symbols missing from powrprof.dll!");
                hPOWRPROF = NULL;
            }
            else if (!systemPowerSchemeGUID)
                powrprof_PowerGetActiveScheme(NULL, &systemPowerSchemeGUID);
        }

        if (!hAVRT && (hAVRT = LoadLibrary("avrt.dll")))
        {
            avrt_AvSetMmThreadCharacteristics    = (PFNAVSETMMTHREADCHARACTERISTICS)   (void(*))GetProcAddress(hAVRT, "AvSetMmThreadCharacteristicsA");
            avrt_AvSetMmThreadPriority           = (PFNAVSETMMTHREADPRIORITY)          (void(*))GetProcAddress(hAVRT, "AvSetMmThreadPriority");
            avrt_AvRevertMmThreadCharacteristics = (PFNAVREVERTMMTHREADCHARACTERISTICS)(void(*))GetProcAddress(hAVRT, "AvRevertMmThreadCharacteristics");

            if (avrt_AvSetMmThreadCharacteristics == nullptr || avrt_AvSetMmThreadPriority == nullptr || avrt_AvRevertMmThreadCharacteristics == nullptr)
            {
                LOG_F(ERROR, "AvSetMmThreadCharacteristicsA, AvSetMmThreadPriority or AvRevertMmThreadCharacteristics symbols missing from avrt.dll!");
                hAVRT = NULL;
            }
        }
    }

    if (!hNTDLL && (hNTDLL = GetModuleHandle("ntdll.dll")))
    {
        ntdll_NtQueryTimerResolution = (PFNQUERYTIMERRESOLUTION) (void(*))GetProcAddress(hNTDLL, "NtQueryTimerResolution");
        ntdll_NtSetTimerResolution   = (PFNSETTIMERRESOLUTION)   (void(*))GetProcAddress(hNTDLL, "NtSetTimerResolution");

        if (ntdll_NtQueryTimerResolution == nullptr || ntdll_NtSetTimerResolution == nullptr)
        {
            LOG_F(ERROR, "NtQueryTimerResolution or NtSetTimerResolution symbols missing from ntdll.dll!");
            hNTDLL = NULL;
            win_systemtimermode = 0;
        }
    }
}

typedef UINT D3DKMT_HANDLE;
typedef UINT D3DDDI_VIDEO_PRESENT_SOURCE_ID;

typedef struct _D3DKMT_OPENADAPTERFROMHDC
{
    HDC           hDc;
    D3DKMT_HANDLE hAdapter;
    LUID          AdapterLuid;

    D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId;
} D3DKMT_OPENADAPTERFROMHDC;

typedef struct _D3DKMT_CLOSEADAPTER
{
    D3DKMT_HANDLE hAdapter;
} D3DKMT_CLOSEADAPTER;

typedef struct _D3DKMT_WAITFORVERTICALBLANKEVENT
{
    D3DKMT_HANDLE hAdapter;
    D3DKMT_HANDLE hDevice;

    D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId;
} D3DKMT_WAITFORVERTICALBLANKEVENT;

typedef NTSTATUS(APIENTRY *PFND3DKMTOPENADAPTERFROMHDC)(D3DKMT_OPENADAPTERFROMHDC *);
typedef NTSTATUS(APIENTRY *PFND3DKMTCLOSEADAPTER)(D3DKMT_CLOSEADAPTER *);
typedef NTSTATUS(APIENTRY *PFND3DKMTWAITFORVERTICALBLANKEVENT)(D3DKMT_WAITFORVERTICALBLANKEVENT *);

typedef HRESULT(WINAPI *PFNDWMENABLECOMPOSITION)(UINT);
typedef HRESULT(WINAPI *PFNDWMGETCOMPOSITIONTIMINGINFO)(HWND, DWM_TIMING_INFO *);
typedef HRESULT(WINAPI *PFNDWMISCOMPOSITIONENABLED)(BOOL *);
typedef HRESULT(WINAPI *PFNDWMFLUSH)(void);

static HMODULE hDWMApi;
static PFNDWMFLUSH dwmapi_DwmFlush;
static PFNDWMISCOMPOSITIONENABLED dwmapi_DwmIsCompositionEnabled;

void windowsWaitForVBlank(void)
{
    // if we don't have these, we aren't going to have the WDDM functions below either, so bailing here is fine.
    if (osv.dwMajorVersion < 6 || !dwmapi_DwmFlush || !dwmapi_DwmIsCompositionEnabled)
        return;

    static int useDWMsync;

    // here comes the voodoo bullshit ;)
    static HMODULE hGDI32;
    static PFND3DKMTOPENADAPTERFROMHDC        gdi32_D3DKMTOpenAdapterFromHdc;
    static PFND3DKMTCLOSEADAPTER              gdi32_D3DKMTCloseAdapter;
    static PFND3DKMTWAITFORVERTICALBLANKEVENT gdi32_D3DKMTWaitForVBlank;

    if (!hGDI32 && (hGDI32 = GetModuleHandle("gdi32.dll")))
    {
        gdi32_D3DKMTOpenAdapterFromHdc = (PFND3DKMTOPENADAPTERFROMHDC)        (void(*))GetProcAddress(hGDI32, "D3DKMTOpenAdapterFromHdc");
        gdi32_D3DKMTCloseAdapter       = (PFND3DKMTCLOSEADAPTER)              (void(*))GetProcAddress(hGDI32, "D3DKMTCloseAdapter");
        gdi32_D3DKMTWaitForVBlank      = (PFND3DKMTWAITFORVERTICALBLANKEVENT) (void(*))GetProcAddress(hGDI32, "D3DKMTWaitForVerticalBlankEvent");
    }

    if (useDWMsync || !fullscreen || !gdi32_D3DKMTOpenAdapterFromHdc || !gdi32_D3DKMTCloseAdapter || !gdi32_D3DKMTWaitForVBlank)
    {
dwm:
        // if we don't have the better APIs but composition is enabled, this is sometimes good enough
        BOOL compositionEnabled = false;

        if (SUCCEEDED(dwmapi_DwmIsCompositionEnabled(&compositionEnabled)) && compositionEnabled && dwmapi_DwmFlush() != S_OK)
            LOG_F(ERROR, "Failed flushing DWM!");

        return;
    }

    MONITORINFOEX mi = {};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(MonitorFromWindow(win_gethwnd(), MONITOR_DEFAULTTONULL), &mi);

    D3DKMT_OPENADAPTERFROMHDC activeAdapter = {};
    activeAdapter.hDc = CreateDC(mi.szDevice, mi.szDevice, nullptr, nullptr);

    if (activeAdapter.hDc == nullptr)
    {
        LOG_F(ERROR, "null device context for display: %s windowx: %d windowy: %d.", mi.szDevice, g_windowPos.x, g_windowPos.y);
        useDWMsync = 1;
        goto dwm;
    }

    auto status = gdi32_D3DKMTOpenAdapterFromHdc(&activeAdapter);
    DeleteDC(activeAdapter.hDc);

    if (NT_SUCCESS(status))
    {
        D3DKMT_WAITFORVERTICALBLANKEVENT vBlankEvent = { activeAdapter.hAdapter, 0, activeAdapter.VidPnSourceId };

        if (NT_SUCCESS(status = gdi32_D3DKMTWaitForVBlank(&vBlankEvent)))
        {
            // the D3DKMT_CLOSEADAPTER struct only contains one member, and it's
            // the same as the first member in D3DKMT_WAITFORVERTICALBLANKEVENT
            if (NT_SUCCESS(status = gdi32_D3DKMTCloseAdapter((D3DKMT_CLOSEADAPTER *)&vBlankEvent)))
                return;
            else
                LOG_F(ERROR, "D3DKMTCloseAdapter() failed: NTSTATUS: 0x%08x (%s)", (unsigned)status, windowsGetErrorMessage(windowsConvertNTSTATUS(status)));
        }
        else
            LOG_F(ERROR, "D3DKMTWaitForVerticalBlankEvent() failed: NTSTATUS: 0x%08x (%s)", (unsigned)status, windowsGetErrorMessage(windowsConvertNTSTATUS(status)));
    }
    else
        LOG_F(ERROR, "D3DKMTOpenAdapterFromHdc() failed: NTSTATUS: 0x%08x (%s)", (unsigned)status, windowsGetErrorMessage(windowsConvertNTSTATUS(status)));

    LOG_F(ERROR, "Unable to use D3DKMT, falling back to DWM sync.");

    useDWMsync = 1;
    goto dwm;
}

void windowsDwmSetupComposition(int const compEnable)
{
    if (osv.dwMajorVersion < 6)
        return;

    static PFNDWMENABLECOMPOSITION        dwmapi_DwmEnableComposition;
    static PFNDWMGETCOMPOSITIONTIMINGINFO dwmapi_DwmGetCompositionTimingInfo;

    if (!hDWMApi && (hDWMApi = GetModuleHandle("dwmapi.dll")))
    {
        dwmapi_DwmEnableComposition        = (PFNDWMENABLECOMPOSITION)        (void(*))GetProcAddress(hDWMApi, "DwmEnableComposition");
        dwmapi_DwmFlush                    = (PFNDWMFLUSH)                    (void(*))GetProcAddress(hDWMApi, "DwmFlush");
        dwmapi_DwmGetCompositionTimingInfo = (PFNDWMGETCOMPOSITIONTIMINGINFO) (void(*))GetProcAddress(hDWMApi, "DwmGetCompositionTimingInfo");
        dwmapi_DwmIsCompositionEnabled     = (PFNDWMISCOMPOSITIONENABLED)     (void(*))GetProcAddress(hDWMApi, "DwmIsCompositionEnabled");
    }

    if (dwmapi_DwmGetCompositionTimingInfo)
    {
        timingInfo = {};
        timingInfo.cbSize = sizeof(DWM_TIMING_INFO);

        // the HWND parameter was deprecated in Windows 8.1 because DWM always syncs to the primary monitor's refresh...

        HRESULT result = dwmapi_DwmGetCompositionTimingInfo(nullptr, &timingInfo);

        if (FAILED(result))
            LOG_F(ERROR, "DwmGetCompositionTimingInfo() failed: HRESULT: 0x%08x (%s).",
                         (unsigned)result, std::system_category().message(result).c_str());
    }

    if (win_togglecomposition && dwmapi_DwmEnableComposition && osv.dwMinorVersion < 2)
    {
        dwmapi_DwmEnableComposition(compEnable);

        if (!win_silentvideomodeswitch)
            VLOG_F(LOG_GFX, "%sabling DWM desktop composition", (compEnable) ? "En" : "Dis");
    }
}

void windowsPlatformCleanup(void)
{
    if (hAVRT)
    {
        if (hMMTHREAD)
        {
            avrt_AvRevertMmThreadCharacteristics(hMMTHREAD);
            hMMTHREAD = NULL;
        }
        FreeLibrary(hAVRT);
        hAVRT = NULL;
    }

    if (g_singleInstanceSemaphore)
        CloseHandle(g_singleInstanceSemaphore);

    windowsSetKeyboardLayout(windowsGetSystemKeyboardLayoutName());

    if (systemPowerSchemeGUID)
    {
        powrprof_PowerSetActiveScheme(NULL, systemPowerSchemeGUID);
        LocalFree(systemPowerSchemeGUID);
    }
}


//
// GetWindowsErrorMsg() -- gives a pointer to a static buffer containing the Windows error message
//
LPTSTR windowsGetErrorMessage(DWORD code)
{
    static TCHAR lpMsgBuf[1024];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)lpMsgBuf, 1024, NULL);

    return lpMsgBuf;
}


// Keyboard layout switching

static char const * windowsDecodeKeyboardLayoutName(char const * keyboardLayout)
{
    int const   localeID = Bstrtol(keyboardLayout, NULL, 16);
    static char localeName[16];

    int const result = GetLocaleInfo(MAKELCID(localeID, SORT_DEFAULT), LOCALE_SNAME, localeName, ARRAY_SIZE(localeName));

    if (!result)
    {
        LOG_F(ERROR, "Unable to decode name for locale ID '%d': %s.", localeID, windowsGetErrorMessage(GetLastError()));
        return keyboardLayout;
    }

    return localeName;
}

void windowsSetKeyboardLayout(char const *layout, int focusChanged /*= 0*/)
{
    char layoutName[KL_NAMELENGTH];

    GetKeyboardLayoutName(layoutName);

    if (!Bstrcmp(layoutName, layout))
        return;

    //if (!win_silentfocuschange)
    {
        if (focusChanged)
            VLOG_F(LOG_GFX, "Window focus changed");

        if (layout == enUSLayoutString)
            VLOG_F(LOG_INPUT, "Loaded %s keyboard layout", windowsDecodeKeyboardLayoutName(layout));
        else
            VLOG_F(LOG_INPUT, "Restored %s keyboard layout", windowsDecodeKeyboardLayoutName(layout));
    }

    static int enUSLoaded;
    static HKL enUSLayout;

    if (layout == enUSLayoutString)
    {
        if (enUSLoaded)
            ActivateKeyboardLayout(enUSLayout, KLF_SETFORPROCESS);
        else if ((enUSLayout = LoadKeyboardLayout(enUSLayoutString, KLF_ACTIVATE | KLF_SETFORPROCESS | KLF_SUBSTITUTE_OK)))
            enUSLoaded = true;
    }
    else
        ActivateKeyboardLayout(windowsGetSystemKeyboardLayout(), KLF_SETFORPROCESS);
}


char *windowsGetSystemKeyboardLayoutName(void)
{
    static char systemLayoutName[KL_NAMELENGTH];
    static int layoutSaved;

    if (!layoutSaved)
    {
        if (!GetKeyboardLayoutName(systemLayoutName))
            LOG_F(ERROR, "Error determining system keyboard layout: %s.", windowsGetErrorMessage(GetLastError()));

        layoutSaved = true;
    }

    return systemLayoutName;
}

HKL windowsGetSystemKeyboardLayout(void)
{
    static HKL systemLayout;
    static int layoutSaved;

    if (!layoutSaved)
    {
        systemLayout = GetKeyboardLayout(0);
        layoutSaved  = true;
    }

    return systemLayout;
}

void windowsHandleFocusChange(int const appactive)
{
    static HANDLE hProcess = GetCurrentProcess();
    static DWORD index = 0;

#ifndef DEBUGGINGAIDS
    win_silentfocuschange = true;
#endif

    if (appactive)
    {
        if (win_boostpriority)
        {
            SetPriorityClass(hProcess, win_boostpriority == 2 ? HIGH_PRIORITY_CLASS : ABOVE_NORMAL_PRIORITY_CLASS);

            if (!editorIsInitialized())
            {
                if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
                    LOG_F(ERROR, "Error setting thread priority: %s.", windowsGetErrorMessage(GetLastError()));

                if (hAVRT)
                {
                    if (hMMTHREAD || (hMMTHREAD = avrt_AvSetMmThreadCharacteristics(TEXT("Games"), &index)))
                    {
                        DLOG_IF_F(INFO, !win_silentfocuschange, "Successfully set AVRT thread characteristics with index %d.", (int)index);

                        if (!avrt_AvSetMmThreadPriority(hMMTHREAD, AVRT_PRIORITY_CRITICAL))
                        {
                            auto err = GetLastError();
                            LOG_F(ERROR, "Error setting AVRT thread priority: error %ld: %s.", err, windowsGetErrorMessage(err));
                        }
                    }
                    else
                    {
                        auto err = GetLastError();
                        LOG_F(ERROR, "Error setting AVRT thread characteristics with index %ld: error %ld: %s.", index, err, windowsGetErrorMessage(err));
                    }
                }
            }
        }

        windowsSetupTimer(true);
        windowsSetKeyboardLayout(enUSLayoutString, true);

        if (win_performancemode && systemPowerSchemeGUID)
            powrprof_PowerSetActiveScheme(NULL, &GUID_MIN_POWER_SAVINGS);
    }
    else
    {
        if (win_boostpriority)
        {
            if (!SetPriorityClass(hProcess, (win_boostpriority == 2 || !editorIsInitialized()) ? BELOW_NORMAL_PRIORITY_CLASS: IDLE_PRIORITY_CLASS))
                LOG_F(ERROR, "Error setting thread priority: %s.", windowsGetErrorMessage(GetLastError()));

            if (hAVRT && hMMTHREAD)
            {
                if (!avrt_AvRevertMmThreadCharacteristics(hMMTHREAD))
                {
                    auto err = GetLastError();
                    LOG_F(ERROR, "Error reverting AVRT thread characteristics: error %ld: %s.", err, windowsGetErrorMessage(err));
                }
                else
                {
                    DLOG_IF_F(INFO, !win_silentfocuschange, "Successfully reverted AVRT thread characteristics.");

                    index = 0;
                    hMMTHREAD = NULL;
                }
            }
        }

        windowsSetupTimer(false);
        windowsSetKeyboardLayout(windowsGetSystemKeyboardLayoutName(), true);

        if (win_performancemode && systemPowerSchemeGUID)
            powrprof_PowerSetActiveScheme(NULL, systemPowerSchemeGUID);
    }

    win_silentfocuschange = false;
}

//
// ShowErrorBox() -- shows an error message box
//
void windowsShowError(const char *m)
{
    TCHAR msg[1024];

    wsprintf(msg, "%s: %s", m, windowsGetErrorMessage(GetLastError()));
    MessageBox(0, msg, apptitle, MB_OK|MB_ICONSTOP);
}


//
// Miscellaneous
//
int windowsGetCommandLine(char **argvbuf)
{
    int buildargc = 0;

    *argvbuf = Bstrdup(GetCommandLine());

    if (*argvbuf)
    {
        char quoted = 0, instring = 0, swallownext = 0;
        char *wp;
        for (const char *p = wp = *argvbuf; *p; p++)
        {
            if (*p == ' ')
            {
                if (instring)
                {
                    if (!quoted)
                    {
                        // end of a string
                        *(wp++) = 0;
                        instring = 0;
                    }
                    else
                        *(wp++) = *p;
                }
            }
            else if (*p == '"' && !swallownext)
            {
                if (instring)
                {
                    if (quoted && p[1] == ' ')
                    {
                        // end of a string
                        *(wp++) = 0;
                        instring = 0;
                    }
                    quoted = !quoted;
                }
                else
                {
                    instring = 1;
                    quoted = 1;
                    buildargc++;
                }
            }
            else if (*p == '\\' && p[1] == '"' && !swallownext)
                swallownext = 1;
            else
            {
                if (!instring)
                    buildargc++;

                instring = 1;
                *(wp++) = *p;
                swallownext = 0;
            }
        }
        *wp = 0;
    }

    return buildargc;
}

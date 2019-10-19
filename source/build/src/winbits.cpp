// Windows layer-independent code

#include "winbits.h"

#include "baselayer.h"
#include "build.h"
#include "cache1d.h"
#include "compat.h"
#include "osd.h"
#include "windows_inc.h"

#include <mmsystem.h>
#include <winnls.h>

#ifdef BITNESS64
# define EBACKTRACEDLL "ebacktrace1-64.dll"
#else
# define EBACKTRACEDLL "ebacktrace1.dll"
#endif

int32_t    win_priorityclass;
char       win_silentvideomodeswitch;
static int win_silentfocuschange;

static HANDLE  g_singleInstanceSemaphore = nullptr;
static int32_t win_togglecomposition;
static int32_t win_systemtimermode;

static OSVERSIONINFOEX osv;

FARPROC pwinever;

void windowsSetupTimer(int ntDllVoodoo)
{
    typedef HRESULT(NTAPI* pSetTimerResolution)(ULONG, BOOLEAN, PULONG);
    typedef HRESULT(NTAPI* pQueryTimerResolution)(PULONG, PULONG, PULONG);

    TIMECAPS timeCaps;

    if (pwinever)
        return;

    if (timeGetDevCaps(&timeCaps, sizeof(TIMECAPS)) == MMSYSERR_NOERROR)
    {
#ifdef RENDERTYPESDL
        int const onBattery = (SDL_GetPowerInfo(NULL, NULL) == SDL_POWERSTATE_ON_BATTERY);
#else
        static constexpr int const onBattery = 0;
#endif
        static int   setPeriod;
        static ULONG setTimerNT;
        HMODULE      ntDllHandle = GetModuleHandle("ntdll.dll");

        static pQueryTimerResolution NtQueryTimerResolution;
        static pSetTimerResolution   NtSetTimerResolution;

        if (ntDllVoodoo)
        {
            if (!onBattery)
            {
                if (ntDllHandle != nullptr)
                {
                    NtQueryTimerResolution = (pQueryTimerResolution)(void(*))GetProcAddress(ntDllHandle, "NtQueryTimerResolution");
                    NtSetTimerResolution   = (pSetTimerResolution)(void(*))GetProcAddress(ntDllHandle, "NtSetTimerResolution");

                    if (NtQueryTimerResolution == nullptr || NtSetTimerResolution == nullptr)
                    {
                        OSD_Printf("ERROR: unable to locate NtQueryTimerResolution or NtSetTimerResolution symbols in ntdll.dll!\n");
                        goto failsafe;
                    }

                    ULONG minRes, maxRes, actualRes;

                    NtQueryTimerResolution(&minRes, &maxRes, &actualRes);

                    if (setTimerNT != 0)
                    {
                        if (setTimerNT == actualRes)
                            return;

                        NtSetTimerResolution(actualRes, FALSE, &actualRes);
                    }

                    NtSetTimerResolution(maxRes, TRUE, &actualRes);

                    setTimerNT = actualRes;
                    setPeriod  = 0;

                    if (!win_silentfocuschange)
                        OSD_Printf("Low-latency system timer enabled: set %.1fms timer resolution\n", actualRes / 10000.0);

                    return;
                }
                else
                    OSD_Printf("ERROR: couldn't load ntdll.dll!\n");
            }
            else if (!win_silentfocuschange)
                OSD_Printf("Low-latency timer mode not supported on battery power!\n");
        }
        else if (setTimerNT != 0)
        {
            NtSetTimerResolution(setTimerNT, FALSE, &setTimerNT);
            setTimerNT = 0;
        }

failsafe:
        int const requestedPeriod = min(max(timeCaps.wPeriodMin, 1u << onBattery), timeCaps.wPeriodMax);
            
        if (setPeriod != 0)
        {
            if (setPeriod == requestedPeriod)
                return;

            timeEndPeriod(requestedPeriod);
        }

        timeBeginPeriod(requestedPeriod);

        setPeriod  = requestedPeriod;
        setTimerNT = 0;

        if (!win_silentfocuschange)
            OSD_Printf("Initialized %ums system timer\n", requestedPeriod);

        return;
    }

    OSD_Printf("ERROR: unable to configure system timer!\n");
}

//
// CheckWinVersion() -- check to see what version of Windows we happen to be running under
//
BOOL windowsGetVersion(void)
{
    HMODULE hntdll = GetModuleHandle("ntdll.dll");

    if (hntdll)
        pwinever = GetProcAddress(hntdll, "wine_get_version");

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

    if (pwinever)
        len = Bsprintf(buf, "Wine %s, identifying as Windows %s", (char *)pwinever(), ver);
    else
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

    initprintf("Running on %s (build %lu.%lu.%lu)\n", buf, osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber);
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

#ifdef DEBUGGINGAIDS
    HMODULE ebacktrace = LoadLibraryA(EBACKTRACEDLL);
    if (ebacktrace)
    {
        dllSetString SetTechnicalName = (dllSetString) (void (*)(void))GetProcAddress(ebacktrace, "SetTechnicalName");
        dllSetString SetProperName = (dllSetString) (void (*)(void))GetProcAddress(ebacktrace, "SetProperName");

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

    windowsSetupTimer(win_systemtimermode);

    return OSDCMD_OK;
}

void windowsPlatformInit(void)
{
    static osdcvardata_t cvars_win[] = {
        { "win_togglecomposition", "disables Windows Vista/7 DWM composition", (void *)&win_togglecomposition, CVAR_BOOL, 0, 1 },

        { "win_priorityclass",
          "Windows process priority class:\n"
          "  -1: do not alter process priority\n"
          "   0: HIGH when game has focus, NORMAL when interacting with other programs\n"
          "   1: NORMAL when game has focus, IDLE when interacting with other programs\n",
          (void *)&win_priorityclass, CVAR_INT, -1, 1 },
    };

    static osdcvardata_t win_timer_cvar = { "win_systemtimermode",
                                            "Windows timer interrupt resolution:\n"
                                            "   0: 1.0ms\n"
                                            "   1: 0.5ms low-latency\n"
#ifdef RENDERTYPESDL
                                            "This option has no effect when running on battery power.\n",
#else
                                            ,
#endif
                                            (void *)&win_systemtimermode, CVAR_BOOL, 0, 1 };

    OSD_RegisterCvar(&win_timer_cvar, osdcmd_win_systemtimermode);

    for (int i=0; i<ARRAY_SSIZE(cvars_win); i++)
        OSD_RegisterCvar(&cvars_win[i], osdcmd_cvar_set);

    windowsPrintVersion();
    windowsSetupTimer(0);
}

void windowsDwmEnableComposition(int const compEnable)
{
    if (!win_togglecomposition || osv.dwMajorVersion != 6 || osv.dwMinorVersion >= 2)
        return;

    static HMODULE hDWMApiDLL = NULL;
    static HRESULT(WINAPI * aDwmEnableComposition)(UINT);

    if (!hDWMApiDLL && (hDWMApiDLL = LoadLibrary("DWMAPI.DLL")))
        aDwmEnableComposition = (HRESULT(WINAPI *)(UINT))(void (*)(void))GetProcAddress(hDWMApiDLL, "DwmEnableComposition");

    if (aDwmEnableComposition)
    {
        aDwmEnableComposition(compEnable);
        if (!win_silentvideomodeswitch)
            OSD_Printf("%sabling DWM desktop composition...\n", (compEnable) ? "En" : "Dis");
    }
}

void windowsPlatformCleanup(void)
{
    if (g_singleInstanceSemaphore)
        CloseHandle(g_singleInstanceSemaphore);

    windowsSetKeyboardLayout(windowsGetSystemKeyboardLayout());
}


//
// GetWindowsErrorMsg() -- gives a pointer to a static buffer containing the Windows error message
//
static LPTSTR windowsGetErrorMessage(DWORD code)
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
        OSD_Printf("Error decoding name for locale ID %d: %s\n", localeID, windowsGetErrorMessage(GetLastError()));
        return keyboardLayout;
    }

    return localeName;
}

void windowsSetKeyboardLayout(char const * keyboardLayout)
{
    char layoutName[KL_NAMELENGTH];

    GetKeyboardLayoutName(layoutName);

    if (!Bstrcmp(layoutName, keyboardLayout))
        return;

    if (!win_silentfocuschange)
    {
        if (!Bstrcmp(keyboardLayout, windowsGetSystemKeyboardLayout()))
            OSD_Printf("Restored %s keyboard layout\n", windowsDecodeKeyboardLayoutName(keyboardLayout));
        else
            OSD_Printf("Loaded %s keyboard layout\n", windowsDecodeKeyboardLayoutName(layoutName));
    }

    LoadKeyboardLayout(keyboardLayout, KLF_ACTIVATE|KLF_SETFORPROCESS|KLF_SUBSTITUTE_OK);
}


char *windowsGetSystemKeyboardLayout(void)
{
    static char systemLayoutName[KL_NAMELENGTH];
    static int layoutSaved;

    if (!layoutSaved)
    {
        if (!GetKeyboardLayoutName(systemLayoutName))
            OSD_Printf("Error determining system keyboard layout: %s\n", windowsGetErrorMessage(GetLastError()));

        layoutSaved = true;
    }

    return systemLayoutName;
}

void windowsHandleFocusChange(int const appactive)
{
#ifndef DEBUGGINGAIDS
    win_silentfocuschange = true;
#endif

    if (appactive)
    {
        if (win_priorityclass != -1)
            SetPriorityClass(GetCurrentProcess(), win_priorityclass ? NORMAL_PRIORITY_CLASS : HIGH_PRIORITY_CLASS);

        windowsSetupTimer(win_systemtimermode);
        windowsSetKeyboardLayout(EDUKE32_KEYBOARD_LAYOUT);
    }
    else
    {
        if (win_priorityclass != -1)
            SetPriorityClass(GetCurrentProcess(), win_priorityclass ? IDLE_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS);

        windowsSetupTimer(0);
        windowsSetKeyboardLayout(windowsGetSystemKeyboardLayout());
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

    *argvbuf = Xstrdup(GetCommandLine());

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


// Workaround for a bug in mingwrt-4.0.0 and up where a function named main() in misc/src/libcrt/gdtoa/qnan.c takes precedence over the proper one in src/libcrt/crt/main.c.
#if 0 && (defined __MINGW32__ && EDUKE32_GCC_PREREQ(4,8)) || EDUKE32_CLANG_PREREQ(3,4)
# undef main
# include "mingw_main.cpp"
#endif

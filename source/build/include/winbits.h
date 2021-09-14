// Windows layer-independent code

#ifndef winbits_h__
#define winbits_h__

#define NEED_DWMAPI_H
#define NEED_BCRYPT_H

#include "windows_inc.h"

#include "compat.h"

#ifdef APPNAME
# define WindowClass APPNAME
#else
# define WindowClass "buildapp"
#endif

#if !defined NDEBUG
# pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD  dwType;     /* must be 0x1000 */
    LPCSTR szName;     /* pointer to name (in user addr space) */
    DWORD  dwThreadID; /* thread ID (-1=caller thread) */
    DWORD  dwFlags;    /* reserved for future use, must be zero */
} THREADNAME_INFO;
# pragma pack(pop)

static inline void debugThreadName(char const *name)
{
    UNREFERENCED_PARAMETER(name);
    if (IsDebuggerPresent())
    {
        THREADNAME_INFO wtf = { 0x1000, name, (DWORD)-1, 0 };
        RaiseException(0x406D1388, 0, sizeof(wtf) / sizeof(ULONG_PTR), (const ULONG_PTR *)&wtf);
    }
}
#endif

// TODO: actually make the layout switching unnecessary :/

extern int32_t win_priorityclass;
extern char    win_silentvideomodeswitch;
extern DWM_TIMING_INFO timingInfo;

int32_t windowsCheckForUpdates(char *buffer);
int     windowsCheckAlreadyRunning(void);
void    windowsDwmSetupComposition(int compEnable);
int     windowsGetCommandLine(char **argvbuf);
LPTSTR  windowsGetErrorMessage(DWORD code);
HKL     windowsGetSystemKeyboardLayout(void);
char *  windowsGetSystemKeyboardLayoutName(void);
BOOL    windowsGetVersion(void);
void    windowsHandleFocusChange(int const appactive);
void    windowsShowError(const char *m);
void    windowsPlatformCleanup(void);
void    windowsPlatformInit(void);
int     windowsPreInit(void);
void    windowsSetupTimer(int useNtTimer);
void    windowsSetKeyboardLayout(char const *layout, int focusChanged = false);
void    windowsWaitForVBlank(void);
#endif // winbits_h__

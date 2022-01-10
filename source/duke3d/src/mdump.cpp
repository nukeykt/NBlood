
#include "compat.h"
#include "mdump.h"

#include <tchar.h>
LPCSTR MiniDumper::m_szAppName;

MiniDumper g_dumper("eduke32");

MiniDumper::MiniDumper(LPCSTR szAppName)
{
    // if this assert fires then you have two instances of MiniDumper
    // which is not allowed
    assert(m_szAppName == NULL);

    m_szAppName = szAppName ? _strdup(szAppName) : "Application";

    ::SetUnhandledExceptionFilter(TopLevelFilter);
}

LONG MiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
    HMODULE hDll = NULL;
    char    szDbgHelpPath[_MAX_PATH];

    if (GetModuleFileName(NULL, szDbgHelpPath, _MAX_PATH))
    {
        auto pSlash = _tcsrchr(szDbgHelpPath, '\\');

        if (pSlash)
        {
            _tcscpy(pSlash + 1, "DBGHELP.DLL");
            hDll = ::LoadLibrary(szDbgHelpPath);
        }
    }

    if (hDll == NULL)
    {
        // load any version we can
        hDll = ::LoadLibrary("DBGHELP.DLL");
    }

    LONG    retval   = EXCEPTION_CONTINUE_SEARCH;
    LPCTSTR szResult = "DBGHELP.DLL not found";
    char    szScratch[1024];

    if (hDll)
    {
        auto pDump = MINIDUMPWRITEDUMP(::GetProcAddress(hDll, "MiniDumpWriteDump"));

        if (pDump)
        {
            char szDumpPath[_MAX_PATH];

            snprintf(szDumpPath, sizeof(szDumpPath), "%s_%u.dmp", m_szAppName, timeGetTime());

            HANDLE hFile = ::CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                _MINIDUMP_EXCEPTION_INFORMATION ExInfo = { ::GetCurrentThreadId(), pExceptionInfo, NULL };

                // take a dump
                BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);

                if (bOK)
                {
                    snprintf(szScratch, sizeof(szScratch), "Crash dump written to %s", szDumpPath);
                    szResult = szScratch;
                    retval   = EXCEPTION_EXECUTE_HANDLER;
                }
                else
                {
                    snprintf(szScratch, sizeof(szScratch), "Unable to write crash dump to %s: error %d.", szDumpPath, GetLastError());
                    szResult = szScratch;
                }

                ::CloseHandle(hFile);
            }
            else
            {
                snprintf(szScratch, sizeof(szScratch), "Unable to write crash dump to %s: error %d", szDumpPath, GetLastError());
                szResult = szScratch;
            }
        }
        else
        {
            szResult = "DBGHELP.DLL too old";
        }

        ::FreeLibrary(hDll);
    }

    ::MessageBox(NULL, szResult, m_szAppName, MB_OK);

    return retval;
}

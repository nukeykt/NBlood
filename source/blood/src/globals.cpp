//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#include <stdlib.h>

#include "compat.h"
#include "build.h"
#include "common_game.h"
#include "globals.h"
#include "resource.h"
#include "renderlayer.h"


ud_setup_t gSetup;
bool bVanilla = false;
ClockTicks gFrameClock;
ClockTicks gFrameTicks;
int gFrame;
//int volatile gGameClock;
int gFrameRate;
int gGamma;

char gVersionString[16];
int gVersionPal;

Resource gSysRes;

static const char *_module;
static int _line;

void _SetErrorLoc(const char *pzFile, int nLine)
{
    _module = pzFile;
    _line = nLine;
}

void _ThrowError(const char *pzFormat, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, pzFormat);
    vsprintf(buffer, pzFormat, args);
    LOG_F(ERROR, "%s(%i): %s", _module, _line, buffer);

#ifdef WM_MSGBOX_WINDOW
    char titlebuf[256];
    Bsprintf(titlebuf, APPNAME " %s", s_buildRev);
    wm_msgbox(titlebuf, "%s(%i): %s\n", _module, _line, buffer);
#endif

    Bfflush(NULL);
    QuitGame();
}

// by NoOne: show warning msgs in game instead of throwing errors (in some cases)
void _consoleSysMsg(const char* pzFormat, ...) {

    char buffer[1024];
    va_list args;
    va_start(args, pzFormat);
    vsprintf(buffer, pzFormat, args);
    OSD_Printf(OSDTEXT_RED "%s(%i): %s\n", _module, _line, buffer);
}


void __dassert(const char * pzExpr, const char * pzFile, int nLine)
{
    LOG_F(ERROR, "Assertion failed: %s in file %s at line %i", pzExpr, pzFile, nLine);

#ifdef WM_MSGBOX_WINDOW
    char titlebuf[256];
    Bsprintf(titlebuf, APPNAME " %s", s_buildRev);
    wm_msgbox(titlebuf, "Assertion failed: %s in file %s at line %i\n", pzExpr, pzFile, nLine);
#endif

    Bfflush(NULL);
    exit(0);
}

void InitVersionString(void)
{
    Bstrncpyz(gVersionString, s_buildRev, sizeof(gVersionString));

    char * const pHyphen = strchr(gVersionString, '-');
    if (pHyphen != nullptr)
        pHyphen[0] = '\0';

    char * const pBracket = strchr(gVersionString, '[');
    if (pBracket != nullptr)
    {
        pBracket[0] = '\0';
        gVersionPal = 9;
    }
}

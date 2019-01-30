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
#include "build.h"
#include "compat.h"
#include "keyboard.h"
#include "common_game.h"
#include "blood.h"
#include "globals.h"
#include "screen.h"
#include "sound.h"

char exitCredits = 0;

char Wait(int nTicks)
{
    gGameClock = 0;
    while (gGameClock < nTicks)
    {
        timerUpdate();
        char key = keyGetScan();
        if (key)
        {
            if (key == sc_Escape) // sc_Escape
                exitCredits = 1;
            return 0;
        }
    }
    return 1;
}

char DoFade(char r, char g, char b, int nTicks)
{
    dassert(nTicks > 0);
    scrSetupFade(r, g, b);
    gGameClock = gFrameClock = 0;
    do
    {
        while (gGameClock < gFrameClock) { timerUpdate();};
        gFrameClock += 2;
        scrNextPage();
        scrFadeAmount(divscale16(ClipHigh(gGameClock, nTicks), nTicks));
        if (keyGetScan())
            return 0;
    } while (gGameClock <= nTicks);
    return 1;
}

char DoUnFade(int nTicks)
{
    dassert(nTicks > 0);
    scrSetupUnfade();
    gGameClock = gFrameClock = 0;
    do
    {
        while (gGameClock < gFrameClock) { timerUpdate(); };
        scrNextPage();
        scrFadeAmount(0x10000-divscale16(ClipHigh(gGameClock, nTicks), nTicks));
        if (keyGetScan())
            return 0;
    } while (gGameClock <= nTicks);
    return 1;
}

void credLogosDos(void)
{
    char bShift = keystatus[sc_LeftShift] | keystatus[sc_RightShift];
    videoSetViewableArea(0, 0, xdim-1, ydim-1);
    DoUnFade(1);
    videoClearScreen(0);
    if (bShift)
        return;
    {
        //CSMKPlayer smkPlayer;
        //if (smkPlayer.PlaySMKWithWAV("LOGO.SMK", 300) == 1)
        //{
            rotatesprite(160<<16, 100<<16, 65536, 0, 2050, 0, 0, 0x4a, 0, 0, xdim-1, ydim-1);
            sndStartSample("THUNDER2", 128, -1);
            scrNextPage();
            if (!Wait(360))
                return;
            if (!DoFade(0, 0, 0, 60))
                return;
        //}
        //if (smkPlayer.PlaySMKWithWAV("GTI.SMK", 301) == 1)
        //{
            videoClearScreen(0);
            rotatesprite(160<<16, 100<<16, 65536, 0, 2052, 0, 0, 0x0a, 0, 0, xdim-1, ydim-1);
            scrNextPage();
            DoUnFade(1);
            sndStartSample("THUNDER2", 128, -1);
            if (!Wait(360))
                return;
        //}
    }
    sndPlaySpecialMusicOrNothing(MUS_INTRO);
    sndStartSample("THUNDER2", 128, -1);
    if (!DoFade(0, 0, 0, 60))
        return;
    videoClearScreen(0);
    scrNextPage();
    if (!DoUnFade(1))
        return;
    videoClearScreen(0);
    rotatesprite(160<<16, 100<<16, 65536, 0, 2518, 0, 0, 0x4a, 0, 0, xdim-1, ydim-1);
    scrNextPage();
    Wait(360);
    sndFadeSong(4000);
}

void credReset(void)
{
    videoClearScreen(0);
    scrNextPage();
    DoFade(0,0,0,1);
    scrSetupUnfade();
    DoUnFade(1);
}

void credPlaySmk(const char *pzSMK, int nWAV)
{
#if 0
    CSMKPlayer smkPlayer;
    if (dword_148E14 >= 0)
    {
        if (toupper(*pzSMK) == 'A'+dword_148E14)
        {
            if (Redbook.sub_82258() == 0 || Redbook.sub_82258() > 20)
                return;
        }
        Redbook.sub_82554();
    }
    smkPlayer.PlaySMKWithWAV(pzSMK, nWAV);
#endif
}

void credPlaySmk(const char *pzSMK, const char *pzWAV)
{
#if 0
    CSMKPlayer smkPlayer;
    if (dword_148E14 >= 0)
    {
        if (toupper(*pzSMK) == 'A'+dword_148E14)
        {
            if (Redbook.sub_82258() == 0 || Redbook.sub_82258() > 20)
                return;
        }
        Redbook.sub_82554();
    }
    smkPlayer.sub_82E6C(pzSMK, pzWAV);
#endif
}

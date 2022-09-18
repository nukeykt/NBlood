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
#include "common_game.h"
#include "blood.h"
#include "choke.h"
#include "globals.h"
#include "levels.h"
#include "player.h"
#include "qav.h"
#include "resource.h"

CChoke gChoke;

void CChoke::Init(char *pzFile, int _x, int _y, void (*pCallback)(PLAYER *))
{
    x = _x;
    y = _y;
    Process = pCallback;
    if (!hQav && pzFile)
    {
        hQav = gSysRes.Lookup(pzFile, "QAV");
        if (!hQav)
            ThrowError("Could not load QAV %s\n", pzFile);
        pQav = (QAV*)gSysRes.Lock(hQav);
        pQav->nSprite = -1;
        pQav->x = x;
        pQav->y = y;
        pQav->Preload();
        chokeTimeInit();
    }
}

void CChoke::Init(int qavId, void (*pCallback)(PLAYER *))
{
    Process = pCallback;
    if (!hQav && qavId != -1)
    {
        hQav = gSysRes.Lookup(qavId, "QAV");
        if (!hQav)
            ThrowError("Could not load QAV %d\n", qavId);
        pQav = (QAV*)gSysRes.Lock(hQav);
        pQav->nSprite = -1;
        pQav->x = x;
        pQav->y = y;
        pQav->Preload();
        chokeTimeInit();
    }
}

void CChoke::Init(char *pzFile, void (*pCallback)(PLAYER *))
{
    Process = pCallback;
    if (!hQav && pzFile)
    {
        hQav = gSysRes.Lookup(pzFile, "QAV");
        if (!hQav)
            ThrowError("Could not load QAV %s\n", pzFile);
        pQav = (QAV*)gSysRes.Lock(hQav);
        pQav->nSprite = -1;
        pQav->x = x;
        pQav->y = y;
        pQav->Preload();
        chokeTimeInit();
    }
}

void CChoke::Draw(int x, int y)
{
    if (!hQav)
        return;
    const ClockTicks bakClock = gFrameClock;
    gFrameClock = totalclock;
    pQav->x = x;
    pQav->y = y;
    int diff = (int)totalclock-clock;
    clock = (int)totalclock;
    duration -= diff;
    if (duration <= 0 || duration > pQav->at10)
        duration = pQav->at10;
    int vdi = pQav->at10-duration;
    pQav->Play(vdi-diff, vdi, -1, NULL);
    const int win1x = windowxy1.x;
    const int win1y = windowxy1.y;
    const int win2x = windowxy2.x;
    const int win2y = windowxy2.y;
    windowxy1.x = 0;
    windowxy1.y = 0;
    windowxy2.x = xdim-1;
    windowxy2.y = ydim-1;
    pQav->Draw(vdi, 10, 0, 0);
    windowxy1.x = win1x;
    windowxy1.y = win1y;
    windowxy2.x = win2x;
    windowxy2.y = win2y;
    gFrameClock = bakClock;
}

void CChoke::chokeTimeInit()
{
    duration = pQav->at10;
    clock = (int)totalclock;
}

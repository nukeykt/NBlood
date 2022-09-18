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
#pragma once

#include "common_game.h"
#include "player.h"
#include "qav.h"
#include "resource.h"

class CChoke
{
public:
    CChoke()
    {
        hQav = NULL;
        pQav = NULL;
        duration = 0;
        clock = 0;
        x = 0;
        y = 0;
        Process = NULL;
    };
    void Init(char *pzFile, int _x, int _y, void (*pCallback)(PLAYER *));
    void Init(int qavId, void (*pCallback)(PLAYER *));
    void Init(char *pzFile, void (*pCallback)(PLAYER *));
    void Draw(int x, int y);
    DICTNODE *hQav;
    QAV *pQav;
    int duration;
    int clock;
    int x;
    int y;
    void (*Process)(PLAYER *);
private:
    void chokeTimeInit(void);
};

extern CChoke gChoke;

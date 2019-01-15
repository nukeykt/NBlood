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

#include "controls.h"
#include "levels.h"

#define kInputBufferSize 1024

#pragma pack(push, 1)

struct DEMOHEADER
{
    int signature;
    short nVersion;
    int nBuild;
    int nInputCount;
    int nNetPlayers;
    short nMyConnectIndex;
    short nConnectHead;
    short connectPoints[8];
    GAMEOPTIONS gameOptions;
};

#pragma pack(pop)

class CDemo {
public:
    CDemo();
    ~CDemo();
    bool Create(const char *);
    void Write(GINPUT *);
    void Close(void);
    bool SetupPlayback(const char *);
    void ProcessKeys(void);
    void Playback(void);
    void StopPlayback(void);
    void LoadDemoInfo(void);
    void NextDemo(void);
    void FlushInput(int nCount);
    void ReadInput(int nCount);
    bool at0; // record
    bool at1; // playback
    char at2;
    int at3;
    FILE *at7;
    int atb;
    DEMOHEADER atf;
    GINPUT at1aa[kInputBufferSize];
    char at59aa[5][13];
    int at59eb;
    int at59ef;
};

extern CDemo gDemo;

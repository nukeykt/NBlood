//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
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

#ifndef __runlist_h__
#define __runlist_h__

#include "compat.h"

#define kMaxRuns		25600
#define kMaxChannels	4096

struct RunStruct
{
    union
    {
        int nMoves;
        struct
        {
#if B_BIG_ENDIAN == 1
            short nRef;
            short nVal;
#else
            short nVal;
            short nRef;
#endif
        };
    };

    short _4;
    short _6;
};

struct RunChannel
{
    short a;
    short b;
    short c;
    short d;
};

typedef void(*AiFunc)(int, int, int nRun);

extern RunStruct RunData[kMaxRuns];
extern RunChannel sRunChannels[kMaxChannels];
extern short NewRun;
extern int nRadialOwner;
extern short nRadialSpr;

void runlist_InitRun();

int runlist_GrabRun();
int runlist_FreeRun(int nRun);
int runlist_AddRunRec(int a, int b);
int runlist_HeadRun();
void runlist_InitChan();
void runlist_ChangeChannel(int nChannel, short nVal);
void runlist_ReadyChannel(int nChannel);
void runlist_ProcessSectorTag(int nSector, int nLotag, int nHitag);
int runlist_AllocChannel(int a);
void runlist_DoSubRunRec(int RunPtr);
void runlist_SubRunRec(int RunPtr);
void runlist_ProcessWallTag(int nWall, short nLotag, short nHitag);
int runlist_CheckRadialDamage(short nSprite);
void runlist_RadialDamageEnemy(short nSprite, short nDamage, short nRadius);
void runlist_DamageEnemy(int nSprite, int nSprite2, short nDamage);
void runlist_SignalRun(int NxtPtr, int edx);

void runlist_CleanRunRecs();
void runlist_ExecObjects();

#endif

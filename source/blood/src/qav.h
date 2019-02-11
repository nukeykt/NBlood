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
#include "build.h"
#include "common_game.h"
#include "blood.h"

#define kQavOrientationLeft 4096

#pragma pack(push, 1)

struct TILE_FRAME
{
    int picnum;
    int x;
    int y;
    int z;
    int stat;
    signed char shade;
    char palnum;
    unsigned short angle;
};

struct SOUNDINFO
{
    int sound;
    unsigned char at4;
    char pad[3];
};

struct FRAMEINFO
{
    int nCallbackId; // 0
    SOUNDINFO sound; // 4
    TILE_FRAME tiles[8]; // 12
};

struct QAV
{
    char pad1[8]; // 0
    int nFrames; // 8
    int ticksPerFrame; // C
    int at10; // 10
    int x; // 14
    int y; // 18
    int nSprite; // 1c
    //SPRITE *pSprite; // 1c
    char pad3[4]; // 20
    FRAMEINFO frames[1]; // 24
    void Draw(long ticks, int stat, int shade, int palnum);
    void Play(long, long, int, void *);
    void Preload(void);

    void PlaySound(int nSound);
    void PlaySound3D(spritetype *pSprite, int nSound, int a3, int a4);
};

#pragma pack(pop)

int qavRegisterClient(void(*pClient)(int, void *));

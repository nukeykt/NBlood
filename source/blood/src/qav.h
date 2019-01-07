#pragma once
#include "build.h"
#include "common_game.h"
#include "blood.h"

#define kQavOrientationLeft 4096

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
    SPRITE *pSprite; // 1c
    char pad3[4]; // 20
    FRAMEINFO frames[1]; // 24
    void Draw(long ticks, int stat, int shade, int palnum);
    void Play(long, long, int, void *);
    void Preload(void);

    void PlaySound(int nSound);
    void PlaySound3D(SPRITE *pSprite, int nSound, int a3, int a4);
};

int qavRegisterClient(void(*pClient)(int, void *));

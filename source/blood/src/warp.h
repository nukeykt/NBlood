#pragma once
#include "common_game.h"

struct ZONE {
    int x, y, z;
    short sectnum, ang;
};
extern ZONE gStartZone[8];

void warpInit(void);
int CheckLink(SPRITE *pSprite);
int CheckLink(long *x, long *y, long *z, int *nSector);

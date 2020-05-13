
#ifndef __effects_h__
#define __effects_h__

#include "player.h"

extern int lastbat;

void dofx();
void initwater();
void warpsprite(short spritenum);
void makemonstersplash(int picnum, int i);
void bats(short k);
void warpfxsprite(int s);
void makeasplash(int picnum, Player *plr);
void cracks();
void thunder();
void thesplash();
void scary();
void initlava();
void lavadryland();
void warp(int32_t* x, int32_t* y, int32_t* z, short* daang, short* dasector);
void sectorsounds();

#endif

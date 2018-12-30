#pragma once
#include "build.h"
#include "common.h"
#include "common_game.h"

#include "blood.h"
#include "db.h"
#include "eventq.h"

void trTriggerSector(unsigned int nSector, XSECTOR *pXSector, int a3);
void trMessageSector(unsigned int nSector, EVENT a2);
void trTriggerWall(unsigned int nWall, XWALL *pXWall, int a3);
void trMessageWall(unsigned int nWall, EVENT a2);
void trTriggerSprite(unsigned int nSprite, XSPRITE *pXSprite, int a3);
void trMessageSprite(unsigned int nSprite, EVENT a2);
void trProcessBusy(void);
void trInit(void);
void trTextOver(int nId);
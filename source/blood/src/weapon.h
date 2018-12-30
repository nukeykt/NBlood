#pragma once
#include "common_game.h"
#include "blood.h"
#include "db.h"
#include "player.h"

void WeaponInit(void);
void WeaponDraw(PLAYER *pPlayer, int a2, int a3, int a4, int a5);
void WeaponRaise(PLAYER *pPlayer);
void WeaponLower(PLAYER *pPlayer);
char WeaponUpgrade(PLAYER *pPlayer, char newWeapon);
void WeaponProcess(PLAYER *pPlayer);
void sub_51340(SPRITE *pMissile, int a2);
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
#include "db.h"
#include "fx.h"
#include "gameutil.h"

enum DAMAGE_TYPE {
    kDamageFall = 0,
    kDamageBurn,
    kDamageBullet,
    kDamageExplode,
    kDamageDrown,
    kDamageSpirit,
    kDamageTesla,
    kDamageMax = 7,
};

enum VECTOR_TYPE {
    kVectorTine = 0,
    kVectorShell,
    kVectorBullet,
    kVectorTommyAP,
    kVectorShellAP,
    kVectorTommyregular,
    kVectorBatBite,
    kVectorBoneelBite,
    kVectorGillBite,
    kVectorBeastSlash,
    kVectorAxe,
    kVectorCleaver,
    kVectorGhost,
    kVectorGargSlash,
    kVectorCerberusHack,
    kVectorHoundBite,
    kVectorRatBite,
    kVectorSpiderBite,
    VECTOR_TYPE_18, // Unknown
    VECTOR_TYPE_19, // Unknown
    kVectorTchernobogBurn,
    kVectorVoodoo10,
    #ifdef NOONE_EXTENSIONS
    kVectorGenDudePunch,
    #endif
    kVectorMax,
};

struct THINGINFO
{
    short startHealth;
    short mass;
    unsigned char clipdist;
    short flags;
    int elastic; // elasticity
    int dmgResist; // damage resistance
    short cstat;
    short picnum;
    char shade;
    unsigned char pal;
    unsigned char xrepeat; // xrepeat
    unsigned char yrepeat; // yrepeat
    int dmgControl[kDamageMax]; // damage
};

struct AMMOITEMDATA
{
    short cstat;
    short picnum;
    char shade;
    char pal;
    unsigned char xrepeat;
    unsigned char yrepeat;
    short count;
    unsigned char type;
    unsigned char weaponType;
};

struct WEAPONITEMDATA
{
    short cstat;
    short picnum;
    char shade;
    char pal;
    unsigned char xrepeat;
    unsigned char yrepeat;
    short type;
    short ammoType;
    short count;
};

struct ITEMDATA
{
    short cstat;
    short picnum;
    char shade;
    char pal;
    unsigned char xrepeat;
    unsigned char yrepeat;
    short packSlot;
};

struct MissileType
{
    short picnum;
    int velocity;
    int angleOfs;
    unsigned char xrepeat;
    unsigned char yrepeat;
    char shade;
    unsigned char clipDist;
};

struct EXPLOSION
{
    unsigned char repeat;
    char dmg;
    char dmgRng;
    int radius;
    int dmgType;
    int burnTime;
    int ticks;
    int quakeEffect;
    int flashEffect;
};

struct SURFHIT {
    FX_ID fx1;
    FX_ID fx2;
    FX_ID fx3;
    int fxSnd;
};

struct VECTORDATA {
    DAMAGE_TYPE dmgType;
    int dmg; // damage
    int impulse;
    int maxDist;
    int fxChance;
    int burnTime; // burn
    int bloodSplats; // blood splats
    int splatChance; // blood splat chance
    SURFHIT surfHit[15];
};

extern AMMOITEMDATA gAmmoItemData[];
extern WEAPONITEMDATA gWeaponItemData[];
extern ITEMDATA gItemData[];
extern MissileType missileInfo[];
extern EXPLOSION explodeInfo[];
extern THINGINFO thingInfo[];
extern VECTORDATA gVectorData[];

extern int gDudeDrag;
extern short gAffectedSectors[kMaxSectors];
extern short gAffectedXWalls[kMaxXWalls];

template<typename T> bool IsPlayerSprite(T const * const pSprite)
{
    return pSprite->type >= kDudePlayer1 && pSprite->type <= kDudePlayer8;
}

template<typename T> bool IsDudeSprite(T const * const pSprite)
{
    return pSprite->type >= kDudeBase && pSprite->type < kDudeMax;
}

template<typename T> bool IsThingSprite(T const* const pSprite)
{
    return pSprite->type >= kThingBase && pSprite->type < kThingMax;
}

template<typename T> bool IsItemSprite(T const * const pSprite)
{
    return pSprite->type >= kItemBase && pSprite->type < kItemMax;
}

template<typename T> bool IsWeaponSprite(T const * const pSprite)
{
    return pSprite->type >= kItemWeaponBase && pSprite->type < kItemWeaponMax;
}

template<typename T> bool IsAmmoSprite(T const * const pSprite)
{
    return pSprite->type >= kItemAmmoBase && pSprite->type < kItemAmmoMax;
}

inline void actBurnSprite(int nSource, XSPRITE *pXSprite, int nTime)
{
    pXSprite->burnTime = ClipHigh(pXSprite->burnTime + nTime, sprite[pXSprite->reference].statnum == kStatDude ? 2400 : 1200);
    pXSprite->burnSource = nSource;
}

#ifdef POLYMER
void actAddGameLight(int lightRadius, int spriteNum, int zOffset, int lightRange, int lightColor, int lightPrio);
void actDoLight(int spriteNum);
#endif

void FireballSeqCallback(int, int);
void sub_38938(int, int);
void NapalmSeqCallback(int, int);
void sub_3888C(int, int);
void TreeToGibCallback(int, int);
void DudeToGibCallback1(int, int);
void DudeToGibCallback2(int, int);

bool IsUnderwaterSector(int nSector);
int actSpriteOwnerToSpriteId(spritetype *pSprite);
void actPropagateSpriteOwner(spritetype *pTarget, spritetype *pSource);
int actSpriteIdToOwnerId(int nSprite);
int actOwnerIdToSpriteId(int nSprite);
bool actTypeInSector(int nSector, int nType);
void actAllocateSpares(void);
void actInit(bool bSaveLoad);
void ConcussSprite(int a1, spritetype *pSprite, int x, int y, int z, int a6);
int actWallBounceVector(int *x, int *y, int nWall, int a4);
int actFloorBounceVector(int *x, int *y, int *z, int nSector, int a5);
void actRadiusDamage(int nSprite, int x, int y, int z, int nSector, int nDist, int baseDmg, int distDmg, DAMAGE_TYPE dmgType, int flags, int burn);
void actNapalmMove(spritetype *pSprite, XSPRITE *pXSprite);
spritetype *actSpawnFloor(spritetype *pSprite);
spritetype *actDropAmmo(spritetype *pSprite, int nType);
spritetype *actDropWeapon(spritetype *pSprite, int nType);
spritetype *actDropItem(spritetype *pSprite, int nType);
spritetype *actDropKey(spritetype *pSprite, int nType);
spritetype *actDropFlag(spritetype *pSprite, int nType);
spritetype *actDropObject(spritetype *pSprite, int nType);
bool actHealDude(XSPRITE *pXDude, int a2, int a3);
void actKillDude(int a1, spritetype *pSprite, DAMAGE_TYPE a3, int a4);
int actDamageSprite(int nSource, spritetype *pSprite, DAMAGE_TYPE a3, int a4);
void actHitcodeToData(int a1, HITINFO *pHitInfo, int *a3, spritetype **a4, XSPRITE **a5, int *a6, walltype **a7, XWALL **a8, int *a9, sectortype **a10, XSECTOR **a11);
void actImpactMissile(spritetype *pMissile, int hitCode);
void actKickObject(spritetype *pSprite1, spritetype *pSprite2);
void actTouchFloor(spritetype *pSprite, int nSector);
void ProcessTouchObjects(spritetype *pSprite, int nXSprite);
void actAirDrag(spritetype *pSprite, int a2);
int MoveThing(spritetype *pSprite);
void MoveDude(spritetype *pSprite);
int MoveMissile(spritetype *pSprite);
void actExplodeSprite(spritetype *pSprite);
void actActivateGibObject(spritetype *pSprite, XSPRITE *pXSprite);
bool IsUnderWater(spritetype *pSprite);
void actProcessSprites(void);
spritetype * actSpawnSprite(int nSector, int x, int y, int z, int nStat, char a6);
spritetype *actSpawnDude(spritetype *pSource, short nType, int a3, int a4);
spritetype * actSpawnSprite(spritetype *pSource, int nStat);
spritetype * actSpawnThing(int nSector, int x, int y, int z, int nThingType);
spritetype * actFireThing(spritetype *pSprite, int a2, int a3, int a4, int thingType, int a6);
spritetype* actFireMissile(spritetype *pSprite, int a2, int a3, int a4, int a5, int a6, int nType);
int actGetRespawnTime(spritetype *pSprite);
bool actCheckRespawn(spritetype *pSprite);
bool actCanSplatWall(int nWall);
void actFireVector(spritetype *pShooter, int a2, int a3, int a4, int a5, int a6, VECTOR_TYPE vectorType);
void actPostSprite(int nSprite, int nStatus);
void actPostProcess(void);
void MakeSplash(spritetype *pSprite, XSPRITE *pXSprite);
void actBuildMissile(spritetype* pMissile, int nXSprite, int nSprite);

extern int DudeDifficulty[];

//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) NoOne

*****************************************************************
NoOne: AI code for Custom Dude system.
*****************************************************************

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
#ifdef NOONE_EXTENSIONS
#include "nnexts.h"
#include "aicdud.h"
#include "globals.h"
#include "player.h"
#include "trig.h"
#include "endgame.h"
#include "mmulti.h"
#include "view.h"

#define SEQOFFS(x) (kCdudeDefaultSeq + x)

#pragma pack(push, 1)
struct TARGET_INFO
{
    spritetype* pSpr;
    XSPRITE* pXSpr;
    unsigned int nDist  : 32;
    unsigned int nAng   : 12;
    unsigned int nDang  : 12;
    int nCode;
};
#pragma pack(pop)

static void resetTarget(spritetype*, XSPRITE* pXSpr)            { pXSpr->target = -1; }
static void moveStop(spritetype* pSpr, XSPRITE*)                { xvel[pSpr->index] = yvel[pSpr->index] = 0; }
static char THINK_CLOCK(int nSpr, int nClock = 3)               { return ((gFrame & nClock) == (nSpr & nClock)); }
static int qsSortTargets(TARGET_INFO* ref1, TARGET_INFO* ref2)  { return ref1->nDist - ref2->nDist; }

static void thinkSearch(spritetype*, XSPRITE*);
static void thinkChase(spritetype*, XSPRITE*);
static void thinkFlee(spritetype*, XSPRITE*);
static void thinkTarget(spritetype* pSpr, XSPRITE*);
static void thinkMorph(spritetype* pSpr, XSPRITE* pXSpr);
static void thinkDying(spritetype* pSpr, XSPRITE* pXSpr);
static void enterBurnSearchWater(spritetype* pSpr, XSPRITE* pXSpr);
static void enterMorph(spritetype* pSpr, XSPRITE* pXSpr);
static void enterDying(spritetype* pSpr, XSPRITE* pXSpr);
static void enterDeath(spritetype* pSpr, XSPRITE* pXSpr);
static void enterSleep(spritetype* pSpr, XSPRITE* pXSpr);
static void enterWake(spritetype* pSpr, XSPRITE*);
static int getTargetAng(spritetype* pSpr, XSPRITE* pXSpr);
static void turnToTarget(spritetype* pSpr, XSPRITE* pXSpr);
static void moveTurn(spritetype* pSpr, XSPRITE* pXSpr);
static void moveDodge(spritetype* pSpr, XSPRITE* pXSpr);
static void moveForward(spritetype* pSpr, XSPRITE* pXSpr);
static void moveKnockout(spritetype* pSpr, XSPRITE* pXSpr);

static void weaponShot(int, int);
static int nWeaponShot  = seqRegisterClient(weaponShot);
static int weaponShotDummy(CUSTOMDUDE*, CUSTOMDUDE_WEAPON*, POINT3D*, int, int, int) { return -1; }
static int weaponShotHitscan(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz);
static int weaponShotMissile(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz);
static int weaponShotThing(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int, int, int);
static int weaponShotSummon(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz);
static int weaponShotKamikaze(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz);
static int weaponShotSpecialBeastStomp(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON*, POINT3D* pOffs, int, int, int);

int (*gWeaponShotFunc[])(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz) =
{
    weaponShotDummy,    // none
    weaponShotHitscan,
    weaponShotMissile,
    weaponShotThing,
    weaponShotSummon,   // vanilla dude
    weaponShotSummon,   // custom  dude
    weaponShotKamikaze,
    weaponShotSpecialBeastStomp,
};

static AISTATE gCdudeStateDeath = { kAiStateOther, -1, -1, 0, enterDeath, NULL, NULL, NULL }; // just turns dude to a gib

// Land, Crouch, Swim (proper order matters!)
AISTATE gCdudeStateTemplate[kCdudeStateNormalMax][kCdudePostureMax] =
{
    // idle (don't change pos or patrol gets broken!)
    {
        { kAiStateIdle,     SEQOFFS(0),   -1, 0, resetTarget, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFS(17),  -1, 0, resetTarget, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFS(13),  -1, 0, resetTarget, NULL, thinkTarget, NULL },
    },

    // search (don't change pos or patrol gets broken!)
    {
        { kAiStateSearch,   SEQOFFS(9),  -1, 800, NULL, moveForward, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureL] },
        { kAiStateSearch,   SEQOFFS(14), -1, 800, NULL, moveForward, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureC] },
        { kAiStateSearch,   SEQOFFS(13), -1, 800, NULL, moveForward, thinkSearch, &gCdudeStateTemplate[kCdudeStateIdle][kCdudePostureW] },
    },

    // dodge
    {
        { kAiStateMove,     SEQOFFS(9),  -1, 90, NULL,  moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureL] },
        { kAiStateMove,     SEQOFFS(14), -1, 90, NULL,  moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureC] },
        { kAiStateMove,     SEQOFFS(13), -1, 90, NULL,  moveDodge,	NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureW] },
    },

    // chase
    {
        { kAiStateChase,    SEQOFFS(9),  -1, 30, NULL,	moveForward, thinkChase, NULL },
        { kAiStateChase,    SEQOFFS(14), -1, 30, NULL,	moveForward, thinkChase, NULL },
        { kAiStateChase,    SEQOFFS(13), -1, 30, NULL,	moveForward, thinkChase, NULL },
    },

    // flee
    {
        { kAiStateMove,    SEQOFFS(9),  -1, 256, NULL,	moveForward, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureL] },
        { kAiStateMove,    SEQOFFS(14), -1, 256, NULL,	moveForward, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureC] },
        { kAiStateMove,    SEQOFFS(13), -1, 256, NULL,	moveForward, thinkFlee, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureW] },
    },

    // recoil normal
    {
        { kAiStateRecoil,   SEQOFFS(5), -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureL] },
        { kAiStateRecoil,   SEQOFFS(5), -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureC] },
        { kAiStateRecoil,   SEQOFFS(5), -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureW] },
    },

    // recoil tesla
    {
        { kAiStateRecoil,   SEQOFFS(4), -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureL] },
        { kAiStateRecoil,   SEQOFFS(4), -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureC] },
        { kAiStateRecoil,   SEQOFFS(4), -1, 0, NULL, NULL, NULL, &gCdudeStateTemplate[kCdudeStateChase][kCdudePostureW] },
    },

    // burn search
    {
        { kAiStateSearch,   SEQOFFS(3), -1, 3600, enterBurnSearchWater, aiMoveForward, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureL] },
        { kAiStateSearch,   SEQOFFS(3), -1, 3600, enterBurnSearchWater, aiMoveForward, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureC] },
        { kAiStateSearch,   SEQOFFS(3), -1, 3600, enterBurnSearchWater, aiMoveForward, NULL, &gCdudeStateTemplate[kCdudeBurnStateSearch][kCdudePostureW] },
    },

    // morph (put thinkFunc in moveFunc because it supposed to work fast)
    {
        { kAiStateOther,   SEQOFFS(18), -1, 0, enterMorph, thinkMorph, NULL, NULL },
        { kAiStateOther,   SEQOFFS(18), -1, 0, enterMorph, thinkMorph, NULL, NULL },
        { kAiStateOther,   SEQOFFS(18), -1, 0, enterMorph, thinkMorph, NULL, NULL },
    },

    // knock enter
    {
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureL] },
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureC] },
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, moveKnockout, NULL, &gCdudeStateTemplate[kCdudeStateKnock][kCdudePostureW] },
    },

    // knock
    {
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureL] },
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, NULL,         NULL, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureC] },
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, moveKnockout, NULL, &gCdudeStateTemplate[kCdudeStateKnockExit][kCdudePostureW] },
    },

    // knock exit
    {
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureL] },
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureC] },
        { kAiStateKnockout,   SEQOFFS(0), -1, 0, NULL, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureW] },
    },

    // sleep
    {
        { kAiStateIdle,     SEQOFFS(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFS(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
        { kAiStateIdle,     SEQOFFS(0),  -1, 0, enterSleep, NULL, thinkTarget, NULL },
    },

    // wake
    {
        { kAiStateIdle,     SEQOFFS(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureL] },
        { kAiStateIdle,     SEQOFFS(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureC] },
        { kAiStateIdle,     SEQOFFS(0),  -1, 0, enterWake, turnToTarget, NULL, &gCdudeStateTemplate[kCdudeStateSearch][kCdudePostureW] },
    },

    // generic idle (ai fight compat.)
    {
        { kAiStateGenIdle,     SEQOFFS(0),   -1, 0, resetTarget, NULL, NULL, NULL },
        { kAiStateGenIdle,     SEQOFFS(17),  -1, 0, resetTarget, NULL, NULL, NULL },
        { kAiStateGenIdle,     SEQOFFS(13),  -1, 0, resetTarget, NULL, NULL, NULL },
    },
};

// Land, Crouch, Swim
AISTATE gCdudeStateAttackTemplate[kCdudePostureMax] =
{
    // attack (put thinkFunc in moveFunc because it supposed to work fast)
    { kAiStateAttack,   SEQOFFS(6), nWeaponShot, 0, moveStop, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureL] },
    { kAiStateAttack,   SEQOFFS(8), nWeaponShot, 0, moveStop, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureC] },
    { kAiStateAttack,   SEQOFFS(8), nWeaponShot, 0, moveStop, thinkChase, NULL, &gCdudeStateAttackTemplate[kCdudePostureW] },
};

// Random pick
AISTATE gCdudeStateDyingTemplate[kCdudePostureMax] =
{
    // dying
    { kAiStateOther,   SEQOFFS(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
    { kAiStateOther,   SEQOFFS(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
    { kAiStateOther,   SEQOFFS(1), -1, 0, enterDying, NULL, thinkDying, &gCdudeStateDeath },
};

// for kModernThingThrowableRock
static short gCdudeDebrisPics[6] =
{
    2406, 2280, 2185, 2155, 2620, 3135
};

static int weaponShotHitscan(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz)
{
    VECTORDATA* pVect = &gVectorData[pWeap->id];
    spritetype* pSpr = pDude->pSpr;
    int t;

    // ugly hack to make it fire at required distance
    t = pVect->maxDist, pVect->maxDist = pWeap->GetDistance();
    actFireVector(pSpr, pOffs->x, pOffs->z, dx, dy, dz, (VECTOR_TYPE)pWeap->id);
    pVect->maxDist = t;

    return kMaxSprites;
}

static int weaponShotMissile(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz)
{
    spritetype* pSpr = pDude->pSpr, *pShot;
    XSPRITE* pXSpr = pDude->pXSpr, *pXShot;

    pShot = nnExtFireMissile(pSpr, pOffs->x, pOffs->z, dx, dy, dz, pWeap->id);
    if (pShot)
    {
        pXShot = &xsprite[pShot->extra];
        nnExtOffsetSprite(pShot, 0, pOffs->y, 0);

        if (pWeap->shot.clipdist)
            pShot->clipdist = pWeap->shot.clipdist;

        if (pWeap->HaveVelocity())
        {
            pXShot->target = -1; // have to erase, so vanilla won't set velocity back
            nnExtScaleVelocity(pShot, pWeap->shot.velocity, dx, dy, dz);
        }

        pWeap->shot.appearance.Set(pShot);

        if (pWeap->shot.targetFollow)
        {
            pXShot->goalAng = pWeap->shot.targetFollow;
            gFlwSpritesList.Add(pShot->index);
            pXShot->sysData1 = pXSpr->target;
            pXShot->target = -1; // have own target follow code
        }


        return pShot->index;
    }

    return -1;
}

static int weaponShotThing(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int, int, int)
{
    spritetype* pSpr   = pDude->pSpr; XSPRITE* pXSpr = pDude->pXSpr;
    spritetype* pLeech = NULL, *pShot, *pTarget;
    XSPRITE* pXShot;
    
    if (!spriRangeIsFine(pXSpr->target))
        return -1;

    pTarget = &sprite[pXSpr->target];
    int dx = pTarget->x - pSpr->x;
    int dy = pTarget->y - pSpr->y;
    int dz = pTarget->z - pSpr->z;
    int nDist = approxDist(dx, dy);
    int nDiv = 540, nVel, nSlope = 12000;
    char impact = true;
    int nHealth = 0;

    switch (pWeap->id)
    {
        case kModernThingEnemyLifeLeech:
        case kThingDroppedLifeLeech:
            if (!pDude->IsLeechBroken())
            {
                if (pLeech)
                {
                    if (xsprIsFine(pLeech))
                        nHealth = pDude->pXLeech->health;

                    pDude->LeechPickup(); // pickup it before throw
                }
            }
            break;
    }
    
    nSlope = (pWeap->HaveSlope()) ? pWeap->shot.slope : ((dz / 128) - nSlope);
    
    nVel = divscale23(nDist / nDiv, 120);
    pShot = actFireThing(pSpr, -pOffs->x, pOffs->z, nSlope, pWeap->id, nVel);
    if (pShot)
    {
        nnExtOffsetSprite(pShot, 0, pOffs->y, 0);

        THINGINFO* pInfo        = &thingInfo[pWeap->id - kThingBase];
        pShot->picnum           = ClipLow(pInfo->picnum, 0);
        pXShot                  = &xsprite[pShot->extra];
        pShot->owner            = pSpr->index;

        switch (pWeap->id)
        {
            case kModernThingTNTProx:
            case kThingArmedProxBomb:
            case kModernThingThrowableRock:
            case kModernThingEnemyLifeLeech:
            case kThingDroppedLifeLeech:
            case kThingBloodBits:
            case kThingBloodChunks:
                switch (pWeap->id)
                {
                    case kModernThingThrowableRock:
                        pShot->picnum   = gCdudeDebrisPics[Random(LENGTH(gCdudeDebrisPics))];
                        pShot->xrepeat  = pShot->yrepeat = 24 + Random(42);
                        pShot->cstat    |= CSTAT_SPRITE_BLOCK;
                        pShot->pal      = 5;

                        if (Chance(0x5000)) pShot->cstat |= CSTAT_SPRITE_XFLIP;
                        if (Chance(0x5000)) pShot->cstat |= CSTAT_SPRITE_YFLIP;

                        if (pShot->xrepeat > 60)       pXShot->data1 = 43;
                        else if (pShot->xrepeat > 40)  pXShot->data1 = 33;
                        else if (pShot->xrepeat > 30)  pXShot->data1 = 23;
                        else                           pXShot->data1 = 12;
                        break;
                    case kThingArmedProxBomb:
                    case kModernThingTNTProx:
                        pXShot->state = 0;
                        pXShot->Proximity = true;
                        break;
                    case kThingBloodBits:
                    case kThingBloodChunks:
                        DudeToGibCallback1(pShot->index, pShot->extra);
                        break;
                    default:
                        if (pLeech)
                        {
                            pXShot->health = nHealth;
                        }
                        else
                        {
                            pXShot->health = ((pInfo->startHealth << 4) * ClipLow(gGameOptions.nDifficulty, 1)) >> 1;
                        }

                        pShot->cstat        &= ~CSTAT_SPRITE_BLOCK;
                        pShot->pal          = 6;
                        pShot->clipdist     = 0;
                        pXShot->data3       = 512 / (gGameOptions.nDifficulty + 1);
                        pXShot->target      = pTarget->index;
                        pXShot->Proximity   = true;
                        pXShot->stateTimer  = 1;

                        evPost(pShot->index, 3, 80, kCallbackLeechStateTimer);
                        pDude->pXLeech = &xsprite[pShot->extra];
                        break;
                }
                impact = false;
                break;
            case kThingNapalmBall:
                pShot->xrepeat = pShot->yrepeat = 24;
                pXShot->data4 = 3 + Random2(2);
                pXShot->Impact = true;
                break;
        }

        if (pWeap->shot.clipdist)               pShot->clipdist = pWeap->shot.clipdist;
        if (pWeap->HaveVelocity())              nnExtScaleVelocity(pShot, pWeap->shot.velocity << 3, dx, dy, dz, 0x01);
        
        pWeap->shot.appearance.Set(pShot);

        if (pWeap->shot.targetFollow)
        {
            pXShot->goalAng = pWeap->shot.targetFollow;
            pXShot->sysData1 = pXSpr->target;
            gFlwSpritesList.Add(pShot->index);
        }

        if (pWeap->shot.impact > 1)
        {
            if (impact)
                pXShot->Impact = (pXShot->Impact && nDist <= 7680);
        }
        else
        {
            pXShot->Impact = pWeap->shot.impact;
        }

        if (!pXShot->Impact)
            evPost(pShot->index, OBJ_SPRITE, 120 * Random(2) + 120, kCmdOn, pSpr->index);

        return pShot->index;
    }

    return -1;
}

static char posObstructed(int x, int y, int z, int nRadius)
{
    int i = numsectors;
    while (--i >= 0 && !inside(x, y, i));
    if (i < 0)
        return true;

    for (i = 0; i < kMaxSprites; i++)
    {
        spritetype* pSpr = &sprite[i];
        if ((pSpr->flags & kHitagFree) || (pSpr->flags & kHitagRespawn)) continue;
        if ((pSpr->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) != CSTAT_SPRITE_ALIGNMENT_FACING)
            continue;

        if (!(pSpr->cstat & CSTAT_SPRITE_BLOCK))
        {
            if (!IsDudeSprite(pSpr) || !dudeIsAlive(pSpr))
                continue;
        }
        else
        {
            int w = tilesiz[pSpr->picnum].x;
            int h = tilesiz[pSpr->picnum].y;

            if (w <= 0 || h <= 0)
                continue;
        }

        if (CheckProximityPoint(pSpr->x, pSpr->y, pSpr->z, x, y, z, nRadius))
            return true;
    }

    return false;
}



static int weaponShotSummon(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int dx, int dy, int dz)
{
    spritetype* pShot, *pSpr = pDude->pSpr;
    XSPRITE *pXShot, *pXSpr = pDude->pXSpr;

    int x = pSpr->x, y = pSpr->y, z = pSpr->z, a = 0;
    
    int nDude = pWeap->id;
    if (pWeap->type == kCdudeWeaponSummonCdude)
        nDude = kDudeModernCustom;

    nnExtOffsetPos(pOffs->x, ClipLow(pOffs->y, 800), pOffs->z, pSpr->ang, &x, &y, &z);

    while (a < kAng180)
    {
        if (!posObstructed(x, y, z, 32))
        {
            if ((pShot = nnExtSpawnDude(pSpr, nDude, x, y, z)) != NULL)
            {
                pXShot = &xsprite[pShot->extra];
                if (nDude == kDudeModernCustom)
                    pXShot->data1 = pWeap->id;

                if (pWeap->shot.clipdist)
                    pShot->clipdist = pWeap->shot.clipdist;

                if (pWeap->HaveVelocity())
                    nnExtScaleVelocity(pShot, pWeap->shot.velocity, dx, dy, dz);

                pWeap->shot.appearance.Set(pShot);

                aiInitSprite(pShot);

                pXShot->targetX = pXSpr->targetX;
                pXShot->targetY = pXSpr->targetY;
                pXShot->targetZ = pXSpr->targetZ;
                pXShot->target  = pXSpr->target;
                pShot->ang      = pSpr->ang;

                aiActivateDude(pShot, pXShot);

                pDude->pSlaves->Add(pShot->index);
                gKillMgr.AddCount(pShot);
                return pShot->index;
            }
        }
        else
        {
            RotatePoint(&x, &y, a, pSpr->x, pSpr->y);
            a += kAng15;
            continue;
        }

        break;
    }

    return -1;
}

static int weaponShotKamikaze(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeap, POINT3D* pOffs, int, int, int)
{
    spritetype* pSpr = pDude->pSpr;
    spritetype* pShot = actSpawnSprite(pSpr->sectnum, pSpr->x, pSpr->y, pSpr->z, kStatExplosion, true);
    XSPRITE* pXSpr = pDude->pXSpr;
    int nShot = -1;

    if (pShot)
    {
        int nType = pWeap->id - kTrapExploder;
        XSPRITE* pXShot = &xsprite[pShot->extra];
        EXPLOSION* pExpl = &explodeInfo[nType];
        EXPLOSION_EXTRA* pExtra = &gExplodeExtra[nType];

        pShot->type = nType;
        pShot->cstat |= CSTAT_SPRITE_INVISIBLE;
        pShot->owner = pSpr->index;
        pShot->shade = -127;
        pShot->yrepeat = pShot->xrepeat = pExpl->repeat;
        pShot->ang = pSpr->ang;
        
        pXShot->data1 = pExpl->ticks;
        pXShot->data2 = pExpl->quakeEffect;
        pXShot->data3 = pExpl->flashEffect;
        pXShot->data4 = ClipLow(pWeap->GetDistance() >> 4, pExpl->radius);

        seqSpawn(pExtra->seq, OBJ_SPRITE, pShot->extra, -1);

        if (pExtra->ground)
           pShot->z = getflorzofslope(pShot->sectnum, pShot->x, pShot->y);

        pWeap->shot.appearance.Set(pShot);

        clampSprite(pShot);
        nnExtOffsetSprite(pShot, pOffs->x, pOffs->y, pOffs->z); // offset after default sprite placement
        nShot = pShot->index;
    }

    if (pXSpr->health)
    {
        pXSpr->health = 0; // it supposed to attack once
        pDude->Kill(pSpr->index, kDamageExplode, 0x10000);
    }
    
    return nShot;
}

static int weaponShotSpecialBeastStomp(CUSTOMDUDE* pDude, CUSTOMDUDE_WEAPON* pWeapon, POINT3D*, int, int, int)
{
    spritetype* pSpr = pDude->pSpr;
    
    int i, j;
    int vc = 400 << 4;
    int v1c = 7 * gGameOptions.nDifficulty;
    int v10 = 55 * gGameOptions.nDifficulty;

    char tmp[] = { kStatDude, kStatThing };
    for (i = 0; i < LENGTH(tmp); i++)
    {
        for (j = headspritestat[tmp[i]]; j >= 0; j = nextspritestat[j])
        {
            spritetype* pSpr2 = &sprite[j];
            if (pSpr2->index == pSpr->index || !xsprIsFine(pSpr2) || pSpr2->owner == pSpr->index)
                continue;

            if (CheckProximity(pSpr2, pSpr->x, pSpr->y, pSpr->z, pSpr->sectnum, pWeapon->GetDistance() << 4))
            {
                int dx = klabs(pSpr->x - pSpr2->x);
                int dy = klabs(pSpr->y - pSpr2->y);
                int nDist2 = ksqrt(dx * dx + dy * dy);
                if (nDist2 <= vc)
                {
                    int nDamage;
                    if (!nDist2)
                        nDamage = v1c + v10;
                    else
                        nDamage = v1c + ((vc - nDist2) * v10) / vc;
                        
                    if (IsPlayerSprite(pSpr2))
                    {
                        PLAYER* pPlayer = &gPlayer[pSpr2->type - kDudePlayer1];
                        pPlayer->quakeEffect = ClipHigh(pPlayer->quakeEffect + (nDamage << 2), 1280);
                    }

                    actDamageSprite(pSpr->index, pSpr2, kDamageFall, nDamage << 4);
                }
            }
        }
    }

    return kMaxSprites;
}



static void weaponShot(int, int nXIndex)
{
    if (!xspriRangeIsFine(nXIndex))
        return;

    XSPRITE* pXSpr = &xsprite[nXIndex];
    CUSTOMDUDE* pDude = cdudeGet(pXSpr->reference);
    CUSTOMDUDE_WEAPON *pCurWeap = pDude->pWeapon, *pWeap;
    spritetype* pSpr = pDude->pSpr, *pShot;
    POINT3D shotOffs, *pStyleOffs;

    int nShots, nTime, nCode;
    int dx1, dy1, dz1;
    int dx2, dy2, dz2=0;
    int dx3=0, dy3=0, dz3;
    int i, j;

    int txof; char hxof=0;
    int sang=0; int  hsht;
    int tang=0; char styled;
    

    nnExtCoSin(pSpr->ang, &dx1, &dy1);

    if (pCurWeap)
    {
        for (i = 0; i < pDude->numWeapons; i++)
        {
            pWeap = &pDude->weapons[i];
            if (pWeap->available)
            {
                if (pCurWeap != pWeap)
                {
                    // check if this weapon could be used in conjunction with current
                    if (!pCurWeap->sharedId || pCurWeap->sharedId != pWeap->sharedId)
                        continue;
                }

                nShots = pWeap->GetNumshots(); pWeap->ammo.Dec(nShots);
                styled = (nShots > 1 && pWeap->style.available);
                shotOffs = pWeap->shot.offset;

                if (styled)
                {
                    pStyleOffs = &pWeap->style.offset; hsht = nShots >> 1;
                    sang = pWeap->style.angle / nShots;
                    hxof = 0;
                    tang = 0;
                }

                dz1 = (pWeap->shot.slope == INT32_MAX) ?
                        pDude->AdjustSlope(pXSpr->target, pWeap->shot.offset.z) : pWeap->shot.slope;

                for (j = nShots; j > 0; j--)
                {
                    if (!styled || j == nShots)
                    {
                        dx3 = Random3(pWeap->dispersion[0]);
                        dy3 = Random3(pWeap->dispersion[0]);
                        dz3 = Random3(pWeap->dispersion[1]);

                        dx2 = dx1 + dx3;
                        dy2 = dy1 + dy3;
                        dz2 = dz1 + dz3;
                    }

                    nCode = gWeaponShotFunc[pWeap->type](pDude, pWeap, &shotOffs, dx2, dy2, dz2);
                    if (nCode >= 0)
                    {
                        pShot = (nCode < kMaxSprites) ? &sprite[nCode] : NULL;
                        if (pShot)
                        {
                            // override removal timer
                            if ((nTime = pWeap->shot.remTime) >= 0)
                            {
                                evKill(pShot->index, OBJ_SPRITE, kCallbackRemove);
                                if (nTime)
                                    evPost(pShot->index, OBJ_SPRITE, nTime, kCallbackRemove);
                            }
                        }

                        // setup style
                        if (styled)
                        {
                            if (pStyleOffs->x)
                            {
                                txof = pStyleOffs->x;
                                if (j <= hsht)
                                {
                                    if (!hxof)
                                    {
                                        shotOffs.x = pWeap->shot.offset.x;
                                        hxof = 1;
                                    }

                                    txof = -txof;
                                }

                                shotOffs.x += txof;
                            }

                            shotOffs.y += pStyleOffs->y;
                            shotOffs.z += pStyleOffs->z;

                            if (pWeap->style.angle)
                            {
                                // for sprites
                                if (pShot)
                                {
                                    if (j <= hsht && sang > 0)
                                    {
                                        sang = -sang;
                                        tang = 0;
                                    }

                                    tang += sang;
                                    RotatePoint(&xvel[pShot->index], &yvel[pShot->index], tang, pSpr->x, pSpr->y);
                                    pShot->ang = getVelocityAngle(pShot);
                                }
                                // for hitscan
                                else
                                {
                                    if (j <= hsht && sang > 0)
                                    {
                                        nnExtCoSin(pSpr->ang, &dx2, &dy2);
                                        dx2 += dx3; dy2 += dy3;
                                        sang = -sang;
                                    }
                                    
                                    RotatePoint(&dx2, &dy2, sang, pSpr->x, pSpr->y);
                                }
                            }
                        }
                    }
                }

                pWeap->sound.Play(pSpr);
                if (pWeap->cooldown.Check())
                    pWeap->available = 0;
            }
        }
    }
}

static int checkTarget(CUSTOMDUDE* pDude, spritetype* pTarget, TARGET_INFO* pOut)
{
    spritetype*pSpr = pDude->pSpr;
    if (!xspriRangeIsFine(pTarget->extra))
        return -1;

    XSPRITE* pXTarget = &xsprite[pTarget->extra];
    if (pSpr->owner == pTarget->index || pXTarget->health <= 0)
        return -2;

    if (IsPlayerSprite(pTarget))
    {
        PLAYER* pPlayer = &gPlayer[pTarget->type - kDudePlayer1];
        if (powerupCheck(pPlayer, kPwUpShadowCloak) > 0)
            return -3;
    }

    int x = pTarget->x;
    int y = pTarget->y;
    int z = pTarget->z;
    int nSector = pTarget->sectnum;
    int dx = x - pSpr->x;
    int dy = y - pSpr->y;
    int nDist = approxDist(dx, dy);
    char s = (nDist < pDude->seeDist);
    char h = (nDist < pDude->hearDist);
    
    if (!s && !h)
        return -4;

    DUDEINFO* pInfo = pDude->pInfo;
    if (!cansee(x, y, z, nSector, pSpr->x, pSpr->y, pSpr->z - ((pInfo->eyeHeight * pSpr->yrepeat) << 2), pSpr->sectnum))
        return -5;

    int nAng = getangle(dx, dy);
    if (s)
    {
        int nDang = klabs(((nAng + kAng180 - pSpr->ang) & kAngMask) - kAng180);
        if (nDang <= pDude->periphery)
        {
            pOut->pSpr  = pTarget;
            pOut->pXSpr = pXTarget;
            pOut->nDist = nDist;
            pOut->nDang = nDang;
            pOut->nAng  = nAng;
            pOut->nCode = 1;
            return 1;
        }
    }
    
    if (h)
    {
        pOut->pSpr  = pTarget;
        pOut->pXSpr = pXTarget;
        pOut->nDist = nDist;
        pOut->nDang = 0;
        pOut->nAng  = nAng;
        pOut->nCode = 2;
        return 2;
    }

    return -255;
}

static void thinkTarget(spritetype* pSpr, XSPRITE* pXSpr)
{
    int i; spritetype* pTarget;
    TARGET_INFO targets[kMaxPlayers], *pInfo = targets;
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int numTargets = 0;

    if (Chance(pDude->pInfo->alertChance))
    {
        for (i = connecthead; i >= 0; i = connectpoint2[i])
        {
            PLAYER* pPlayer = &gPlayer[i];
            if (checkTarget(pDude, pPlayer->pSprite, &targets[numTargets]) > 0)
                numTargets++;
        }

        if (numTargets)
        {
            if (numTargets > 1) // closest first
                qsort(targets, numTargets, sizeof(targets[0]), (int(*)(const void*, const void*))qsSortTargets);

            pTarget = pInfo->pSpr;
            if (pDude->pExtra->stats.active)
            {
                if (pXSpr->target != pTarget->index || Chance(0x0400))
                    pDude->PlaySound(kCdudeSndTargetSpot);
            }
            
            pXSpr->goalAng = pInfo->nAng & kAngMask;
            if (pInfo->nCode == 1) aiSetTarget(pXSpr, pTarget->index);
            else aiSetTarget(pXSpr, pTarget->x, pTarget->y, pTarget->z);
            aiActivateDude(pSpr, pXSpr);
        }
    }
}

static void thinkFlee(spritetype* pSpr, XSPRITE* pXSpr)
{
    int nAng = getangle(pSpr->x - pXSpr->targetX, pSpr->y - pXSpr->targetY);
    int nDang = klabs(((nAng + kAng180 - pSpr->ang) & kAngMask) - kAng180);
    if (nDang > kAng45)
        pXSpr->goalAng = (nAng + (kAng15 * Random2(2))) & kAngMask;

    aiChooseDirection(pSpr, pXSpr, pXSpr->goalAng);

}

static void thinkSearch(spritetype* pSpr, XSPRITE* pXSpr)
{
    aiChooseDirection(pSpr, pXSpr, pXSpr->goalAng);
    thinkTarget(pSpr, pXSpr);
}

static void thinkChase(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index); HITINFO* pHit = &gHitInfo; DUDEINFO* pInfo = pDude->pInfo;
    int nDist, nHeigh, dx, dy, nDAng, nSlope = 0;
    char thinkTime = THINK_CLOCK(pSpr->index);
    char turn2target = 0, interrupt = 0;
    char inAttack = pDude->IsAttacking();
    char changePos = 0;

    if (!spriRangeIsFine(pXSpr->target))
    {
        pDude->NewState(kCdudeStateSearch);
        return;
    }

    spritetype* pTarget = &sprite[pXSpr->target];
    if (pTarget->owner == pSpr->index || !IsDudeSprite(pTarget) || !xsprIsFine(pTarget)) // target lost
    {
        pDude->NewState(kCdudeStateSearch);
        return;
    }

    XSPRITE* pXTarget = &xsprite[pTarget->extra];
    if (pXTarget->health <= 0) // target is dead
    {
        PLAYER* pPlayer = NULL;
        if ((!IsPlayerSprite(pTarget)) || ((pPlayer = getPlayerById(pTarget->type)) != NULL && pPlayer->fraggerId == pSpr->index))
            pDude->PlaySound(kCdudeSndTargetDead);
        
        if (inAttack) pDude->NextState(kCdudeStateSearch);
        else pDude->NewState(kCdudeStateSearch);
        return;
    }

    if (IsPlayerSprite(pTarget))
    {
        PLAYER* pPlayer = &gPlayer[pTarget->type - kDudePlayer1];
        if (powerupCheck(pPlayer, kPwUpShadowCloak) > 0)
        {
            pDude->NewState(kCdudeStateSearch);
            return;
        }
    }

    // check target
    dx = pTarget->x - pSpr->x;
    dy = pTarget->y - pSpr->y;
    nDist = approxDist(dx, dy);

    nDAng = klabs(((getangle(dx, dy) + kAng180 - pSpr->ang) & kAngMask) - kAng180);
    nHeigh = (pInfo->eyeHeight * pSpr->yrepeat) << 2;

    if (thinkTime && !inAttack)
        aiChooseDirection(pSpr, pXSpr, getangle(dx, dy));

    // is the target visible?
    if (nDist > pInfo->seeDist || !cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSpr->x, pSpr->y, pSpr->z - nHeigh, pSpr->sectnum))
    {
        if (inAttack) pDude->NextState(kCdudeStateSearch);
        else pDude->NewState(kCdudeStateSearch);
        return;
    }
    else if (nDAng > pInfo->periphery)
    {
        if (inAttack) pDude->NextState(kCdudeStateChase);
        else pDude->NewState(kCdudeStateChase);
        return;
    }

    ARG_PICK_WEAPON* pPickArg = new ARG_PICK_WEAPON(pSpr, pXSpr, pTarget, pXTarget, nDist, nDAng);
    CUSTOMDUDE_WEAPON* pWeapon = pDude->pWeapon;
    if (pWeapon)
    {
        nSlope      = pDude->AdjustSlope(pXSpr->target, pWeapon->shot.offset.z);
        turn2target = pWeapon->turnToTarget;
        interrupt   = pWeapon->interruptable;
    }

    if (thinkTime && Chance(0x2000))
        pDude->PlaySound(kCdudeSndTargetChase);

    // in attack
    if (inAttack)
    {
        if (turn2target)
        {
            pXSpr->goalAng = getTargetAng(pSpr, pXSpr);
            moveTurn(pSpr, pXSpr);
        }

        if (pXSpr->aiState->stateTicks) // attack timer set
        {
            if (!pXSpr->stateTimer)
            {
                pWeapon = pDude->PickWeapon(pPickArg);
                if (pWeapon && pWeapon == pDude->pWeapon)
                {
                    pDude->pWeapon = pWeapon;
                    pDude->NewState(pWeapon->stateID);
                }
                else
                    pDude->NewState(kCdudeStateChase);
            }
            else if (interrupt)
            {
                pDude->PickWeapon(pPickArg);
                if (!pWeapon->available)
                    pDude->NewState(kCdudeStateChase);
            }

            return;
        }

        if (!pDude->SeqPlaying()) // final frame
        {
            pWeapon = pDude->PickWeapon(pPickArg);
            if (!pWeapon)
            {
                pDude->NewState(kCdudeStateChase);
                return;
            }
            else
            {
                pDude->pWeapon = pWeapon;
            }
        }
        else // playing previous animation
        {
            if (!interrupt)
            {
                if (!pWeapon)
                {
                    pDude->NextState(kCdudeStateChase);
                }

                return;
            }
            else
            {
                pDude->PickWeapon(pPickArg);
                if (!pWeapon->available)
                {
                    pDude->NewState(kCdudeStateChase);
                    return;
                }
            }
        }
    }
    else
    {
        // enter attack
        pWeapon = pDude->PickWeapon(pPickArg);
        if (pWeapon)
            pDude->pWeapon = pWeapon;
    }

    if (pWeapon)
    {
        switch (pWeapon->type)
        {
            case kCdudeWeaponNone:
                if (pDude->CanMove()) pDude->NextState(kCdudeStateFlee);
                else pDude->NextState(kCdudeStateSearch);
                return;
            case kCdudeWeaponHitscan:
            case kCdudeWeaponMissile:
            case kCdudeWeaponThrow:
                if (pDude->CanMove())
                {
                    HitScan(pSpr, pSpr->z, dx, dy, nSlope, pWeapon->clipMask, nDist);
                    if (pHit->hitsprite != pXSpr->target && !pDude->AdjustSlope(nDist, &nSlope))
                    {
                        changePos = 1;
                        if (spriRangeIsFine(pHit->hitsprite))
                        {
                            spritetype* pHitSpr = &sprite[pHit->hitsprite];
                            XSPRITE* pXHitSpr = NULL;
                            if (xsprIsFine(pHitSpr))
                                pXHitSpr = &xsprite[pHitSpr->extra];

                            if (IsDudeSprite(pHitSpr))
                            {
                                if (pXHitSpr)
                                {
                                    if (pXHitSpr->target == pSpr->index)
                                        return;

                                    if (pXHitSpr->dodgeDir > 0)
                                        pXSpr->dodgeDir = -pXHitSpr->dodgeDir;
                                }
                            }
                            else if (pHitSpr->owner == pSpr->index) // projectiles, things, fx etc...
                            {
                                if (!pXHitSpr || !pXHitSpr->health)
                                    changePos = 0;
                            }
                            
                            if (changePos)
                            {
                                // prefer dodge
                                if (pDude->dodge.onAimMiss.Allow())
                                {
                                    pDude->NewState(kCdudeStateDodge, 30 * (Random(2) + 1));
                                    return;
                                }
                            }
                        }

                        if (changePos)
                        {
                            // prefer chase
                            pDude->NewState(kCdudeStateChase);
                            return;
                        }
                    }
                }
                fallthrough__;
            default:
                pDude->NewState(pWeapon->stateID);
                pDude->NextState(pWeapon->nextStateID);
                return;
        }
    }

    if (!pDude->CanMove())
        pDude->NextState(kCdudeStateSearch);
}

static int getTargetAng(spritetype* pSpr, XSPRITE* pXSpr)
{
    int x, y;
    if (spriRangeIsFine(pXSpr->target))
    {
        spritetype* pTarg = &sprite[pXSpr->target];
        x = pTarg->x;
        y = pTarg->y;
    }
    else
    {
        x = pXSpr->targetX;
        y = pXSpr->targetY;
    }

    return getangle(x - pSpr->x, y - pSpr->y);
}

static void turnToTarget(spritetype* pSpr, XSPRITE* pXSpr)
{
    pSpr->ang = getTargetAng(pSpr, pXSpr);
    pXSpr->goalAng = pSpr->ang;
}

static void moveTurn(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr);
    int nVelTurn = pDude->GetVelocity(kParVelocityTurn);
    int nAng = ((kAng180 + pXSpr->goalAng - pSpr->ang) & kAngMask) - kAng180;
    pSpr->ang = ((pSpr->ang + ClipRange(nAng, -nVelTurn, nVelTurn)) & kAngMask);
}

static void moveDodge(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    moveTurn(pSpr, pXSpr);

    if (pXSpr->dodgeDir && pDude->CanMove())
    {
        int nVelDodge = pDude->GetVelocity(kParVelocityDodge);
        int nCos = Cos(pSpr->ang);                  int nSin = Sin(pSpr->ang);
        int dX = xvel[pSpr->index];                 int dY = yvel[pSpr->index];
        int t1 = dmulscale30(dX, nCos, dY, nSin);   int t2 = dmulscale30(dX, nSin, -dY, nCos);

        if (pXSpr->dodgeDir > 0)
        {
            t2 += nVelDodge;
        }
        else
        {
            t2 -= nVelDodge;
        }

        xvel[pSpr->index] = dmulscale30(t1, nCos, t2, nSin);
        yvel[pSpr->index] = dmulscale30(t1, nSin, -t2, nCos);
    }
}

static void moveKnockout(spritetype* pSpr, XSPRITE*)
{
    int zv = zvel[pSpr->index];
    zvel[pSpr->index] = ClipRange(zv + mulscale16(zv, 0x3000), 0x1000, 0x40000);
}

static void moveForward(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    int nVelTurn    = pDude->GetVelocity(kParVelocityTurn);
    int nVelForward = pDude->GetVelocity(kParVelocityForward);
    int nAng = ((kAng180 + pXSpr->goalAng - pSpr->ang) & kAngMask) - kAng180;
    pSpr->ang = ((pSpr->ang + ClipRange(nAng, -nVelTurn, nVelTurn)) & kAngMask);
    int z = 0;

    if (pDude->CanMove())
    {
        if (pDude->IsUnderwater())
        {
            if (spriRangeIsFine(pXSpr->target))
            {
                spritetype* pTarget = &sprite[pXSpr->target];
                if (spriteIsUnderwater(pTarget, true))
                    z = (pTarget->z - pSpr->z) + (10 << Random(12));
            }
            else
            {
                z = (pXSpr->targetZ - pSpr->z);
            }

            if (Chance(0x0500))
                z <<= 1;

            zvel[pSpr->index] += z;
        }
        
        // don't move forward if trying to turn around
        if (klabs(nAng) <= kAng60)
        {
            xvel[pSpr->index] += mulscale30(Cos(pSpr->ang), nVelForward);
            yvel[pSpr->index] += mulscale30(Sin(pSpr->ang), nVelForward);
        }
    }
}

static void enterSleep(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    pDude->StatusSet(kCdudeStatusSleep);
    resetTarget(pSpr, pXSpr);
    moveStop(pSpr, pXSpr);

    // reduce distances while sleeping
    pDude->seeDist      = kCdudeMinSeeDist;
    pDude->hearDist     = kCdudeMinHearDist;
    pDude->periphery    = kAng360;
}

static void enterWake(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    if (pDude->StatusTest(kCdudeStatusSleep))
    {
        pDude->StatusRem(kCdudeStatusSleep);

        // restore distances when awaked
        pDude->seeDist      = pDude->pInfo->seeDist;
        pDude->hearDist     = pDude->pInfo->hearDist;
        pDude->periphery    = pDude->pInfo->periphery;
    }

    pDude->PlaySound(kCdudeSndWake);
}


static void enterDying(spritetype* pSpr, XSPRITE*)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    if (pDude->mass > 48)
        pDude->mass = ClipLow(pDude->mass >> 2, 48);
}

static void thinkDying(spritetype* pSpr, XSPRITE*)
{
    SPRITEHIT* pHit = &gSpriteHit[pSpr->extra];
    if (!pHit->florhit && spriteIsUnderwater(pSpr, true))
        zvel[pSpr->index] = ClipLow(zvel[pSpr->index], 0x40000);
}

static void enterDeath(spritetype* pSpr, XSPRITE*)
{
    // don't let the data fields gets overwritten!
    if (!(pSpr->flags & kHitagRespawn))
        DudeToGibCallback1(pSpr->index, pSpr->extra);

    pSpr->type = kThingBloodChunks;
    actPostSprite(pSpr->index, kStatThing);
}

static void enterMorph(spritetype* pSpr, XSPRITE* pXSpr)
{
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    if (!pDude->IsMorphing())
    {
        pDude->PlaySound(kCdudeSndTransforming);
        pDude->StatusSet(kCdudeStatusMorph); // set morph status
        pXSpr->locked = 1; // lock it while morphing

        pSpr->flags &= ~kPhysMove;
        moveStop(pSpr, pXSpr);
        if (pXSpr->aiState->seqId <= 0)
            seqKill(OBJ_SPRITE, pSpr->extra);
    }
}

static void thinkMorph(spritetype* pSpr, XSPRITE* pXSpr)
{
    int nTarget; char triggerOn, triggerOff;
    CUSTOMDUDE* pDude = cdudeGet(pSpr->index);
    spritetype* pNext = NULL; XSPRITE* pXNext = NULL;
    int nextDude = pDude->nextDude;

    if (pDude->SeqPlaying())
    {
        moveStop(pSpr, pXSpr);
        return;
    }
    
    pDude->ClearEffectCallbacks();
    pDude->StatusRem(kCdudeStatusMorph);    // clear morph status
    pXSpr->burnSource = -1;
    pXSpr->burnTime = 0;
    pXSpr->locked = 0;
    pXSpr->scale = 0;

    if (rngok(nextDude, 0, kMaxSprites))
    {
        // classic morphing to already inserted sprite by TX ID
        pNext  = &sprite[nextDude]; pXNext = &xsprite[pNext->extra];

        pXSpr->key = pXSpr->dropMsg = 0;

        // save incarnation's going on and off options
        triggerOn = pXNext->triggerOn, triggerOff = pXNext->triggerOff;

        // then remove it from incarnation so it won't send the commands
        pXNext->triggerOn = pXNext->triggerOff = 0;

        // trigger dude death before morphing
        trTriggerSprite(pSpr->index, pXSpr, kCmdOff, pSpr->index);

        pSpr->type = pSpr->inittype = pNext->type;
        pSpr->flags = pNext->flags;
        pSpr->pal = pNext->pal;
        pSpr->shade = pNext->shade;
        pSpr->clipdist = pNext->clipdist;
        pSpr->xrepeat = pNext->xrepeat;
        pSpr->yrepeat = pNext->yrepeat;

        pXSpr->txID = pXNext->txID;
        pXSpr->command = pXNext->command;
        pXSpr->triggerOn = triggerOn;
        pXSpr->triggerOff = triggerOff;
        pXSpr->busyTime = pXNext->busyTime;
        pXSpr->waitTime = pXNext->waitTime;

        // inherit respawn properties
        pXSpr->respawn = pXNext->respawn;
        pXSpr->respawnPending = pXNext->respawnPending;

        pXSpr->data1    = pXNext->data1;                        // for v1 this is weapon id, v2 - descriptor id
        pXSpr->data2    = pXNext->data2;                        // for v1 this is seqBase id
        pXSpr->data3    = pXSpr->sysData1 = pXNext->sysData1;   // for v1 this is soundBase id
        pXSpr->data4    = pXSpr->sysData2 = pXNext->sysData2;   // start hp

        // inherit dude flags
        pXSpr->dudeGuard = pXNext->dudeGuard;
        pXSpr->dudeDeaf = pXNext->dudeDeaf;
        pXSpr->dudeAmbush = pXNext->dudeAmbush;
        pXSpr->dudeFlag4 = pXNext->dudeFlag4;
        pXSpr->unused1 = pXNext->unused1;

        pXSpr->dropMsg = pXNext->dropMsg;
        pXSpr->key = pXNext->key;

        pXSpr->Decoupled = pXNext->Decoupled;
        pXSpr->locked = pXNext->locked;

        // set health
        pXSpr->health = nnExtDudeStartHealth(pSpr, pXSpr->data4);

        // restore values for incarnation
        pXNext->triggerOn = triggerOn;
        pXNext->triggerOff = triggerOff;
    }
    else
    {
        // v2 morphing
        if (nextDude >= kMaxSprites)
        {
            // morph to another custom dude
            pXSpr->data1 = nextDude - kMaxSprites;
        }
        else if (nextDude < -1)
        {
            // morph to some vanilla dude
            pSpr->type      = klabs(nextDude) - 1;
            pSpr->clipdist  = getDudeInfo(pSpr->type)->clipdist;
            pXSpr->data1    = 0;
        }

        pSpr->inittype  = pSpr->type;
        pXSpr->health   = nnExtDudeStartHealth(pSpr, 0);
        pXSpr->data4    = pXSpr->sysData2 = 0;
        pXSpr->data2    = 0;
        pXSpr->data3    = 0;
    }

    // clear init status
    pDude->initialized = 0;

    nTarget = pXSpr->target;        // save target
    aiInitSprite(pSpr);             // re-init sprite with all new settings

    switch (pSpr->type)
    {
        case kDudePodMother:        // fake dude
        case kDudeTentacleMother:   // fake dude
            break;
        default:
            if (pXSpr->dudeFlag4) break;
            else if (spriRangeIsFine(nTarget)) aiSetTarget(pXSpr, nTarget); // try to restore target
            else aiSetTarget(pXSpr, pSpr->x, pSpr->y, pSpr->z);
            aiActivateDude(pSpr, pXSpr); // finally activate it
            break;
    }
}

// get closest visible underwater sector it can fall in
static void enterBurnSearchWater(spritetype* pSpr, XSPRITE* pXSpr)
{
    int i = numsectors;
    int nClosest = 0x7FFFFF;
    int nDist, s, e;

    int x1 = pSpr->x;
    int y1 = pSpr->y;
    int z1, z2;
    int x2, y2;

    pXSpr->aiState->thinkFunc = NULL;
    if (!Chance(0x8000))
    {
        pXSpr->aiState->thinkFunc = thinkSearch; // try follow to the target
        return;
    }

    GetSpriteExtents(pSpr, &z1, &z2);

    while (--i >= 0)
    {
        if (gUpperLink[i] < 0)
            continue;

        spritetype* pUp = &sprite[gUpperLink[i]];
        if (!spriRangeIsFine(pUp->owner))
            continue;

        spritetype* pLow = &sprite[pUp->owner];
        if (sectRangeIsFine(pLow->sectnum) && isUnderwaterSector(pLow->sectnum))
        {
            s = sector[i].wallptr;
            e = s + sector[i].wallnum;

            while (--e >= s)
            {
                x2 = wall[e].x; y2 = wall[e].y;
                if (!cansee(x1, y1, z1, pSpr->sectnum, x2, y2, z1, i))
                    continue;

                if ((nDist = approxDist(x1 - x2, y1 - y2)) < nClosest)
                {
                    x2 = (x2 + wall[wall[e].point2].x) >> 1;
                    y2 = (y2 + wall[wall[e].point2].y) >> 1;
                    pXSpr->goalAng = getangle(x2 - x1, y2 - y1) & kAngMask;
                    nClosest = nDist;
                }
            }
        }
    }

    if (Chance(0xB000) && spriRangeIsFine(pXSpr->target))
    {
        spritetype* pTarget = &sprite[pXSpr->target];
        x2 = pTarget->x;
        y2 = pTarget->y;

        if (approxDist(x1 - x2, y1 - y2) < nClosest)  // water sector is not closer than target
        {
            pXSpr->goalAng = getangle(x2 - x1, y2 - y1) & kAngMask;
            pXSpr->aiState->thinkFunc = thinkSearch; // try follow to the target
            return;
        }
    }
}

void cdudeDoExplosion(CUSTOMDUDE* pDude)
{
    CUSTOMDUDE_WEAPON* pWeap = pDude->pWeapon;
    if (pWeap && pWeap->type == kCdudeWeaponKamikaze)
        weaponShotKamikaze(pDude, pWeap, &pWeap->shot.offset, 0, 0, 0);
}

#endif

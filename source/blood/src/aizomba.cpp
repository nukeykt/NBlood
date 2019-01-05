#include "compat.h"
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "aizomba.h"
#include "blood.h"
#include "db.h"
#include "dude.h"
#include "eventq.h"
#include "levels.h"
#include "player.h"
#include "seq.h"
#include "sfx.h"
#include "trig.h"

static void HackSeqCallback(int, int);
static void StandSeqCallback(int, int);
static void thinkSearch(SPRITE *, XSPRITE *);
static void thinkGoto(SPRITE *, XSPRITE *);
static void thinkChase(SPRITE *, XSPRITE *);
static void thinkPonder(SPRITE *, XSPRITE *);
static void myThinkTarget(SPRITE *, XSPRITE *);
static void myThinkSearch(SPRITE *, XSPRITE *);
static void entryEZombie(SPRITE *, XSPRITE *);
static void entryAIdle(SPRITE *, XSPRITE *);
static void entryEStand(SPRITE *, XSPRITE *);

static int nHackClient = seqRegisterClient(HackSeqCallback);
static int nStandClient = seqRegisterClient(StandSeqCallback);

AISTATE zombieAIdle = { 0, -1, 0, entryAIdle, NULL, aiThinkTarget, NULL };
AISTATE zombieAChase = { 8, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE zombieAPonder = { 0, -1, 0, NULL, aiMoveTurn, thinkPonder, NULL };
AISTATE zombieAGoto = { 8, -1, 1800, NULL, aiMoveForward, thinkGoto, &zombieAIdle };
AISTATE zombieAHack = { 6, nHackClient, 80, NULL, NULL, NULL, &zombieAPonder };
AISTATE zombieASearch = { 8, -1, 1800, NULL, aiMoveForward, thinkSearch, &zombieAIdle };
AISTATE zombieARecoil = { 5, -1, 0, NULL, NULL, NULL, &zombieAPonder };
AISTATE zombieATeslaRecoil = { 4, -1, 0, NULL, NULL, NULL, &zombieAPonder };
AISTATE zombieARecoil2 = { 1, -1, 360, NULL, NULL, NULL, &zombieAStand };
AISTATE zombieAStand = { 11, nStandClient, 0, NULL, NULL, NULL, &zombieAPonder };
AISTATE zombieEIdle = { 12, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE zombieEUp2 = { 0, -1, 1, entryEZombie, NULL, NULL, &zombieASearch };
AISTATE zombieEUp = { 9, -1, 180, entryEStand, NULL, NULL, &zombieEUp2 };
AISTATE zombie2Idle = { 0, -1, 0, entryAIdle, NULL, myThinkTarget, NULL };
AISTATE zombie2Search = { 8, -1, 1800, NULL, NULL, myThinkSearch, &zombie2Idle };
AISTATE zombieSIdle = { 10, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE zombie13AC2C = { 11, nStandClient, 0, entryEZombie, NULL, NULL, &zombieAPonder };

static void HackSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    SPRITE *pSprite = &qsprite[nSprite];
    SPRITE *pTarget = &qsprite[pXSprite->target];
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
    DUDEINFO *pDudeInfoT = &dudeInfo[pTarget->type-kDudeBase];
    int tx = pXSprite->at20_0-pSprite->x;
    int ty = pXSprite->at24_0-pSprite->y;
    int nDist = approxDist(tx, ty);
    int nAngle = getangle(tx, ty);
    int height = (pSprite->yrepeat*pDudeInfo->atb)<<2;
    int height2 = (pTarget->yrepeat*pDudeInfoT->atb)<<2;
    int dz = height-height2;
    int dx = Cos(nAngle)>>16;
    int dy = Sin(nAngle)>>16;
    sfxPlay3DSound(pSprite, 1101, 1, 0);
    actFireVector(pSprite, 0, 0, dx, dy, dz, VECTOR_TYPE_10);
}

static void StandSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    sfxPlay3DSound(&qsprite[nSprite], 1102, -1, 0);
}

static void thinkSearch(SPRITE *pSprite, XSPRITE *pXSprite)
{
    aiChooseDirection(pSprite, pXSprite, pXSprite->at16_0);
    sub_5F15C(pSprite, pXSprite);
}

static void thinkGoto(SPRITE *pSprite, XSPRITE *pXSprite)
{
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int dx = pXSprite->at20_0-pSprite->x;
    int dy = pXSprite->at24_0-pSprite->y;
    int nAngle = getangle(dx, dy);
    int nDist = approxDist(dx, dy);
    aiChooseDirection(pSprite, pXSprite, nAngle);
    if (nDist < 921 && klabs(pSprite->ang - nAngle) < pDudeInfo->at1b)
        aiNewState(pSprite, pXSprite, &zombieASearch);
    aiThinkTarget(pSprite, pXSprite);
}

static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        aiNewState(pSprite, pXSprite, &zombieASearch);
        return;
    }
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    SPRITE *pTarget = &qsprite[pXSprite->target];
    XSPRITE *pXTarget = &xsprite[pTarget->extra];
    int dx = pTarget->x-pSprite->x;
    int dy = pTarget->y-pSprite->y;
    aiChooseDirection(pSprite, pXSprite, getangle(dx, dy));
    if (pXTarget->health == 0)
    {
        aiNewState(pSprite, pXSprite, &zombieASearch);
        return;
    }
    if (IsPlayerSprite(pTarget) && (powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 13) > 0 || powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 31) > 0))
    {
        aiNewState(pSprite, pXSprite, &zombieAGoto);
        return;
    }
    int nDist = approxDist(dx, dy);
    if (nDist <= pDudeInfo->at17)
    {
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
        if (cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSprite->x, pSprite->y, pSprite->z - height, pSprite->sectnum))
        {
            if (klabs(nDeltaAngle) <= pDudeInfo->at1b)
            {
                aiSetTarget(pXSprite, pXSprite->target);
                if (nDist < 0x400 && klabs(nDeltaAngle) < 85)
                    aiNewState(pSprite, pXSprite, &zombieAHack);
                return;
            }
        }
    }

    aiNewState(pSprite, pXSprite, &zombieAGoto);
    pXSprite->target = -1;
}

static void thinkPonder(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        aiNewState(pSprite, pXSprite, &zombieASearch);
        return;
    }
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    SPRITE *pTarget = &qsprite[pXSprite->target];
    XSPRITE *pXTarget = &xsprite[pTarget->extra];
    int dx = pTarget->x-pSprite->x;
    int dy = pTarget->y-pSprite->y;
    aiChooseDirection(pSprite, pXSprite, getangle(dx, dy));
    if (pXTarget->health == 0)
    {
        aiNewState(pSprite, pXSprite, &zombieASearch);
        return;
    }
    if (IsPlayerSprite(pTarget) && (powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 13) > 0 || powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 31) > 0))
    {
        aiNewState(pSprite, pXSprite, &zombieAGoto);
        return;
    }
    int nDist = approxDist(dx, dy);
    if (nDist <= pDudeInfo->at17)
    {
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
        if (cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSprite->x, pSprite->y, pSprite->z - height, pSprite->sectnum))
        {
            if (klabs(nDeltaAngle) <= pDudeInfo->at1b)
            {
                aiSetTarget(pXSprite, pXSprite->target);
                if (nDist < 0x400 && klabs(nDeltaAngle) < 85)
                {
                    sfxPlay3DSound(pSprite, 1101, 1, 0);
                    aiNewState(pSprite, pXSprite, &zombieAHack);
                    return;
                }
            }
        }
    }

    aiNewState(pSprite, pXSprite, &zombieAChase);
}

static void myThinkTarget(SPRITE *pSprite, XSPRITE *pXSprite)
{
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
    for (int p = connecthead; p >= 0; p = connectpoint2[p])
    {
        PLAYER *pPlayer = &gPlayer[p];
        int nOwner = (pSprite->owner & 0x1000) ? (pSprite->owner&0xfff) : -1;
        if (nOwner == pPlayer->at5b || pPlayer->pXSprite->health == 0 || powerupCheck(pPlayer, 13) > 0)
            continue;
        int x = pPlayer->pSprite->x;
        int y = pPlayer->pSprite->y;
        int z = pPlayer->pSprite->z;
        int nSector = pPlayer->pSprite->sectnum;
        int dx = x-pSprite->x;
        int dy = y-pSprite->y;
        int nDist = approxDist(dx, dy);
        if (nDist > pDudeInfo->at17 && nDist > pDudeInfo->at13)
            continue;
        if (!cansee(x, y, z, nSector, pSprite->x, pSprite->y, pSprite->z-((pDudeInfo->atb*pSprite->yrepeat)<<2), pSprite->sectnum))
            continue;
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        if (nDist < pDudeInfo->at17 && klabs(nDeltaAngle) <= pDudeInfo->at1b)
        {
            aiSetTarget(pXSprite, pPlayer->at5b);
            aiActivateDude(pSprite, pXSprite);
        }
        else if (nDist < pDudeInfo->at13)
        {
            aiSetTarget(pXSprite, x, y, z);
            aiActivateDude(pSprite, pXSprite);
        }
        else
            continue;
        break;
    }
}

static void myThinkSearch(SPRITE *pSprite, XSPRITE *pXSprite)
{
    aiChooseDirection(pSprite, pXSprite, pXSprite->at16_0);
    myThinkTarget(pSprite, pXSprite);
}

static void entryEZombie(SPRITE *pSprite, XSPRITE *pXSprite)
{
    pSprite->type = 203;
    pSprite->hitag |= 1;
}

static void entryAIdle(SPRITE *pSprite, XSPRITE *pXSprite)
{
    pXSprite->target = -1;
}

static void entryEStand(SPRITE *pSprite, XSPRITE *pXSprite)
{
    sfxPlay3DSound(pSprite, 1100, -1, 0);
    pSprite->ang = getangle(pXSprite->at20_0-pSprite->x, pXSprite->at24_0-pSprite->y);
}

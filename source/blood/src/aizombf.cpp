#include "compat.h"
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "aizombf.h"
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
static void PukeSeqCallback(int, int);
static void ThrowSeqCallback(int, int);
static void thinkSearch(SPRITE *pSprite, XSPRITE *pXSprite);
static void thinkGoto(SPRITE *pSprite, XSPRITE *pXSprite);
static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite);

static int nHackClient = seqRegisterClient(HackSeqCallback);
static int nPukeClient = seqRegisterClient(PukeSeqCallback);
static int nThrowClient = seqRegisterClient(ThrowSeqCallback);

AISTATE zombieFIdle = { 0, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE zombieFChase = { 8, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE zombieFGoto = { 8, -1, 600, NULL, aiMoveForward, thinkGoto, &zombieFIdle };
AISTATE zombieFDodge = { 8, -1, 0, NULL, aiMoveDodge, thinkChase, &zombieFChase };
AISTATE zombieFHack = { 6, nHackClient, 120, NULL, NULL, NULL, &zombieFChase };
AISTATE zombieFPuke = { 9, nPukeClient, 120, NULL, NULL, NULL, &zombieFChase };
AISTATE zombieFThrow = { 6, nThrowClient, 120, NULL, NULL, NULL, &zombieFChase };
AISTATE zombieFSearch = { 8, -1, 1800, NULL, aiMoveForward, thinkSearch, &zombieFIdle };
AISTATE zombieFRecoil = { 5, -1, 0, NULL, NULL, NULL, &zombieFChase };
AISTATE zombieFTeslaRecoil = { 4, -1, 0, NULL, NULL, NULL, &zombieFChase };

static void HackSeqCallback(int, int nXSprite)
{
    if (nXSprite <= 0 || nXSprite >= kMaxXSprites)
        return;
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    if (nXSprite < 0 || nXSprite >= kMaxSprites)
        return;
    SPRITE *pSprite = &qsprite[nSprite];
    if (pSprite->type != 204)
        return;
    SPRITE *pTarget = &qsprite[pXSprite->target];
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
    int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
    DUDEINFO *pDudeInfoT = &dudeInfo[pTarget->type-kDudeBase];
    int height2 = (pDudeInfoT->atb*pTarget->yrepeat)<<2;
    actFireVector(pSprite, 0, 0, Cos(pSprite->ang)>>16, Sin(pSprite->ang)>>16, height-height2, VECTOR_TYPE_11);
}

static void PukeSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    SPRITE *pSprite = &qsprite[nSprite];
    SPRITE *pTarget = &qsprite[pXSprite->target];
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
    DUDEINFO *pDudeInfoT = &dudeInfo[pTarget->type-kDudeBase];
    int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
    int height2 = (pDudeInfoT->atb*pTarget->yrepeat)<<2;
    int tx = pXSprite->at20_0-pSprite->x;
    int ty = pXSprite->at24_0-pSprite->y;
    int nDist = approxDist(tx, ty);
    int nAngle = getangle(tx, ty);
    int dx = Cos(nAngle)>>16;
    int dy = Sin(nAngle)>>16;
    sfxPlay3DSound(pSprite, 1203, 1, 0);
    actFireMissile(pSprite, 0, -(height-height2), dx, dy, 0, 309);
}

static void ThrowSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    SPRITE *pSprite = &qsprite[nSprite];
    actFireMissile(pSprite, 0, dudeInfo[pSprite->type].atb, Cos(pSprite->ang)>>16, Sin(pSprite->ang)>>16, 0, 300);
}

static void thinkSearch(SPRITE *pSprite, XSPRITE *pXSprite)
{
    aiChooseDirection(pSprite, pXSprite, pXSprite->at16_0);
    aiThinkTarget(pSprite, pXSprite);
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
    if (nDist < 512 && klabs(pSprite->ang - nAngle) < pDudeInfo->at1b)
        aiNewState(pSprite, pXSprite, &zombieFSearch);
    aiThinkTarget(pSprite, pXSprite);
}

static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        aiNewState(pSprite, pXSprite, &zombieFGoto);
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
        aiNewState(pSprite, pXSprite, &zombieFSearch);
        return;
    }
    if (IsPlayerSprite(pTarget) && (powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 13) > 0 || powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 31) > 0))
    {
        aiNewState(pSprite, pXSprite, &zombieFSearch);
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
                if (nDist < 0x1400 && nDist > 0xe00 && klabs(nDeltaAngle) < 85)
                {
                    int hit = HitScan(pSprite, pSprite->z, dx, dy, 0, CLIPMASK1, 0);
                    switch (hit)
                    {
                    case -1:
                        aiNewState(pSprite, pXSprite, &zombieFThrow);
                        break;
                    case 3:
                        if (pSprite->type != qsprite[gHitInfo.hitsprite].type)
                            aiNewState(pSprite, pXSprite, &zombieFThrow);
                        else
                            aiNewState(pSprite, pXSprite, &zombieFDodge);
                        break;
                    default:
                        aiNewState(pSprite, pXSprite, &zombieFThrow);
                        break;
                    }
                }
                else if (nDist < 0x1400 && nDist > 0x600 && klabs(nDeltaAngle) < 85)
                {
                    int hit = HitScan(pSprite, pSprite->z, dx, dy, 0, CLIPMASK1, 0);
                    switch (hit)
                    {
                    case -1:
                        aiNewState(pSprite, pXSprite, &zombieFPuke);
                        break;
                    case 3:
                        if (pSprite->type != qsprite[gHitInfo.hitsprite].type)
                            aiNewState(pSprite, pXSprite, &zombieFPuke);
                        else
                            aiNewState(pSprite, pXSprite, &zombieFDodge);
                        break;
                    default:
                        aiNewState(pSprite, pXSprite, &zombieFPuke);
                        break;
                    }
                }
                else if (nDist < 0x400 && klabs(nDeltaAngle) < 85)
                {
                    int hit = HitScan(pSprite, pSprite->z, dx, dy, 0, CLIPMASK1, 0);
                    switch (hit)
                    {
                    case -1:
                        aiNewState(pSprite, pXSprite, &zombieFHack);
                        break;
                    case 3:
                        if (pSprite->type != qsprite[gHitInfo.hitsprite].type)
                            aiNewState(pSprite, pXSprite, &zombieFHack);
                        else
                            aiNewState(pSprite, pXSprite, &zombieFDodge);
                        break;
                    default:
                        aiNewState(pSprite, pXSprite, &zombieFHack);
                        break;
                    }
                }
                return;
            }
        }
    }

    aiNewState(pSprite, pXSprite, &zombieFSearch);
    pXSprite->target = -1;
}

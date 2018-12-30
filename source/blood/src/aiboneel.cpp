
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "aiboneel.h"
#include "blood.h"
#include "db.h"
#include "dude.h"
#include "levels.h"
#include "player.h"
#include "seq.h"
#include "sfx.h"
#include "trig.h"

static void BiteSeqCallback(int, int);
static void thinkTarget(SPRITE *, XSPRITE *);
static void thinkSearch(SPRITE *, XSPRITE *);
static void thinkGoto(SPRITE *, XSPRITE *);
static void thinkPonder(SPRITE *, XSPRITE *);
static void MoveDodgeUp(SPRITE *, XSPRITE *);
static void MoveDodgeDown(SPRITE *, XSPRITE *);
static void thinkChase(SPRITE *, XSPRITE *);
static void MoveForward(SPRITE *, XSPRITE *);
static void MoveSwoop(SPRITE *, XSPRITE *);
static void MoveAscend(SPRITE *pSprite, XSPRITE *pXSprite);
static void MoveFly(SPRITE *, XSPRITE *);
static void MoveToCeil(SPRITE *, XSPRITE *);

static int nBiteClient = seqRegisterClient(BiteSeqCallback);

AISTATE eelIdle = { 0, -1, 0, NULL, NULL, thinkTarget, NULL };
AISTATE eelFlyIdle = { 0, -1, 0, NULL, NULL, thinkTarget, NULL };
AISTATE eelChase = { 0, -1, 0, NULL, MoveForward, thinkTarget, &eelIdle };
AISTATE eelPonder = { 0, -1, 0, NULL, NULL, thinkPonder, NULL };
AISTATE eelGoto = { 0, -1, 600, NULL, NULL, thinkGoto, &eelIdle };
AISTATE eelBite = { 7, nBiteClient, 60, NULL, NULL, NULL, &eelChase };
AISTATE eelRecoil = { 5, -1, 0, NULL, NULL, NULL, &eelChase };
AISTATE eelSearch = { 0, -1, 120, NULL, MoveForward, thinkSearch, &eelIdle };
AISTATE eelSwoop = { 0, -1, 60, NULL, MoveSwoop, thinkChase, &eelChase };
AISTATE eelFly = { 0, -1, 0, NULL, MoveAscend, thinkChase, &eelChase };
AISTATE eelTurn = { 0, -1, 60, NULL, aiMoveTurn, NULL, &eelChase };
AISTATE eelHide = { 0, -1, 0, NULL, MoveToCeil, MoveForward, NULL };
AISTATE eelDodgeUp = { 0, -1, 120, NULL, MoveDodgeUp, NULL, &eelChase };
AISTATE eelDodgeUpRight = { 0, -1, 90, NULL, MoveDodgeUp, NULL, &eelChase };
AISTATE eelDodgeUpLeft = { 0, -1, 90, NULL, MoveDodgeUp, NULL, &eelChase };
AISTATE eelDodgeDown = { 0, -1, 120, NULL, MoveDodgeDown, NULL, &eelChase };
AISTATE eelDodgeDownRight = { 0, -1, 90, NULL, MoveDodgeDown, NULL, &eelChase };
AISTATE eelDodgeDownLeft = { 0, -1, 90, NULL, MoveDodgeDown, NULL, &eelChase };

static void BiteSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    SPRITE *pSprite = &qsprite[pXSprite->reference];
    SPRITE *pTarget = &qsprite[pXSprite->target];
    int dx = Cos(pSprite->ang) >> 16;
    int dy = Sin(pSprite->ang) >> 16;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
    DUDEINFO *pDudeInfoT = &dudeInfo[pTarget->type-kDudeBase];
    int height = (pSprite->yrepeat*pDudeInfo->atb)<<2;
    int height2 = (pTarget->yrepeat*pDudeInfoT->atb)<<2;
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    actFireVector(pSprite, 0, 0, dx, dy, height2-height, VECTOR_TYPE_7);
}

static void thinkTarget(SPRITE *pSprite, XSPRITE *pXSprite)
{
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
    DUDEEXTRA_at6_u1 *pDudeExtraE = &gDudeExtra[pSprite->extra].at6.u1;
    if (pDudeExtraE->at8 && pDudeExtraE->at4 < 10)
        pDudeExtraE->at4++;
    else if (pDudeExtraE->at4 >= 10 && pDudeExtraE->at8)
    {
        pDudeExtraE->at4 = 0;
        pXSprite->at16_0 += 256;
        POINT3D *pTarget = &baseSprite[pSprite->index];
        aiSetTarget(pXSprite, pTarget->x, pTarget->y, pTarget->z);
        aiNewState(pSprite, pXSprite, &eelTurn);
        return;
    }
    if (Chance(pDudeInfo->at33))
    {
        for (int p = connecthead; p >= 0; p = connectpoint2[p])
        {
            PLAYER *pPlayer = &gPlayer[p];
            if (pPlayer->pXSprite->health == 0 || powerupCheck(pPlayer, 13) > 0)
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
                pDudeExtraE->at4 = 0;
                aiSetTarget(pXSprite, pPlayer->at5b);
                aiActivateDude(pSprite, pXSprite);
            }
            else if (nDist < pDudeInfo->at13)
            {
                pDudeExtraE->at4 = 0;
                aiSetTarget(pXSprite, x, y, z);
                aiActivateDude(pSprite, pXSprite);
            }
            else
                continue;
            break;
        }
    }
}

static void thinkSearch(SPRITE *pSprite, XSPRITE *pXSprite)
{
    aiChooseDirection(pSprite, pXSprite, pXSprite->at16_0);
    thinkTarget(pSprite, pXSprite);
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
        aiNewState(pSprite, pXSprite, &eelSearch);
    thinkTarget(pSprite, pXSprite);
}

static void thinkPonder(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        aiNewState(pSprite, pXSprite, &eelSearch);
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
        aiNewState(pSprite, pXSprite, &eelSearch);
        return;
    }
    int nDist = approxDist(dx, dy);
    if (nDist <= pDudeInfo->at17)
    {
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
        int height2 = (dudeInfo[pTarget->type-kDudeBase].atb*pTarget->yrepeat)<<2;
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        if (cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSprite->x, pSprite->y, pSprite->z - height, pSprite->sectnum))
        {
            aiSetTarget(pXSprite, pXSprite->target);
            if (height2-height < -0x2000 && nDist < 0x1800 && nDist > 0xc00 && klabs(nDeltaAngle) < 85)
                aiNewState(pSprite, pXSprite, &eelDodgeUp);
            else if (height2-height > 0xccc && nDist < 0x1800 && nDist > 0xc00 && klabs(nDeltaAngle) < 85)
                aiNewState(pSprite, pXSprite, &eelDodgeDown);
            else if (height2-height < 0xccc && nDist < 0x399 && klabs(nDeltaAngle) < 85)
                aiNewState(pSprite, pXSprite, &eelDodgeUp);
            else if (height2-height > 0xccc && nDist < 0x1400 && nDist > 0x800 && klabs(nDeltaAngle) < 85)
                aiNewState(pSprite, pXSprite, &eelDodgeDown);
            else if (height2-height < -0x2000 && nDist < 0x1400 && nDist > 0x800 && klabs(nDeltaAngle) < 85)
                aiNewState(pSprite, pXSprite, &eelDodgeUp);
            else if (height2-height < -0x2000 && klabs(nDeltaAngle) < 85 && nDist > 0x1400)
                aiNewState(pSprite, pXSprite, &eelDodgeUp);
            else if (height2-height > 0xccc)
                aiNewState(pSprite, pXSprite, &eelDodgeDown);
            else
                aiNewState(pSprite, pXSprite, &eelDodgeUp);
            return;
        }
    }
    aiNewState(pSprite, pXSprite, &eelGoto);
    pXSprite->target = -1;
}

static void MoveDodgeUp(SPRITE *pSprite, XSPRITE *pXSprite)
{
    int nSprite = pSprite->index;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int nAng = ((pXSprite->at16_0+1024-pSprite->ang)&2047)-1024;
    int nTurnRange = (pDudeInfo->at44<<2)>>4;
    pSprite->ang = (pSprite->ang+ClipRange(nAng, -nTurnRange, nTurnRange))&2047;
    int nCos = Cos(pSprite->ang);
    int nSin = Sin(pSprite->ang);
    int dx = xvel[nSprite];
    int dy = yvel[nSprite];
    int t1 = dmulscale30(dx, nCos, dy, nSin);
    int t2 = dmulscale30(dx, nSin, -dy, nCos);
    if (pXSprite->at17_3 > 0)
        t2 += pDudeInfo->at3c;
    else
        t2 -= pDudeInfo->at3c;

    xvel[nSprite] = dmulscale30(t1, nCos, t2, nSin);
    yvel[nSprite] = dmulscale30(t1, nSin, -t2, nCos);
    zvel[nSprite] = -0x8000;
}

static void MoveDodgeDown(SPRITE *pSprite, XSPRITE *pXSprite)
{
    int nSprite = pSprite->index;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int nAng = ((pXSprite->at16_0+1024-pSprite->ang)&2047)-1024;
    int nTurnRange = (pDudeInfo->at44<<2)>>4;
    pSprite->ang = (pSprite->ang+ClipRange(nAng, -nTurnRange, nTurnRange))&2047;
    if (pXSprite->at17_3 == 0)
        return;
    int nCos = Cos(pSprite->ang);
    int nSin = Sin(pSprite->ang);
    int dx = xvel[nSprite];
    int dy = yvel[nSprite];
    int t1 = dmulscale30(dx, nCos, dy, nSin);
    int t2 = dmulscale30(dx, nSin, -dy, nCos);
    if (pXSprite->at17_3 > 0)
        t2 += pDudeInfo->at3c;
    else
        t2 -= pDudeInfo->at3c;

    xvel[nSprite] = dmulscale30(t1, nCos, t2, nSin);
    yvel[nSprite] = dmulscale30(t1, nSin, -t2, nCos);
    zvel[nSprite] = 0x44444;
}

static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        aiNewState(pSprite, pXSprite, &eelGoto);
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
        aiNewState(pSprite, pXSprite, &eelSearch);
        return;
    }
    if (IsPlayerSprite(pTarget) && powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 13) > 0)
    {
        aiNewState(pSprite, pXSprite, &eelSearch);
        return;
    }
    int nDist = approxDist(dx, dy);
    if (nDist <= pDudeInfo->at17)
    {
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        int top2, bottom2;
        GetSpriteExtents(pTarget, &top2, &bottom2);
        if (cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSprite->x, pSprite->y, pSprite->z - height, pSprite->sectnum))
        {
            if (nDist < pDudeInfo->at17 && klabs(nDeltaAngle) <= pDudeInfo->at1b)
            {
                aiSetTarget(pXSprite, pXSprite->target);
                if (nDist < 0x399 && top2 > top && klabs(nDeltaAngle) < 85)
                    aiNewState(pSprite, pXSprite, &eelSwoop);
                else if (nDist <= 0x399 && klabs(nDeltaAngle) < 85)
                    aiNewState(pSprite, pXSprite, &eelBite);
                else if (bottom2 > top && klabs(nDeltaAngle) < 85)
                    aiNewState(pSprite, pXSprite, &eelSwoop);
                else if (top2 < top && klabs(nDeltaAngle) < 85)
                    aiNewState(pSprite, pXSprite, &eelFly);
            }
        }
        return;
    }

    pXSprite->target = -1;
    aiNewState(pSprite, pXSprite, &eelSearch);
}

static void MoveForward(SPRITE *pSprite, XSPRITE *pXSprite)
{
    int nSprite = pSprite->index;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int nAng = ((pXSprite->at16_0+1024-pSprite->ang)&2047)-1024;
    int nTurnRange = (pDudeInfo->at44<<2)>>4;
    pSprite->ang = (pSprite->ang+ClipRange(nAng, -nTurnRange, nTurnRange))&2047;
    int nAccel = (pDudeInfo->at38-(((4-gGameOptions.nDifficulty)<<26)/120)/120)<<2;
    if (klabs(nAng) > 341)
        return;
    if (pXSprite->target == -1)
        pSprite->ang = (pSprite->ang+256)&2047;
    int dx = pXSprite->at20_0-pSprite->x;
    int dy = pXSprite->at24_0-pSprite->y;
    int nAngle = getangle(dx, dy);
    int nDist = approxDist(dx, dy);
    if (nDist <= 0x399)
        return;
    int nCos = Cos(pSprite->ang);
    int nSin = Sin(pSprite->ang);
    int vx = xvel[nSprite];
    int vy = yvel[nSprite];
    int t1 = dmulscale30(vx, nCos, vy, nSin);
    int t2 = dmulscale30(vx, nSin, -vy, nCos);
    if (pXSprite->target == -1)
        t1 += nAccel;
    else
        t1 += nAccel>>1;
    xvel[nSprite] = dmulscale30(t1, nCos, t2, nSin);
    yvel[nSprite] = dmulscale30(t1, nSin, -t2, nCos);
}

static void MoveSwoop(SPRITE *pSprite, XSPRITE *pXSprite)
{
    int nSprite = pSprite->index;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int nAng = ((pXSprite->at16_0+1024-pSprite->ang)&2047)-1024;
    int nTurnRange = (pDudeInfo->at44<<2)>>4;
    pSprite->ang = (pSprite->ang+ClipRange(nAng, -nTurnRange, nTurnRange))&2047;
    int nAccel = (pDudeInfo->at38-(((4-gGameOptions.nDifficulty)<<26)/120)/120)<<2;
    if (klabs(nAng) > 341)
        return;
    int dx = pXSprite->at20_0-pSprite->x;
    int dy = pXSprite->at24_0-pSprite->y;
    int nAngle = getangle(dx, dy);
    int nDist = approxDist(dx, dy);
    if (Chance(0x8000) && nDist <= 0x399)
        return;
    int nCos = Cos(pSprite->ang);
    int nSin = Sin(pSprite->ang);
    int vx = xvel[nSprite];
    int vy = yvel[nSprite];
    int t1 = dmulscale30(vx, nCos, vy, nSin);
    int t2 = dmulscale30(vx, nSin, -vy, nCos);
    t1 += nAccel>>1;
    xvel[nSprite] = dmulscale30(t1, nCos, t2, nSin);
    yvel[nSprite] = dmulscale30(t1, nSin, -t2, nCos);
    zvel[nSprite] = 0x22222;
}

static void MoveAscend(SPRITE *pSprite, XSPRITE *pXSprite)
{
    int nSprite = pSprite->index;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int nAng = ((pXSprite->at16_0+1024-pSprite->ang)&2047)-1024;
    int nTurnRange = (pDudeInfo->at44<<2)>>4;
    pSprite->ang = (pSprite->ang+ClipRange(nAng, -nTurnRange, nTurnRange))&2047;
    int nAccel = (pDudeInfo->at38-(((4-gGameOptions.nDifficulty)<<26)/120)/120)<<2;
    if (klabs(nAng) > 341)
        return;
    int dx = pXSprite->at20_0-pSprite->x;
    int dy = pXSprite->at24_0-pSprite->y;
    int nAngle = getangle(dx, dy);
    int nDist = approxDist(dx, dy);
    if (Chance(0x4000) && nDist <= 0x399)
        return;
    int nCos = Cos(pSprite->ang);
    int nSin = Sin(pSprite->ang);
    int vx = xvel[nSprite];
    int vy = yvel[nSprite];
    int t1 = dmulscale30(vx, nCos, vy, nSin);
    int t2 = dmulscale30(vx, nSin, -vy, nCos);
    t1 += nAccel>>1;
    xvel[nSprite] = dmulscale30(t1, nCos, t2, nSin);
    yvel[nSprite] = dmulscale30(t1, nSin, -t2, nCos);
    zvel[nSprite] = -0x8000;
}

void MoveToCeil(SPRITE *pSprite, XSPRITE *pXSprite)
{
    int x = pSprite->x;
    int y = pSprite->y;
    int z = pSprite->z;
    int nSector = pSprite->sectnum;
    if (z - pXSprite->at28_0 < 0x1000)
    {
        DUDEEXTRA_at6_u1 *pDudeExtraE = &gDudeExtra[pSprite->extra].at6.u1;
        pDudeExtraE->at8 = 0;
        pSprite->hitag = 0;
        aiNewState(pSprite, pXSprite, &eelIdle);
    }
    else
        aiSetTarget(pXSprite, x, y, sector[nSector].ceilingz);
}

#include "compat.h"
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "airat.h"
#include "blood.h"
#include "db.h"
#include "dude.h"
#include "eventq.h"
#include "levels.h"
#include "player.h"
#include "seq.h"
#include "sfx.h"
#include "trig.h"

static void BiteSeqCallback(int, int);
static void thinkSearch(SPRITE *, XSPRITE *);
static void thinkGoto(SPRITE *, XSPRITE *);
static void thinkChase(SPRITE *, XSPRITE *);

static int nBiteClient = seqRegisterClient(BiteSeqCallback);

AISTATE ratIdle = { 0, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE ratSearch = { 7, -1, 1800, NULL, aiMoveForward, thinkSearch, &ratIdle };
AISTATE ratChase = { 7, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE ratDodge = { 7, -1, 0, NULL, NULL, NULL, &ratChase };
AISTATE ratRecoil = { 7, -1, 0, NULL, NULL, NULL, &ratDodge };
AISTATE ratGoto = { 7, -1, 600, NULL, aiMoveForward, thinkGoto, &ratIdle };
AISTATE ratBite = { 6, nBiteClient, 120, NULL, NULL, NULL, &ratChase };

static void BiteSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    SPRITE *pSprite = &qsprite[nSprite];
    int dx = Cos(pSprite->ang)>>16;
    int dy = Sin(pSprite->ang)>>16;
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    SPRITE *pTarget = &qsprite[pXSprite->target];
    if (IsPlayerSprite(pTarget))
        actFireVector(pSprite, 0, 0, dx, dy, pTarget->z-pSprite->z, VECTOR_TYPE_16);
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
        aiNewState(pSprite, pXSprite, &ratSearch);
    aiThinkTarget(pSprite, pXSprite);
}

static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        aiNewState(pSprite, pXSprite, &ratGoto);
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
        aiNewState(pSprite, pXSprite, &ratSearch);
        return;
    }
    if (IsPlayerSprite(pTarget) && powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 13) > 0)
    {
        aiNewState(pSprite, pXSprite, &ratSearch);
        return;
    }
    int nDist = approxDist(dx, dy);
    if (nDist <= pDudeInfo->at17)
    {
        int nDeltaAngle = ((getangle(dx,dy)+1024-pSprite->ang)&2047)-1024;
        int height = (pDudeInfo->atb*pSprite->yrepeat)<<2;
        if (cansee(pTarget->x, pTarget->y, pTarget->z, pTarget->sectnum, pSprite->x, pSprite->y, pSprite->z - height, pSprite->sectnum))
        {
            if (nDist < pDudeInfo->at17 && klabs(nDeltaAngle) <= pDudeInfo->at1b)
            {
                aiSetTarget(pXSprite, pXSprite->target);
                if (nDist < 0x399 && klabs(nDeltaAngle) < 85)
                    aiNewState(pSprite, pXSprite, &ratBite);
                return;
            }
        }
    }

    aiNewState(pSprite, pXSprite, &ratGoto);
    pXSprite->target = -1;
}

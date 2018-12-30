#include "compat.h"
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "aiburn.h"
#include "blood.h"
#include "db.h"
#include "dude.h"
#include "levels.h"
#include "player.h"
#include "seq.h"
#include "sfx.h"
#include "trig.h"

static void BurnSeqCallback(int, int);
static void thinkSearch(SPRITE*, XSPRITE*);
static void thinkGoto(SPRITE*, XSPRITE*);
static void thinkChase(SPRITE*, XSPRITE*);

static int nBurnClient = seqRegisterClient(BurnSeqCallback);

AISTATE cultistBurnIdle = { 3, -1, 0, NULL, NULL, aiThinkTarget, NULL };
AISTATE cultistBurnChase = { 3, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE cultistBurnGoto = { 3, -1, 3600, NULL, aiMoveForward, thinkGoto, &cultistBurnSearch };
AISTATE cultistBurnSearch = { 3, -1, 3600, NULL, aiMoveForward, thinkSearch, &cultistBurnSearch };
AISTATE cultistBurnAttack = { 3, nBurnClient, 120, NULL, NULL, NULL, &cultistBurnChase };

AISTATE zombieABurnChase = { 3, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE zombieABurnGoto = { 3, -1, 3600, NULL, aiMoveForward, thinkGoto, &zombieABurnSearch };
AISTATE zombieABurnSearch = { 3, -1, 3600, NULL, aiMoveForward, thinkSearch, NULL };
AISTATE zombieABurnAttack = { 3, nBurnClient, 120, NULL, NULL, NULL, &zombieABurnChase };

AISTATE zombieFBurnChase = { 3, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE zombieFBurnGoto = { 3, -1, 3600, NULL, aiMoveForward, thinkGoto, &zombieFBurnSearch };
AISTATE zombieFBurnSearch = { 3, -1, 3600, NULL, aiMoveForward, thinkSearch, NULL };
AISTATE zombieFBurnAttack = { 3, nBurnClient, 120, NULL, NULL, NULL, &zombieFBurnChase };

AISTATE innocentBurnChase = { 3, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE innocentBurnGoto = { 3, -1, 3600, NULL, aiMoveForward, thinkGoto, &zombieFBurnSearch };
AISTATE innocentBurnSearch = { 3, -1, 3600, NULL, aiMoveForward, thinkSearch, NULL };
AISTATE innocentBurnAttack = { 3, nBurnClient, 120, NULL, NULL, NULL, &zombieFBurnChase };

AISTATE beastBurnChase = { 3, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE beastBurnGoto = { 3, -1, 3600, NULL, aiMoveForward, thinkGoto, &beastBurnSearch };
AISTATE beastBurnSearch = { 3, -1, 3600, NULL, aiMoveForward, thinkSearch, &beastBurnSearch };
AISTATE beastBurnAttack = { 3, nBurnClient, 120, NULL, NULL, NULL, &beastBurnChase };

AISTATE tinycalebBurnChase = { 3, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE tinycalebBurnGoto = { 3, -1, 3600, NULL, aiMoveForward, thinkGoto, &tinycalebBurnSearch };
AISTATE tinycalebBurnSearch = { 3, -1, 3600, NULL, aiMoveForward, thinkSearch, &tinycalebBurnSearch };
AISTATE tinycalebBurnAttack = { 3, nBurnClient, 120, NULL, NULL, NULL, &tinycalebBurnChase };

static void BurnSeqCallback(int, int)
{
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
    {
        switch (pSprite->type)
        {
        case 240:
            aiNewState(pSprite, pXSprite, &cultistBurnSearch);
            break;
        case 241:
            aiNewState(pSprite, pXSprite, &zombieABurnSearch);
            break;
        case 242:
            aiNewState(pSprite, pXSprite, &zombieFBurnSearch);
            break;
        case 239:
            aiNewState(pSprite, pXSprite, &innocentBurnSearch);
            break;
        case 253:
            aiNewState(pSprite, pXSprite, &beastBurnSearch);
            break;
        case 252:
            aiNewState(pSprite, pXSprite, &tinycalebBurnSearch);
            break;
        }
    }
    aiThinkTarget(pSprite, pXSprite);
}

static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite)
{
    if (pXSprite->target == -1)
    {
        switch (pSprite->type)
        {
        case 240:
            aiNewState(pSprite, pXSprite, &cultistBurnGoto);
            break;
        case 241:
            aiNewState(pSprite, pXSprite, &zombieABurnGoto);
            break;
        case 242:
            aiNewState(pSprite, pXSprite, &zombieFBurnGoto);
            break;
        case 239:
            aiNewState(pSprite, pXSprite, &innocentBurnGoto);
            break;
        case 253:
            aiNewState(pSprite, pXSprite, &beastBurnGoto);
            break;
        case 252:
            aiNewState(pSprite, pXSprite, &tinycalebBurnGoto);
            break;
        }
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
        switch (pSprite->type)
        {
        case 240:
            aiNewState(pSprite, pXSprite, &cultistBurnSearch);
            break;
        case 241:
            aiNewState(pSprite, pXSprite, &zombieABurnSearch);
            break;
        case 242:
            aiNewState(pSprite, pXSprite, &zombieFBurnSearch);
            break;
        case 239:
            aiNewState(pSprite, pXSprite, &innocentBurnSearch);
            break;
        case 253:
            aiNewState(pSprite, pXSprite, &beastBurnSearch);
            break;
        case 252:
            aiNewState(pSprite, pXSprite, &tinycalebBurnSearch);
            break;
        }
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
                {
                    switch (pSprite->type)
                    {
                    case 240:
                        aiNewState(pSprite, pXSprite, &cultistBurnAttack);
                        break;
                    case 241:
                        aiNewState(pSprite, pXSprite, &zombieABurnAttack);
                        break;
                    case 242:
                        aiNewState(pSprite, pXSprite, &zombieFBurnAttack);
                        break;
                    case 239:
                        aiNewState(pSprite, pXSprite, &innocentBurnAttack);
                        break;
                    case 253:
                        aiNewState(pSprite, pXSprite, &beastBurnAttack);
                        break;
                    case 252:
                        aiNewState(pSprite, pXSprite, &tinycalebBurnAttack);
                        break;
                    }
                }
                return;
            }
        }
    }
    
    switch (pSprite->type)
    {
    case 240:
        aiNewState(pSprite, pXSprite, &cultistBurnGoto);
        break;
    case 241:
        aiNewState(pSprite, pXSprite, &zombieABurnGoto);
        break;
    case 242:
        aiNewState(pSprite, pXSprite, &zombieFBurnGoto);
        break;
    case 239:
        aiNewState(pSprite, pXSprite, &innocentBurnGoto);
        break;
    case 253:
        aiNewState(pSprite, pXSprite, &beastBurnGoto);
        break;
    case 252:
        aiNewState(pSprite, pXSprite, &tinycalebBurnGoto);
        break;
    }
    pXSprite->target = -1;
}

#include "compat.h"
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "aicerber.h"
#include "blood.h"
#include "db.h"
#include "dude.h"
#include "levels.h"
#include "player.h"
#include "seq.h"
#include "sfx.h"
#include "trig.h"

static void BiteSeqCallback(int, int);
static void BurnSeqCallback(int, int);
static void BurnSeqCallback2(int, int);
static void thinkSearch(SPRITE *pSprite, XSPRITE *pXSprite);
static void thinkTarget(SPRITE *pSprite, XSPRITE *pXSprite);
static void thinkGoto(SPRITE *pSprite, XSPRITE *pXSprite);
static void thinkChase(SPRITE *pSprite, XSPRITE *pXSprite);

static int nBiteClient = seqRegisterClient(BiteSeqCallback);
static int nBurnClient = seqRegisterClient(BurnSeqCallback);
static int nBurnClient2 = seqRegisterClient(BurnSeqCallback2);

AISTATE cerberusIdle = { 0, -1, 0, NULL, NULL, thinkTarget, NULL };
AISTATE cerberusSearch = { 7, -1, 1800, NULL, aiMoveForward, thinkSearch, &cerberusIdle };
AISTATE cerberusChase = { 7, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE cerberusRecoil = { 5, -1, 0, NULL, NULL, NULL, &cerberusSearch };
AISTATE cerberusTeslaRecoil = { 4, -1, 0, NULL, NULL, NULL, &cerberusSearch };
AISTATE cerberusGoto = { 7, -1, 600, NULL, aiMoveForward, thinkGoto, &cerberusIdle };
AISTATE cerberusBite = { 6, nBiteClient, 60, NULL, NULL, NULL, &cerberusChase };
AISTATE cerberusBurn = { 6, nBurnClient, 60, NULL, NULL, NULL, &cerberusChase };
AISTATE cerberus3Burn = { 6, nBurnClient2, 60, NULL, NULL, NULL, &cerberusChase };
AISTATE cerberus2Idle = { 0, -1, 0, NULL, NULL, thinkTarget, NULL };
AISTATE cerberus2Search = { 7, -1, 1800, NULL, aiMoveForward, thinkSearch, &cerberus2Idle };
AISTATE cerberus2Chase = { 7, -1, 0, NULL, aiMoveForward, thinkChase, NULL };
AISTATE cerberus2Recoil = { 5, -1, 0, NULL, NULL, NULL, &cerberus2Search };
AISTATE cerberus2Goto = { 7, -1, 600, NULL, aiMoveForward, thinkGoto, &cerberus2Idle };
AISTATE cerberus2Bite = { 6, nBiteClient, 60, NULL, NULL, NULL, &cerberus2Chase };
AISTATE cerberus2Burn = { 6, nBurnClient, 60, NULL, NULL, NULL, &cerberus2Chase };
AISTATE cerberus4Burn = { 6, nBurnClient2, 60, NULL, NULL, NULL, &cerberus2Chase };
AISTATE cerberus139890 = { 7, -1, 120, NULL, aiMoveTurn, NULL, &cerberusChase };
AISTATE cerberus1398AC = { 7, -1, 120, NULL, aiMoveTurn, NULL, &cerberusChase };

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
    int dz = pTarget->z-pSprite->z;
    actFireVector(pSprite, 350, -100, dx, dy, dz, VECTOR_TYPE_14);
    actFireVector(pSprite, -350, 0, dx, dy, dz, VECTOR_TYPE_14);
    actFireVector(pSprite, 0, 0, dx, dy, dz, VECTOR_TYPE_14);
}

static void BurnSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    SPRITE *pSprite = &qsprite[nSprite];
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int height = pDudeInfo->atb*pSprite->yrepeat;
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    int x = pSprite->x;
    int y = pSprite->y;
    int z = height; // ???
    TARGETTRACK tt1 = { 0x10000, 0x10000, 0x100, 0x55, 0x1aaaaa };
    Aim aim;
    aim.dx = Cos(pSprite->ang)>>16;
    aim.dy = Sin(pSprite->ang)>>16;
    aim.dz = gDudeSlope[nXSprite];
    int nClosest = 0x7fffffff;
    for (short nSprite2 = headspritestat[6]; nSprite2 >= 0; nSprite2 = nextspritestat[nSprite2])
    {
        SPRITE *pSprite2 = &qsprite[nSprite2];
        if (pSprite == pSprite2 || !(pSprite2->hitag&8))
            continue;
        int x2 = pSprite2->x;
        int y2 = pSprite2->y;
        int z2 = pSprite2->z;
        int nDist = approxDist(x2-x, y2-y);
        if (nDist == 0 || nDist > 0x2800)
            continue;
        if (tt1.at10)
        {
            int t = divscale(nDist, tt1.at10, 12);
            x2 += (xvel[nSprite2]*t)>>12;
            y2 += (yvel[nSprite2]*t)>>12;
            z2 += (zvel[nSprite2]*t)>>8;
        }
        int tx = x+mulscale30(Cos(pSprite->ang), nDist);
        int ty = y+mulscale30(Sin(pSprite->ang), nDist);
        int tz = z+mulscale(gDudeSlope[nXSprite], nDist, 10);
        int tsr = mulscale(9460, nDist, 10);
        int top, bottom;
        GetSpriteExtents(pSprite2, &top, &bottom);
        if (tz-tsr > bottom || tz+tsr < top)
            continue;
        int dx = (tx-x2)>>4;
        int dy = (ty-y2)>>4;
        int dz = (tz-z2)>>8;
        int nDist2 = ksqrt(dx*dx+dy*dy+dz*dz);
        if (nDist2 < nClosest)
        {
            int nAngle = getangle(x2-x, y2-y);
            int nDeltaAngle = ((nAngle-pSprite->ang+1024)&2047)-1024;
            if (klabs(nDeltaAngle) <= tt1.at8)
            {
                int tz = pSprite2->z-pSprite->z;
                if (cansee(x, y, z, pSprite->sectnum, x2, y2, z2, pSprite2->sectnum))
                {
                    nClosest = nDist2;
                    aim.dx = Cos(nAngle)>>16;
                    aim.dy = Sin(nAngle)>>16;
                    aim.dz = divscale(tz, nDist, 10);
                }
                else
                    aim.dz = tz;
            }
        }
    }
    switch (pSprite->type)
    {
    case 227:
        actFireMissile(pSprite, -350, 0, aim.dx, aim.dy, aim.dz, 313);
        actFireMissile(pSprite, -350, -100, aim.dx, aim.dy, aim.dz, 313);
        break;
    case 228:
        actFireMissile(pSprite, -350, -100, aim.dx, aim.dy, aim.dz, 313);
        break;
    }
}

static void BurnSeqCallback2(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    SPRITE *pSprite = &qsprite[nSprite];
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type - kDudeBase];
    int height = pDudeInfo->atb*pSprite->yrepeat;
    dassert(pXSprite->target >= 0 && pXSprite->target < kMaxSprites);
    int x = pSprite->x;
    int y = pSprite->y;
    int z = height; // ???
    TARGETTRACK tt1 = { 0x10000, 0x10000, 0x100, 0x55, 0x1aaaaa };
    Aim aim;
    int ax, ay, az;
    aim.dx = ax = Cos(pSprite->ang)>>16;
    aim.dy = ay = Sin(pSprite->ang)>>16;
    aim.dz = gDudeSlope[nXSprite];
    az = 0;
    int nClosest = 0x7fffffff;
    for (short nSprite2 = headspritestat[6]; nSprite2 >= 0; nSprite2 = nextspritestat[nSprite2])
    {
        SPRITE *pSprite2 = &qsprite[nSprite2];
        if (pSprite == pSprite2 || !(pSprite2->hitag&8))
            continue;
        int x2 = pSprite2->x;
        int y2 = pSprite2->y;
        int z2 = pSprite2->z;
        int nDist = approxDist(x2-x, y2-y);
        if (nDist == 0 || nDist > 0x2800)
            continue;
        if (tt1.at10)
        {
            int t = divscale(nDist, tt1.at10, 12);
            x2 += (xvel[nSprite2]*t)>>12;
            y2 += (yvel[nSprite2]*t)>>12;
            z2 += (zvel[nSprite2]*t)>>8;
        }
        int tx = x+mulscale30(Cos(pSprite->ang), nDist);
        int ty = y+mulscale30(Sin(pSprite->ang), nDist);
        int tz = z+mulscale(gDudeSlope[nXSprite], nDist, 10);
        int tsr = mulscale(9460, nDist, 10);
        int top, bottom;
        GetSpriteExtents(pSprite2, &top, &bottom);
        if (tz-tsr > bottom || tz+tsr < top)
            continue;
        int dx = (tx-x2)>>4;
        int dy = (ty-y2)>>4;
        int dz = (tz-z2)>>8;
        int nDist2 = ksqrt(dx*dx+dy*dy+dz*dz);
        if (nDist2 < nClosest)
        {
            int nAngle = getangle(x2-x, y2-y);
            int nDeltaAngle = ((nAngle-pSprite->ang+1024)&2047)-1024;
            if (klabs(nDeltaAngle) <= tt1.at8)
            {
                DUDEINFO *pDudeInfo2 = &dudeInfo[pSprite2->type - kDudeBase];
                int height = (pDudeInfo2->atf*pSprite2->yrepeat)<<2;
                int tz = (z2-height)-z;
                if (cansee(x, y, z, pSprite->sectnum, x2, y2, z2, pSprite2->sectnum))
                {
                    nClosest = nDist2;
                    aim.dx = Cos(nAngle)>>16;
                    aim.dy = Sin(nAngle)>>16;
                    aim.dz = divscale(tz, nDist, 10);
                }
                else
                    aim.dz = tz;
            }
        }
    }
    switch (pSprite->type)
    {
    case 227:
        actFireMissile(pSprite, 350, -100, aim.dx, aim.dy, -aim.dz, 308);
        actFireMissile(pSprite, -350, 0, ax, ay, az, 308);
        break;
    case 228:
        actFireMissile(pSprite, 350, -100, aim.dx, aim.dy, -aim.dz, 308);
        break;
    }
}

static void thinkSearch(SPRITE *pSprite, XSPRITE *pXSprite)
{
    aiChooseDirection(pSprite, pXSprite, pXSprite->at16_0);
    aiThinkTarget(pSprite, pXSprite);
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
        pXSprite->at16_0 += 256;
        POINT3D *pTarget = &baseSprite[pSprite->index];
        aiSetTarget(pXSprite, pTarget->x, pTarget->y, pTarget->z);
        if (pSprite->type == 227)
            aiNewState(pSprite, pXSprite, &cerberus139890);
        else
            aiNewState(pSprite, pXSprite, &cerberus1398AC);
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
                pDudeExtraE->at0 = 0;
                aiSetTarget(pXSprite, pPlayer->at5b);
                aiActivateDude(pSprite, pXSprite);
            }
            else if (nDist < pDudeInfo->at13)
            {
                pDudeExtraE->at0 = 0;
                aiSetTarget(pXSprite, x, y, z);
                aiActivateDude(pSprite, pXSprite);
            }
            else
                continue;
            break;
        }
    }
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
        case 227:
            aiNewState(pSprite, pXSprite, &cerberusSearch);
            break;
        case 228:
            aiNewState(pSprite, pXSprite, &cerberus2Search);
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
        case 227:
            aiNewState(pSprite, pXSprite, &cerberusGoto);
            break;
        case 228:
            aiNewState(pSprite, pXSprite, &cerberus2Goto);
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
        case 227:
            aiNewState(pSprite, pXSprite, &cerberusSearch);
            break;
        case 228:
            aiNewState(pSprite, pXSprite, &cerberus2Search);
            break;
        }
        return;
    }
    if (IsPlayerSprite(pTarget) && powerupCheck(&gPlayer[pTarget->type-kDudePlayer1], 13) > 0)
    {
        switch (pSprite->type)
        {
        case 227:
            aiNewState(pSprite, pXSprite, &cerberusSearch);
            break;
        case 228:
            aiNewState(pSprite, pXSprite, &cerberus2Search);
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
                if (nDist < 0x1b00 && nDist > 0xd00 && klabs(nDeltaAngle) < 85)
                {
                    switch (pSprite->type)
                    {
                    case 227:
                        aiNewState(pSprite, pXSprite, &cerberusBurn);
                        break;
                    case 228:
                        aiNewState(pSprite, pXSprite, &cerberus2Burn);
                        break;
                    }
                }
                else if (nDist < 0xb00 && nDist > 0x500 && klabs(nDeltaAngle) < 85)
                {
                    switch (pSprite->type)
                    {
                    case 227:
                        aiNewState(pSprite, pXSprite, &cerberus3Burn);
                        break;
                    case 228:
                        aiNewState(pSprite, pXSprite, &cerberus4Burn);
                        break;
                    }
                }
                else if (nDist < 0x200 && klabs(nDeltaAngle) < 85)
                {
                    int hit = HitScan(pSprite, pSprite->z, dx, dy, 0, CLIPMASK1, 0);
                    switch (pSprite->type)
                    {
                    case 227:
                        switch (hit)
                        {
                        case -1:
                            aiNewState(pSprite, pXSprite, &cerberusBite);
                            break;
                        case 3:
                            if (pSprite->type != qsprite[gHitInfo.hitsprite].type && qsprite[gHitInfo.hitsprite].type != 211)
                                aiNewState(pSprite, pXSprite, &cerberusBite);
                            break;
                        case 0:
                        case 4:
                            break;
                        default:
                            aiNewState(pSprite, pXSprite, &cerberusBite);
                            break;
                        }
                        break;
                    case 228:
                        switch (hit)
                        {
                        case -1:
                            aiNewState(pSprite, pXSprite, &cerberus2Bite);
                            break;
                        case 3:
                            if (pSprite->type != qsprite[gHitInfo.hitsprite].type && qsprite[gHitInfo.hitsprite].type != 211)
                                aiNewState(pSprite, pXSprite, &cerberus2Bite);
                            break;
                        case 0:
                        case 4:
                            break;
                        default:
                            aiNewState(pSprite, pXSprite, &cerberus2Bite);
                            break;
                        }
                        break;
                    }
                }
                return;
            }
        }
    }

    switch (pSprite->type)
    {
    case 227:
        aiNewState(pSprite, pXSprite, &cerberusGoto);
        break;
    case 228:
        aiNewState(pSprite, pXSprite, &cerberus2Goto);
        break;
    }
    pXSprite->target = -1;
}

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
#include "build.h"
#include "compat.h"
#include "mmulti.h"
#include "common_game.h"

#include "ai.h"
#include "actor.h"
#include "blood.h"
#include "db.h"
#include "endgame.h"
#include "eventq.h"
#include "fx.h"
#include "gameutil.h"
#include "gib.h"
#include "levels.h"
#include "loadsave.h"
#include "player.h"
#include "seq.h"
#include "sfx.h"
#include "sound.h"
#include "triggers.h"
#include "trig.h"
#include "view.h"

int basePath[kMaxSectors];

void FireballTrapSeqCallback(int, int);
void MGunFireSeqCallback(int, int);
void MGunOpenSeqCallback(int, int);

int nFireballTrapClient = seqRegisterClient(FireballTrapSeqCallback);
int nMGunFireClient = seqRegisterClient(MGunFireSeqCallback);
int nMGunOpenClient = seqRegisterClient(MGunOpenSeqCallback);

unsigned int GetWaveValue(unsigned int nPhase, int nType)
{
    switch (nType)
    {
    case 0:
        return 0x8000-(Cos((nPhase<<10)>>16)>>15);
    case 1:
        return nPhase;
    case 2:
        return 0x10000-(Cos((nPhase<<9)>>16)>>14);
    case 3:
        return Sin((nPhase<<9)>>16)>>14;
    }
    return nPhase;
}

char SetSpriteState(int nSprite, XSPRITE *pXSprite, int nState)
{
    if ((pXSprite->at1_7&0xffff) == 0 && pXSprite->at1_6 == nState)
        return 0;
    pXSprite->at1_7 = nState<<16;
    pXSprite->at1_6 = nState;
    evKill(nSprite, 3);
    if ((sprite[nSprite].hitag & 16) != 0 && sprite[nSprite].type >= kDudeBase && sprite[nSprite].type < kDudeMax)
    {
        pXSprite->atb_4 = 3;
        evPost(nSprite, 3, gGameOptions.nMonsterRespawnTime, CALLBACK_ID_9);
        return 1;
    }
    if (pXSprite->atb_0 != nState && pXSprite->at9_4 > 0)
        evPost(nSprite, 3, (pXSprite->at9_4*120) / 10, pXSprite->atb_0 ? COMMAND_ID_1 : COMMAND_ID_0);
    if (pXSprite->at4_0)
    {
        if (pXSprite->at6_4 != 5 && pXSprite->at7_4 && pXSprite->at1_6)
            evSend(nSprite, 3, pXSprite->at4_0, (COMMAND_ID)pXSprite->at6_4);
        if (pXSprite->at6_4 != 5 && pXSprite->at7_5 && !pXSprite->at1_6)
            evSend(nSprite, 3, pXSprite->at4_0, (COMMAND_ID)pXSprite->at6_4);
    }
    return 1;
}

char SetWallState(int nWall, XWALL *pXWall, int nState)
{
    if ((pXWall->at1_7&0xffff) == 0 && pXWall->at1_6 == nState)
        return 0;
    pXWall->at1_7 = nState<<16;
    pXWall->at1_6 = nState;
    evKill(nWall, 0);
    if (pXWall->atd_4 != nState && pXWall->atc_0 > 0)
        evPost(nWall, 0, (pXWall->atc_0*120) / 10, pXWall->atd_4 ? COMMAND_ID_1 : COMMAND_ID_0);
    if (pXWall->at6_0)
    {
        if (pXWall->at9_2 != 5 && pXWall->ata_2 && pXWall->at1_6)
            evSend(nWall, 0, pXWall->at6_0, (COMMAND_ID)pXWall->at9_2);
        if (pXWall->at9_2 != 5 && pXWall->ata_3 && !pXWall->at1_6)
            evSend(nWall, 0, pXWall->at6_0, (COMMAND_ID)pXWall->at9_2);
    }
    return 1;
}

char SetSectorState(int nSector, XSECTOR *pXSector, int nState)
{
    if ((pXSector->at1_7&0xffff) == 0 && pXSector->at1_6 == nState)
        return 0;
    pXSector->at1_7 = nState<<16;
    pXSector->at1_6 = nState;
    evKill(nSector, 6);
    if (nState == 1)
    {
        if (pXSector->at9_2 != 5 && pXSector->ata_2 && pXSector->at6_0)
            evSend(nSector, 6, pXSector->at6_0, (COMMAND_ID)pXSector->at9_2);
        if (pXSector->at1b_2)
        {
            pXSector->at1b_2 = 0;
            pXSector->at1b_3 = 0;
        }
        else if (pXSector->atf_6)
            evPost(nSector, 6, (pXSector->atc_0 * 120) / 10, COMMAND_ID_0);
    }
    else
    {
        if (pXSector->at9_2 != 5 && pXSector->ata_3 && pXSector->at6_0)
            evSend(nSector, 6, pXSector->at6_0, (COMMAND_ID)pXSector->at9_2);
        if (pXSector->at1b_3)
        {
            pXSector->at1b_2 = 0;
            pXSector->at1b_3 = 0;
        }
        else if (pXSector->atf_7)
            evPost(nSector, 6, (pXSector->at19_6 * 120) / 10, COMMAND_ID_1);
    }
    return 1;
}

int gBusyCount = 0;

enum BUSYID {
    BUSYID_0 = 0,
    BUSYID_1,
    BUSYID_2,
    BUSYID_3,
    BUSYID_4,
    BUSYID_5,
    BUSYID_6,
    BUSYID_7,
};

struct BUSY {
    int at0;
    int at4;
    int at8;
    BUSYID atc;
};

BUSY gBusy[128];

void AddBusy(int a1, BUSYID a2, int nDelta)
{
    dassert(nDelta != 0);
    int i;
    for (i = 0; i < gBusyCount; i++)
    {
        if (gBusy[i].at0 == a1 && gBusy[i].atc == a2)
            break;
    }
    if (i == gBusyCount)
    {
        if (gBusyCount == 128)
            return;
        gBusy[i].at0 = a1;
        gBusy[i].atc = a2;
        gBusy[i].at8 = nDelta > 0 ? 0 : 65536;
        gBusyCount++;
    }
    gBusy[i].at4 = nDelta;
}

void ReverseBusy(int a1, BUSYID a2)
{
    int i;
    for (i = 0; i < gBusyCount; i++)
    {
        if (gBusy[i].at0 == a1 && gBusy[i].atc == a2)
        {
            gBusy[i].at4 = -gBusy[i].at4;
            break;
        }
    }
}

unsigned int GetSourceBusy(EVENT a1)
{
    int nIndex = a1.at0_0;
    switch (a1.at1_5)
    {
    case 6:
    {
        int nXIndex = sector[nIndex].extra;
        dassert(nXIndex > 0 && nXIndex < kMaxXSectors);
        return xsector[nXIndex].at1_7;
    }
    case 0:
    {
        int nXIndex = wall[nIndex].extra;
        dassert(nXIndex > 0 && nXIndex < kMaxXWalls);
        return xwall[nXIndex].at1_7;
    }
    case 3:
    {
        int nXIndex = sprite[nIndex].extra;
        dassert(nXIndex > 0 && nXIndex < kMaxXSprites);
        return xsprite[nXIndex].at1_7;
    }
    }
    return 0;
}

void sub_43CF8(spritetype *pSprite, XSPRITE *pXSprite, EVENT a3)
{
    switch (a3.at2_0)
    {
    case 30:
    {
        int nPlayer = pXSprite->at18_2;
        if (nPlayer >= 0 && nPlayer < gNetPlayers)
        {
            PLAYER *pPlayer = &gPlayer[nPlayer];
            if (pPlayer->pXSprite->health > 0)
            {
                pPlayer->at181[8] = ClipHigh(pPlayer->at181[8]+pXSprite->at14_0, gAmmoInfo[8].at0);
                pPlayer->atcb[9] = 1;
                if (pPlayer->atbd != 9)
                {
                    pPlayer->atc3 = 0;
                    pPlayer->atbe = 9;
                }
                evKill(pSprite->index, 3);
            }
        }
        break;
    }
    case 35:
    {
        int nTarget = pXSprite->target;
        if (nTarget >= 0 && nTarget < kMaxSprites)
        {
            if (!pXSprite->at32_0)
            {
                spritetype *pTarget = &sprite[nTarget];
                if (pTarget->statnum == 6 && !(pTarget->hitag&32) && pTarget->extra > 0 && pTarget->extra < kMaxXSprites)
                {
                    int top, bottom;
                    GetSpriteExtents(pSprite, &top, &bottom);
                    int nType = pTarget->type-kDudeBase;
                    DUDEINFO *pDudeInfo = &dudeInfo[nType];
                    int z1 = (top-pSprite->z)-256;
                    int x = pTarget->x;
                    int y = pTarget->y;
                    int z = pTarget->z;
                    int nDist = approxDist(x - pSprite->x, y - pSprite->y);
                    if (nDist != 0 && cansee(pSprite->x, pSprite->y, top, pSprite->sectnum, x, y, z, pTarget->sectnum))
                    {
                        int t = divscale(nDist, 0x1aaaaa, 12);
                        x += (xvel[nTarget]*t)>>12;
                        y += (yvel[nTarget]*t)>>12;
                        int angBak = pSprite->ang;
                        pSprite->ang = getangle(x-pSprite->x, y-pSprite->y);
                        int dx = Cos(pSprite->ang)>>16;
                        int dy = Sin(pSprite->ang)>>16;
                        int tz = pTarget->z - (pTarget->yrepeat * pDudeInfo->atf) * 4;
                        int dz = divscale(tz - top - 256, nDist, 10);
                        int nMissileType = 316+(pXSprite->at14_0 ? 1 : 0);
                        int t2;
                        if (!pXSprite->at14_0)
                            t2 = 120 / 10.0;
                        else
                            t2 = (3*120) / 10.0;
                        spritetype *pMissile = actFireMissile(pSprite, 0, z1, dx, dy, dz, nMissileType);
                        if (pMissile)
                        {
                            pMissile->owner = pSprite->owner;
                            pXSprite->at32_0 = 1;
                            evPost(pSprite->index, 3, t2, CALLBACK_ID_20);
                            pXSprite->at14_0 = ClipLow(pXSprite->at14_0-1, 0);
                        }
                        pSprite->ang = angBak;
                    }
                }
            }
        }
        return;
    }
    }
    actPostSprite(pSprite->index, kStatFree);
}

void ActivateGenerator(int);

void OperateSprite(int nSprite, XSPRITE *pXSprite, EVENT a3)
{
    spritetype *pSprite = &sprite[nSprite];
    switch (a3.at2_0)
    {
    case 6:
        pXSprite->at17_5 = 1;
        return;
    case 7:
        pXSprite->at17_5 = 0;
        return;
    case 8:
        pXSprite->at17_5 = pXSprite->at17_5 ^ 1;
        return;
    }
    if (pSprite->statnum == 6 && pSprite->type >= kDudeBase && pSprite->type < kDudeMax)
    {
        switch (a3.at2_0)
        {
        case 0:
            SetSpriteState(nSprite, pXSprite, 0);
            break;
        case 35:
            if (pXSprite->at1_6)
                break;
            fallthrough__;
        case 1:
        case 30:
        case 33:
            if (!pXSprite->at1_6)
                SetSpriteState(nSprite, pXSprite, 1);
            aiActivateDude(pSprite, pXSprite);
            break;
        }
        return;
    }
    switch (pSprite->type)
    {
    case 413:
        if (pXSprite->health > 0)
        {
            if (a3.at2_0 == 1)
            {
                if (SetSpriteState(nSprite, pXSprite, 1))
                {
                    seqSpawn(38, 3, pSprite->extra, nMGunOpenClient);
                    if (pXSprite->at10_0 > 0)
                        pXSprite->at12_0 = pXSprite->at10_0;
                }
            }
            else if (a3.at2_0 == 0)
            {
                if (SetSpriteState(nSprite, pXSprite, 0))
                    seqSpawn(40, 3, pSprite->extra, -1);
            }
        }
        break;
    case 414:
        if (SetSpriteState(nSprite, pXSprite, 1))
            pSprite->hitag |= 7;
        break;
    case 408:
        if (SetSpriteState(nSprite, pXSprite, 0))
            actPostSprite(nSprite, kStatFree);
        break;
    case 405:
        if (SetSpriteState(nSprite, pXSprite, 0))
            actPostSprite(nSprite, kStatFree);
        break;
    case 456:
        switch (a3.at2_0)
        {
        case 0:
            pXSprite->at1_6 = 0;
            pSprite->cstat |= 32768;
            pSprite->cstat &= ~1;
            break;
        case 1:
            pXSprite->at1_6 = 1;
            pSprite->cstat &= (unsigned short)~32768;
            pSprite->cstat |= 1;
            break;
        case 3:
            pXSprite->at1_6 ^= 1;
            pSprite->cstat ^= 32768;
            pSprite->cstat ^= 1;
            break;
        }
        break;
    case 452:
        if (a3.at2_0 == 1)
        {
            if (SetSpriteState(nSprite, pXSprite, 1))
            {
                seqSpawn(38, 3, pSprite->extra, -1);
                sfxPlay3DSound(pSprite, 441, 0, 0);
            }
        }
        else if (a3.at2_0 == 0)
        {
            if (SetSpriteState(nSprite, pXSprite, 0))
            {
                seqSpawn(40, 3, pSprite->extra, -1);
                sfxKill3DSound(pSprite, 0, -1);
            }
        }
        break;
    case 23:
        switch (a3.at2_0)
        {
        case 0:
            SetSpriteState(nSprite, pXSprite, 0);
            break;
        case 1:
            if (SetSpriteState(nSprite, pXSprite, 1))
                seqSpawn(37, 3, pSprite->extra, -1);
            break;
        default:
            SetSpriteState(nSprite, pXSprite, pXSprite->at1_6 ^ 1);
            if (pXSprite->at1_6)
                seqSpawn(37, 3, pSprite->extra, -1);
            break;
        }
        break;
    case 20:
        switch (a3.at2_0)
        {
        case 0:
            if (SetSpriteState(nSprite, pXSprite, 0))
                sfxPlay3DSound(pSprite, pXSprite->at12_0, 0, 0);
            break;
        case 1:
            if (SetSpriteState(nSprite, pXSprite, 1))
                sfxPlay3DSound(pSprite, pXSprite->at10_0, 0, 0);
            break;
        default:
            if (SetSpriteState(nSprite, pXSprite, pXSprite->at1_6 ^ 1))
            {
                if (pXSprite->at1_6)
                    sfxPlay3DSound(pSprite, pXSprite->at10_0, 0, 0);
                else
                    sfxPlay3DSound(pSprite, pXSprite->at12_0, 0, 0);
            }
            break;
        }
        break;
    case 21:
        switch (a3.at2_0)
        {
        case 0:
            if (SetSpriteState(nSprite, pXSprite, 0))
                sfxPlay3DSound(pSprite, pXSprite->at12_0, 0, 0);
            break;
        case 1:
            if (SetSpriteState(nSprite, pXSprite, 1))
                sfxPlay3DSound(pSprite, pXSprite->at10_0, 0, 0);
            break;
        default:
            if (SetSpriteState(nSprite, pXSprite, pXSprite->atb_0 ^ 1))
            {
                if (pXSprite->at1_6)
                    sfxPlay3DSound(pSprite, pXSprite->at10_0, 0, 0);
                else
                    sfxPlay3DSound(pSprite, pXSprite->at12_0, 0, 0);
            }
            break;
        }
        break;
    case 22:
        switch (a3.at2_0)
        {
        case 0:
            pXSprite->at10_0--;
            if (pXSprite->at10_0 < 0)
                pXSprite->at10_0 += pXSprite->at14_0;
            break;
        default:
            pXSprite->at10_0++;
            if (pXSprite->at10_0 >= pXSprite->at14_0)
                pXSprite->at10_0 -= pXSprite->at14_0;
            break;
        }
        if (pXSprite->at6_4 == 5 && pXSprite->at4_0)
            evSend(nSprite, 3, pXSprite->at4_0, COMMAND_ID_5);
        sfxPlay3DSound(pSprite, pXSprite->at18_2, -1, 0);
        if (pXSprite->at10_0 == pXSprite->at12_0)
            SetSpriteState(nSprite, pXSprite, 1);
        else
            SetSpriteState(nSprite, pXSprite, 0);
        break;
    case 18:
        if (gGameOptions.nMonsterSettings && pXSprite->at10_0 >= kDudeBase && pXSprite->at10_0 < kDudeMax)
        {
            spritetype *pSpawn = sub_36878(pSprite, pXSprite->at10_0, -1, 0);
            if (pSpawn)
            {
                XSPRITE *pXSpawn = &xsprite[pSpawn->extra];
                gKillMgr.sub_263E0(1);
                switch (pXSprite->at10_0)
                {
                case 239:
                case 240:
                case 242:
                case 252:
                case 253:
                {
                    pXSpawn->health = dudeInfo[pXSprite->at10_0 - kDudeBase].at2 << 4;
                    pXSpawn->at2c_0 = 10;
                    pXSpawn->target = -1;
                    aiActivateDude(pSpawn, pXSpawn);
                    break;
                }
                }
            }
        }
        break;
    case 19:
        pXSprite->at7_4 = 0;
        pXSprite->atd_2 = 1;
        SetSpriteState(nSprite, pXSprite, 1);
        for (int p = connecthead; p >= 0; p = connectpoint2[p])
        {
            spritetype *pPlayerSprite = gPlayer[p].pSprite;
            int dx = (pSprite->x - pPlayerSprite->x)>>4;
            int dy = (pSprite->y - pPlayerSprite->y)>>4;
            int dz = (pSprite->z - pPlayerSprite->z)>>8;
            int nDist = dx*dx+dy*dy+dz*dz+0x40000;
            gPlayer[p].at37f = divscale16(pXSprite->at10_0, nDist);
        }
        break;
    case 400:
        if (pSprite->hitag&16)
            return;
        fallthrough__;
    case 418:
    case 419:
    case 420:
        actExplodeSprite(pSprite);
        break;
    case 459:
        switch (a3.at2_0)
        {
        case 1:
            SetSpriteState(nSprite, pXSprite, 1);
            break;
        default:
            pSprite->cstat &= (unsigned short)~32768;
            actExplodeSprite(pSprite);
            break;
        }
        break;
    case 402:
        if (pSprite->statnum == 8)
            break;
        if (a3.at2_0 != 1)
            actExplodeSprite(pSprite);
        else
        {
            sfxPlay3DSound(pSprite, 454, 0, 0);
            evPost(nSprite, 3, 18, COMMAND_ID_0);
        }
        break;
    case 401:
        if (pSprite->statnum == 8)
            break;
        switch (a3.at2_0)
        {
        case 35:
            if (!pXSprite->at1_6)
            {
                sfxPlay3DSound(pSprite, 452, 0, 0);
                evPost(nSprite, 3, 30, COMMAND_ID_0);
                pXSprite->at1_6 = 1;
            }
            break;
        case 1:
            sfxPlay3DSound(pSprite, 451, 0, 0);
            pXSprite->ate_4 = 1;
            break;
        default:
            actExplodeSprite(pSprite);
            break;
        }
        break;
    case 431:
        sub_43CF8(pSprite, pXSprite, a3);
        break;
    case 700:
    case 701:
    case 702:
    case 703:
    case 704:
    case 705:
    case 706:
    case 707:
    case 708:
        switch (a3.at2_0)
        {
        case 0:
            SetSpriteState(nSprite, pXSprite, 0);
            break;
        case 21:
            if (pSprite->type != 700)
                ActivateGenerator(nSprite);
            if (pXSprite->at4_0)
                evSend(nSprite, 3, pXSprite->at4_0, (COMMAND_ID)pXSprite->at6_4);
            if (pXSprite->at8_0 > 0)
            {
                int nRand = Random2(pXSprite->at10_0);
                evPost(nSprite, 3, 120*(nRand+pXSprite->at8_0) / 10, COMMAND_ID_21);
            }
            break;
        default:
            if (!pXSprite->at1_6)
            {
                SetSpriteState(nSprite, pXSprite, 1);
                evPost(nSprite, 3, 0, COMMAND_ID_21);
            }
            break;
        }
        break;
    case 711:
        if (gGameOptions.nGameType == 0)
        {
            if (gMe->pXSprite->health <= 0)
                break;
            gMe->at30a = 0;
        }
        sndStartSample(pXSprite->at10_0, -1, 1, 0);
        break;
    case 416:
    case 417:
    case 425:
    case 426:
    case 427:
        switch (a3.at2_0)
        {
        case 0:
            if (SetSpriteState(nSprite, pXSprite, 0))
                actActivateGibObject(pSprite, pXSprite);
            break;
        case 1:
            if (SetSpriteState(nSprite, pXSprite, 1))
                actActivateGibObject(pSprite, pXSprite);
            break;
        default:
            if (SetSpriteState(nSprite, pXSprite, pXSprite->at1_6 ^ 1))
                actActivateGibObject(pSprite, pXSprite);
            break;
        }
        break;
    default:
        switch (a3.at2_0)
        {
        case 0:
            SetSpriteState(nSprite, pXSprite, 0);
            break;
        case 1:
            SetSpriteState(nSprite, pXSprite, 1);
            break;
        default:
            SetSpriteState(nSprite, pXSprite, pXSprite->at1_6 ^ 1);
            break;
        }
        break;
    }
}

void SetupGibWallState(walltype *pWall, XWALL *pXWall)
{
    walltype *pWall2 = NULL;
    if (pWall->nextwall >= 0)
        pWall2 = &wall[pWall->nextwall];
    if (pXWall->at1_6)
    {
        pWall->cstat &= ~65;
        if (pWall2)
        {
            pWall2->cstat &= ~65;
            pWall->cstat &= ~16;
            pWall2->cstat &= ~16;
        }
        return;
    }
    char bVector = pXWall->at10_6 != 0;
    pWall->cstat |= 1;
    if (bVector)
        pWall->cstat |= 64;
    if (pWall2)
    {
        pWall2->cstat |= 1;
        if (bVector)
            pWall2->cstat |= 64;
        pWall->cstat |= 16;
        pWall2->cstat |= 16;
    }
}

void OperateWall(int nWall, XWALL *pXWall, EVENT a3)
{
    walltype *pWall = &wall[nWall];
    switch (a3.at2_0)
    {
    case 6:
        pXWall->at13_2 = 1;
        return;
    case 7:
        pXWall->at13_2 = 0;
        return;
    case 8:
        pXWall->at13_2 ^= 1;
        return;
    }
    if (pWall->lotag == 511)
    {
        char bStatus;
        switch (a3.at2_0)
        {
        case 1:
        case 51:
            bStatus = SetWallState(nWall, pXWall, 1);
            break;
        case 0:
            bStatus = SetWallState(nWall, pXWall, 0);
            break;
        default:
            bStatus = SetWallState(nWall, pXWall, pXWall->at1_6^1);
            break;
        }
        if (bStatus)
        {
            SetupGibWallState(pWall, pXWall);
            if (pXWall->at1_6)
            {
                CGibVelocity vel(100, 100, 250);
                int nType = ClipRange(pXWall->at4_0, 0, 31);
                if (nType > 0)
                    GibWall(nWall, (GIBTYPE)nType, &vel);
            }
        }
        return;
    }
    switch (a3.at2_0)
    {
    case 0:
        SetWallState(nWall, pXWall, 0);
        break;
    case 1:
        SetWallState(nWall, pXWall, 1);
        break;
    default:
        SetWallState(nWall, pXWall, pXWall->at1_6 ^ 1);
        break;
    }
}

void SectorStartSound(int nSector, int nState)
{
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 0 && pSprite->type == 709)
        {
            int nXSprite = pSprite->extra;
            dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            if (nState)
            {
                if (pXSprite->at14_0)
                    sfxPlay3DSound(pSprite, pXSprite->at14_0, 0, 0);
            }
            else
            {
                if (pXSprite->at10_0)
                    sfxPlay3DSound(pSprite, pXSprite->at10_0, 0, 0);
            }
        }
    }
}

void SectorEndSound(int nSector, int nState)
{
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 0 && pSprite->type == 709)
        {
            int nXSprite = pSprite->extra;
            dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            if (nState)
            {
                if (pXSprite->at12_0)
                    sfxPlay3DSound(pSprite, pXSprite->at12_0, 0, 0);
            }
            else
            {
                if (pXSprite->at18_2)
                    sfxPlay3DSound(pSprite, pXSprite->at18_2, 0, 0);
            }
        }
    }
}

void PathSound(int nSector, int nSound)
{
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 0 && pSprite->type == 709)
            sfxPlay3DSound(pSprite, nSound, 0, 0);
    }
}

void DragPoint(int nWall, int x, int y)
{
    viewInterpolateWall(nWall, &wall[nWall]);
    wall[nWall].x = x;
    wall[nWall].y = y;

    int vsi = numwalls;
    int vb = nWall;
    do
    {
        if (wall[vb].nextwall >= 0)
        {
            vb = wall[wall[vb].nextwall].point2;
            viewInterpolateWall(vb, &wall[vb]);
            wall[vb].x = x;
            wall[vb].y = y;
        }
        else
        {
            vb = nWall;
            do
            {
                if (wall[lastwall(vb)].nextwall >= 0)
                {
                    vb = wall[lastwall(vb)].nextwall;
                    viewInterpolateWall(vb, &wall[vb]);
                    wall[vb].x = x;
                    wall[vb].y = y;
                }
                else
                    break;
                vsi--;
            } while (vb != nWall && vsi > 0);
            break;
        }
        vsi--;
    } while (vb != nWall && vsi > 0);
}

void TranslateSector(int nSector, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, char a12)
{
    int x, y;
    int nXSector = sector[nSector].extra;
    XSECTOR *pXSector = &xsector[nXSector];
    int v20 = interpolate(a6, a9, a2);
    int vc = interpolate(a6, a9, a3);
    int v28 = vc - v20;
    int v24 = interpolate(a7, a10, a2);
    int v8 = interpolate(a7, a10, a3);
    int v2c = v8 - v24;
    int v44 = interpolate(a8, a11, a2);
    int vbp = interpolate(a8, a11, a3);
    int v14 = vbp - v44;
    int nWall = sector[nSector].wallptr;
    if (a12)
    {
        for (int i = 0; i < sector[nSector].wallnum; nWall++, i++)
        {
            x = baseWall[nWall].x;
            y = baseWall[nWall].y;
            if (vbp)
                RotatePoint((int*)&x, (int*)&y, vbp, a4, a5);
            DragPoint(nWall, x+vc-a4, y+v8-a5);
        }
    }
    else
    {
        for (int i = 0; i < sector[nSector].wallnum; nWall++, i++)
        {
            int v10 = wall[nWall].point2;
            x = baseWall[nWall].x;
            y = baseWall[nWall].y;
            if (wall[nWall].cstat&16384)
            {
                if (vbp)
                    RotatePoint((int*)&x, (int*)&y, vbp, a4, a5);
                DragPoint(nWall, x+vc-a4, y+v8-a5);
                if ((wall[v10].cstat&49152) == 0)
                {
                    x = baseWall[v10].x;
                    y = baseWall[v10].y;
                    if (vbp)
                        RotatePoint((int*)&x, (int*)&y, vbp, a4, a5);
                    DragPoint(v10, x+vc-a4, y+v8-a5);
                }
                continue;
            }
            if (wall[nWall].cstat&32768)
            {
                if (vbp)
                    RotatePoint((int*)&x, (int*)&y, -vbp, a4, a5);
                DragPoint(nWall, x-(vc-a4), y-(v8-a5));
                if ((wall[v10].cstat&49152) == 0)
                {
                    x = baseWall[v10].x;
                    y = baseWall[v10].y;
                    if (vbp)
                        RotatePoint((int*)&x, (int*)&y, -vbp, a4, a5);
                    DragPoint(v10, x-(vc-a4), y-(v8-a5));
                }
                continue;
            }
        }
    }
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 10 || pSprite->statnum == 16)
            continue;
        x = baseSprite[nSprite].x;
        y = baseSprite[nSprite].y;
        if (sprite[nSprite].cstat&8192)
        {
            if (vbp)
                RotatePoint((int*)&x, (int*)&y, vbp, a4, a5);
            viewBackupSpriteLoc(nSprite, pSprite);
            pSprite->ang = (pSprite->ang+v14)&2047;
            pSprite->x = x+vc-a4;
            pSprite->y = y+v8-a5;
        }
        else if (sprite[nSprite].cstat&16384)
        {
            if (vbp)
                RotatePoint((int*)&x, (int*)&y, -vbp, a4, a5);
            viewBackupSpriteLoc(nSprite, pSprite);
            pSprite->ang = (pSprite->ang-v14)&2047;
            pSprite->x = x-(vc-a4);
            pSprite->y = y-(v8-a5);
        }
        else if (pXSector->at13_3)
        {
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            int floorZ = getflorzofslope(nSector, pSprite->x, pSprite->y);
            if (!(pSprite->cstat&48) && floorZ <= bottom)
            {
                if (v14)
                    RotatePoint((int*)&pSprite->x, (int*)&pSprite->y, v14, v20, v24);
                viewBackupSpriteLoc(nSprite, pSprite);
                pSprite->ang = (pSprite->ang+v14)&2047;
                pSprite->x += v28;
                pSprite->y += v2c;
            }
        }
    }
}

void ZTranslateSector(int nSector, XSECTOR *pXSector, int a3, int a4)
{
    sectortype *pSector = &sector[nSector];
    viewInterpolateSector(nSector, pSector);
    int dz = pXSector->at28_0-pXSector->at24_0;
    if (dz != 0)
    {
        int oldZ = pSector->floorz;
        baseFloor[nSector] = pSector->floorz = pXSector->at24_0 + mulscale16(dz, GetWaveValue(a3, a4));
        velFloor[nSector] += (pSector->floorz-oldZ)<<8;
        for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
        {
            spritetype *pSprite = &sprite[nSprite];
            if (pSprite->statnum == 10 || pSprite->statnum == 16)
                continue;
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            if (pSprite->cstat&8192)
            {
                viewBackupSpriteLoc(nSprite, pSprite);
                pSprite->z += pSector->floorz-oldZ;
            }
            else if (pSprite->hitag&2)
                pSprite->hitag |= 4;
            else if (oldZ <= bottom && !(pSprite->cstat&48))
            {
                viewBackupSpriteLoc(nSprite, pSprite);
                pSprite->z += pSector->floorz-oldZ;
            }
        }
    }
    dz = pXSector->at20_0-pXSector->at1c_0;
    if (dz != 0)
    {
        int oldZ = pSector->ceilingz;
        baseCeil[nSector] = pSector->ceilingz = pXSector->at1c_0 + mulscale16(dz, GetWaveValue(a3, a4));
        velCeil[nSector] += (pSector->ceilingz-oldZ)<<8;
        for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
        {
            spritetype *pSprite = &sprite[nSprite];
            if (pSprite->statnum == 10 || pSprite->statnum == 16)
                continue;
            if (pSprite->cstat&16384)
            {
                viewBackupSpriteLoc(nSprite, pSprite);
                pSprite->z += pSector->ceilingz-oldZ;
            }
        }
    }
}

int GetHighestSprite(int nSector, int nStatus, int *a3)
{
    *a3 = sector[nSector].floorz;
    int v8 = -1;
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        if (sprite[nSprite].statnum == nStatus || nStatus == 1024)
        {
            spritetype *pSprite = &sprite[nSprite];
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            if (top-pSprite->z > *a3)
            {
                *a3 = top-pSprite->z;
                v8 = nSprite;
            }
        }
    }
    return v8;
}

int GetCrushedSpriteExtents(unsigned int nSector, int *pzTop, int *pzBot)
{
    dassert(pzTop != NULL && pzBot != NULL);
    dassert(nSector < (unsigned int)numsectors);
    int vc = -1;
    sectortype *pSector = &sector[nSector];
    int vbp = pSector->ceilingz;
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 6 || pSprite->statnum == 4)
        {
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
            if (vbp > top)
            {
                vbp = top;
                *pzTop = top;
                *pzBot = bottom;
                vc = nSprite;
            }
        }
    }
    return vc;
}

int VCrushBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    int nXSector = sector[nSector].extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    int nWave;
    if (pXSector->at1_7 < a2)
        nWave = pXSector->at7_2;
    else
        nWave = pXSector->at7_5;
    int dz1 = pXSector->at20_0 - pXSector->at1c_0;
    int vc = pXSector->at1c_0;
    if (dz1 != 0)
        vc += mulscale16(dz1, GetWaveValue(a2, nWave));
    int dz2 = pXSector->at28_0 - pXSector->at24_0;
    int v10 = pXSector->at24_0;
    if (dz2 != 0)
        v10 += mulscale16(dz2, GetWaveValue(a2, nWave));
    int v18;
    if (GetHighestSprite(nSector, 6, &v18) >= 0 && vc >= v18)
        return 1;
    viewInterpolateSector(nSector, &sector[nSector]);
    if (dz1 != 0)
        sector[nSector].ceilingz = vc;
    if (dz2 != 0)
        sector[nSector].floorz = v10;
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        return 3;
    }
    return 0;
}

int VSpriteBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    int nXSector = sector[nSector].extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    int nWave;
    if (pXSector->at1_7 < a2)
        nWave = pXSector->at7_2;
    else
        nWave = pXSector->at7_5;
    int dz1 = pXSector->at28_0 - pXSector->at24_0;
    if (dz1 != 0)
    {
        for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
        {
            spritetype *pSprite = &sprite[nSprite];
            if (pSprite->cstat&8192)
            {
                viewBackupSpriteLoc(nSprite, pSprite);
                pSprite->z = baseSprite[nSprite].z+mulscale16(dz1, GetWaveValue(a2, nWave));
            }
        }
    }
    int dz2 = pXSector->at20_0 - pXSector->at1c_0;
    if (dz2 != 0)
    {
        for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
        {
            spritetype *pSprite = &sprite[nSprite];
            if (pSprite->cstat&16384)
            {
                viewBackupSpriteLoc(nSprite, pSprite);
                pSprite->z = baseSprite[nSprite].z+mulscale16(dz2, GetWaveValue(a2, nWave));
            }
        }
    }
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        return 3;
    }
    return 0;
}

int VDoorBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    int nXSector = sector[nSector].extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    int vbp;
    if (pXSector->at1_6)
        vbp = 65536/ClipLow((120*pXSector->ata_4)/10, 1);
    else
        vbp = -65536/ClipLow((120*pXSector->at18_2)/10, 1);
    int top, bottom;
    int nSprite = GetCrushedSpriteExtents(nSector,&top,&bottom);
    if (nSprite >= 0 && a2 > pXSector->at1_7)
    {
        spritetype *pSprite = &sprite[nSprite];
        dassert(pSprite->extra > 0 && pSprite->extra < kMaxXSprites);
        XSPRITE *pXSprite = &xsprite[pSprite->extra];
        if (pXSector->at20_0 > pXSector->at1c_0 || pXSector->at28_0 < pXSector->at24_0)
        {
            if (pXSector->atd_5)
            {
                if (pXSector->at30_0)
                {
                    if (pXSprite->health <= 0)
                        return 2;
                    int nDamage;
                    if (pXSector->at4_0 == 0)
                        nDamage = 500;
                    else
                        nDamage = pXSector->at4_0;
                    actDamageSprite(nSprite, &sprite[nSprite], DAMAGE_TYPE_0, nDamage<<4);
                }
                a2 = ClipRange(a2-(vbp/2)*4, 0, 65536);
            }
            else if (pXSector->at30_0 && pXSprite->health > 0)
            {
                int nDamage;
                if (pXSector->at4_0 == 0)
                    nDamage = 500;
                else
                    nDamage = pXSector->at4_0;
                actDamageSprite(nSprite, &sprite[nSprite], DAMAGE_TYPE_0, nDamage<<4);
                a2 = ClipRange(a2-(vbp/2)*4, 0, 65536);
            }
        }
    }
    else if (nSprite >= 0 && a2 < pXSector->at1_7)
    {
        spritetype *pSprite = &sprite[nSprite];
        dassert(pSprite->extra > 0 && pSprite->extra < kMaxXSprites);
        XSPRITE *pXSprite = &xsprite[pSprite->extra];
        if (pXSector->at1c_0 > pXSector->at20_0 || pXSector->at24_0 < pXSector->at28_0)
        {
            if (pXSector->atd_5)
            {
                if (pXSector->at30_0)
                {
                    if (pXSprite->health <= 0)
                        return 2;
                    int nDamage;
                    if (pXSector->at4_0 == 0)
                        nDamage = 500;
                    else
                        nDamage = pXSector->at4_0;
                    actDamageSprite(nSprite, &sprite[nSprite], DAMAGE_TYPE_0, nDamage<<4);
                }
                a2 = ClipRange(a2+(vbp/2)*4, 0, 65536);
            }
            else if (pXSector->at30_0 && pXSprite->health > 0)
            {
                int nDamage;
                if (pXSector->at4_0 == 0)
                    nDamage = 500;
                else
                    nDamage = pXSector->at4_0;
                actDamageSprite(nSprite, &sprite[nSprite], DAMAGE_TYPE_0, nDamage<<4);
                a2 = ClipRange(a2+(vbp/2)*4, 0, 65536);
            }
        }
    }
    int nWave;
    if (pXSector->at1_7 < a2)
        nWave = pXSector->at7_2;
    else
        nWave = pXSector->at7_5;
    ZTranslateSector(nSector, pXSector, a2, nWave);
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        return 3;
    }
    return 0;
}

int HDoorBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    sectortype *pSector = &sector[nSector];
    int nXSector = pSector->extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    int nWave;
    if (pXSector->at1_7 < a2)
        nWave = pXSector->at7_2;
    else
        nWave = pXSector->at7_5;
    spritetype *pSprite1 = &sprite[pXSector->at2c_0];
    spritetype *pSprite2 = &sprite[pXSector->at2e_0];
    TranslateSector(nSector, GetWaveValue(pXSector->at1_7, nWave), GetWaveValue(a2, nWave), pSprite1->x, pSprite1->y, pSprite1->x, pSprite1->y, pSprite1->ang, pSprite2->x, pSprite2->y, pSprite2->ang, pSector->lotag == 616);
    ZTranslateSector(nSector, pXSector, a2, nWave);
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        return 3;
    }
    return 0;
}

int RDoorBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    sectortype *pSector = &sector[nSector];
    int nXSector = pSector->extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    int nWave;
    if (pXSector->at1_7 < a2)
        nWave = pXSector->at7_2;
    else
        nWave = pXSector->at7_5;
    spritetype *pSprite = &sprite[pXSector->at2c_0];
    TranslateSector(nSector, GetWaveValue(pXSector->at1_7, nWave), GetWaveValue(a2, nWave), pSprite->x, pSprite->y, pSprite->x, pSprite->y, 0, pSprite->x, pSprite->y, pSprite->ang, pSector->lotag == 617);
    ZTranslateSector(nSector, pXSector, a2, nWave);
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        return 3;
    }
    return 0;
}

int StepRotateBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    sectortype *pSector = &sector[nSector];
    int nXSector = pSector->extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    spritetype *pSprite = &sprite[pXSector->at2c_0];
    int vbp;
    if (pXSector->at1_7 < a2)
    {
        vbp = pXSector->at4_0+pSprite->ang;
        int nWave = pXSector->at7_2;
        TranslateSector(nSector, GetWaveValue(pXSector->at1_7, nWave), GetWaveValue(a2, nWave), pSprite->x, pSprite->y, pSprite->x, pSprite->y, pXSector->at4_0, pSprite->x, pSprite->y, vbp, 1);
    }
    else
    {
        vbp = pXSector->at4_0-pSprite->ang;
        int nWave = pXSector->at7_5;
        TranslateSector(nSector, GetWaveValue(pXSector->at1_7, nWave), GetWaveValue(a2, nWave), pSprite->x, pSprite->y, pSprite->x, pSprite->y, vbp, pSprite->x, pSprite->y, pXSector->at4_0, 1);
    }
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        pXSector->at4_0 = vbp&2047;
        return 3;
    }
    return 0;
}

int GenSectorBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    sectortype *pSector = &sector[nSector];
    int nXSector = pSector->extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    pXSector->at1_7 = a2;
    if (pXSector->at9_2 == 5 && pXSector->at6_0)
        evSend(nSector, 6, pXSector->at6_0, COMMAND_ID_5);
    if ((a2&0xffff) == 0)
    {
        SetSectorState(nSector, pXSector, a2>>16);
        SectorEndSound(nSector, a2>>16);
        return 3;
    }
    return 0;
}

int PathBusy(unsigned int nSector, unsigned int a2)
{
    dassert(nSector < (unsigned int)numsectors);
    sectortype *pSector = &sector[nSector];
    int nXSector = pSector->extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR *pXSector = &xsector[nXSector];
    spritetype *pSprite = &sprite[basePath[nSector]];
    spritetype *pSprite1 = &sprite[pXSector->at2c_0];
    XSPRITE *pXSprite1 = &xsprite[pSprite1->extra];
    spritetype *pSprite2 = &sprite[pXSector->at2e_0];
    XSPRITE *pXSprite2 = &xsprite[pSprite2->extra];
    int nWave = pXSprite1->at7_6;
    TranslateSector(nSector, GetWaveValue(pXSector->at1_7, nWave), GetWaveValue(a2, nWave), pSprite->x, pSprite->y, pSprite1->x, pSprite1->y, pSprite1->ang, pSprite2->x, pSprite2->y, pSprite2->ang, 1);
    ZTranslateSector(nSector, pXSector, a2, nWave);
    pXSector->at1_7 = a2;
    if ((a2&0xffff) == 0)
    {
        evPost(nSector, 6, (120*pXSprite2->at9_4)/10, COMMAND_ID_1);
        pXSector->at1_6 = 0;
        pXSector->at1_7 = 0;
        if (pXSprite1->at18_2)
            PathSound(nSector, pXSprite1->at18_2);
        pXSector->at2c_0 = pXSector->at2e_0;
        pXSector->at4_0 = pXSprite2->at10_0;
        return 3;
    }
    return 0;
}

void OperateDoor(unsigned int nSector, XSECTOR *pXSector, EVENT a3, BUSYID a4) 
{
    switch (a3.at2_0)
    {
    case 0:
        if (pXSector->at1_7)
        {
            AddBusy(nSector, a4, -65536/ClipLow((pXSector->at18_2*120)/10, 1));
            SectorStartSound(nSector, 1);
        }
        break;
    case 1:
        if (pXSector->at1_7 != 0x10000)
        {
            AddBusy(nSector, a4, 65536/ClipLow((pXSector->ata_4*120)/10, 1));
            SectorStartSound(nSector, 0);
        }
        break;
    default:
        if (pXSector->at1_7&0xffff)
        {
            if (pXSector->atd_5)
            {
                ReverseBusy(nSector, a4);
                pXSector->at1_6 = !pXSector->at1_6;
            }
        }
        else
        {
            char t = !pXSector->at1_6;
            int nDelta;
            if (t)
                nDelta = 65536/ClipLow((pXSector->ata_4*120)/10, 1);
            else
                nDelta = -65536/ClipLow((pXSector->at18_2*120)/10, 1);
            AddBusy(nSector, a4, nDelta);
            SectorStartSound(nSector, pXSector->at1_6);
        }
        break;
    }
}

char SectorContainsDudes(int nSector)
{
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        if (sprite[nSprite].statnum == 6)
            return 1;
    }
    return 0;
}

void TeleFrag(int nKiller, int nSector)
{
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 6)
            actDamageSprite(nKiller, pSprite, DAMAGE_TYPE_3, 4000);
        else if (pSprite->statnum == 4)
            actDamageSprite(nKiller, pSprite, DAMAGE_TYPE_3, 4000);
    }
}

void OperateTeleport(unsigned int nSector, XSECTOR *pXSector)
{
    dassert(nSector < (unsigned int)numsectors);
    int nDest = pXSector->at2c_0;
    dassert(nDest < kMaxSprites);
    spritetype *pDest = &sprite[nDest];
    dassert(pDest->statnum == kStatMarker);
    dassert(pDest->type == kMarkerWarpDest);
    dassert(pDest->sectnum >= 0 && pDest->sectnum < kMaxSectors);
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
    {
        spritetype *pSprite = &sprite[nSprite];
        if (pSprite->statnum == 6)
        {
            PLAYER *pPlayer;
            char bPlayer = IsPlayerSprite(pSprite);
            if (bPlayer)
                pPlayer = &gPlayer[pSprite->type-kDudePlayer1];
            else
                pPlayer = NULL;
            if (bPlayer || !SectorContainsDudes(pDest->sectnum))
            {
                if (!(gGameOptions.uNetGameFlags&2))
                    TeleFrag(pXSector->at4_0, pDest->sectnum);
                pSprite->x = pDest->x;
                pSprite->y = pDest->y;
                pSprite->z += sector[pDest->sectnum].floorz-sector[nSector].floorz;
                pSprite->ang = pDest->ang;
                ChangeSpriteSect(nSprite, pDest->sectnum);
                sfxPlay3DSound(pDest, 201, -1, 0);
                xvel[nSprite] = yvel[nSprite] = zvel[nSprite] = 0;
                ClearBitString(gInterpolateSprite, nSprite);
                viewBackupSpriteLoc(nSprite, pSprite);
                if (pPlayer)
                {
                    playerResetInertia(pPlayer);
                    pPlayer->at6b = pPlayer->at73 = 0;
                }
            }
        }
    }
}

void OperatePath(unsigned int nSector, XSECTOR *pXSector, EVENT a3)
{
    int nSprite;
    spritetype *pSprite = NULL;
    XSPRITE *pXSprite;
    dassert(nSector < (unsigned int)numsectors);
    spritetype *pSprite2 = &sprite[pXSector->at2c_0];
    XSPRITE *pXSprite2 = &xsprite[pSprite2->extra];
    int nId = pXSprite2->at12_0;
    for (nSprite = headspritestat[16]; nSprite >= 0; nSprite = nextspritestat[nSprite])
    {
        pSprite = &sprite[nSprite];
        if (pSprite->type == 15)
        {
            pXSprite = &xsprite[pSprite->extra];
            if (pXSprite->at10_0 == nId)
                break;
        }
    }
    if (nSprite < 0)
        ThrowError("Unable to find path marker with id #%d", nId);
    pXSector->at2e_0 = nSprite;
    pXSector->at24_0 = pSprite2->z;
    pXSector->at28_0 = pSprite->z;
    switch (a3.at2_0)
    {
    case 1:
        pXSector->at1_6 = 0;
        pXSector->at1_7 = 0;
        AddBusy(nSector, BUSYID_7, 65536/ClipLow((120*pXSprite2->at8_0)/10,1));
        if (pXSprite2->at14_0)
            PathSound(nSector, pXSprite2->at14_0);
        break;
    }
}

void OperateSector(unsigned int nSector, XSECTOR *pXSector, EVENT a3)
{
    dassert(nSector < (unsigned int)numsectors);
    sectortype *pSector = &sector[nSector];
    switch (a3.at2_0)
    {
    case 6:
        pXSector->at35_0 = 1;
        break;
    case 7:
        pXSector->at35_0 = 0;
        break;
    case 8:
        pXSector->at35_0 ^= 1;
        break;
    case 9:
        pXSector->at1b_2 = 0;
        pXSector->at1b_3 = 1;
        break;
    case 10:
        pXSector->at1b_2 = 1;
        pXSector->at1b_3 = 0;
        break;
    case 11:
        pXSector->at1b_2 = 1;
        pXSector->at1b_3 = 1;
        break;
    default:
        switch (pSector->lotag)
        {
        case 602:
            OperateDoor(nSector, pXSector, a3, BUSYID_1);
            break;
        case 600:
            OperateDoor(nSector, pXSector, a3, BUSYID_2);
            break;
        case 614:
        case 616:
            OperateDoor(nSector, pXSector, a3, BUSYID_3);
            break;
        case 615:
        case 617:
            OperateDoor(nSector, pXSector, a3, BUSYID_4);
            break;
        case 613:
            switch (a3.at2_0)
            {
            case 1:
                pXSector->at1_6 = 0;
                pXSector->at1_7 = 0;
                AddBusy(nSector, BUSYID_5, 65536/ClipLow((120*pXSector->ata_4)/10, 1));
                SectorStartSound(nSector, 0);
                break;
            case 0:
                pXSector->at1_6 = 1;
                pXSector->at1_7 = 65536;
                AddBusy(nSector, BUSYID_5, -65536/ClipLow((120*pXSector->at18_2)/10, 1));
                SectorStartSound(nSector, 1);
                break;
            }
            break;
        case 604:
            OperateTeleport(nSector, pXSector);
            break;
        case 612:
            OperatePath(nSector, pXSector, a3);
            break;
        default:
            if (pXSector->ata_4 || pXSector->at18_2)
                OperateDoor(nSector, pXSector, a3, BUSYID_6);
            else
            {
                switch (a3.at2_0)
                {
                case 0:
                    SetSectorState(nSector, pXSector, 0);
                    break;
                case 1:
                    SetSectorState(nSector, pXSector, 1);
                    break;
                default:
                    SetSectorState(nSector, pXSector, pXSector->at1_6^1);
                    break;
                }
            }
            break;
        }
        break;
    }
}

void InitPath(unsigned int nSector, XSECTOR *pXSector)
{
    int nSprite;
    spritetype *pSprite;
    XSPRITE *pXSprite;
    dassert(nSector < (unsigned int)numsectors);
    int nId = pXSector->at4_0;
    for (nSprite = headspritestat[16]; nSprite >= 0; nSprite = nextspritestat[nSprite])
    {
        pSprite = &sprite[nSprite];
        if (pSprite->type == 15)
        {
            pXSprite = &xsprite[pSprite->extra];
            if (pXSprite->at10_0 == nId)
                break;
        }
    }
    if (nSprite < 0)
        ThrowError("Unable to find path marker with id #%d", nId);
    pXSector->at2c_0 = nSprite;
    basePath[nSector] = nSprite;
    if (pXSector->at1_6)
        evPost(nSector, 6, 0, COMMAND_ID_1);
}

void LinkSector(int nSector, XSECTOR *pXSector, EVENT a3)
{
    sectortype *pSector = &sector[nSector];
    int nBusy = GetSourceBusy(a3);
    switch (pSector->lotag)
    {
    case 602:
        VSpriteBusy(nSector, nBusy);
        break;
    case 600:
        VDoorBusy(nSector, nBusy);
        break;
    case 614:
    case 616:
        HDoorBusy(nSector, nBusy);
        break;
    case 615:
    case 617:
        RDoorBusy(nSector, nBusy);
        break;
    default:
        pXSector->at1_7 = nBusy;
        if ((pXSector->at1_7&0xffff) == 0)
            SetSectorState(nSector, pXSector, nBusy>>16);
        break;
    }
}

void LinkSprite(int nSprite, XSPRITE *pXSprite, EVENT a3)
{
    spritetype *pSprite = &sprite[nSprite];
    int nBusy = GetSourceBusy(a3);
    if (pSprite->type == 22)
    {
        if (a3.at1_5 == 3)
        {
            int nSprite2 = a3.at0_0;
            int nXSprite2 = sprite[nSprite2].extra;
            dassert(nXSprite2 > 0 && nXSprite2 < kMaxXSprites);
            pXSprite->at10_0 = xsprite[nXSprite2].at10_0;
            if (pXSprite->at10_0 == pXSprite->at12_0)
                SetSpriteState(nSprite, pXSprite, 1);
            else
                SetSpriteState(nSprite, pXSprite, 0);
        }
    }
    else
    {
        pXSprite->at1_7 = nBusy;
        if ((pXSprite->at1_7&0xffff) == 0)
            SetSpriteState(nSprite, pXSprite, nBusy>>16);
    }
}

void LinkWall(int nWall, XWALL *pXWall, EVENT a3)
{
    int nBusy = GetSourceBusy(a3);
    pXWall->at1_7 = nBusy;
    if ((pXWall->at1_7 & 0xffff) == 0)
        SetWallState(nWall, pXWall, nBusy>>16);
}

void trTriggerSector(unsigned int nSector, XSECTOR *pXSector, int a3)
{
    dassert(nSector < (unsigned int)numsectors);
    if (!pXSector->at35_0 && !pXSector->at16_6)
    {
        if (pXSector->at16_5)
            pXSector->at16_6 = 1;
        if (pXSector->at16_4)
        {
            if (pXSector->at6_0)
                evSend(nSector, 6, pXSector->at6_0, (COMMAND_ID)pXSector->at9_2);
        }
        else
        {
            EVENT evnt;
            evnt.at2_0 = a3;
            OperateSector(nSector, pXSector, evnt);
        }
    }
}

void trMessageSector(unsigned int nSector, EVENT a2)
{
    dassert(nSector < (unsigned int)numsectors);
    dassert(sector[nSector].extra > 0 && sector[nSector].extra < kMaxXSectors);
    int nXSector = sector[nSector].extra;
    XSECTOR *pXSector = &xsector[nXSector];
    if (!pXSector->at35_0 || a2.at2_0 == 7 || a2.at2_0 == 8)
    {
        if (a2.at2_0 == 5)
            LinkSector(nSector, pXSector, a2);
        else
            OperateSector(nSector, pXSector, a2);
    }
}

void trTriggerWall(unsigned int nWall, XWALL *pXWall, int a3)
{
    dassert(nWall < (unsigned int)numwalls);
    if (!pXWall->at13_2 && !pXWall->at10_1)
    {
        if (pXWall->at10_0)
            pXWall->at10_1 = 1;
        if (pXWall->atf_7)
        {
            if (pXWall->at6_0)
                evSend(nWall, 0, pXWall->at6_0, (COMMAND_ID)pXWall->at9_2);
        }
        else
        {
            EVENT evnt;
            evnt.at2_0 = a3;
            OperateWall(nWall, pXWall, evnt);
        }
    }
}

void trMessageWall(unsigned int nWall, EVENT a2)
{
    dassert(nWall < (unsigned int)numwalls);
    dassert(wall[nWall].extra > 0 && wall[nWall].extra < kMaxXWalls);
    int nXWall = wall[nWall].extra;
    XWALL *pXWall = &xwall[nXWall];
    if (!pXWall->at13_2 || a2.at2_0 == 7 || a2.at2_0 == 8)
    {
        if (a2.at2_0 == 5)
            LinkWall(nWall, pXWall, a2);
        else
            OperateWall(nWall, pXWall, a2);
    }
}

void trTriggerSprite(unsigned int nSprite, XSPRITE *pXSprite, int a3)
{
    if (!pXSprite->at17_5 && !pXSprite->atd_2)
    {
        if (pXSprite->atd_1)
            pXSprite->atd_2 = 1;
        if (pXSprite->atd_0)
        {
            if (pXSprite->at4_0)
                evSend(nSprite, 3, pXSprite->at4_0, (COMMAND_ID)pXSprite->at6_4);
        }
        else
        {
            EVENT evnt;
            evnt.at2_0 = a3;
            OperateSprite(nSprite, pXSprite, evnt);
        }
    }
}

void trMessageSprite(unsigned int nSprite, EVENT a2)
{
    if (sprite[nSprite].statnum == kStatFree)
        return;
    int nXSprite = sprite[nSprite].extra;
    XSPRITE *pXSprite = &xsprite[nXSprite];
    if (!pXSprite->at17_5 || a2.at2_0 == 7 || a2.at2_0 == 8)
    {
        if (a2.at2_0 == 5)
            LinkSprite(nSprite, pXSprite, a2);
        else
            OperateSprite(nSprite, pXSprite, a2);
    }
}

void ProcessMotion(void)
{
    sectortype *pSector;
    int nSector;
    for (pSector = sector, nSector = 0; nSector < numsectors; nSector++, pSector++)
    {
        int nXSector = pSector->extra;
        if (nXSector <= 0)
            continue;
        XSECTOR *pXSector = &xsector[nXSector];
        if (pXSector->at3a_0 != 0)
        {
            if (pXSector->at3b_4)
                pXSector->at38_0 += pXSector->at3a_0;
            else if (pXSector->at1_7 == 0)
                continue;
            else
                pXSector->at38_0 += mulscale16(pXSector->at3a_0, pXSector->at1_7);
            int vdi = mulscale30(Sin(pXSector->at38_0), pXSector->at39_3<<8);
            for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
            {
                spritetype *pSprite = &sprite[nSprite];
                if (pSprite->cstat&24576)
                {
                    viewBackupSpriteLoc(nSprite, pSprite);
                    pSprite->z += vdi;
                }
            }
            if (pXSector->at3b_5)
            {
                int floorZ = pSector->floorz;
                viewInterpolateSector(nSector, pSector);
                pSector->floorz = baseFloor[nSector]+vdi;
                for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
                {
                    spritetype *pSprite = &sprite[nSprite];
                    if (pSprite->hitag&2)
                        pSprite->hitag |= 4;
                    else
                    {
                        int top, bottom;
                        GetSpriteExtents(pSprite, &top, &bottom);
                        if (bottom >= floorZ && (pSprite->cstat&48) == 0)
                        {
                            viewBackupSpriteLoc(nSprite, pSprite);
                            pSprite->z += vdi;
                        }
                    }
                }
            }
            if (pXSector->at3b_6)
            {
                int ceilZ = pSector->ceilingz;
                viewInterpolateSector(nSector, pSector);
                pSector->ceilingz = baseCeil[nSector]+vdi;
                for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite])
                {
                    spritetype *pSprite = &sprite[nSprite];
                    int top, bottom;
                    GetSpriteExtents(pSprite, &top, &bottom);
                    if (top <= ceilZ && (pSprite->cstat&48) == 0)
                    {
                        viewBackupSpriteLoc(nSprite, pSprite);
                        pSprite->z += vdi;
                    }
                }
            }
        }
    }
}

void AlignSlopes(void)
{
    sectortype *pSector;
    int nSector;
    for (pSector = sector, nSector = 0; nSector < numsectors; nSector++, pSector++)
    {
        if (qsector_filler[nSector])
        {
            walltype *pWall = &wall[pSector->wallptr+qsector_filler[nSector]];
            walltype *pWall2 = &wall[pWall->point2];
            int nNextSector = pWall->nextsector;
            if (nNextSector >= 0)
            {
                int x = (pWall->x+pWall2->x)/2;
                int y = (pWall->y+pWall2->y)/2;
                viewInterpolateSector(nSector, pSector);
                alignflorslope(nSector, x, y, getflorzofslope(nNextSector, x, y));
                alignceilslope(nSector, x, y, getceilzofslope(nNextSector, x, y));
            }
        }
    }
}

int(*gBusyProc[])(unsigned int, unsigned int) =
{
    VCrushBusy,
    VSpriteBusy,
    VDoorBusy,
    HDoorBusy,
    RDoorBusy,
    StepRotateBusy,
    GenSectorBusy,
    PathBusy
};

void trProcessBusy(void)
{
    memset(velFloor, 0, sizeof(velFloor));
    memset(velCeil, 0, sizeof(velCeil));
    for (int i = gBusyCount-1; i >= 0; i--)
    {
        int oldBusy = gBusy[i].at8;
        gBusy[i].at8 = ClipRange(oldBusy+gBusy[i].at4*4, 0, 65536);
        int nStatus = gBusyProc[gBusy[i].atc](gBusy[i].at0, gBusy[i].at8);
        switch (nStatus)
        {
        case 1:
            gBusy[i].at8 = oldBusy;
            break;
        case 2:
            gBusy[i].at8 = oldBusy;
            gBusy[i].at4 = -gBusy[i].at4;
            break;
        case 3:
            gBusy[i] = gBusy[--gBusyCount];
            break;
        }
    }
    ProcessMotion();
    AlignSlopes();
}

void InitGenerator(int);

void trInit(void)
{
    gBusyCount = 0;
    for (int i = 0; i < numwalls; i++)
    {
        baseWall[i].x = wall[i].x;
        baseWall[i].y = wall[i].y;
    }
    for (int i = 0; i < kMaxSprites; i++)
    {
        if (sprite[i].statnum < kStatFree)
        {
            sprite[i].zvel = sprite[i].type;
            baseSprite[i].x = sprite[i].x;
            baseSprite[i].y = sprite[i].y;
            baseSprite[i].z = sprite[i].z;
        }
        else
            sprite[i].zvel = -1;
    }
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = wall[i].extra;
        dassert(nXWall < kMaxXWalls);
        if (nXWall > 0)
        {
            XWALL *pXWall = &xwall[nXWall];
            if (pXWall->at1_6)
                pXWall->at1_7 = 65536;
        }
    }
    dassert((numsectors >= 0) && (numsectors < kMaxSectors));
    for (int i = 0; i < numsectors; i++)
    {
        sectortype *pSector = &sector[i];
        baseFloor[i] = pSector->floorz;
        baseCeil[i] = pSector->ceilingz;
        int nXSector = pSector->extra;
        if (nXSector > 0)
        {
            dassert(nXSector < kMaxXSectors);
            XSECTOR *pXSector = &xsector[nXSector];
            if (pXSector->at1_6)
                pXSector->at1_7 = 65536;
            switch (pSector->lotag)
            {
            case 619:
                pXSector->at16_5 = 1;
                evPost(i, 6, 0, CALLBACK_ID_12);
                break;
            case 600:
            case 602:
                ZTranslateSector(i, pXSector, pXSector->at1_7, 1);
                break;
            case 614:
            case 616:
            {
                spritetype *pSprite1 = &sprite[pXSector->at2c_0];
                spritetype *pSprite2 = &sprite[pXSector->at2e_0];
                TranslateSector(i, 0, -65536, pSprite1->x, pSprite1->y, pSprite1->x, pSprite1->y, pSprite1->ang, pSprite2->x, pSprite2->y, pSprite2->ang, pSector->lotag == 616);
                for (int j = 0; j < pSector->wallnum; j++)
                {
                    baseWall[pSector->wallptr+j].x = wall[pSector->wallptr+j].x;
                    baseWall[pSector->wallptr+j].y = wall[pSector->wallptr+j].y;
                }
                for (int nSprite = headspritesect[i]; nSprite >= 0; nSprite = nextspritesect[nSprite])
                {
                    baseSprite[nSprite].x = sprite[nSprite].x;
                    baseSprite[nSprite].y = sprite[nSprite].y;
                    baseSprite[nSprite].z = sprite[nSprite].z;
                }
                TranslateSector(i, 0, pXSector->at1_7, pSprite1->x, pSprite1->y, pSprite1->x, pSprite1->y, pSprite1->ang, pSprite2->x, pSprite2->y, pSprite2->ang, pSector->lotag == 616);
                ZTranslateSector(i, pXSector, pXSector->at1_7, 1);
                break;
            }
            case 615:
            case 617:
            {
                spritetype *pSprite1 = &sprite[pXSector->at2c_0];
                TranslateSector(i, 0, -65536, pSprite1->x, pSprite1->y, pSprite1->x, pSprite1->y, 0, pSprite1->x, pSprite1->y, pSprite1->ang, pSector->lotag == 617);
                for (int j = 0; j < pSector->wallnum; j++)
                {
                    baseWall[pSector->wallptr+j].x = wall[pSector->wallptr+j].x;
                    baseWall[pSector->wallptr+j].y = wall[pSector->wallptr+j].y;
                }
                for (int nSprite = headspritesect[i]; nSprite >= 0; nSprite = nextspritesect[nSprite])
                {
                    baseSprite[nSprite].x = sprite[nSprite].x;
                    baseSprite[nSprite].y = sprite[nSprite].y;
                    baseSprite[nSprite].z = sprite[nSprite].z;
                }
                TranslateSector(i, 0, pXSector->at1_7, pSprite1->x, pSprite1->y, pSprite1->x, pSprite1->y, 0, pSprite1->x, pSprite1->y, pSprite1->ang, pSector->lotag == 617);
                ZTranslateSector(i, pXSector, pXSector->at1_7, 1);
                break;
            }
            case 612:
                InitPath(i, pXSector);
                break;
            default:
                break;
            }
        }
    }
    for (int i = 0; i < kMaxSprites; i++)
    {
        int nXSprite = sprite[i].extra;
        if (sprite[i].statnum < kStatFree && nXSprite > 0)
        {
            dassert(nXSprite < kMaxXSprites);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            if (pXSprite->at1_6)
                pXSprite->at1_7 = 65536;
            switch (sprite[i].type)
            {
            case 23:
                pXSprite->atd_1 = 1;
                break;
            case 700:
            case 701:
            case 702:
            case 703:
            case 704:
            case 705:
            case 706:
            case 707:
            case 708:
                InitGenerator(i);
                break;
            case 401:
                pXSprite->ate_4 = 1;
                break;
            case 414:
                if (pXSprite->at1_6)
                    sprite[i].hitag |= 7;
                else
                    sprite[i].hitag &= ~7;
                break;
            }
            if (pXSprite->atd_7)
                sprite[i].cstat |= 256;
            if (pXSprite->atd_6)
                sprite[i].cstat |= 4096;
        }
    }
    evSend(0, 0, 7, COMMAND_ID_1);
    if (gGameOptions.nGameType == 1)
        evSend(0, 0, 9, COMMAND_ID_1);
    else if (gGameOptions.nGameType == 2)
        evSend(0, 0, 8, COMMAND_ID_1);
    else if (gGameOptions.nGameType == 3)
    {
        evSend(0, 0, 8, COMMAND_ID_1);
        evSend(0, 0, 10, COMMAND_ID_1);
    }
}

void trTextOver(int nId)
{
    char *pzMessage = levelGetMessage(nId);
    if (pzMessage)
        viewSetMessage(pzMessage);
}

void InitGenerator(int nSprite)
{
    dassert(nSprite < kMaxSprites);
    spritetype *pSprite = &sprite[nSprite];
    dassert(pSprite->statnum != kMaxStatus);
    int nXSprite = pSprite->extra;
    dassert(nXSprite > 0);
    XSPRITE *pXSprite = &xsprite[nXSprite];
    switch (sprite[nSprite].type)
    {
    case 700:
        pSprite->cstat &= (unsigned short)~(32768+1);
        pSprite->cstat |= 32768;
        break;
    }
    if (pXSprite->at1_6 != pXSprite->atb_0 && pXSprite->at8_0 > 0)
        evPost(nSprite, 3, (120*(pXSprite->at8_0+Random2(pXSprite->at10_0)))/10, COMMAND_ID_21);
}

void ActivateGenerator(int nSprite)
{
    dassert(nSprite < kMaxSprites);
    spritetype *pSprite = &sprite[nSprite];
    dassert(pSprite->statnum != kMaxStatus);
    int nXSprite = pSprite->extra;
    dassert(nXSprite > 0);
    XSPRITE *pXSprite = &xsprite[nXSprite];
    switch (pSprite->type)
    {
    case 701:
    {
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        actSpawnThing(pSprite->sectnum, pSprite->x, pSprite->y, bottom, 423);
        break;
    }
    case 702:
    {
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        actSpawnThing(pSprite->sectnum, pSprite->x, pSprite->y, bottom, 424);
        break;
    }
    case 708:
        sfxPlay3DSound(pSprite, pXSprite->at12_0, -1, 0);
        break;
    case 703:
        switch (pXSprite->at12_0)
        {
        case 0:
            FireballTrapSeqCallback(3, nXSprite);
            break;
        case 1:
            seqSpawn(35, 3, nXSprite, nFireballTrapClient);
            break;
        case 2:
            seqSpawn(36, 3, nXSprite, nFireballTrapClient);
            break;
        }
        break;
    case 706:
    {
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        gFX.fxSpawn(FX_23, pSprite->sectnum, pSprite->x, pSprite->y, top, 0);
        break;
    }
    case 707:
    {
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        gFX.fxSpawn(FX_26, pSprite->sectnum, pSprite->x, pSprite->y, top, 0);
        break;
    }
    }
}

void FireballTrapSeqCallback(int, int nXSprite)
{
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int nSprite = pXSprite->reference;
    spritetype *pSprite = &sprite[nSprite];
    if (pSprite->cstat&32)
        actFireMissile(pSprite, 0, 0, 0, 0, (pSprite->cstat&8) ? 0x4000 : -0x4000, 305);
    else
        actFireMissile(pSprite, 0, 0, Cos(pSprite->ang)>>16, Sin(pSprite->ang)>>16, 0, 305);
}

void MGunFireSeqCallback(int, int nXSprite)
{
    int nSprite = xsprite[nXSprite].reference;
    spritetype *pSprite = &sprite[nSprite];
    XSPRITE *pXSprite = &xsprite[nXSprite];
    if (pXSprite->at12_0 > 0 || pXSprite->at10_0 == 0)
    {
        if (pXSprite->at12_0 > 0)
        {
            pXSprite->at12_0--;
            if (pXSprite->at12_0 == 0)
                evPost(nSprite, 3, 1, COMMAND_ID_0);
        }
        int dx = (Cos(pSprite->ang)>>16)+Random2(1000);
        int dy = (Sin(pSprite->ang)>>16)+Random2(1000);
        int dz = Random2(1000);
        actFireVector(pSprite, 0, 0, dx, dy, dz, VECTOR_TYPE_2);
        sfxPlay3DSound(pSprite, 359, -1, 0);
    }
}

void MGunOpenSeqCallback(int, int nXSprite)
{
    seqSpawn(39, 3, nXSprite, nMGunFireClient);
}

class TriggersLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void TriggersLoadSave::Load()
{
    Read(&gBusyCount, sizeof(gBusyCount));
    Read(gBusy, sizeof(gBusy));
    Read(basePath, sizeof(basePath));
}

void TriggersLoadSave::Save()
{
    Write(&gBusyCount, sizeof(gBusyCount));
    Write(gBusy, sizeof(gBusy));
    Write(basePath, sizeof(basePath));
}

static TriggersLoadSave *myLoadSave;

void TriggersLoadSaveConstruct(void)
{
    myLoadSave = new TriggersLoadSave();
}

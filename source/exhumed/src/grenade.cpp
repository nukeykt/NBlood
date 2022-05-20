//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
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

#include "grenade.h"
#include "engine.h"
#include "player.h"
#include "runlist.h"
#include "exhumed.h"
#include "sound.h"
#include "move.h"
#include "init.h"
#include "bullet.h"
#include "gun.h"
#include "anims.h"
#include "lighting.h"
#include "sequence.h"
#include "random.h"
#include "save.h"
#include <assert.h>

struct Grenade
{
    int16_t field_0;
    int16_t field_2;
    int16_t nSprite;
    int16_t field_6;
    int16_t field_8;
    int16_t field_A;
    int16_t field_C;
    int16_t field_E;
    int field_10;
    int x;
    int y;
};

int nGrenadeCount = 0;
int nGrenadesFree;

int16_t GrenadeFree[kMaxGrenades];
Grenade GrenadeList[kMaxGrenades];


void InitGrenades()
{
    nGrenadeCount = 0;

    for (int i = 0; i < kMaxGrenades; i++) {
        GrenadeFree[i] = i;
    }

    nGrenadesFree = kMaxGrenades;
}

int GrabGrenade()
{
    return GrenadeFree[--nGrenadesFree];
}

void DestroyGrenade(int nGrenade)
{
    runlist_DoSubRunRec(GrenadeList[nGrenade].field_6);
    runlist_SubRunRec(GrenadeList[nGrenade].field_8);
    runlist_DoSubRunRec(sprite[GrenadeList[nGrenade].nSprite].lotag - 1);

    mydeletesprite(GrenadeList[nGrenade].nSprite);
    GrenadeFree[nGrenadesFree] = nGrenade;

    nGrenadesFree++;
}

void BounceGrenade(int nGrenade, int nAngle)
{
    GrenadeList[nGrenade].field_10 >>= 1;

    GrenadeList[nGrenade].x = (Cos(nAngle) >> 5) * GrenadeList[nGrenade].field_10;
    GrenadeList[nGrenade].y = (Sin(nAngle) >> 5) * GrenadeList[nGrenade].field_10;

    D3PlayFX(StaticSound[kSoundGrenadeDrop], GrenadeList[nGrenade].nSprite);
}

int ThrowGrenade(int nPlayer, int UNUSED(edx), int UNUSED(ebx), int ecx, int push1)
{
    if (nPlayerGrenade[nPlayer] < 0)
        return -1;

    int nGrenade = nPlayerGrenade[nPlayer];

    int nGrenadeSprite = GrenadeList[nGrenade].nSprite;
    int nPlayerSprite = PlayerList[nPlayer].nSprite;

    int nAngle = sprite[nPlayerSprite].ang;

    mychangespritesect(nGrenadeSprite, nPlayerViewSect[nPlayer]);

    sprite[nGrenadeSprite].x = sprite[nPlayerSprite].x;
    sprite[nGrenadeSprite].y = sprite[nPlayerSprite].y;
    sprite[nGrenadeSprite].z = sprite[nPlayerSprite].z;

    if (nAngle < 0) {
        nAngle = sprite[nPlayerSprite].ang;
    }

    sprite[nGrenadeSprite].cstat &= 0x7FFF;
    sprite[nGrenadeSprite].ang = nAngle;

    if (push1 >= -3000)
    {
        int nVel = totalvel[nPlayer] << 5;

        GrenadeList[nGrenade].field_10 = ((90 - GrenadeList[nGrenade].field_E) * (90 - GrenadeList[nGrenade].field_E)) + nVel;
        sprite[nGrenadeSprite].zvel = (-64 * push1) - 4352;

        int nMov = movesprite(nGrenadeSprite, Cos(nAngle) * (sprite[nPlayerSprite].clipdist << 3), Sin(nAngle) * (sprite[nPlayerSprite].clipdist << 3), ecx, 0, 0, CLIPMASK1);
        if (nMov & 0x8000)
        {
            nAngle = GetWallNormal(nMov & 0x3FFF);
            BounceGrenade(nGrenade, nAngle);
        }
    }
    else
    {
        GrenadeList[nGrenade].field_10 = 0;
        sprite[nGrenadeSprite].zvel = sprite[nPlayerSprite].zvel;
    }

    GrenadeList[nGrenade].x = Cos(nAngle) >> 4;
    GrenadeList[nGrenade].x *= GrenadeList[nGrenade].field_10;

    GrenadeList[nGrenade].y = Sin(nAngle) >> 4;
    GrenadeList[nGrenade].y *= GrenadeList[nGrenade].field_10;

    nPlayerGrenade[nPlayer] = -1;

    return nGrenadeSprite;
}

int BuildGrenade(int nPlayer)
{
    if (nGrenadesFree == 0)
        return -1;

    int nGrenade = GrabGrenade();

    int nSprite = insertsprite(nPlayerViewSect[nPlayer], 201);
    assert(nSprite >= 0 && nSprite < kMaxSprites);

    int nPlayerSprite = PlayerList[nPlayer].nSprite;

    sprite[nSprite].x = sprite[nPlayerSprite].x;
    sprite[nSprite].y = sprite[nPlayerSprite].y;
    sprite[nSprite].z = sprite[nPlayerSprite].z - 3840;
    sprite[nSprite].shade = -64;
    sprite[nSprite].xrepeat = 20;
    sprite[nSprite].yrepeat = 20;
    sprite[nSprite].cstat = 0x8000;
    sprite[nSprite].picnum = 1;
    sprite[nSprite].pal = 0;
    sprite[nSprite].clipdist = 30;
    sprite[nSprite].xoffset = 0;
    sprite[nSprite].yoffset = 0;
    sprite[nSprite].ang = sprite[nPlayerSprite].ang;
    sprite[nSprite].owner = nPlayerSprite;
    sprite[nSprite].xvel = 0;
    sprite[nSprite].yvel = 0;
    sprite[nSprite].zvel = 0;
    sprite[nSprite].hitag = 0;
    sprite[nSprite].lotag = runlist_HeadRun() + 1;
    sprite[nSprite].extra = -1;

//	GrabTimeSlot(3);

    GrenadeList[nGrenade].field_E = 90;
    GrenadeList[nGrenade].field_2 = 0;
    GrenadeList[nGrenade].field_0 = 16;
    GrenadeList[nGrenade].field_10 = -1;
    GrenadeList[nGrenade].nSprite = nSprite;
    GrenadeList[nGrenade].field_A = 0;
    GrenadeList[nGrenade].field_C = 0;
    GrenadeList[nGrenade].field_6 = runlist_AddRunRec(sprite[nSprite].lotag - 1, nGrenade, kRunGrenade);
    GrenadeList[nGrenade].field_8 = runlist_AddRunRec(NewRun, nGrenade, kRunGrenade);

    nGrenadePlayer[nGrenade] = nPlayer;
    nPlayerGrenade[nPlayer] = nGrenade;

    return nGrenade;
}

void ExplodeGrenade(int nGrenade)
{
    int nSeq, nRepeat;

    int nPlayer = nGrenadePlayer[nGrenade];
    int nGrenadeSprite = GrenadeList[nGrenade].nSprite;
    int nGrenadeSect = sprite[nGrenadeSprite].sectnum;

    GrenadeList[nGrenade].field_C = 1;

    if (SectFlag[nGrenadeSect] & kSectUnderwater)
    {
        nSeq = kSeqGrenBubb;
        nRepeat = 60;
    }
    else
    {
        if (sprite[nGrenadeSprite].z < sector[nGrenadeSect].floorz)
        {
            nSeq = kSeqGrenPow;
            nRepeat = 200;
        }
        else
        {
            nSeq = kSeqGrenBoom;
            nRepeat = 150;
        }
    }

    if (GrenadeList[nGrenade].field_10 < 0)
    {
        int nPlayerSprite = PlayerList[nPlayer].nSprite;
        int nAngle = sprite[nPlayerSprite].ang;

        sprite[nGrenadeSprite].z = sprite[nPlayerSprite].z;
        sprite[nGrenadeSprite].x = (Cos(nAngle) >> 5) + sprite[nPlayerSprite].x;
        sprite[nGrenadeSprite].y = (Sin(nAngle) >> 5) + sprite[nPlayerSprite].y;

        changespritesect(nGrenadeSprite, sprite[nPlayerSprite].sectnum);

        if (!PlayerList[nPlayer].invincibility) {
            PlayerList[nPlayer].nHealth = 1;
        }
    }

    int nDamage = BulletInfo[kWeaponGrenade].nDamage;

    if (nPlayerDouble[nPlayer] > 0) {
        nDamage *= 2;
    }

    runlist_RadialDamageEnemy(nGrenadeSprite, nDamage, BulletInfo[kWeaponGrenade].nRadius);

    BuildAnim(-1, nSeq, 0, sprite[nGrenadeSprite].x, sprite[nGrenadeSprite].y, sprite[nGrenadeSprite].z, sprite[nGrenadeSprite].sectnum, nRepeat, 4);
    AddFlash(sprite[nGrenadeSprite].sectnum, sprite[nGrenadeSprite].x, sprite[nGrenadeSprite].y, sprite[nGrenadeSprite].z, 128);

    nGrenadePlayer[nGrenade] = -1;
    DestroyGrenade(nGrenade);
}

void FuncGrenade(int a, int UNUSED(nDamage), int nRun)
{
    int nGrenade = RunData[nRun].nVal;
    assert(nGrenade >= 0 && nGrenade < kMaxGrenades);

    int nGrenadeSprite = GrenadeList[nGrenade].nSprite;
    int nSeq;

    if (GrenadeList[nGrenade].field_C)
    {
        nSeq = SeqOffsets[kSeqGrenBoom];
    }
    else
    {
        nSeq = SeqOffsets[kSeqGrenRoll] + GrenadeList[nGrenade].field_A;
    }

    int nMessage = a & kMessageMask;

    switch (nMessage)
    {
        case 0x90000:
        {
            seq_PlotSequence(a & 0xFFFF, nSeq, GrenadeList[nGrenade].field_2 >> 8, 1);
            break;
        }

        default:
        {
            DebugOut("unknown msg %d for grenade\n", nMessage);
            return;
        }

        case 0x20000:
        {
            seq_MoveSequence(nGrenadeSprite, nSeq, GrenadeList[nGrenade].field_2 >> 8);
            sprite[nGrenadeSprite].picnum = seq_GetSeqPicnum2(nSeq, GrenadeList[nGrenade].field_2 >> 8);

            GrenadeList[nGrenade].field_E--;
            if (!GrenadeList[nGrenade].field_E)
            {
                int nPlayer = nGrenadePlayer[nGrenade];

                if (GrenadeList[nGrenade].field_10 < 0)
                {
                    PlayerList[nPlayer].nWeaponState = 0;
                    PlayerList[nPlayer].nWeaponFrame = 0;

                    if (PlayerList[nPlayer].nAmmo[kWeaponGrenade])
                    {
                        PlayerList[nPlayer].bIsFiring = kFalse;
                    }
                    else
                    {
                        SelectNewWeapon(nPlayer);

                        PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nNewWeapon;
                        PlayerList[nPlayer].nNewWeapon = -1;
                    }
                }

                ExplodeGrenade(nGrenade);
                return;
            }
            else
            {
                if (GrenadeList[nGrenade].field_10 < 0) {
                    return;
                }

                int nFrame = (GrenadeList[nGrenade].field_2 + GrenadeList[nGrenade].field_0) >> 8;

                GrenadeList[nGrenade].field_2 += GrenadeList[nGrenade].field_0;

                if (nFrame < 0)
                {
                    GrenadeList[nGrenade].field_2 += SeqSize[nSeq] << 8;
                }
                else
                {
                    if (nFrame >= SeqSize[nSeq])
                    {
                        if (GrenadeList[nGrenade].field_C)
                        {
                            DestroyGrenade(nGrenade);
                            return;
                        }
                        else
                        {
                            GrenadeList[nGrenade].field_2 = GrenadeList[nGrenade].field_C;
                        }
                    }
                }

                if (GrenadeList[nGrenade].field_C) {
                    return;
                }

                int zVel = sprite[nGrenadeSprite].zvel;

                Gravity(nGrenadeSprite);
                int nMov = movesprite(nGrenadeSprite, GrenadeList[nGrenade].x, GrenadeList[nGrenade].y, sprite[nGrenadeSprite].zvel, sprite[nGrenadeSprite].clipdist >> 1, sprite[nGrenadeSprite].clipdist >> 1, CLIPMASK1);

                if (!nMov)
                    return;

                if (nMov & 0x20000)
                {
                    if (zVel)
                    {
                        if (SectDamage[sprite[nGrenadeSprite].sectnum] > 0)
                        {
                            ExplodeGrenade(nGrenade);
                            return;
                        }

                        GrenadeList[nGrenade].field_0 = (uint8_t)totalmoves; // limit to 8bits?

                        D3PlayFX(StaticSound[kSoundGrenadeDrop], nGrenadeSprite);

                        sprite[nGrenadeSprite].zvel = -(zVel >> 1);

                        if (sprite[nGrenadeSprite].zvel > -1280)
                        {
                            D3PlayFX(StaticSound[kSoundGrenadeRoll], nGrenadeSprite);
                            GrenadeList[nGrenade].field_0 = 0;
                            GrenadeList[nGrenade].field_2 = 0;
                            sprite[nGrenadeSprite].zvel = 0;
                            GrenadeList[nGrenade].field_A = 1;
                        }
                    }

                    GrenadeList[nGrenade].field_0 = 255 - (RandomByte() * 2);
                    GrenadeList[nGrenade].x -= (GrenadeList[nGrenade].x >> 4);
                    GrenadeList[nGrenade].y -= (GrenadeList[nGrenade].y >> 4);
                }

                // loc_2CF60:
                if ((nMov & 0xC000) >= 0x8000)
                {
                    if ((nMov & 0xC000) <= 0x8000)
                    {
                        BounceGrenade(nGrenade, GetWallNormal(nMov & 0x3FFF));
                    }
                    else if ((nMov & 0xC000) == 0xC000)
                    {
                        BounceGrenade(nGrenade, sprite[nMov & 0x3FFF].ang);
                    }
                }

                GrenadeList[nGrenade].field_2 = 0;
                return;
            }

            break;
        }

        case 0xA0000:
        {
            if (nGrenadeSprite != nRadialSpr && !GrenadeList[nGrenade].field_C)
            {
                if (runlist_CheckRadialDamage(nGrenadeSprite) > 280)
                {
                    GrenadeList[nGrenade].field_E = RandomSize(4) + 1;
                }
            }
            break;
        }
    }
}

class GrenadeLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void GrenadeLoadSave::Load()
{
    Read(&nGrenadeCount, sizeof(nGrenadeCount));
    Read(&nGrenadesFree, sizeof(nGrenadesFree));

    Read(GrenadeFree, sizeof(GrenadeFree));
    Read(GrenadeList, sizeof(GrenadeList));
}

void GrenadeLoadSave::Save()
{
    Write(&nGrenadeCount, sizeof(nGrenadeCount));
    Write(&nGrenadesFree, sizeof(nGrenadesFree));

    Write(GrenadeFree, sizeof(GrenadeFree));
    Write(GrenadeList, sizeof(GrenadeList));
}

static GrenadeLoadSave* myLoadSave;

void GrenadeLoadSaveConstruct()
{
    myLoadSave = new GrenadeLoadSave();
}

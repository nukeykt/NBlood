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

#include "gun.h"
#include "engine.h"
#include "init.h"
#include "player.h"
#include "exhumed.h"
#include "view.h"
#include "move.h"
#include "status.h"
#include "bubbles.h"
#include "typedefs.h"
#include "sound.h"
#include "ra.h"
#include "snake.h"
#include "grenade.h"
#include "lighting.h"
#include "light.h"
#include "input.h"
#include "util.h"
#include "anims.h"
#include "runlist.h"
#include "bullet.h"
#include "trigdat.h"
#include "object.h"
#include "save.h"
#include <string.h>
#include <assert.h>

Weapon WeaponInfo[] = {
    { kSeqSword,   { 0, 1, 3,  7, -1,  2,  4, 5, 6, 8, 9, 10 }, 0, 0, 0, kTrue },
    { kSeqPistol,  { 0, 3, 2,  4, -1,  1,  0, 0, 0, 0, 0, 0 },  1, 0, 1, kFalse },
    { kSeqM60,     { 0, 5, 6, 16, -1, 21,  0, 0, 0, 0, 0, 0 },  2, 0, 1, kFalse },
    { kSeqFlamer,  { 0, 2, 5,  5,  6,  1,  0, 0, 0, 0, 0, 0 },  3, 4, 1, kFalse },
    { kSeqGrenade, { 0, 2, 3,  4, -1,  1,  0, 0, 0, 0, 0, 0 },  4, 0, 1, kTrue },
    { kSeqCobra,   { 0, 1, 2,  2, -1,  4,  0, 0, 0, 0, 0, 0 },  5, 0, 1, kTrue },
    { kSeqRavolt,  { 0, 1, 2,  3, -1,  4,  0, 0, 0, 0, 0, 0 },  6, 0, 1, kTrue },
    { kSeqRothands,{ 0, 1, 2, -1, -1, -1,  0, 0, 0, 0, 0, 0 },  7, 0, 0, kTrue },
    { kSeqDead,    { 1, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 },  0, 1, 0, kFalse },
    { kSeqDeadEx,  { 1, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 },  0, 1, 0, kFalse },
    { kSeqDeadBrn, { 1, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0 },  0, 1, 0, kFalse }
};

short nTemperature[kMaxPlayers];
short nMinAmmo[] = { 0, 24, 51, 50, 1, 0, 0 };
short word_96E26 = 0;


void RestoreMinAmmo(short nPlayer)
{
    for (int i = 0; i < kMaxWeapons; i++)
    {
        if (i == kWeaponGrenade) {
            continue;
        }

        if ((1 << i) & nPlayerWeapons[nPlayer])
        {
            if (nMinAmmo[i] > PlayerList[nPlayer].nAmmo[i]) {
                PlayerList[nPlayer].nAmmo[i] = nMinAmmo[i];
            }
        }
    }

    CheckClip(nPlayer);
}

void FillWeapons(short nPlayer)
{
    nPlayerWeapons[nPlayer] = 0xFFFF; // turn on all bits

    for (int i = 0; i < kMaxWeapons; i++)
    {
        if (WeaponInfo[i].bUsesAmmo) {
            PlayerList[nPlayer].nAmmo[i] = 99;
        }
    }

    CheckClip(nPlayer);

    if (nPlayer == nLocalPlayer)
    {
        short nWeapon = PlayerList[nPlayer].nCurrentWeapon;
        SetCounter(PlayerList[nPlayer].nAmmo[nWeapon]);
    }
}

void ResetPlayerWeapons(short nPlayer)
{
    for (int i = 0; i < kMaxWeapons; i++)
    {
        PlayerList[nPlayer].nAmmo[i] = 0;
    }

    PlayerList[nPlayer].nCurrentWeapon = 0;
    PlayerList[nPlayer].nWeaponState = 0;
    PlayerList[nPlayer].nWeaponFrame = 0;

    nPlayerGrenade[nPlayer] = -1;
    nPlayerWeapons[nPlayer] = 0x1; // turn on bit 1 only
}

void InitWeapons()
{
    memset(nPlayerGrenade, 0, sizeof(nPlayerGrenade));
    memset(nGrenadePlayer, 0, sizeof(nGrenadePlayer));
}

void SetNewWeapon(short nPlayer, short nWeapon)
{
    if (nWeapon == kWeaponMummified)
    {
        PlayerList[nPlayer].nLastWeapon = PlayerList[nPlayer].nCurrentWeapon;
        PlayerList[nPlayer].bIsFiring = kFalse;
        PlayerList[nPlayer].nWeaponState = 5;
        SetPlayerMummified(nPlayer, kTrue);

        PlayerList[nPlayer].nWeaponFrame = 0;
    }
    else
    {
        if (nWeapon < 0)
        {
            nPlayerOldWeapon[nPlayer] = PlayerList[nPlayer].nCurrentWeapon;
        }
        else if (nWeapon != kWeaponGrenade || PlayerList[nPlayer].nAmmo[kWeaponGrenade] > 0)
        {
            short nCurrentWeapon = PlayerList[nPlayer].nCurrentWeapon;

            if (nCurrentWeapon != kWeaponMummified)
            {
                if (PlayerList[nPlayer].bIsFiring || nWeapon == nCurrentWeapon) {
                    return;
                }
            }
            else
            {
                PlayerList[nPlayer].nCurrentWeapon = nWeapon;
                PlayerList[nPlayer].nWeaponFrame = 0;
            }
        }
        else {
            return;
        }
    }

    PlayerList[nPlayer].nNewWeapon = nWeapon;

    if (nPlayer == nLocalPlayer)
    {
        int nCounter;

        if (nWeapon >= kWeaponSword && nWeapon <= kWeaponRing) {
            nCounter = PlayerList[nPlayer].nAmmo[nWeapon];
        }
        else {
            nCounter = 0;
        }

        SetCounterImmediate(nCounter);
    }
}

void SetNewWeaponImmediate(short nPlayer, short nWeapon)
{
    SetNewWeapon(nPlayer, nWeapon);

    PlayerList[nPlayer].nCurrentWeapon = nWeapon;
    PlayerList[nPlayer].nNewWeapon = -1;
    PlayerList[nPlayer].nWeaponFrame = 0;
    PlayerList[nPlayer].nWeaponState = 0;
}

void SetNewWeaponIfBetter(short nPlayer, short nWeapon)
{
    if (nWeapon > PlayerList[nPlayer].nCurrentWeapon) {
        SetNewWeapon(nPlayer, nWeapon);
    }
}

void SelectNextWeapon()
{
    if (PlayerList[nLocalPlayer].bIsFiring)
        return;

    int nCurrentWeapon = PlayerList[nLocalPlayer].nCurrentWeapon;
    if (nCurrentWeapon == kWeaponMummified)
        return; // can't change weapon when mummified

    int nStartWeapon = nCurrentWeapon++;
    int nBits = nPlayerWeapons[nLocalPlayer];

    while (nCurrentWeapon != nStartWeapon)
    {
        if (((nBits >> nCurrentWeapon) & 1) && nCurrentWeapon != kWeaponMummified)
        {
            int nAmmoType = WeaponInfo[nCurrentWeapon].nAmmoType;
            if (!WeaponInfo[nCurrentWeapon].bUsesAmmo || PlayerList[nLocalPlayer].nAmmo[nAmmoType])
            {
                StopFiringWeapon(nLocalPlayer);
                SetNewWeapon(nLocalPlayer, nCurrentWeapon);
                return;
            }
        }

        nCurrentWeapon++;
        if (nCurrentWeapon >= kWeaponMummified)
            nCurrentWeapon = 0;
    }
}

void SelectPreviousWeapon()
{
    if (PlayerList[nLocalPlayer].bIsFiring)
        return;

    int nCurrentWeapon = PlayerList[nLocalPlayer].nCurrentWeapon;
    if (nCurrentWeapon == kWeaponMummified)
        return; // can't change weapon when mummified

    int nStartWeapon = nCurrentWeapon--;
    int nBits = nPlayerWeapons[nLocalPlayer];

    while (nCurrentWeapon != nStartWeapon)
    {
        if (((nBits >> nCurrentWeapon) & 1) && nCurrentWeapon != kWeaponMummified)
        {
            int nAmmoType = WeaponInfo[nCurrentWeapon].nAmmoType;
            if (!WeaponInfo[nCurrentWeapon].bUsesAmmo || PlayerList[nLocalPlayer].nAmmo[nAmmoType])
            {
                StopFiringWeapon(nLocalPlayer);
                SetNewWeapon(nLocalPlayer, nCurrentWeapon);
                return;
            }
        }

        nCurrentWeapon--;
        if (nCurrentWeapon < 0)
            nCurrentWeapon = kWeaponMummified - 1;
    }
}

void SelectNewWeapon(short nPlayer)
{
    int nWeapon = kWeaponRing; // start at the highest weapon number

    uint16_t nAvailableWeaponBits = nPlayerWeapons[nPlayer];
    uint16_t nBitToCheck = 0x40; // bit 7 turned on

    while (nBitToCheck)
    {
        if (nAvailableWeaponBits & nBitToCheck)
        {
            // we have this weapon
            if (!WeaponInfo[nWeapon].bUsesAmmo || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                break;
        }

        nWeapon--;
        nBitToCheck >>= 1;
    }

    if (nWeapon < 0)
        nWeapon = kWeaponSword;

    PlayerList[nPlayer].bIsFiring = kFalse;

    SetNewWeapon(nPlayer, nWeapon);
}

void StopFiringWeapon(short nPlayer)
{
    PlayerList[nPlayer].bIsFiring = kFalse;
}

void FireWeapon(short nPlayer)
{
    if (!PlayerList[nPlayer].bIsFiring) {
        PlayerList[nPlayer].bIsFiring = kTrue;
    }
}

void SetWeaponStatus(short nPlayer)
{
    if (nPlayer != nLocalPlayer)
        return;

    short nWeapon = PlayerList[nPlayer].nCurrentWeapon;

    if (nWeapon < 0)
    {
        nCounterBullet = -1;
        SetCounterImmediate(0);
    }
    else
    {
        nCounterBullet = WeaponInfo[nWeapon].nAmmoType;
        SetCounterImmediate(PlayerList[nPlayer].nAmmo[nCounterBullet]);
    }
}

uint8_t WeaponCanFire(short nPlayer)
{
    short nWeapon = PlayerList[nPlayer].nCurrentWeapon;
    short nSector = nPlayerViewSect[nPlayer];

    if (!(SectFlag[nSector] & kSectUnderwater) || WeaponInfo[nWeapon].bFireUnderwater)
    {
        short nAmmoType = WeaponInfo[nWeapon].nAmmoType;

        if (WeaponInfo[nWeapon].bUsesAmmo <= PlayerList[nPlayer].nAmmo[nAmmoType]) {
            return kTrue;
        }
    }

    return kFalse;
}

// UNUSED
void ResetSwordSeqs()
{
    WeaponInfo[kWeaponSword].b[2] = 3;
    WeaponInfo[kWeaponSword].b[3] = 7;
}

int CheckCloseRange(short nPlayer, int *x, int *y, int *z, short *nSector)
{
    short hitSect, hitWall, hitSprite;
    int hitX, hitY, hitZ;

    short nSprite = PlayerList[nPlayer].nSprite;

    int xVect = Sin(sprite[nSprite].ang + 512);
    int yVect = Sin(sprite[nSprite].ang);

    vec3_t startPos = { *x, *y, *z };
    hitdata_t hitData;
    hitscan(&startPos, *nSector, xVect, yVect, 0, &hitData, CLIPMASK1);
    hitX = hitData.x;
    hitY = hitData.y;
    hitZ = hitData.z;
    hitSprite = hitData.sprite;
    hitSect = hitData.sect;
    hitWall = hitData.wall;

    int ecx = sintable[150] >> 3;

    uint32_t xDiff = klabs(hitX - *x);
    uint32_t yDiff = klabs(hitY - *y);

    uint32_t sqrtNum = xDiff * xDiff + yDiff * yDiff;

    if (sqrtNum > INT_MAX)
    {
        OSD_Printf("%s %d: overflow\n", EDUKE32_FUNCTION, __LINE__);
        sqrtNum = INT_MAX;
    }

    if (ksqrt(sqrtNum) >= ecx)
        return 0;

    *x = hitX;
    *y = hitY;
    *z = hitZ;
    *nSector = hitSect;

    if (hitSprite > -1) {
        return hitSprite | 0xC000;
    }
    if (hitWall > -1) {
        return hitWall | 0x8000;
    }

    return 0;
}

void CheckClip(short nPlayer)
{
    if (nPlayerClip[nPlayer] <= 0)
    {
        nPlayerClip[nPlayer] = PlayerList[nPlayer].nAmmo[kWeaponM60];

        if (nPlayerClip[nPlayer] > 99) {
            nPlayerClip[nPlayer] = 99;
        }
    }
}

void MoveWeapons(short nPlayer)
{
    static int dword_96E22 = 0;

    short nSectFlag = SectFlag[nPlayerViewSect[nPlayer]];

    if ((nSectFlag & kSectUnderwater) && (totalmoves & 1)) {
        return;
    }

    nPilotLightFrame++;

    if (nPilotLightFrame >= nPilotLightCount)
        nPilotLightFrame = 0;

    if (!PlayerList[nPlayer].bIsFiring || (nSectFlag & kSectUnderwater))
        nTemperature[nPlayer] = 0;

    short nPlayerSprite = PlayerList[nPlayer].nSprite;
    short nWeapon = PlayerList[nPlayer].nCurrentWeapon;

    if (nWeapon < -1)
    {
        if (PlayerList[nPlayer].nNewWeapon != -1)
        {
            PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nNewWeapon;
            PlayerList[nPlayer].nWeaponState = 0;
            PlayerList[nPlayer].nWeaponFrame = 0;
            PlayerList[nPlayer].nNewWeapon = -1;
        }

        return;
    }

    // loc_26ACC
    short eax = PlayerList[nPlayer].nWeaponState;
    short nSeq = WeaponInfo[nWeapon].nSeq;

    short var_3C = WeaponInfo[nWeapon].b[eax] + SeqOffsets[nSeq];

    int var_1C = (nPlayerDouble[nPlayer] > 0) + 1;

    frames = var_1C - 1;

    for (frames = var_1C; frames > 0; frames--)
    {
        seq_MoveSequence(nPlayerSprite, var_3C, PlayerList[nPlayer].nWeaponFrame);

        PlayerList[nPlayer].nWeaponFrame++;

        dword_96E22++;
        if (dword_96E22 >= 15) {
            dword_96E22 = 0;
        }

        if (PlayerList[nPlayer].nWeaponFrame >= SeqSize[var_3C])
        {
            if (PlayerList[nPlayer].nNewWeapon == -1)
            {
                switch (PlayerList[nPlayer].nWeaponState)
                {
                    default:
                        break;

                    case 0:
                    {
                        PlayerList[nPlayer].nWeaponState = 1;
                        SetWeaponStatus(nPlayer);
                        break;
                    }
                    case 1:
                    {
                        if (PlayerList[nPlayer].bIsFiring)
                        {
                            if (!WeaponCanFire(nPlayer))
                            {
                                if (!dword_96E22) {
                                    D3PlayFX(StaticSound[4], PlayerList[nPlayer].nSprite);
                                }
                            }
                            else
                            {
                                if (nWeapon == kWeaponRing)
                                {
                                    if (Ra[nPlayer].nTarget == -1)
                                        break;

                                    Ra[nPlayer].nAction = 0;
                                    Ra[nPlayer].nFrame  = 0;
                                    Ra[nPlayer].field_C = 1;
                                }

                                PlayerList[nPlayer].nWeaponState = 2;

                                if (nWeapon == 0)
                                    break;

                                if (nWeapon == kWeaponGrenade)
                                {
                                    BuildGrenade(nPlayer);
                                    AddAmmo(nPlayer, 4, -1);
                                }
                                else if (nWeapon == kWeaponMummified)
                                {
                                    ShootStaff(nPlayer);
                                }
                            }
                        }
                        break;
                    }

                    case 2:
                    case 6:
                    case 7:
                    case 8:
                    {
                        if (nWeapon == kWeaponPistol && nPistolClip[nPlayer] <= 0)
                        {
                            PlayerList[nPlayer].nWeaponState = 3;
                            PlayerList[nPlayer].nWeaponFrame = 0;

                            nPistolClip[nPlayer] = Min(6, PlayerList[nPlayer].nAmmo[kWeaponPistol]);
                            break;
                        }
                        else if (nWeapon == kWeaponGrenade)
                        {
                            if (!PlayerList[nPlayer].bIsFiring)
                            {
                                PlayerList[nPlayer].nWeaponState = 3;
                                break;
                            }
                            else
                            {
                                PlayerList[nPlayer].nWeaponFrame = SeqSize[var_3C] - 1;
                                continue;
                            }
                        }
                        else if (nWeapon == kWeaponMummified)
                        {
                            PlayerList[nPlayer].nWeaponState = 0;
                            PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nLastWeapon;

                            nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                            SetPlayerMummified(nPlayer, kFalse);
                            break;
                        }
                        else
                        {
                            // loc_26D88:
                            if (PlayerList[nPlayer].bIsFiring && WeaponCanFire(nPlayer))
                            {
                                if (nWeapon != kWeaponM60 && nWeapon != kWeaponPistol) {
                                    PlayerList[nPlayer].nWeaponState = 3;
                                }
                            }
                            else
                            {
                                if (WeaponInfo[nWeapon].b[4] == -1)
                                {
                                    PlayerList[nPlayer].nWeaponState = 1;
                                }
                                else
                                {
                                    if (nWeapon == kWeaponFlamer && (nSectFlag & kSectUnderwater))
                                    {
                                        PlayerList[nPlayer].nWeaponState = 1;
                                    }
                                    else
                                    {
                                        PlayerList[nPlayer].nWeaponState = 4;
                                    }
                                }
                            }

                            break;
                        }
                    }

                    case 3:
                    case 9:
                    case 10:
                    case 11:
                    {
                        if (nWeapon == kWeaponMummified)
                        {
                            PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nLastWeapon;

                            nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                            PlayerList[nPlayer].nWeaponState = 0;
                            break;
                        }
                        else if (nWeapon == kWeaponRing)
                        {
                            if (!WeaponInfo[nWeapon].bUsesAmmo || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                            {
                                if (!PlayerList[nPlayer].bIsFiring) {
                                    PlayerList[nPlayer].nWeaponState = 1;
                                }
                                else {
                                    break;
                                }
                            }
                            else
                            {
                                SelectNewWeapon(nPlayer);
                            }

                            Ra[nPlayer].field_C = 0;
                            break;
                        }
                        else if (nWeapon == kWeaponM60)
                        {
                            CheckClip(nPlayer);
                            PlayerList[nPlayer].nWeaponState = 1;
                            break;
                        }
                        else if (nWeapon == kWeaponGrenade)
                        {
                            if (!WeaponInfo[nWeapon].bUsesAmmo || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                            {
                                PlayerList[nPlayer].nWeaponState = 0;
                                break;
                            }
                            else
                            {
                                SelectNewWeapon(nPlayer);
                                PlayerList[nPlayer].nWeaponState = 5;

                                PlayerList[nPlayer].nWeaponFrame = SeqSize[WeaponInfo[kWeaponGrenade].b[0] + SeqOffsets[nSeq]] - 1; // CHECKME
                                goto loc_flag; // FIXME
                            }
                        }
                        else
                        {
                            if (PlayerList[nPlayer].bIsFiring && WeaponCanFire(nPlayer)) {
                                PlayerList[nPlayer].nWeaponState = 2;
                                break;
                            }

                            if (WeaponInfo[nWeapon].b[4] == -1)
                            {
                                PlayerList[nPlayer].nWeaponState = 1;
                                break;
                            }

                            if (nWeapon == kWeaponFlamer && (nSectFlag & kSectUnderwater))
                            {
                                PlayerList[nPlayer].nWeaponState = 1;
                            }
                            else
                            {
                                PlayerList[nPlayer].nWeaponState = 4;
                            }
                        }
                        break;
                    }

                    case 4:
                    {
                        PlayerList[nPlayer].nWeaponState = 1;
                        break;
                    }

                    case 5:
                    {
                        PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nNewWeapon;

                        nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                        PlayerList[nPlayer].nWeaponState = 0;
                        PlayerList[nPlayer].nNewWeapon = -1;

                        SetWeaponStatus(nPlayer);
                        break;
                    }
                }

                // loc_26FC5
                var_3C = SeqOffsets[WeaponInfo[nWeapon].nSeq] + WeaponInfo[nWeapon].b[PlayerList[nPlayer].nWeaponState];
                PlayerList[nPlayer].nWeaponFrame = 0;
            }
            else
            {
                if (PlayerList[nPlayer].nWeaponState == 5)
                {
                    PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nNewWeapon;

                    nWeapon = PlayerList[nPlayer].nCurrentWeapon;

                    PlayerList[nPlayer].nNewWeapon = -1;
                    PlayerList[nPlayer].nWeaponState = 0;
                }
                else
                {
                    PlayerList[nPlayer].nWeaponState = 5;
                }

                PlayerList[nPlayer].nWeaponFrame = 0;
                continue;
            }
        } // end of if (PlayerList[nPlayer].field_34 >= SeqSize[var_3C])

loc_flag:

        // loc_27001
        short nFrameFlag = seq_GetFrameFlag(var_3C, PlayerList[nPlayer].nWeaponFrame);

        if (((!(nSectFlag & kSectUnderwater)) || nWeapon == kWeaponRing) && (nFrameFlag & 4))
        {
            BuildFlash(nPlayer, sprite[nPlayerSprite].sectnum, 512);
            AddFlash(
                sprite[nPlayerSprite].sectnum,
                sprite[nPlayerSprite].x,
                sprite[nPlayerSprite].y,
                sprite[nPlayerSprite].z,
                0);
        }

        if (nFrameFlag & 0x80)
        {
            int nAction = PlayerList[nPlayer].nAction;

            int var_38 = 1;

            if (nAction < 10 || nAction > 12) {
                var_38 = 0;
            }

            if (nPlayer == nLocalPlayer) {
                obobangle = bobangle = 512;
            }

            if (nWeapon == kWeaponFlamer && (!(nSectFlag & kSectUnderwater)))
            {
                nTemperature[nPlayer]++;

                if (nTemperature[nPlayer] > 50)
                {
                    nTemperature[nPlayer] = 0;
                    PlayerList[nPlayer].nWeaponState = 4;
                    PlayerList[nPlayer].nWeaponFrame = 0;
                }
            }

            short nAmmoType = WeaponInfo[nWeapon].nAmmoType;
            short nAngle = sprite[nPlayerSprite].ang;
            int theX = sprite[nPlayerSprite].x;
            int theY = sprite[nPlayerSprite].y;
            int theZ = sprite[nPlayerSprite].z;

            int ebp = Cos(nAngle) * (sprite[nPlayerSprite].clipdist << 3);
            int ebx = Sin(nAngle) * (sprite[nPlayerSprite].clipdist << 3);

            if (WeaponInfo[nWeapon].c)
            {
                int ecx;

                int theVal = (totalmoves + 101) & (WeaponInfo[nWeapon].c - 1);
                if (theVal & 1)
                    ecx = -theVal;
                else
                    ecx = theVal;

                int var_44 = (nAngle + 512) & kAngleMask;
                ebp += (Cos(var_44) >> 11) * ecx;
                ebx += (Sin(var_44) >> 11) * ecx;
            }

            int nHeight = (-GetSpriteHeight(nPlayerSprite)) >> 1;

            if (nAction < 6)
            {
                nHeight -= 1792;
            }
            else
            {
                if (!var_38)
                {
                    nHeight += 1024;
                }
                else {
                    nHeight -= 2560;
                }
            }

            short nSectorB = sprite[nPlayerSprite].sectnum;

            switch (nWeapon)
            {
                // loc_27266:
                case kWeaponSword:
                {
                    nHeight += (92 - fix16_to_int(sPlayerInput[nPlayer].horizon)) << 6;

                    theZ += nHeight;

                    int var_28;

                    if (PlayerList[nPlayer].nWeaponState == 2) {
                        var_28 = 6;
                    }
                    else {
                        var_28 = 9;
                    }

                    int cRange = CheckCloseRange(nPlayer, &theX, &theY, &theZ, &nSectorB);

                    if (cRange)
                    {
                        short nDamage = BulletInfo[kWeaponSword].nDamage;

                        if (nPlayerDouble[nPlayer]) {
                            nDamage *= 2;
                        }

                        if ((cRange & 0xC000) >= 0x8000)
                        {
                            if ((cRange & 0xC000) == 0x8000) // hit wall
                            {
                                // loc_2730E:
                                var_28 += 2;
                            }
                            else if ((cRange & 0xC000) == 0xC000) // hit sprite
                            {
                                short nSprite2 = cRange & 0x3FFF;

                                if (sprite[nSprite2].cstat & 0x50)
                                {
                                    var_28 += 2;
                                }
                                else if (sprite[nSprite2].statnum > 90 && sprite[nSprite2].statnum <= 199)
                                {
                                    runlist_DamageEnemy(nSprite2, nPlayerSprite, nDamage);

                                    if (sprite[nSprite2].statnum < 102) {
                                        var_28++;
                                    }
                                    else if (sprite[nSprite2].statnum == 102)
                                    {
                                        // loc_27370:
                                        BuildAnim(-1, 12, 0, theX, theY, theZ, nSectorB, 30, 0);
                                    }
                                    else if (sprite[nSprite2].statnum == kStatExplodeTrigger) {
                                        var_28 += 2;
                                    }
                                    else {
                                        var_28++;
                                    }
                                }
                                else
                                {
                                    // loc_27370:
                                    BuildAnim(-1, 12, 0, theX, theY, theZ, nSectorB, 30, 0);
                                }
                            }
                        }
                    }

                    // loc_27399:
                    PlayerList[nPlayer].nWeaponState = var_28;
                    PlayerList[nPlayer].nWeaponFrame = 0;
                    break;
                }
                case kWeaponFlamer:
                {
                    if (nSectFlag & kSectUnderwater)
                    {
                        DoBubbles(nPlayer);
                        PlayerList[nPlayer].nWeaponState = 1;
                        PlayerList[nPlayer].nWeaponFrame = 0;
                        StopSpriteSound(nPlayerSprite);
                        break;
                    }
                    else
                    {
                        if (var_38) {
                            nHeight += 768;
                        }
                        else {
                            nHeight -= 2560;
                        }

                        // fall through to case 1 (kWeaponPistol)
                        fallthrough__;
                    }
                }

                case kWeaponM60:
                {
                    if (nWeapon == kWeaponM60) { // hack(?) to do fallthrough from kWeapon3 into kWeaponPistol without doing the nQuake[] change
                        nQuake[nPlayer] = 128;
                    }
                    // fall through
                    fallthrough__;
                }
                case kWeaponPistol:
                {
                    int var_50 = (fix16_to_int(sPlayerInput[nPlayer].horizon) - 92) << 2;
                    nHeight -= var_50;

                    if (sPlayerInput[nPlayer].nTarget >= 0)
                    {
                                                assert(sprite[sPlayerInput[nPlayer].nTarget].sectnum < kMaxSectors);
                        var_50 = sPlayerInput[nPlayer].nTarget + 10000;
                    }

                    BuildBullet(nPlayerSprite, nAmmoType, ebp, ebx, nHeight, nAngle, var_50, var_1C);
                    break;
                }

                case kWeaponGrenade:
                {
                    ThrowGrenade(nPlayer, ebp, ebx, nHeight - 2560, fix16_to_int(sPlayerInput[nPlayer].horizon) - 92);
                    break;
                }
                case kWeaponStaff:
                {
                    BuildSnake(nPlayer, nHeight);
                    nQuake[nPlayer] = 512;

                    nXDamage[nPlayer] -= Sin(sprite[nPlayerSprite].ang + 512) << 9;
                    nYDamage[nPlayer] -= Sin(sprite[nPlayerSprite].ang) << 9;
                    break;
                }
                case kWeaponRing:
                    break;

                case kWeaponMummified:
                {
                    short nDamage = BulletInfo[kWeaponMummified].nDamage;
                    if (nPlayerDouble[nPlayer]) {
                        nDamage *= 2;
                    }

                    runlist_RadialDamageEnemy(nPlayerSprite, nDamage, BulletInfo[kWeaponMummified].nRadius);
                    break;
                }
            }

            // end of switch, nw_break_2:
            if (nWeapon < kWeaponMummified)
            {
                if (nWeapon != kWeaponGrenade)
                {   
                    if (WeaponInfo[nWeapon].bUsesAmmo) {
                        AddAmmo(nPlayer, nAmmoType, -1);
                    }

                    if (nWeapon == kWeaponM60) {
                        nPlayerClip[nPlayer]--;
                    }
                    else if (nWeapon == kWeaponPistol) {
                        nPistolClip[nPlayer]--;
                    }
                }

                if (!WeaponInfo[nWeapon].bUsesAmmo || PlayerList[nPlayer].nAmmo[WeaponInfo[nWeapon].nAmmoType])
                {
                    if (nWeapon == kWeaponM60 && nPlayerClip[nPlayer] <= 0)
                    {
                        PlayerList[nPlayer].nWeaponState = 3;
                        PlayerList[nPlayer].nWeaponFrame = 0;
                        // goto loc_27609:
                    }
                }
                else if (nWeapon != kWeaponGrenade)
                {
                    SelectNewWeapon(nPlayer);
                    // go to loc_27609:
                }
            }
        }
    }
}

void DrawWeapons(int smooth)
{
    if (bCamera) {
        return;
    }

    short nWeapon = PlayerList[nLocalPlayer].nCurrentWeapon;
    if (nWeapon < -1) {
        return;
    }

    short var_34 = PlayerList[nLocalPlayer].nWeaponState;

    short var_30 = SeqOffsets[WeaponInfo[nWeapon].nSeq];

    short var_28 = var_30 + WeaponInfo[nWeapon].b[var_34];

    int8_t nShade = sector[initsect].ceilingshade;

    int nDouble = nPlayerDouble[nLocalPlayer];
    int nPal = kPalNormal;

    if (nDouble)
    {
        if (word_96E26) {
            nPal = kPalRedBrite;
        }

        word_96E26 = word_96E26 == 0;
    }

    nPal = RemapPLU(nPal);

    int nVal = totalvel[nLocalPlayer] >> 1;

    // CHECKME - not & 0x7FF?
    int nBobAngle = angle_interpolate16(obobangle, bobangle, smooth);
    int	yOffset = (nVal * (sintable[nBobAngle & 0x3FF] >> 8)) >> 9;

    int xOffset = 0;

    if (var_34 == 1)
    {
        xOffset = ((Sin(nBobAngle + 512) >> 8) * nVal) >> 8;
    }
    else
    {
        xOffset = 0;
        obobangle = bobangle = 512;
    }

    if (nWeapon == 3 && var_34 == 1) {
        seq_DrawPilotLightSeq(xOffset, yOffset);
    }

    if (nWeapon < 0) {
        nShade = sprite[PlayerList[nLocalPlayer].nSprite].shade;
    }

    seq_DrawGunSequence(var_28, PlayerList[nLocalPlayer].nWeaponFrame, xOffset, yOffset, nShade, nPal);

    if (nWeapon != kWeaponM60)
        return;

    switch (var_34)
    {
        default:
            return;

        case 0:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            if (nClip <= 0)
                return;

            int nSeqOffset;

            if (nClip <= 3)
            {
                nSeqOffset = var_30 + 1;
            }
            else if (nClip <= 6)
            {
                nSeqOffset = var_30 + 2;
            }
            else if (nClip <= 25)
            {
                nSeqOffset = var_30 + 3;
            }
            else if (nClip > 25)
            {
                nSeqOffset = var_30 + 4;
            }

            seq_DrawGunSequence(nSeqOffset, PlayerList[nLocalPlayer].nWeaponFrame, xOffset, yOffset, nShade, nPal);
            return;
        }
        case 1:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            short edx = (nClip % 3) * 4;

            if (nClip <= 0) {
                return;
            }

            seq_DrawGunSequence(var_30 + 8, edx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 3) {
                return;
            }

            seq_DrawGunSequence(var_30 + 9, edx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 6) {
                return;
            }

            seq_DrawGunSequence(var_30 + 10, edx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 25) {
                return;
            }

            seq_DrawGunSequence(var_30 + 11, edx, xOffset, yOffset, nShade, nPal);
            return;
        }
        case 2:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            short dx = PlayerList[nLocalPlayer].nWeaponFrame;

            if (nClip <= 0) {
                return;
            }

            seq_DrawGunSequence(var_30 + 8, dx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 3) {
                return;
            }

            seq_DrawGunSequence(var_30 + 9, dx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 6) {
                return;
            }

            seq_DrawGunSequence(var_30 + 10, dx, xOffset, yOffset, nShade, nPal);

            if (nClip <= 25) {
                return;
            }

            seq_DrawGunSequence(var_30 + 11, dx, xOffset, yOffset, nShade, nPal);
            return;
        }

        case 3:
        case 4:
            return;

        case 5:
        {
            int nClip = nPlayerClip[nLocalPlayer];

            short ax = PlayerList[nLocalPlayer].nWeaponFrame;

            if (nClip <= 0) {
                return;
            }

            int nSeqOffset;

            if (nClip <= 3)
            {
                nSeqOffset = var_30 + 20;
            }
            else if (nClip <= 6)
            {
                nSeqOffset = var_30 + 19;
            }
            else if (nClip <= 25)
            {
                nSeqOffset = var_30 + 18;
            }
            else
            {
                nSeqOffset = var_30 + 17;
            }

            seq_DrawGunSequence(nSeqOffset, ax, xOffset, yOffset, nShade, nPal);
            return;
        }
    }
}

class GunLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void GunLoadSave::Load()
{
    Read(&nTemperature[nLocalPlayer], sizeof(nTemperature[nLocalPlayer]));
    Read(&word_96E26, sizeof(word_96E26));
}

void GunLoadSave::Save()
{
    Write(&nTemperature[nLocalPlayer], sizeof(nTemperature[nLocalPlayer]));
    Write(&word_96E26, sizeof(word_96E26));
}

static GunLoadSave* myLoadSave;

void GunLoadSaveConstruct()
{
    myLoadSave = new GunLoadSave();
}

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

#include "compat.h"
#include "player.h"
#include "runlist.h"
#include "exhumed.h"
#include "names.h"
#include "ramses.h"
#include "gun.h"
#include "items.h"
#include "engine.h"
#include "move.h"
#include "sequence.h"
#include "lighting.h"
#include "view.h"
#include "bubbles.h"
#include "random.h"
#include "ra.h"
#include "input.h"
#include "light.h"
#include "status.h"
#include "mouse.h"
#include "keyboard.h"
#include "control.h"
#include "config.h"
#include "sound.h"
#include "init.h"
#include "move.h"
#include "trigdat.h"
#include "anims.h"
#include "grenade.h"
#include "menu.h"
#include "cd.h"
#include "map.h"
#include "sound.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

bool bDemoPlayerFinishedLevel = false;

struct PlayerSave
{
    int x;
    int y;
    int z;
    short nSector;
    short nAngle;
};

fix16_t lPlayerXVel = 0;
fix16_t lPlayerYVel = 0;
fix16_t nPlayerDAng = 0;
short obobangle = 0, bobangle  = 0;
short bPlayerPan = 0;
short bLockPan  = 0;
bool bLookCentre = false;


static actionSeq ActionSeq[] = {
    {18,  0}, {0,   0}, {9,   0}, {27,  0}, {63,  0},
    {72,  0}, {54,  0}, {45,  0}, {54,  0}, {81,  0},
    {90,  0}, {99,  0}, {108, 0}, {8,   0}, {0,   0},
    {139, 0}, {117, 1}, {119, 1}, {120, 1}, {121, 1},
    {122, 1}
};

static short nHeightTemplate[] = { 0, 0, 0, 0, 0, 0, 7, 7, 7, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0 };

short nActionEyeLevel[] = {
    -14080, -14080, -14080, -14080, -14080, -14080, -8320,
    -8320,  -8320,  -8320,  -8320,  -8320,  -8320,  -14080,
    -14080, -14080, -14080, -14080, -14080, -14080, -14080
};

uint16_t nGunLotag[] = { 52, 53, 54, 55, 56, 57 };
uint16_t nGunPicnum[] = { 57, 488, 490, 491, 878, 899, 3455 };

int16_t nItemText[] = {
    -1, -1, -1, -1, -1, -1, 18, 20, 19, 13, -1, 10, 1, 0, 2, -1, 3,
    -1, 4, 5, 9, 6, 7, 8, -1, 11, -1, 13, 12, 14, 15, -1, 16, 17,
    -1, -1, -1, 21, 22, -1, -1, -1, -1, -1, -1, 23, 24, 25, 26, 27,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};


int nLocalPlayer = 0;

short nBreathTimer[kMaxPlayers];
short nPlayerSwear[kMaxPlayers];
short nPlayerPushSect[kMaxPlayers];
short nDeathType[kMaxPlayers];
short nPlayerScore[kMaxPlayers];
short nPlayerColor[kMaxPlayers];
int nPlayerDY[kMaxPlayers];
int nPlayerDX[kMaxPlayers];
char playerNames[kMaxPlayers][11];
short nPistolClip[kMaxPlayers];
int nXDamage[kMaxPlayers];
int nYDamage[kMaxPlayers];
short nDoppleSprite[kMaxPlayers];
short namelen[kMaxPlayers];
short nPlayerOldWeapon[kMaxPlayers];
short nPlayerClip[kMaxPlayers];
short nPlayerPushSound[kMaxPlayers];
short nTauntTimer[kMaxPlayers];
short nPlayerTorch[kMaxPlayers];
uint16_t nPlayerWeapons[kMaxPlayers]; // each set bit represents a weapon the player has
short nPlayerLives[kMaxPlayers];
short nPlayerItem[kMaxPlayers];
Player PlayerList[kMaxPlayers];
short nPlayerInvisible[kMaxPlayers];
short nPlayerDouble[kMaxPlayers];
short nPlayerViewSect[kMaxPlayers];
short nPlayerFloorSprite[kMaxPlayers];
PlayerSave sPlayerSave[kMaxPlayers];
int totalvel[kMaxPlayers] = { 0 };
int16_t eyelevel[kMaxPlayers], oeyelevel[kMaxPlayers];
short nNetStartSprite[kMaxPlayers] = { 0 };

short nStandHeight;

short nPlayerGrenade[kMaxPlayers];
short nGrenadePlayer[50];

short word_D282A[32];

short PlayerCount;

short nNetStartSprites;
short nCurStartSprite;

short nLocalSpr;

/*
typedef struct
{
fixed     dx;
fixed     dy;
fixed     dz;
fixed     dyaw;
fixed     dpitch;
fixed     droll;
} ControlInfo;
*/

extern void testsave();
extern void testload();

void PlayerInterruptKeys()
{
    ControlInfo info;
    CONTROL_ProcessBinds();
    memset(&info, 0, sizeof(ControlInfo)); // this is done within CONTROL_GetInput() anyway
    CONTROL_GetInput(&info);

    if (mouseaiming) {
        aimmode = 0;
    }

    #if 0
    if (keystatus[sc_J])
    {
        keystatus[sc_J] = 0;
        testsave();
    }
    if (keystatus[sc_K])
    {
        keystatus[sc_K] = 0;
        testload();
    }
    #endif

    if (BUTTON(gamefunc_Mouse_Aiming))
    {
        if (mouseaiming)
            aimmode = 1;
        else
        {
            CONTROL_ClearButton(gamefunc_Mouse_Aiming);
            aimmode = !aimmode;
            if (aimmode)
            {
                StatusMessage(150, "Mouse aiming ON");
            }
            else
            {
                StatusMessage(150, "Mouse aiming OFF");
                bLookCentre = true;
            }
        }
    }
    else if (mouseaiming)
    {
        bLookCentre = true;
    }

    if (BUTTON(gamefunc_AutoRun))
    {
        CONTROL_ClearButton(gamefunc_AutoRun);
        auto_run = !auto_run;
        if (auto_run)
            StatusMessage(150, "Auto run ON");
        else
            StatusMessage(150, "Auto run OFF");
    }

    if (MouseDeadZone)
    {
        if (info.mousey > 0)
            info.mousey = max(info.mousey - MouseDeadZone, 0);
        else if (info.mousey < 0)
            info.mousey = min(info.mousey + MouseDeadZone, 0);

        if (info.mousex > 0)
            info.mousex = max(info.mousex - MouseDeadZone, 0);
        else if (info.mousex < 0)
            info.mousex = min(info.mousex + MouseDeadZone, 0);
    }

    if (MouseBias)
    {
        if (klabs(info.mousex) > klabs(info.mousey))
            info.mousey = tabledivide32_noinline(info.mousey, MouseBias);
        else
            info.mousex = tabledivide32_noinline(info.mousex, MouseBias);
    }

    if (PlayerList[nLocalPlayer].nHealth == 0)
    {
        lPlayerYVel = 0;
        lPlayerXVel = 0;
        nPlayerDAng = 0;
        return;
    }

    // JBF: Run key behaviour is selectable
    int const playerRunning = (runkey_mode) ? (BUTTON(gamefunc_Run) | auto_run) : (auto_run ^ BUTTON(gamefunc_Run));
    int const turnAmount = playerRunning ? 12 : 8;
    int const keyMove    = playerRunning ? 12 : 6;
    constexpr int const analogTurnAmount = 12;
    constexpr int const analogExtent = 32767; // KEEPINSYNC sdlayer.cpp
    int fvel = 0, svel = 0;
    fix16_t q16avel = 0, q16horz = 0;

    if (BUTTON(gamefunc_Strafe))
    {
        static int strafeyaw;

        svel = -(info.mousex + strafeyaw) >> 6;
        strafeyaw  = (info.mousex + strafeyaw) % 64;

        svel -= info.dyaw * keyMove / analogExtent;
    }
    else
    {
        q16avel = fix16_div(fix16_from_int(info.mousex), F16(32));
        q16avel += fix16_from_int(info.dyaw) / analogExtent * (analogTurnAmount << 1);
    }

    if (aimmode)
        q16horz = fix16_div(fix16_from_int(info.mousey), F16(64));
    else
        fvel = -(info.mousey >> 6);

    if (mouseflip) q16horz = -q16horz;

    q16horz -= fix16_from_int(info.dpitch) / analogExtent * analogTurnAmount;
    svel -= info.dx * keyMove / analogExtent;
    fvel -= info.dz * keyMove / analogExtent;

    if (BUTTON(gamefunc_Strafe))
    {
        if (BUTTON(gamefunc_Turn_Left))
            svel -= -keyMove;

        if (BUTTON(gamefunc_Turn_Right))
            svel -= keyMove;
    }
    else
    {
        static int turn = 0;
        static int counter = 0;
        // normal, non strafing movement
        if (BUTTON(gamefunc_Turn_Left))
        {
            turn -= 2;

            if (turn < -turnAmount)
                turn = -turnAmount;
        }
        else if (BUTTON(gamefunc_Turn_Right))
        {
            turn += 2;

            if (turn > turnAmount)
                turn = turnAmount;
        }

        if (turn < 0)
        {
            turn++;
            if (turn > 0)
                turn = 0;
        }

        if (turn > 0)
        {
            turn--;
            if (turn < 0)
                turn = 0;
        }

        if ((counter++) % 4 == 0)
            q16avel += fix16_from_int(turn<<2);
    }

    if (BUTTON(gamefunc_Strafe_Left))
        svel += keyMove;

    if (BUTTON(gamefunc_Strafe_Right))
        svel += -keyMove;

    if (BUTTON(gamefunc_Move_Forward))
        fvel += keyMove;

    if (BUTTON(gamefunc_Move_Backward))
        fvel += -keyMove;

    fvel = clamp(fvel, -12, 12);
    svel = clamp(svel, -12, 12);

    if (!bFollowMode && nMapMode)
    {
        mapTurn += q16avel << 3;
        mapForward += fvel << 8;
        mapStrafe += svel << 8;;

        lPlayerYVel = 0;
        lPlayerXVel = 0;
        nPlayerDAng = 0;
        return;
    }

    nPlayerDAng += q16avel;

    inita &= kAngleMask;

    lPlayerXVel += fvel * Cos(inita) + svel * Sin(inita);
    lPlayerYVel += fvel * Sin(inita) - svel * Cos(inita);

    lPlayerXVel -= (lPlayerXVel >> 5) + (lPlayerXVel >> 6);
    lPlayerYVel -= (lPlayerYVel >> 5) + (lPlayerYVel >> 6);

    // A horiz diff of 128 equal 45 degrees,
    // so we convert horiz to 1024 angle units

    float horizAngle = atan2f(nVertPan[nLocalPlayer] - F16(92), F16(128)) * (512.f / fPI) + fix16_to_float(q16horz);
    nVertPan[nLocalPlayer] = fix16_clamp(F16(92) + Blrintf(F16(128) * tanf(horizAngle * (fPI / 512.f))), F16(0), F16(184));

#if 0
    info.dyaw *= (lMouseSens >> 1) + 1;

    int nXVel, nYVel;

    inita &= kAngleMask;

    if (BUTTON(gamefunc_Run))
    {
        nXVel = Cos(inita) * 12;
        nYVel = Sin(inita) * 12;
    }
    else
    {
        nXVel = Cos(inita) * 6;
        nYVel = Sin(inita) * 6;
    }

    // loc_18E60
    if (BUTTON(gamefunc_Move_Forward))
    {
        lPlayerXVel += nXVel;
        lPlayerYVel += nYVel;
    }
    else if (BUTTON(gamefunc_Move_Backward))
    {
        lPlayerXVel -= nXVel;
        lPlayerYVel -= nYVel;
    }
    if (info.mousey)
    {
        if (info.mousey < -6400)
        {
            info.mousey = -6400;
        }
        else if (info.mousey > 6400)
        {
            info.mousey = 6400;
        }

        if (mouseaiming)
            aimmode = BUTTON(gamefunc_Mouseview);
        else
        {
            CONTROL_ClearButton(gamefunc_Mouseview);
            aimmode = !aimmode;
        }

        // loc_18EE4
        if (aimmode)
        {
            fix16_t nVPan = nVertPan[nLocalPlayer] - fix16_from_int((info.mousey >> 7));

            if (nVPan < F16(0))
            {
                nVPan = F16(0);
            }
            else if (nVPan > F16(184))
            {
                nVPan = F16(184);
            }

            nVertPan[nLocalPlayer] = nVPan;
        }
        else
        {
            if (BUTTON(gamefunc_Run))
            {
                lPlayerXVel += Cos(inita) * ((-info.mousey) >> 7);
                lPlayerYVel += Sin(inita) * ((-info.mousey) >> 7);
            }
            else
            {
                lPlayerXVel += Cos(inita) * ((-info.mousey) >> 8);
                lPlayerYVel += Sin(inita) * ((-info.mousey) >> 8);
            }
        }
    }

    // loc_18FD4
    if (BUTTON(gamefunc_Strafe_Left))
    {
        lPlayerXVel += nYVel / 4;
        lPlayerYVel -= nXVel / 4;
    }
    else if (BUTTON(gamefunc_Strafe_Right))
    {
        lPlayerXVel -= nYVel / 4;
        lPlayerYVel += nXVel / 4;
    }
    else
    {
        if (BUTTON(gamefunc_Strafe))
        {
            if (BUTTON(gamefunc_Turn_Left))
            {
                lPlayerXVel += nYVel;
                lPlayerYVel -= nXVel;
            }
            else if (BUTTON(gamefunc_Turn_Right))
            {
                lPlayerXVel -= nYVel;
                lPlayerYVel += nXVel;
            }
            else
            {
                if (BUTTON(gamefunc_Run))
                {
                    lPlayerXVel -= Sin(inita) * (info.dyaw >> 5);
                    lPlayerYVel += Sin(inita + 512) * (info.dyaw >> 5);
                }
                else
                {
                    lPlayerXVel -= Sin(inita) * (info.dyaw >> 7);
                    lPlayerYVel += Sin(inita + 512) * (info.dyaw >> 7);
                }
            }
        }
        else
        {
            // normal, non strafing movement
            if (BUTTON(gamefunc_Turn_Left))
            {
                nPlayerDAng -= 2;

                if (BUTTON(gamefunc_Run))
                {
                    if (nPlayerDAng < -12)
                        nPlayerDAng = -12;
                }
                else if (nPlayerDAng < -8)
                {
                    nPlayerDAng = -8;
                }
            }
            else if (BUTTON(gamefunc_Turn_Right))
            {
                nPlayerDAng += 2;

                if (BUTTON(gamefunc_Run))
                {
                    if (nPlayerDAng > 12)
                        nPlayerDAng = 12;
                }
                else if (nPlayerDAng > 8)
                {
                    nPlayerDAng = 8;
                }
            }
            else
            {
                int nAng = info.dyaw >> 8;

                // loc_19201:
                if (info.dyaw < 0)
                {
                    if (nAng > -2) {
                        nAng = -2;
                    }

                    nPlayerDAng = nAng;
                }
                else if (info.dyaw > 0)
                {
                    if (nAng < 2) {
                        nAng = 2;
                    }

                    nPlayerDAng = nAng;
                }
            }
        }
    }

    // loc_19231
    if (nPlayerDAng < 0)
    {
        nPlayerDAng++;
        if (nPlayerDAng > 0)
            nPlayerDAng = 0;
    }

    if (nPlayerDAng > 0)
    {
        nPlayerDAng--;
        if (nPlayerDAng < 0)
            nPlayerDAng = 0;
    }

    lPlayerXVel -= (lPlayerXVel >> 5) + (lPlayerXVel >> 6);
    lPlayerYVel -= (lPlayerYVel >> 5) + (lPlayerYVel >> 6);
#endif
}

void RestoreSavePoint(int nPlayer, int *x, int *y, int *z, short *nSector, short *nAngle)
{
    *x = sPlayerSave[nPlayer].x;
    *y = sPlayerSave[nPlayer].y;
    *z = sPlayerSave[nPlayer].z;
    *nSector = sPlayerSave[nPlayer].nSector;
    *nAngle  = sPlayerSave[nPlayer].nAngle;
}

void SetSavePoint(int nPlayer, int x, int y, int z, short nSector, short nAngle)
{
    sPlayerSave[nPlayer].x = x;
    sPlayerSave[nPlayer].y = y;
    sPlayerSave[nPlayer].z = z;
    sPlayerSave[nPlayer].nSector = nSector;
    sPlayerSave[nPlayer].nAngle = nAngle;
}

void feebtag(int x, int y, int z, int nSector, int *nSprite, int nVal2, int nVal3)
{
    *nSprite = -1;

    int startwall = sector[nSector].wallptr;

    int nWalls = sector[nSector].wallnum;

    int var_20 = nVal2 & 2;
    int var_14 = nVal2 & 1;

    while (1)
    {
        if (nSector != -1)
        {
            short i = headspritesect[nSector];

            while (i != -1)
            {
                short nNextSprite = nextspritesect[i];
                short nStat = sprite[i].statnum;

                if (nStat >= 900 && !(sprite[i].cstat & 0x8000))
                {
                    uint32_t xDiff = klabs(sprite[i].x - x);
                    uint32_t yDiff = klabs(sprite[i].y - y);
                    int zDiff = sprite[i].z - z;

                    if (zDiff < 5120 && zDiff > -25600)
                    {
                        uint32_t diff = xDiff * xDiff + yDiff * yDiff;

                        if (diff > INT_MAX)
                        {
                            OSD_Printf("%s %d: overflow\n", EDUKE32_FUNCTION, __LINE__);
                            diff = INT_MAX; 
                        }

                        int theSqrt = ksqrt(diff);

                        if (theSqrt < nVal3 && (nStat != 950 && nStat != 949 || !(var_14 & 1)) && (nStat != 912 && nStat != 913 || !(var_20 & 2)))
                        {
                            nVal3 = theSqrt;
                            *nSprite = i;
                        }
                    }
                }

                i = nNextSprite;
            }
        }

        nWalls--;
        if (nWalls < -1)
            return;

        nSector = wall[startwall].nextsector;
        startwall++;
    }
}

void InitPlayer()
{
    for (int i = 0; i < kMaxPlayers; i++) {
        PlayerList[i].nSprite = -1;
    }
}

void InitPlayerKeys(short nPlayer)
{
    PlayerList[nPlayer].keys = 0;
}

void InitPlayerInventory(short nPlayer)
{
    memset(&PlayerList[nPlayer], 0, sizeof(Player));

    nPlayerItem[nPlayer] = -1;
    nPlayerSwear[nPlayer] = 4;

    ResetPlayerWeapons(nPlayer);

    nPlayerLives[nPlayer] = kDefaultLives;

    PlayerList[nPlayer].nSprite = -1;
    PlayerList[nPlayer].nRun = -1;

    nPistolClip[nPlayer] = 6;
    nPlayerClip[nPlayer] = 0;

    PlayerList[nPlayer].nCurrentWeapon = 0;

    if (nPlayer == nLocalPlayer) {
        nMapMode = 0;
    }

    nPlayerScore[nPlayer] = 0;

    short nTile = kTile3571 + nPlayer;

    if (!waloff[nTile]) tileLoad(nTile);

    nPlayerColor[nPlayer] = *(uint8_t*)(waloff[nTile] + tilesiz[nTile].x * tilesiz[nTile].y / 2);
}

short GetPlayerFromSprite(short nSprite)
{
    return RunData[sprite[nSprite].owner].nVal;
}

void RestartPlayer(short nPlayer)
{
    int nSprite = PlayerList[nPlayer].nSprite;
    int nDopSprite = nDoppleSprite[nPlayer];

    int floorspr;

    if (nSprite > -1)
    {
        runlist_DoSubRunRec(sprite[nSprite].owner);
        runlist_FreeRun(sprite[nSprite].lotag - 1);

        changespritestat(nSprite, 0);

        PlayerList[nPlayer].nSprite = -1;

        int nFloorSprite = nPlayerFloorSprite[nPlayer];
        if (nFloorSprite > -1) {
            mydeletesprite(nFloorSprite);
        }

        if (nDopSprite > -1)
        {
            runlist_DoSubRunRec(sprite[nDopSprite].owner);
            runlist_FreeRun(sprite[nDopSprite].lotag - 1);
            mydeletesprite(nDopSprite);
        }
    }

    nSprite = GrabBody();

    mychangespritesect(nSprite, sPlayerSave[nPlayer].nSector);
    changespritestat(nSprite, 100);

    assert(nSprite >= 0 && nSprite < kMaxSprites);

    int nDSprite = insertsprite(sprite[nSprite].sectnum, 100);
    nDoppleSprite[nPlayer] = nDSprite;

    assert(nDSprite >= 0 && nDSprite < kMaxSprites);

    if (nTotalPlayers > 1)
    {
        int nNStartSprite = nNetStartSprite[nCurStartSprite];
        nCurStartSprite++;

        if (nCurStartSprite >= nNetStartSprites) {
            nCurStartSprite = 0;
        }

        sprite[nSprite].x = sprite[nNStartSprite].x;
        sprite[nSprite].y = sprite[nNStartSprite].y;
        sprite[nSprite].z = sprite[nNStartSprite].z;
        mychangespritesect(nSprite, sprite[nNStartSprite].sectnum);
        PlayerList[nPlayer].q16angle = fix16_from_int(sprite[nNStartSprite].ang & kAngleMask);
        sprite[nSprite].ang = fix16_to_int(PlayerList[nPlayer].q16angle);

        floorspr = insertsprite(sprite[nSprite].sectnum, 0);
        assert(floorspr >= 0 && floorspr < kMaxSprites);

        sprite[floorspr].x = sprite[nSprite].x;
        sprite[floorspr].y = sprite[nSprite].y;
        sprite[floorspr].z = sprite[nSprite].z;
        sprite[floorspr].yrepeat = 64;
        sprite[floorspr].xrepeat = 64;
        sprite[floorspr].cstat = 32;
        sprite[floorspr].picnum = nPlayer + kTile3571;
    }
    else
    {
        sprite[nSprite].x = sPlayerSave[nPlayer].x;
        sprite[nSprite].y = sPlayerSave[nPlayer].y;
        sprite[nSprite].z = sector[sPlayerSave[nPlayer].nSector].floorz;
        PlayerList[nPlayer].q16angle = fix16_from_int(sPlayerSave[nPlayer].nAngle&kAngleMask);
        sprite[nSprite].ang = fix16_to_int(PlayerList[nPlayer].q16angle);

        floorspr = -1;
    }

    PlayerList[nPlayer].opos = sprite[nSprite].xyz;
    PlayerList[nPlayer].q16oangle = PlayerList[nPlayer].q16angle;

    nPlayerFloorSprite[nPlayer] = floorspr;

    sprite[nSprite].cstat = 0x101;
    sprite[nSprite].shade = -12;
    sprite[nSprite].clipdist = 58;
    sprite[nSprite].pal = 0;
    sprite[nSprite].xrepeat = 40;
    sprite[nSprite].yrepeat = 40;
    sprite[nSprite].xoffset = 0;
    sprite[nSprite].yoffset = 0;
    sprite[nSprite].picnum = seq_GetSeqPicnum(kSeqJoe, 18, 0);

    int nHeight = GetSpriteHeight(nSprite);
    sprite[nSprite].xvel = 0;
    sprite[nSprite].yvel = 0;
    sprite[nSprite].zvel = 0;

    nStandHeight = nHeight;

    sprite[nSprite].hitag = 0;
    sprite[nSprite].extra = -1;
    sprite[nSprite].lotag = runlist_HeadRun() + 1;

    sprite[nDSprite].x = sprite[nSprite].x;
    sprite[nDSprite].y = sprite[nSprite].y;
    sprite[nDSprite].z = sprite[nSprite].z;
    sprite[nDSprite].xrepeat = sprite[nSprite].xrepeat;
    sprite[nDSprite].yrepeat = sprite[nSprite].yrepeat;
    sprite[nDSprite].xoffset = 0;
    sprite[nDSprite].yoffset = 0;
    sprite[nDSprite].shade = sprite[nSprite].shade;
    sprite[nDSprite].ang = sprite[nSprite].ang;
    sprite[nDSprite].cstat = sprite[nSprite].cstat;

    sprite[nDSprite].lotag = runlist_HeadRun() + 1;

    PlayerList[nPlayer].nAction = 0;
    PlayerList[nPlayer].nHealth = kMaxHealth;

    if (nNetPlayerCount) {
        PlayerList[nPlayer].nHealth = kMaxHealth * 2;
    }

    PlayerList[nPlayer].nFrame = 0;
    PlayerList[nPlayer].nSprite = nSprite;
    PlayerList[nPlayer].bIsMummified = kFalse;

    if (PlayerList[nPlayer].invincibility >= 0) {
        PlayerList[nPlayer].invincibility = 0;
    }

    nPlayerTorch[nPlayer] = 0;
    PlayerList[nPlayer].nMaskAmount = 0;

    SetTorch(nPlayer, 0);

    nPlayerInvisible[nPlayer] = 0;

    PlayerList[nPlayer].bIsFiring = 0;
    PlayerList[nPlayer].nWeaponFrame = 0;
    nPlayerViewSect[nPlayer] = sPlayerSave[nPlayer].nSector;
    PlayerList[nPlayer].nWeaponState = 0;

    nPlayerDouble[nPlayer] = 0;

    PlayerList[nPlayer].nSeq = kSeqJoe;

    nPlayerPushSound[nPlayer] = -1;

    PlayerList[nPlayer].nNewWeapon = -1;

    if (PlayerList[nPlayer].nCurrentWeapon == 7) {
        PlayerList[nPlayer].nCurrentWeapon = PlayerList[nPlayer].nLastWeapon;
    }

    PlayerList[nPlayer].nLastWeapon = 0;
    PlayerList[nPlayer].nAir = 100;
    airpages = 0;

    if (levelnum <= kMap20)
    {
        RestoreMinAmmo(nPlayer);
    }
    else
    {
        ResetPlayerWeapons(nPlayer);
        PlayerList[nPlayer].nMagic = 0;
    }

    nPlayerGrenade[nPlayer] = -1;
    oeyelevel[nPlayer] = eyelevel[nPlayer] = -14080;
    dVertPan[nPlayer] = 0;

    nTemperature[nPlayer] = 0;

    nYDamage[nPlayer] = 0;
    nXDamage[nPlayer] = 0;

    PlayerList[nPlayer].q16ohoriz = PlayerList[nPlayer].q16horiz = nVertPan[nPlayer] = F16(92);
    nDestVertPan[nPlayer] = F16(92);
    nBreathTimer[nPlayer] = 90;

    nTauntTimer[nPlayer] = RandomSize(3) + 3;

    sprite[nDSprite].owner = runlist_AddRunRec(sprite[nDSprite].lotag - 1, nPlayer, kRunPlayer);
    sprite[nSprite].owner = runlist_AddRunRec(sprite[nSprite].lotag - 1, nPlayer, kRunPlayer);

    if (PlayerList[nPlayer].nRun < 0) {
        PlayerList[nPlayer].nRun = runlist_AddRunRec(NewRun, nPlayer, kRunPlayer);
    }

    BuildRa(nPlayer);

    if (nPlayer == nLocalPlayer)
    {
        nLocalSpr = nSprite;
        nPlayerDAng = 0;

        SetMagicFrame();
        RestoreGreenPal();

        bPlayerPan = 0;
        bLockPan = 0;

		nCameraDist = 0;
		nCameraClock = (int32_t)totalclock;

        SetMapPosition(sprite[nSprite].x, sprite[nSprite].y, sprite[nSprite].ang);
    }

    sprintf(playerNames[nPlayer], "JOE%d", nPlayer);
    namelen[nPlayer] = strlen(playerNames[nPlayer]);

    totalvel[nPlayer] = 0;

    memset(&sPlayerInput[nPlayer], 0, sizeof(PlayerInput));
    sPlayerInput[nPlayer].nItem = -1;

    nDeathType[nPlayer] = 0;
    nQuake[nPlayer] = 0;

    if (nPlayer == nLocalPlayer) {
        SetHealthFrame(0);
    }
}

int GrabPlayer()
{
    if (PlayerCount >= kMaxPlayers) {
        return -1;
    }

    return PlayerCount++;
}

void StartDeathSeq(int nPlayer, int nVal)
{
    FreeRa(nPlayer);

    short nSprite = PlayerList[nPlayer].nSprite;
    PlayerList[nPlayer].nHealth = 0;

    short nLotag = sector[sprite[nSprite].sectnum].lotag;

    if (nLotag > 0) {
        runlist_SignalRun(nLotag - 1, nPlayer | 0x70000);
    }

    if (nPlayerGrenade[nPlayer] >= 0)
    {
        ThrowGrenade(nPlayer, 0, 0, 0, -10000);
    }
    else
    {
        if (nNetPlayerCount)
        {
            int nWeapon = PlayerList[nPlayer].nCurrentWeapon;

            if (nWeapon > kWeaponSword && nWeapon <= kWeaponRing)
            {
                short nSector = sprite[nSprite].sectnum;
                if (SectBelow[nSector] > -1) {
                    nSector = SectBelow[nSector];
                }

                int nGunSprite = GrabBodyGunSprite();
                changespritesect(nGunSprite, nSector);

                sprite[nGunSprite].x = sprite[nSprite].x;
                sprite[nGunSprite].y = sprite[nSprite].y;
                sprite[nGunSprite].z = sector[nSector].floorz - 512;

                changespritestat(nGunSprite, nGunLotag[nWeapon] + 900);

                sprite[nGunSprite].picnum = nGunPicnum[nWeapon];

                BuildItemAnim(nGunSprite);
            }
        }
    }

    StopFiringWeapon(nPlayer);

    PlayerList[nPlayer].q16ohoriz = PlayerList[nPlayer].q16horiz = nVertPan[nPlayer] = F16(92);
    oeyelevel[nPlayer] = eyelevel[nPlayer] = -14080;
    nPlayerInvisible[nPlayer] = 0;
    dVertPan[nPlayer] = 15;

    sprite[nSprite].cstat &= 0x7FFF;

    SetNewWeaponImmediate(nPlayer, -2);

    if (SectDamage[sprite[nSprite].sectnum] <= 0)
    {
        nDeathType[nPlayer] = nVal;
    }
    else
    {
        nDeathType[nPlayer] = 2;
    }

    nVal *= 2;

    if (nVal || !(SectFlag[sprite[nSprite].sectnum] & kSectUnderwater))
    {
        PlayerList[nPlayer].nAction = nVal + 17;
    }
    else {
        PlayerList[nPlayer].nAction = 16;
    }

    PlayerList[nPlayer].nFrame = 0;

    sprite[nSprite].cstat &= 0xFEFE;

    if (nTotalPlayers == 1)
    {
        short nLives = nPlayerLives[nPlayer];

        if (nLives > 0) {
            BuildStatusAnim((3 * (nLives - 1)) + 7, 0);
        }

        if (levelnum > 0) { // if not on the training level
            nPlayerLives[nPlayer]--;
        }

        if (nPlayerLives[nPlayer] < 0) {
            nPlayerLives[nPlayer] = 0;
        }
    }

    totalvel[nPlayer] = 0;

    if (nPlayer == nLocalPlayer) {
        RefreshStatus();
    }
}

int AddAmmo(int nPlayer, int nWeapon, int nAmmoAmount)
{
    if (!nAmmoAmount) {
        nAmmoAmount = 1;
    }

    short nCurAmmo = PlayerList[nPlayer].nAmmo[nWeapon];

    if (nCurAmmo >= 300 && nAmmoAmount > 0) {
        return 0;
    }

    nAmmoAmount = nCurAmmo + nAmmoAmount;
    if (nAmmoAmount > 300) {
        nAmmoAmount = 300;
    }

    PlayerList[nPlayer].nAmmo[nWeapon] = nAmmoAmount;

    if (nPlayer == nLocalPlayer)
    {
        if (nWeapon == nCounterBullet) {
            SetCounter(nAmmoAmount);
        }
    }

    if (nWeapon == 1)
    {
        if (!nPistolClip[nPlayer]) {
            nPistolClip[nPlayer] = 6;
        }
    }

    return 1;
}

void SetPlayerMummified(int nPlayer, int bIsMummified)
{
    int nSprite = PlayerList[nPlayer].nSprite;

    sprite[nSprite].yvel = 0;
    sprite[nSprite].xvel = 0;

    PlayerList[nPlayer].bIsMummified = bIsMummified;

    if (bIsMummified)
    {
        PlayerList[nPlayer].nAction = 13;
        PlayerList[nPlayer].nSeq = kSeqMummy;
    }
    else
    {
        PlayerList[nPlayer].nAction = 0;
        PlayerList[nPlayer].nSeq = kSeqJoe;
    }

    PlayerList[nPlayer].nFrame = 0;
}

void ShootStaff(int nPlayer)
{
    PlayerList[nPlayer].nAction = 15;
    PlayerList[nPlayer].nFrame = 0;
    PlayerList[nPlayer].nSeq = kSeqJoe;
}

void PlayAlert(const char *str)
{
    StatusMessage(300, str);
    PlayLocalSound(StaticSound[kSoundSawOn], 0);
}

void DoKenTest()
{
    int nPlayerSprite = PlayerList[0].nSprite;
    if ((unsigned int)nPlayerSprite >= kMaxSprites)
    {
        return;
    }

    int nSector = sprite[nPlayerSprite].sectnum;
    if ((unsigned int)nSector >= kMaxSectors)
    {
        initprintf("DoKenTest: (unsigned int)nSector >= kMaxSectors\n");
        return;
    }

    for (int i = headspritesect[nSector]; ; i = nextspritesect[i])
    {
        if (i == -1) {
            return;
        }

        if (nextspritesect[i] == i) {
            bail2dos("ERROR in Ken's linked list!\n");
        }
    }
}

static void PlayerCheckItemRespawnOrDelete(int nItemSprite, int nVal)
{
    // CHECKME - is order of evaluation correct?
    if (levelnum <= 20 || nVal >= 25 && (nVal <= 25 || nVal == 50))
    {
        DestroyItemAnim(nItemSprite);
        mydeletesprite(nItemSprite);
    }
    else
    {
        StartRegenerate(nItemSprite);
    }
}

static void SetItemPickupMessage(int nPlayer, int nVal)
{
    if (nPlayer != nLocalPlayer)
        return;

    if (nItemText[nVal] > -1 && nTotalPlayers == 1)
    {
        StatusMessage(400, gString[nItemTextIndex + nItemText[nVal]]);
    }
}

static void WeaponPickup(int nPlayer, int nSprite, int nWeapon, int nAddAmmo, int nItem, int rTint, int gTint)
{
    int nPickupSound = -1;
    int weaponBit = 1 << nWeapon;

    if (nPlayerWeapons[nPlayer] & weaponBit)
    {
        if (levelnum > 20)
        {
            AddAmmo(nPlayer, WeaponInfo[nWeapon].nAmmoType, nAddAmmo);
        }
    }
    else
    {
        SetNewWeaponIfBetter(nPlayer, nWeapon);

        nPlayerWeapons[nPlayer] |= weaponBit;

        AddAmmo(nPlayer, WeaponInfo[nWeapon].nAmmoType, nAddAmmo);

        nPickupSound = StaticSound[kSoundWeapon];
    }

    if (nWeapon == 2) {
        CheckClip(nPlayer);
    }

    if (nItem <= 50)
    {
        PlayerCheckItemRespawnOrDelete(nSprite, nItem);
    }
    else
    {
        sprite[nSprite].cstat = 0x8000;
        DestroyItemAnim(nSprite);
    }

    SetItemPickupMessage(nPlayer, nItem);
    TintPalette(rTint * 4, gTint * 4, 0);

    if (nPickupSound > -1) {
        PlayLocalSound(nPickupSound, 0);
    }
}

static void PlayerPickupKey(int nPlayer, int nKey, int nSprite, int nItem)
{
    int keyBit = 0x1000 << nKey;

    if (!(PlayerList[nPlayer].keys & keyBit))
    {
        if (nPlayer == nLocalPlayer) {
            BuildStatusAnim(36 + (nKey * 2), 0);
        }

        PlayerList[nPlayer].keys |= keyBit;

        if (nTotalPlayers <= 1)
        {
            PlayerCheckItemRespawnOrDelete(nSprite, nItem);
        }

        SetItemPickupMessage(nPlayer, nItem);
        TintPalette(0, 16 * 4, 0);
    }
}

void PlayerCheckItemPickup(int nPlayer, int nPickupSprite, int valueFlag)
{
    int nPlayerSprite = PlayerList[nPlayer].nSprite;

    if (nPickupSprite >= 0 && sprite[nPickupSprite].statnum >= 900)
    {
        int rTint = 0;
        int gTint = 16;
        int nPickupSound = 9;

        int nItem = sprite[nPickupSprite].statnum - 900;

        // TODO - handle 0-5 ?

        if (nItem > 60) // item end at number 60
            return;

        switch (nItem)
        {
            default:
            {
                PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                break;
            }
            case 6: // Speed Loader
            {
                if (AddAmmo(nPlayer, 1, sprite[nPickupSprite].hitag))
                {
                    nPickupSound = StaticSound[kSoundAmmoPickup];
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }
                return;
            }
            case 7: // Fuel Canister
            {
                if (AddAmmo(nPlayer, 3, sprite[nPickupSprite].hitag))
                {
                    nPickupSound = StaticSound[kSoundAmmoPickup];
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }
                return;
            }
            case 8: // M - 60 Ammo Belt
            {
                if (AddAmmo(nPlayer, 2, sprite[nPickupSprite].hitag))
                {
                    nPickupSound = StaticSound[kSoundAmmoPickup];
                    CheckClip(nPlayer);
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }
                return;
            }
            case 9: // Grenade
            case 27:
            case 55:
            {
                if (AddAmmo(nPlayer, 4, 1))
                {
                    nPickupSound = StaticSound[kSoundAmmoPickup];
                    if (!(nPlayerWeapons[nPlayer] & 0x10))
                    {
                        nPlayerWeapons[nPlayer] |= 0x10;
                        SetNewWeaponIfBetter(nPlayer, 4);
                    }

                    if (nItem == 55)
                    {
                        sprite[nPickupSprite].cstat = 0x8000;
                        DestroyItemAnim(nPickupSprite);
                        break;
                    }
                    else
                    {
                        PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                        break;
                    }
                }
                return;
            }

            case 10: // Pickable item
            case 15: // Pickable item
            case 16: // Reserved
            case 24:
            case 31:
            case 34:
            case 35:
            case 36:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 51:
            case 58:
            {
                PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                break;
            }

            case 11: // Map
            {
                GrabMap();
                PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                break;
            }

            case 12: // Berry Twig
            {
                if (sprite[nPickupSprite].hitag == 0) {
                    return;
                }

                nPickupSound = 20;
                int nHealthModifier = 40;

                if (nHealthModifier <= 0 || (!(valueFlag & 2)))
                {
                    if (!PlayerList[nPlayer].invincibility || nHealthModifier > 0)
                    {
                        PlayerList[nPlayer].nHealth += nHealthModifier;
                        if (PlayerList[nPlayer].nHealth > kMaxHealth)
                        {
                            PlayerList[nPlayer].nHealth = kMaxHealth;
                        }
                        else
                        {
                            if (PlayerList[nPlayer].nHealth < 0)
                            {
                                nPickupSound = -1;
                                StartDeathSeq(nPlayer, 0);
                            }
                        }
                    }

                    if (nLocalPlayer == nPlayer)
                    {
                        SetHealthFrame(1);
                    }

                    if (nItem == 12)
                    {
                        sprite[nPickupSprite].hitag = 0;
                        sprite[nPickupSprite].picnum++;

                        changespritestat(nPickupSprite, 0);
                        break;
                    }
                    else
                    {
                        if (nItem != 14)
                        {
                            nPickupSound = 21;
                        }
                        else
                        {
                            rTint = gTint;
                            nPickupSound = 22;
                            gTint = 0;
                        }

                        PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                        break;
                    }
                }

                return;
            }

            case 13: // Blood Bowl
            {
                int nHealthModifier = 160;

                // Same code as case 12 now till break
                if (nHealthModifier <= 0 || (!(valueFlag & 2)))
                {
                    if (!PlayerList[nPlayer].invincibility || nHealthModifier > 0)
                    {
                        PlayerList[nPlayer].nHealth += nHealthModifier;
                        if (PlayerList[nPlayer].nHealth > kMaxHealth)
                        {
                            PlayerList[nPlayer].nHealth = kMaxHealth;
                        }
                        else
                        {
                            if (PlayerList[nPlayer].nHealth < 0)
                            {
                                nPickupSound = -1;
                                StartDeathSeq(nPlayer, 0);
                            }
                        }
                    }

                    if (nLocalPlayer == nPlayer)
                    {
                        SetHealthFrame(1);
                    }

                    if (nItem == 12)
                    {
                        sprite[nPickupSprite].hitag = 0;
                        sprite[nPickupSprite].picnum++;

                        changespritestat(nPickupSprite, 0);
                        break;
                    }
                    else
                    {
                        if (nItem != 14)
                        {
                            nPickupSound = 21;
                        }
                        else
                        {
                            rTint = gTint;
                            nPickupSound = 22;
                            gTint = 0;
                        }

                        PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                        break;
                    }
                }

                return;
            }

            case 14: // Cobra Venom Bowl
            {
                int nHealthModifier = -200;

                // Same code as case 12 and 13 from now till break
                if (nHealthModifier <= 0 || (!(valueFlag & 2)))
                {
                    if (!PlayerList[nPlayer].invincibility || nHealthModifier > 0)
                    {
                        PlayerList[nPlayer].nHealth += nHealthModifier;
                        if (PlayerList[nPlayer].nHealth > kMaxHealth)
                        {
                            PlayerList[nPlayer].nHealth = kMaxHealth;
                        }
                        else
                        {
                            if (PlayerList[nPlayer].nHealth < 0)
                            {
                                nPickupSound = -1;
                                StartDeathSeq(nPlayer, 0);
                            }
                        }
                    }

                    if (nLocalPlayer == nPlayer)
                    {
                        SetHealthFrame(1);
                    }

                    if (nItem == 12)
                    {
                        sprite[nPickupSprite].hitag = 0;
                        sprite[nPickupSprite].picnum++;

                        changespritestat(nPickupSprite, 0);
                        break;
                    }
                    else
                    {
                        if (nItem != 14)
                        {
                            nPickupSound = 21;
                        }
                        else
                        {
                            rTint = gTint;
                            nPickupSound = 22;
                            gTint = 0;
                        }

                        PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                        break;
                    }
                }

                return;
            }

            case 17: // Bubble Nest
            {
                PlayerList[nPlayer].nAir += 10;
                if (PlayerList[nPlayer].nAir > 100) {
                    PlayerList[nPlayer].nAir = 100; // TODO - constant
                }

                SetAirFrame();

                if (nBreathTimer[nPlayer] < 89)
                {
                    D3PlayFX(StaticSound[kSoundJonBubbleNest], nPlayerSprite);
                }

                nBreathTimer[nPlayer] = 90;
                return;
            }

            case 18: // Still Beating Heart
            {
                if (GrabItem(nPlayer, kItemHeart))
                {
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 19: // Scarab amulet (Invicibility)
            {
                if (GrabItem(nPlayer, kItemInvincibility))
                {
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 20: // Severed Slave Hand(double damage)
            {
                if (GrabItem(nPlayer, kItemDoubleDamage))
                {
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 21: // Unseen eye (Invisibility)
            {
                if (GrabItem(nPlayer, kItemInvisibility))
                {
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 22: // Torch
            {
                if (GrabItem(nPlayer, kItemTorch))
                {
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 23: // Sobek Mask
            {
                if (GrabItem(nPlayer, kItemMask))
                {
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 25: // Extra Life
            {
                nPickupSound = -1;

                if (nPlayerLives[nPlayer] >= kMaxPlayerLives) {
                    return;
                }

                nPlayerLives[nPlayer]++;

                if (nPlayer == nLocalPlayer) {
                    BuildStatusAnim(146 + ((nPlayerLives[nPlayer] - 1) * 2), 0);
                }

                gTint = 32;
                rTint = 32;

                PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                break;
            }

            case 26: // lotag 26, sword pickup??
            {
                WeaponPickup(nPlayer, nPickupSprite, kWeaponSword, 0, nItem, rTint, gTint);
                return;
            }

            case 28: // lotag 28, .357 Magnum Revolver
            case 52:
            {
                WeaponPickup(nPlayer, nPickupSprite, kWeaponPistol, 6, nItem, rTint, gTint);
                return;
            }

            case 29: // M - 60 Machine Gun
            case 53:
            {
                WeaponPickup(nPlayer, nPickupSprite, kWeaponM60, 24, nItem, rTint, gTint);
                return;
            }

            case 30: // Flame Thrower
            case 54:
            {
                WeaponPickup(nPlayer, nPickupSprite, kWeaponFlamer, 100, nItem, rTint, gTint);
                return;
            }

            case 32: // Cobra Staff
            case 56:
            {
                WeaponPickup(nPlayer, nPickupSprite, kWeaponStaff, 20, nItem, rTint, gTint);
                return;
            }

            case 33: // Eye of Ra Gauntlet
            case 57:
            {
                WeaponPickup(nPlayer, nPickupSprite, kWeaponRing, 2, nItem, rTint, gTint);
                return;
            }

            case 37: // Cobra staff ammo
            {
                if (AddAmmo(nPlayer, 5, 1))
                {
                    nPickupSound = StaticSound[kSoundAmmoPickup];
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 38: // Raw Energy
            {
                if (AddAmmo(nPlayer, 6, sprite[nPickupSprite].hitag))
                {
                    nPickupSound = StaticSound[kSoundAmmoPickup];
                    PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                    break;
                }

                return;
            }

            case 45: // Power key
            {
                PlayerPickupKey(nPlayer, 0, nPickupSprite, nItem);
                return;
            }
            case 46: // Time key
            {
                PlayerPickupKey(nPlayer, 1, nPickupSprite, nItem);
                return;
            }
            case 47: // War key
            {
                PlayerPickupKey(nPlayer, 2, nPickupSprite, nItem);
                return;
            }
            case 48: // Earth key
            {
                PlayerPickupKey(nPlayer, 3, nPickupSprite, nItem);
                return;
            }

            case 49: // Magical Essence
            case 50: // ?
            {
                if (PlayerList[nPlayer].nMagic >= kMaxMagic) {
                    return;
                }

                nPickupSound = StaticSound[kSoundMana1];

                PlayerList[nPlayer].nMagic += 100;
                if (PlayerList[nPlayer].nMagic >= kMaxMagic) {
                    PlayerList[nPlayer].nMagic = kMaxMagic;
                }

                if (nLocalPlayer == nPlayer)
                {
                    SetMagicFrame();
                }

                PlayerCheckItemRespawnOrDelete(nPickupSprite, nItem);
                break;
            }

            case 59: // Scarab (Checkpoint)
            {
                if (nLocalPlayer == nPlayer)
                {
                    short nAnim = sprite[nPickupSprite].owner;
                    AnimList[nAnim].nSeq++;
                    AnimFlags[nAnim] &= 0xEF;
                    AnimList[nAnim].nFrame = 0;

                    changespritestat(nPickupSprite, 899);
                }

                SetSavePoint(nPlayer, sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].z, sprite[nPlayerSprite].sectnum, sprite[nPlayerSprite].ang);
                return;
            }

            case 60: // Golden Sarcophagus (End Level)
            {
                if (!bInDemo) {
                    FinishLevel();
                }
                else {
                    // KB_Addch(32);
                    bDemoPlayerFinishedLevel = true;
                }

                DestroyItemAnim(nPickupSprite);
                mydeletesprite(nPickupSprite);
                return;
            }
        }

        SetItemPickupMessage(nPlayer, nItem);
        TintPalette(rTint * 4, gTint * 4, 0);

        if (nPickupSound > -1) {
            PlayLocalSound(nPickupSound, 0);
        }
    }
}

void FuncPlayer(int a, int nDamage, int nRun)
{
    int var_48 = 0;

    short nPlayer = RunData[nRun].nVal;
    assert(nPlayer >= 0 && nPlayer < kMaxPlayers);

    if (PlayerList[nPlayer].someNetVal == -1)
        return;

    short nPlayerSprite = PlayerList[nPlayer].nSprite;

    short nDopple = nDoppleSprite[nPlayer];

    short nAction = PlayerList[nPlayer].nAction;
    short nActionB = PlayerList[nPlayer].nAction;

    int nMessage = a & kMessageMask;

    short nSprite2;

    PlayerList[nPlayer].opos = sprite[nPlayerSprite].xyz;
    PlayerList[nPlayer].q16oangle = PlayerList[nPlayer].q16angle;
    PlayerList[nPlayer].q16ohoriz = PlayerList[nPlayer].q16horiz;
    oeyelevel[nPlayer] = eyelevel[nPlayer];

    switch (nMessage)
    {
        case 0x90000:
        {
            seq_PlotSequence(a & 0xFFFF, SeqOffsets[PlayerList[nPlayer].nSeq] + ActionSeq[nAction].a, PlayerList[nPlayer].nFrame, ActionSeq[nAction].b);
            return;
        }

        case 0xA0000:
        {
            if (PlayerList[nPlayer].nHealth <= 0) {
                return;
            }

            nDamage = runlist_CheckRadialDamage(nPlayerSprite);
            if (!nDamage) {
                return;
            }

            nSprite2 = nRadialOwner;
            // fall through to case 0x80000
            fallthrough__;
        }

        case 0x80000:
        {
            // Dunno how to do this otherwise... we fall through from above but don't want to do this check..
            if (nMessage != 0xA0000)
            {
                if (!nDamage) {
                    return;
                }

                nSprite2 = a & 0xFFFF;
            }

            // ok continue case 0x80000 as normal, loc_1C57C
            if (!PlayerList[nPlayer].nHealth) {
                return;
            }

            if (!PlayerList[nPlayer].invincibility)
            {
                PlayerList[nPlayer].nHealth -= nDamage;
                if (nPlayer == nLocalPlayer)
                {
                    TintPalette(nDamage, 0, 0);
                    SetHealthFrame(-1);
                }
            }

            if (PlayerList[nPlayer].nHealth > 0)
            {
                if (nDamage > 40 || (totalmoves & 0xF) < 2)
                {
                    if (PlayerList[nPlayer].invincibility) {
                        return;
                    }

                    if (SectFlag[sprite[nPlayerSprite].sectnum] & kSectUnderwater)
                    {
                        if (nAction != 12)
                        {
                            PlayerList[nPlayer].nFrame = 0;
                            PlayerList[nPlayer].nAction = 12;
                            return;
                        }
                    }
                    else
                    {
                        if (nAction != 4)
                        {
                            PlayerList[nPlayer].nFrame = 0;
                            PlayerList[nPlayer].nAction = 4;

                            if (nSprite2 > -1)
                            {
                                nPlayerSwear[nPlayer]--;
                                if (nPlayerSwear[nPlayer] <= 0)
                                {
                                    D3PlayFX(StaticSound[kSoundJonHit3], nDopple);
                                    nPlayerSwear[nPlayer] = RandomSize(3) + 4;
                                }
                            }
                        }
                    }
                }

                return;
            }
            else
            {
                // player has died
                if (nSprite2 > -1 && sprite[nSprite2].statnum == 100)
                {
                    short nPlayer2 = GetPlayerFromSprite(nSprite2);

                    if (nPlayer2 == nPlayer) // player caused their own death
                    {
                        nPlayerScore[nPlayer]--;
                    }
                    else
                    {
                        nPlayerScore[nPlayer]++;
                    }
                }
                else if (nSprite2 < 0)
                {
                    nPlayerScore[nPlayer]--;
                }

                if (nMessage == 0xA0000)
                {
                    for (int i = 122; i <= 131; i++)
                    {
                        BuildCreatureChunk(nPlayerSprite, seq_GetSeqPicnum(kSeqJoe, i, 0));
                    }

                    StartDeathSeq(nPlayer, 1);
                }
                else
                {
                    StartDeathSeq(nPlayer, 0);
                }
            }

            return;
        }

        case 0x20000:
        {
            sprite[nPlayerSprite].xvel = sPlayerInput[nPlayer].xVel >> 14;
            sprite[nPlayerSprite].yvel = sPlayerInput[nPlayer].yVel >> 14;

            if (sPlayerInput[nPlayer].nItem > -1)
            {
                UseItem(nPlayer, sPlayerInput[nPlayer].nItem);
                sPlayerInput[nPlayer].nItem = -1;
            }

            int var_EC = PlayerList[nPlayer].nFrame;

            sprite[nPlayerSprite].picnum = seq_GetSeqPicnum(PlayerList[nPlayer].nSeq, ActionSeq[nHeightTemplate[nAction]].a, var_EC);
            sprite[nDopple].picnum = sprite[nPlayerSprite].picnum;

            // for showing correct player animations for player sprite on 2D map mode
            nMapPic = seq_GetSeqPicnum(PlayerList[nPlayer].nSeq, ActionSeq[nAction].a, PlayerList[nPlayer].nFrame);

            if (nPlayerTorch[nPlayer] > 0)
            {
                nPlayerTorch[nPlayer]--;
                if (nPlayerTorch[nPlayer] == 0)
                {
                    SetTorch(nPlayer, 0);
                }
                else
                {
                    if (nPlayer != nLocalPlayer)
                    {
                        nFlashDepth = 5;
                        AddFlash(sprite[nPlayerSprite].sectnum,
                            sprite[nPlayerSprite].x,
                            sprite[nPlayerSprite].y,
                            sprite[nPlayerSprite].z, 0);
                    }
                }
            }

            if (nPlayerDouble[nPlayer] > 0)
            {
                nPlayerDouble[nPlayer]--;
                if (nPlayerDouble[nPlayer] == 150 && nPlayer == nLocalPlayer) {
                    PlayAlert("WEAPON POWER IS ABOUT TO EXPIRE");
                }
            }

            if (nPlayerInvisible[nPlayer] > 0)
            {
                nPlayerInvisible[nPlayer]--;
                if (nPlayerInvisible[nPlayer] == 0)
                {
                    sprite[nPlayerSprite].cstat &= 0x7FFF; // set visible
                    short nFloorSprite = nPlayerFloorSprite[nPlayerSprite];

                    if (nFloorSprite > -1) {
                        sprite[nFloorSprite].cstat &= 0x7FFF; // set visible
                    }
                }
                else if (nPlayerInvisible[nPlayer] == 150 && nPlayer == nLocalPlayer)
                {
                    PlayAlert("INVISIBILITY IS ABOUT TO EXPIRE");
                }
            }

            if (PlayerList[nPlayer].invincibility > 0)
            {
                PlayerList[nPlayer].invincibility--;
                if (PlayerList[nPlayer].invincibility == 150 && nPlayer == nLocalPlayer) {
                    PlayAlert("INVINCIBILITY IS ABOUT TO EXPIRE");
                }
            }

            if (nQuake[nPlayer] != 0)
            {
                nQuake[nPlayer] = -nQuake[nPlayer];
                if (nQuake[nPlayer] > 0)
                {
                    nQuake[nPlayer] -= 512;
                    if (nQuake[nPlayer] < 0)
                        nQuake[nPlayer] = 0;
                }
            }

            // loc_1A494:
            PlayerList[nPlayer].q16angle = (PlayerList[nPlayer].q16angle + sPlayerInput[nPlayer].nAngle) & 0x7FFFFFF;
            PlayerList[nPlayer].q16horiz = sPlayerInput[nPlayer].horizon;
            sprite[nPlayerSprite].ang = fix16_to_int(PlayerList[nPlayer].q16angle);

            // sprite[nPlayerSprite].zvel is modified within Gravity()
            short zVel = sprite[nPlayerSprite].zvel;

            Gravity(nPlayerSprite);

            if (sprite[nPlayerSprite].zvel >= 6500 && zVel < 6500)
            {
                D3PlayFX(StaticSound[kSoundJonFall], nPlayerSprite);
            }

            // loc_1A4E6
            short nSector = sprite[nPlayerSprite].sectnum;
            short nSectFlag = SectFlag[nPlayerViewSect[nPlayer]];

            int playerX = sprite[nPlayerSprite].x;
            int playerY = sprite[nPlayerSprite].y;

            int x = (sPlayerInput[nPlayer].xVel * 4) >> 2;
            int y = (sPlayerInput[nPlayer].yVel * 4) >> 2;
            int z = (sprite[nPlayerSprite].zvel * 4) >> 2;

            if (sprite[nPlayerSprite].zvel > 8192)
                sprite[nPlayerSprite].zvel = 8192;

            if (PlayerList[nPlayer].bIsMummified)
            {
                x /= 2;
                y /= 2;
            }

            int spr_x = sprite[nPlayerSprite].x;
            int spr_y = sprite[nPlayerSprite].y;
            int spr_z = sprite[nPlayerSprite].z;
            int spr_sectnum = sprite[nPlayerSprite].sectnum;

            // TODO
            // nSectFlag & kSectUnderwater;

            zVel = sprite[nPlayerSprite].zvel;

            int nMove = 0; // TEMP

            if (bSlipMode)
            {
                nMove = 0;

                sprite[nPlayerSprite].x += (x >> 14);
                sprite[nPlayerSprite].y += (y >> 14);

                vec3_t pos = { sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].z };
                setsprite(nPlayerSprite, &pos);

                sprite[nPlayerSprite].z = sector[sprite[nPlayerSprite].sectnum].floorz;
            }
            else
            {
                nMove = movesprite(nPlayerSprite, x, y, z, 5120, -5120, CLIPMASK0);

                short var_54 = sprite[nPlayerSprite].sectnum;

                pushmove_old(&sprite[nPlayerSprite].x, &sprite[nPlayerSprite].y, &sprite[nPlayerSprite].z, &var_54, sprite[nPlayerSprite].clipdist << 2, 5120, -5120, CLIPMASK0);
                if (var_54 != sprite[nPlayerSprite].sectnum) {
                    mychangespritesect(nPlayerSprite, var_54);
                }
            }

            // loc_1A6E4
            if (inside(sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].sectnum) != 1)
            {
                mychangespritesect(nPlayerSprite, spr_sectnum);

                sprite[nPlayerSprite].x = spr_x;
                sprite[nPlayerSprite].y = spr_y;

                if (zVel < sprite[nPlayerSprite].zvel) {
                    sprite[nPlayerSprite].zvel = zVel;
                }
            }

//			int _bTouchFloor = bTouchFloor;
            short bUnderwater = SectFlag[sprite[nPlayerSprite].sectnum] & kSectUnderwater;

            if (bUnderwater)
            {
                nXDamage[nPlayer] /= 2;
                nYDamage[nPlayer] /= 2;
            }

            // Trigger Ramses?
            if ((SectFlag[sprite[nPlayerSprite].sectnum] & 0x8000) && bTouchFloor)
            {
                if (nTotalPlayers <= 1)
                {
                    PlayerList[nPlayer].q16angle = fix16_from_int(GetAngleToSprite(nPlayerSprite, nSpiritSprite) & kAngleMask);
                    PlayerList[nPlayer].q16oangle = PlayerList[nPlayer].q16angle;
                    sprite[nPlayerSprite].ang = fix16_to_int(PlayerList[nPlayer].q16angle);

                    PlayerList[nPlayer].q16ohoriz = PlayerList[nPlayer].q16horiz = nVertPan[nPlayer] = F16(92);

                    lPlayerXVel = 0;
                    lPlayerYVel = 0;

                    sprite[nPlayerSprite].xvel = 0;
                    sprite[nPlayerSprite].yvel = 0;
                    sprite[nPlayerSprite].zvel = 0;

                    nPlayerDAng = 0;

                    if (nFreeze < 1)
                    {
                        nFreeze = 1;
                        StopAllSounds();
                        StopLocalSound();
                        InitSpiritHead();

                        nDestVertPan[nPlayer] = F16(92);

                        if (levelnum == 11)
                        {
                            nDestVertPan[nPlayer] += F16(46);
                        }
                        else
                        {
                            nDestVertPan[nPlayer] += F16(11);
                        }
                    }
                }
                else
                {
                    FinishLevel();
                }

                return;
            }

            if (nMove & 0x3C000)
            {
                if (bTouchFloor)
                {
                    // Damage stuff..
                    nXDamage[nPlayer] /= 2;
                    nYDamage[nPlayer] /= 2;

                    if (nPlayer == nLocalPlayer)
                    {
                        short zVelB = zVel;

                        if (zVelB < 0) {
                            zVelB = -zVelB;
                        }

                        if (zVelB > 512 && !bLockPan) {
                            nDestVertPan[nPlayer] = F16(92);
                        }
                    }

                    if (zVel >= 6500)
                    {
                        sprite[nPlayerSprite].xvel >>= 2;
                        sprite[nPlayerSprite].yvel >>= 2;

                        runlist_DamageEnemy(nPlayerSprite, -1, ((zVel - 6500) >> 7) + 10);

                        if (PlayerList[nPlayer].nHealth <= 0)
                        {
                            sprite[nPlayerSprite].xvel = 0;
                            sprite[nPlayerSprite].yvel = 0;

                            StopSpriteSound(nPlayerSprite);
    					    PlayFXAtXYZ(StaticSound[kSoundJonFDie], sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].z, sprite[nPlayerSprite].sectnum |= 0x4000); // CHECKME
                        }
                        else
                        {
                            D3PlayFX(StaticSound[kSoundJonHLand] | 0x2000, nPlayerSprite);
                        }
                    }
                }

                if (((nMove & 0xC000) == 0x4000) || ((nMove & 0xC000) == 0x8000))
                {
                    int bx = 0;

                    if ((nMove & 0xC000) == 0x4000)
                    {
                        bx = nMove & 0x3FFF;
                    }
                    else if ((nMove & 0xC000) == 0x8000)
                    {
                        bx = wall[nMove & 0x3FFF].nextsector;
                    }

                    if (bx >= 0)
                    {
                        int var_B4 = bx;

                        if ((sector[bx].hitag == 45) && bTouchFloor)
                        {
                            int nNormal = GetWallNormal(nMove & 0x3FFF);
                            int nDiff = AngleDiff(nNormal, (sprite[nPlayerSprite].ang + 1024) & kAngleMask);

                            if (nDiff < 0) {
                                nDiff = -nDiff;
                            }

                            if (nDiff <= 256)
                            {
                                nPlayerPushSect[nPlayer] = bx;

                                int var_F4 = sPlayerInput[nPlayer].xVel;
                                int var_F8 = sPlayerInput[nPlayer].yVel;
                                int nMyAngle = GetMyAngle(sPlayerInput[nPlayer].xVel, sPlayerInput[nPlayer].yVel);

                                MoveSector(var_B4, nMyAngle, &var_F4, &var_F8);

                                if (nPlayerPushSound[nPlayer] <= -1)
                                {
                                    nPlayerPushSound[nPlayer] = 1;
                                    short nBlock = sector[nPlayerPushSect[nPlayer]].extra;
                                    int nBlockSprite = sBlockInfo[nBlock].nSprite;

                                    D3PlayFX(StaticSound[kSoundAmbientStone], nBlockSprite | 0x4000);
                                }
                                else
                                {
                                    sprite[nPlayerSprite].x = spr_x;
                                    sprite[nPlayerSprite].y = spr_y;
                                    sprite[nPlayerSprite].z = spr_z;

                                    mychangespritesect(nPlayerSprite, spr_sectnum);
                                }

                                movesprite(nPlayerSprite, var_F4, var_F8, z, 5120, -5120, CLIPMASK0);
                                goto loc_1AB8E;
                            }
                        }
                    }
                }
            }

            // loc_1AB46:
            if (nPlayerPushSound[nPlayer] > -1)
            {
                if (nPlayerPushSect[nPlayer] > -1)
                {
                    StopSpriteSound(sBlockInfo[sector[nPlayerPushSect[nPlayer]].extra].nSprite);
                }

                nPlayerPushSound[nPlayer] = -1;
            }

loc_1AB8E:
            if (!bPlayerPan && !bLockPan)
            {
                fix16_t nPanVal = fix16_from_int(spr_z - sprite[nPlayerSprite].z) / 32 + F16(92);

                if (nPanVal < F16(0)) {
                    nPanVal = F16(0);
                }
                else if (nPanVal > F16(183))
                {
                    nPanVal = F16(183);
                }

                nDestVertPan[nPlayer] = nPanVal;
            }

            playerX -= sprite[nPlayerSprite].x;
            playerY -= sprite[nPlayerSprite].y;

            uint32_t sqrtNum = playerX * playerX + playerY * playerY;

            if (sqrtNum > INT_MAX)
            {
                OSD_Printf("%s %d: overflow\n", EDUKE32_FUNCTION, __LINE__);
                sqrtNum = INT_MAX;
            }

            totalvel[nPlayer] = ksqrt(sqrtNum);

            int nViewSect = sprite[nPlayerSprite].sectnum;

            int EyeZ = eyelevel[nPlayer] + sprite[nPlayerSprite].z + nQuake[nPlayer];

            while (1)
            {
                int nCeilZ = sector[nViewSect].ceilingz;

                if (EyeZ >= nCeilZ)
                    break;

                if (SectAbove[nViewSect] <= -1)
                    break;

                nViewSect = SectAbove[nViewSect];
            }

            // Do underwater sector check
            if (bUnderwater)
            {
                if (nViewSect != sprite[nPlayerSprite].sectnum)
                {
                    if ((nMove & 0xC000) == 0x8000)
                    {
                        int var_C4 = sprite[nPlayerSprite].x;
                        int var_D4 = sprite[nPlayerSprite].y;
                        int var_C8 = sprite[nPlayerSprite].z;

                        mychangespritesect(nPlayerSprite, nViewSect);

                        sprite[nPlayerSprite].x = spr_x;
                        sprite[nPlayerSprite].y = spr_y;

                        int var_FC = sector[nViewSect].floorz + (-5120);

                        sprite[nPlayerSprite].z = var_FC;

                        if ((movesprite(nPlayerSprite, x, y, 0, 5120, 0, CLIPMASK0) & 0xC000) == 0x8000)
                        {
                            mychangespritesect(nPlayerSprite, sprite[nPlayerSprite].sectnum);

                            sprite[nPlayerSprite].x = var_C4;
                            sprite[nPlayerSprite].y = var_D4;
                            sprite[nPlayerSprite].z = var_C8;
                        }
                        else
                        {
                            sprite[nPlayerSprite].z = var_FC - 256;
                            D3PlayFX(StaticSound[kSoundJonWade], nPlayerSprite);
                        }
                    }
                }
            }

            // loc_1ADAF
            nPlayerViewSect[nPlayer] = nViewSect;

            nPlayerDX[nPlayer] = sprite[nPlayerSprite].x - spr_x;
            nPlayerDY[nPlayer] = sprite[nPlayerSprite].y - spr_y;

            int var_5C = SectFlag[nViewSect] & kSectUnderwater;

            uint16_t buttons = sPlayerInput[nPlayer].buttons;

            if (buttons & kButtonCheatGodMode) // LOBODEITY cheat
            {
                char strDeity[96]; // TODO - reduce in size?

                const char *strDMode = NULL;

                if (PlayerList[nPlayer].invincibility >= 0)
                {
                    PlayerList[nPlayer].invincibility = -1;
                    strDMode = "ON";
                }
                else
                {
                    PlayerList[nPlayer].invincibility = 0;
                    strDMode = "OFF";
                }

                sPlayerInput[nPlayer].buttons &= 0xBF;

                sprintf(strDeity, "Deity mode %s for player", strDMode);
                StatusMessage(150, strDeity);
            }
            else if (buttons & kButtonCheatGuns) // LOBOCOP cheat
            {
                FillWeapons(nPlayer);
                StatusMessage(150, "All weapons loaded for player");
            }
            else if (buttons & kButtonCheatKeys) // LOBOPICK cheat
            {
                PlayerList[nPlayer].keys = 0xFFFF;
                StatusMessage(150, "All keys loaded for player");
                RefreshStatus();
            }
            else if (buttons & kButtonCheatItems) // LOBOSWAG cheat
            {
                FillItems(nPlayer);
                StatusMessage(150, "All items loaded for player");
            }

            // loc_1AEF5:
            if (PlayerList[nPlayer].nHealth > 0)
            {
                if (PlayerList[nPlayer].nMaskAmount > 0)
                {
                    PlayerList[nPlayer].nMaskAmount--;
                    if (PlayerList[nPlayer].nMaskAmount == 150 && nPlayer == nLocalPlayer) {
                        PlayAlert("MASK IS ABOUT TO EXPIRE");
                    }
                }

                if (!PlayerList[nPlayer].invincibility)
                {
                    // Handle air
                    nBreathTimer[nPlayer]--;

                    if (nBreathTimer[nPlayer] <= 0)
                    {
                        nBreathTimer[nPlayer] = 90;

                        // if underwater
                        if (var_5C)
                        {
                            airpages = 1;
                            if (PlayerList[nPlayer].nMaskAmount > 0)
                            {
                                if (nPlayer == nLocalPlayer) {
                                    BuildStatusAnim(132, 0);
                                }

                                D3PlayFX(StaticSound[kSoundJonScuba], nPlayerSprite);

                                PlayerList[nPlayer].nAir = 100;
                            }
                            else
                            {
                                PlayerList[nPlayer].nAir -= 25;
                                if (PlayerList[nPlayer].nAir > 0)
                                {
                                    D3PlayFX(StaticSound[kSoundBubbleHigh], nPlayerSprite);
                                }
                                else
                                {
                                    PlayerList[nPlayer].nHealth += (PlayerList[nPlayer].nAir << 2);
                                    if (PlayerList[nPlayer].nHealth <= 0)
                                    {
                                        PlayerList[nPlayer].nHealth = 0;
                                        StartDeathSeq(nPlayer, 0);
                                    }

                                    if (nPlayer == nLocalPlayer)
                                    {
                                        SetHealthFrame(-1);
                                    }

                                    PlayerList[nPlayer].nAir = 0;

                                    if (PlayerList[nPlayer].nHealth < 300)
                                    {
                                        D3PlayFX(StaticSound[kSoundJonAir2], nPlayerSprite);
                                    }
                                    else
                                    {
                                        D3PlayFX(StaticSound[kSoundJonAir1], nPlayerSprite);
                                    }
                                }
                            }

                            DoBubbles(nPlayer);
                            SetAirFrame();
                        }
                        else
                        {
                            if (nPlayer == nLocalPlayer)
                            {
                                BuildStatusAnim(132, 0);
                            }

                            airpages = 0;
                        }
                    }
                }

                // loc_1B0B9
                if (var_5C) // if underwater
                {
                    if (nPlayerTorch[nPlayer] > 0)
                    {
                        nPlayerTorch[nPlayer] = 0;
                        SetTorch(nPlayer, 0);
                    }
                }
                else
                {
                    int nTmpSectNum = sprite[nPlayerSprite].sectnum;

                    if (totalvel[nPlayer] > 25 && sprite[nPlayerSprite].z > sector[nTmpSectNum].floorz)
                    {
                        if (SectDepth[nTmpSectNum] && !SectSpeed[nTmpSectNum] && !SectDamage[nTmpSectNum])
                        {
                            D3PlayFX(StaticSound[kSoundJonWade], nPlayerSprite);
                        }
                    }

                    // CHECKME - wrong place?
                    if (nSectFlag & kSectUnderwater)
                    {
                        if (PlayerList[nPlayer].nAir < 50)
                        {
                            D3PlayFX(StaticSound[kSoundJonGasp], nPlayerSprite);
                        }

                        nBreathTimer[nPlayer] = 1;
                    }

                    airpages = 0;

                    nBreathTimer[nPlayer]--;
                    if (nBreathTimer[nPlayer] <= 0)
                    {
                        nBreathTimer[nPlayer] = 90;
                        if (nPlayer == nLocalPlayer)
                        {
                            // animate lungs
                            BuildStatusAnim(132, 0);
                        }
                    }

                    if (PlayerList[nPlayer].nAir < 100)
                    {
                        PlayerList[nPlayer].nAir = 100;
                        SetAirFrame();
                    }
                }

                // loc_1B1EB
                if (nTotalPlayers > 1)
                {
                    int nFloorSprite = nPlayerFloorSprite[nPlayer];

                    sprite[nFloorSprite].x = sprite[nPlayerSprite].x;
                    sprite[nFloorSprite].y = sprite[nPlayerSprite].y;

                    if (sprite[nFloorSprite].sectnum != sprite[nPlayerSprite].sectnum)
                    {
                        mychangespritesect(nFloorSprite, sprite[nPlayerSprite].sectnum);
                    }

                    sprite[nFloorSprite].z = sector[sprite[nPlayerSprite].sectnum].floorz;
                }

                int var_30 = 0;

                if (PlayerList[nPlayer].nHealth >= kMaxHealth)
                {
                    var_30 = 2;
                }

                if (PlayerList[nPlayer].nMagic >= kMaxMagic)
                {
                    var_30 |= 1;
                }

                // code to handle item pickup?
                short nearTagSector, nearTagWall, nearTagSprite;
                int nearHitDist;

                int nPickupSprite;

                // neartag finds the nearest sector, wall, and sprite which has its hitag and/or lotag set to a value.
                neartag(sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].z, sprite[nPlayerSprite].sectnum, sprite[nPlayerSprite].ang,
                    &nearTagSector, &nearTagWall, &nearTagSprite, (int32_t*)&nearHitDist, 1024, 2, NULL);

                feebtag(sprite[nPlayerSprite].x, sprite[nPlayerSprite].y, sprite[nPlayerSprite].z, sprite[nPlayerSprite].sectnum,
                    &nPickupSprite, var_30, 768);

                PlayerCheckItemPickup(nPlayer, nPickupSprite, var_30);

                // CORRECT ? // loc_1BAF9:
                if (bTouchFloor)
                {
                    if (sector[sprite[nPlayerSprite].sectnum].lotag > 0)
                    {
                        runlist_SignalRun(sector[sprite[nPlayerSprite].sectnum].lotag - 1, nPlayer | 0x50000);
                    }
                }

                if (nSector != sprite[nPlayerSprite].sectnum)
                {
                    if (sector[nSector].lotag > 0)
                    {
                        runlist_SignalRun(sector[nSector].lotag - 1, nPlayer | 0x70000);
                    }

                    if (sector[sprite[nPlayerSprite].sectnum].lotag > 0)
                    {
                        runlist_SignalRun(sector[sprite[nPlayerSprite].sectnum].lotag - 1, nPlayer | 0x60000);
                    }
                }

                if (!PlayerList[nPlayer].bIsMummified)
                {
                    if (buttons & kButtonOpen)
                    {
                        ClearSpaceBar(nPlayer);

                        if (nearTagWall >= 0 && wall[nearTagWall].lotag > 0)
                        {
                            runlist_SignalRun(wall[nearTagWall].lotag - 1, nPlayer | 0x40000);
                        }

                        if (nearTagSector >= 0 && sector[nearTagSector].lotag > 0)
                        {
                            runlist_SignalRun(sector[nearTagSector].lotag - 1, nPlayer | 0x40000);
                        }
                    }

                    // was int var_38 = buttons & 0x8
                    if (buttons & kButtonFire)
                    {
                        FireWeapon(nPlayer);
                    }
                    else
                    {
                        StopFiringWeapon(nPlayer);
                    }

                    // loc_1BC57:

                    // CHECKME - are we finished with 'nSector' variable at this point? if so, maybe set it to sprite[nPlayerSprite].sectnum so we can make this code a bit neater. Don't assume sprite[nPlayerSprite].sectnum == nSector here!!
                    if (nStandHeight > (sector[sprite[nPlayerSprite].sectnum].floorz - sector[sprite[nPlayerSprite].sectnum].ceilingz)) {
                        var_48 = 1;
                    }

                    // Jumping
                    if (buttons & kButtonJump)
                    {
                        if (bUnderwater)
                        {
                            sprite[nPlayerSprite].zvel = -2048;
                            nActionB = 10;
                        }
                        else if (bTouchFloor)
                        {
                            if (nAction < 6 || nAction > 8)
                            {
                                sprite[nPlayerSprite].zvel = -3584;
                                nActionB = 3;
                            }
                        }

                        // goto loc_1BE70:
                    }
                    else if (buttons & kButtonCrouch)
                    {
                        if (bUnderwater)
                        {
                            sprite[nPlayerSprite].zvel = 2048;
                            nActionB = 10;
                        }
                        else
                        {
                            if (eyelevel[nPlayer] < -8320) {
                                eyelevel[nPlayer] += ((-8320 - eyelevel[nPlayer]) >> 1);
                            }

loc_1BD2E:
                            if (totalvel[nPlayer] < 1) {
                                nActionB = 6;
                            }
                            else {
                                nActionB = 7;
                            }
                        }

                        // goto loc_1BE70:
                    }
                    else
                    {
                        if (PlayerList[nPlayer].nHealth > 0)
                        {
                            int var_EC = nActionEyeLevel[nAction];
                            eyelevel[nPlayer] += (var_EC - eyelevel[nPlayer]) >> 1;

                            if (bUnderwater)
                            {
                                if (totalvel[nPlayer] <= 1)
                                    nActionB = 9;
                                else
                                    nActionB = 10;
                            }
                            else
                            {
                                // CHECKME - confirm branching in this area is OK
                                if (var_48)
                                {
                                    goto loc_1BD2E;
                                }
                                else
                                {
                                    if (totalvel[nPlayer] <= 1) {
                                        nActionB = 0;//bUnderwater; // this is just setting to 0
                                    }
                                    else if (totalvel[nPlayer] <= 30) {
                                        nActionB = 2;
                                    }
                                    else
                                    {
                                        nActionB = 1;
                                    }
                                }
                            }
                        }
                        // loc_1BE30
                        if (buttons & kButtonFire) // was var_38
                        {
                            if (bUnderwater)
                            {
                                nActionB = 11;
                            }
                            else
                            {
                                if (nActionB != 2 && nActionB != 1)
                                {
                                    nActionB = 5;
                                }
                            }
                        }
                    }

                    // loc_1BE70:
                    // Handle player pressing number keys to change weapon
                    uint8_t var_90 = (buttons >> 13) & 0xF;

                    if (var_90)
                    {
                        var_90--;

                        if (nPlayerWeapons[nPlayer] & (1 << var_90))
                        {
                            SetNewWeapon(nPlayer, var_90);
                        }
                    }
                }
                else // player is mummified
                {
                    if (buttons & kButtonFire)
                    {
                        FireWeapon(nPlayer);
                    }

                    if (nAction != 15)
                    {
                        if (totalvel[nPlayer] <= 1)
                        {
                            nActionB = 13;
                        }
                        else
                        {
                            nActionB = 14;
                        }
                    }
                }

                // loc_1BF09
                if (nActionB != nAction && nAction != 4)
                {
                    nAction = nActionB;
                    PlayerList[nPlayer].nAction = nActionB;
                    PlayerList[nPlayer].nFrame = 0;
                }

                if (nPlayer == nLocalPlayer)
                {
                    // TODO - tidy / consolidate repeating blocks of code here?
                    if (BUTTON(gamefunc_Look_Up))
                    {
                        bLockPan = kFalse;
                        if (nVertPan[nPlayer] < F16(180)) {
                            nVertPan[nPlayer] += F16(4);
                        }

                        bPlayerPan = kTrue;
                        nDestVertPan[nPlayer] = nVertPan[nPlayer];
                    }
                    else if (BUTTON(gamefunc_Look_Down))
                    {
                        bLockPan = kFalse;
                        if (nVertPan[nPlayer] > F16(4)) {
                            nVertPan[nPlayer] -= F16(4);
                        }

                        bPlayerPan = kTrue;
                        nDestVertPan[nPlayer] = nVertPan[nPlayer];
                    }
                    else if (BUTTON(gamefunc_Look_Straight) || bLookCentre)
                    {
                        bLockPan = kFalse;
                        bPlayerPan = kFalse;
                        nVertPan[nPlayer] = F16(92);
                        nDestVertPan[nPlayer] = F16(92);

                        if (bLookCentre) {
                            bLookCentre = false;
                        }
                    }
                    else if (BUTTON(gamefunc_Aim_Up))
                    {
                        bLockPan = kTrue;
                        if (nVertPan[nPlayer] < F16(180)) {
                            nVertPan[nPlayer] += F16(4);
                        }

                        bPlayerPan = kTrue;
                        nDestVertPan[nPlayer] = nVertPan[nPlayer];
                    }
                    else if (BUTTON(gamefunc_Aim_Down))
                    {
                        bLockPan = kTrue;
                        if (nVertPan[nPlayer] > F16(4)) {
                            nVertPan[nPlayer] -= F16(4);
                        }

                        bPlayerPan = kTrue;
                        nDestVertPan[nPlayer] = nVertPan[nPlayer];
                    }

                    // loc_1C048:
                    if (totalvel[nPlayer] > 20) {
                        bPlayerPan = kFalse;
                    }

                    if (aimmode)
                        bLockPan = kTrue;

                    // loc_1C05E
                    fix16_t ecx = nDestVertPan[nPlayer] - nVertPan[nPlayer];

                    if (aimmode)
                    {
                        ecx = 0;
                    }

                    if (ecx)
                    {
                        if (ecx / 4 == 0)
                        {
                            if (ecx >= 0) {
                                ecx = 1;
                            }
                            else
                            {
                                ecx = -1;
                            }
                        }
                        else
                        {
                            ecx /= 4;

                            if (ecx > F16(4))
                            {
                                ecx = F16(4);
                            }
                            else if (ecx < -F16(4))
                            {
                                ecx = -F16(4);
                            }
                        }

                        nVertPan[nPlayer] += ecx;
                    }
                }
            }
            else // else, player's health is less than 0
            {
                // loc_1C0E9
                if (buttons & kButtonOpen)
                {
                    ClearSpaceBar(nPlayer);

                    if (nAction >= 16)
                    {
                        if (nPlayer == nLocalPlayer)
                        {
                            StopAllSounds();
                            StopLocalSound();
                            GrabPalette();
                        }

                        PlayerList[nPlayer].nCurrentWeapon = nPlayerOldWeapon[nPlayer];

                        if (nPlayerLives[nPlayer] && nNetTime)
                        {
                            if (nAction != 20)
                            {
                                sprite[nPlayerSprite].picnum = seq_GetSeqPicnum(kSeqJoe, 120, 0);
                                sprite[nPlayerSprite].cstat = 0;
                                sprite[nPlayerSprite].z = sector[sprite[nPlayerSprite].sectnum].floorz;
                            }

                            // will invalidate nPlayerSprite
                            RestartPlayer(nPlayer);

                            nPlayerSprite = PlayerList[nPlayer].nSprite;
                            nDopple = nDoppleSprite[nPlayer];
                        }
                        else
                        {
                            if (CDplaying()) {
                                fadecdaudio();
                            }

                            if (levelnum == 20) {
                                DoFailedFinalScene();
                            }
                            else {
                                DoGameOverScene();
                            }

                            levelnew = 100;
                        }
                    }
                }
            }

            // loc_1C201:
            if (nLocalPlayer == nPlayer)
            {
                nLocalEyeSect = nPlayerViewSect[nLocalPlayer];
                CheckAmbience(nLocalEyeSect);
            }

            int var_AC = SeqOffsets[PlayerList[nPlayer].nSeq] + ActionSeq[nAction].a;

            seq_MoveSequence(nPlayerSprite, var_AC, PlayerList[nPlayer].nFrame);
            PlayerList[nPlayer].nFrame++;

            if (PlayerList[nPlayer].nFrame >= SeqSize[var_AC])
            {
                PlayerList[nPlayer].nFrame = 0;

                switch (PlayerList[nPlayer].nAction)
                {
                    default:
                        break;

                    case 3:
                        PlayerList[nPlayer].nFrame = SeqSize[var_AC] - 1;
                        break;
                    case 4:
                        PlayerList[nPlayer].nAction = 0;
                        break;
                    case 16:
                        PlayerList[nPlayer].nFrame = SeqSize[var_AC] - 1;

                        if (sprite[nPlayerSprite].z < sector[sprite[nPlayerSprite].sectnum].floorz) {
                            sprite[nPlayerSprite].z += 256;
                        }

                        if (!RandomSize(5))
                        {
                            int mouthX, mouthY, mouthZ;
                            short mouthSect;
                            WheresMyMouth(nPlayer, &mouthX, &mouthY, &mouthZ, &mouthSect);

                            BuildAnim(-1, 71, 0, mouthX, mouthY, sprite[nPlayerSprite].z + 3840, mouthSect, 75, 128);
                        }
                        break;
                    case 17:
                        PlayerList[nPlayer].nAction = 18;
                        break;
                    case 19:
                        sprite[nPlayerSprite].cstat |= 0x8000;
                        PlayerList[nPlayer].nAction = 20;
                        break;
                }
            }

            // loc_1C3B4:
            if (nPlayer == nLocalPlayer)
            {
                initx = sprite[nPlayerSprite].x;
                inity = sprite[nPlayerSprite].y;
                initz = sprite[nPlayerSprite].z;
                initsect = sprite[nPlayerSprite].sectnum;
                inita = sprite[nPlayerSprite].ang;
            }

            if (!PlayerList[nPlayer].nHealth)
            {
                nYDamage[nPlayer] = 0;
                nXDamage[nPlayer] = 0;

                if (eyelevel[nPlayer] >= -2816)
                {
                    eyelevel[nPlayer] = -2816;
                    dVertPan[nPlayer] = 0;
                }
                else
                {
                    if (nVertPan[nPlayer] < F16(92))
                    {
                        nVertPan[nPlayer] = F16(91);
                        eyelevel[nPlayer] -= (dVertPan[nPlayer] << 8);
                    }
                    else
                    {
                        nVertPan[nPlayer] += fix16_from_int(dVertPan[nPlayer]);
                        if (nVertPan[nPlayer] >= F16(200))
                        {
                            nVertPan[nPlayer] = F16(199);
                        }
                        else if (nVertPan[nPlayer] <= F16(92))
                        {
                            if (!(SectFlag[sprite[nPlayerSprite].sectnum] & kSectUnderwater))
                            {
                                SetNewWeapon(nPlayer, nDeathType[nPlayer] + 8);
                            }
                        }

                        dVertPan[nPlayer]--;
                    }
                }
            }

            // loc_1C4E1
            sprite[nDopple].x = sprite[nPlayerSprite].x;
            sprite[nDopple].y = sprite[nPlayerSprite].y;
            sprite[nDopple].z = sprite[nPlayerSprite].z;

            if (SectAbove[sprite[nPlayerSprite].sectnum] > -1)
            {
                sprite[nDopple].ang = sprite[nPlayerSprite].ang;
                mychangespritesect(nDopple, SectAbove[sprite[nPlayerSprite].sectnum]);
                sprite[nDopple].cstat = 0x101;
            }
            else
            {
                sprite[nDopple].cstat = 0x8000;
            }

            MoveWeapons(nPlayer);
            return;
        }
    }
}

class PlayerLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void PlayerLoadSave::Load()
{
    Read(&nBreathTimer[nLocalPlayer], sizeof(nBreathTimer[nLocalPlayer]));
    Read(&nPlayerSwear[nLocalPlayer], sizeof(nPlayerSwear[nLocalPlayer]));
    Read(&nPlayerPushSect[nLocalPlayer], sizeof(nPlayerPushSect[nLocalPlayer]));
    Read(&nDeathType[nLocalPlayer], sizeof(nDeathType[nLocalPlayer]));
    Read(&nPlayerScore[nLocalPlayer], sizeof(nPlayerScore[nLocalPlayer]));
    Read(&nPlayerColor[nLocalPlayer], sizeof(nPlayerColor[nLocalPlayer]));
    Read(&nPlayerDY[nLocalPlayer], sizeof(nPlayerDY[nLocalPlayer]));
    Read(&nPlayerDX[nLocalPlayer], sizeof(nPlayerDX[nLocalPlayer]));
    Read(&playerNames[nLocalPlayer], sizeof(playerNames[nLocalPlayer]));
    Read(&nPistolClip[nLocalPlayer], sizeof(nPistolClip[nLocalPlayer]));
    Read(&nXDamage[nLocalPlayer], sizeof(nXDamage[nLocalPlayer]));
    Read(&nYDamage[nLocalPlayer], sizeof(nYDamage[nLocalPlayer]));
    Read(&nDoppleSprite[nLocalPlayer], sizeof(nDoppleSprite[nLocalPlayer]));
    Read(&namelen[nLocalPlayer], sizeof(namelen[nLocalPlayer]));
    Read(&nPlayerOldWeapon[nLocalPlayer], sizeof(nPlayerOldWeapon[nLocalPlayer]));
    Read(&nPlayerClip[nLocalPlayer], sizeof(nPlayerClip[nLocalPlayer]));
    Read(&nPlayerPushSound[nLocalPlayer], sizeof(nPlayerPushSound[nLocalPlayer]));
    Read(&nTauntTimer[nLocalPlayer], sizeof(nTauntTimer[nLocalPlayer]));
    Read(&nPlayerTorch[nLocalPlayer], sizeof(nPlayerTorch[nLocalPlayer]));
    Read(&nPlayerWeapons[nLocalPlayer], sizeof(nPlayerWeapons[nLocalPlayer]));
    Read(&nPlayerLives[nLocalPlayer], sizeof(nPlayerLives[nLocalPlayer]));
    Read(&nPlayerItem[nLocalPlayer], sizeof(nPlayerItem[nLocalPlayer]));
    Read(&PlayerList[nLocalPlayer], sizeof(PlayerList[nLocalPlayer]));
    Read(&nPlayerInvisible[nLocalPlayer], sizeof(nPlayerInvisible[nLocalPlayer]));
    Read(&nPlayerDouble[nLocalPlayer], sizeof(nPlayerDouble[nLocalPlayer]));
    Read(&nPlayerViewSect[nLocalPlayer], sizeof(nPlayerViewSect[nLocalPlayer]));
    Read(&nPlayerFloorSprite[nLocalPlayer], sizeof(nPlayerFloorSprite[nLocalPlayer]));
    Read(&sPlayerSave[nLocalPlayer], sizeof(nPlayerFloorSprite[nLocalPlayer]));
    Read(&totalvel[nLocalPlayer], sizeof(totalvel[nLocalPlayer]));
    Read(&eyelevel[nLocalPlayer], sizeof(eyelevel[nLocalPlayer]));
    Read(&oeyelevel[nLocalPlayer], sizeof(oeyelevel[nLocalPlayer]));
    Read(&nNetStartSprite[nLocalPlayer], sizeof(nNetStartSprite[nLocalPlayer]));

    Read(&nStandHeight, sizeof(nStandHeight));

    Read(&nPlayerGrenade[nLocalPlayer], sizeof(nPlayerGrenade[nLocalPlayer]));
    Read(nGrenadePlayer, sizeof(nGrenadePlayer));

    Read(word_D282A, sizeof(word_D282A));

    Read(&PlayerCount, sizeof(PlayerCount));

    Read(&nNetStartSprites, sizeof(nNetStartSprites));
    Read(&nCurStartSprite, sizeof(nCurStartSprite));

    Read(&nLocalSpr, sizeof(nLocalSpr));
}

void PlayerLoadSave::Save()
{
    Write(&nBreathTimer[nLocalPlayer], sizeof(nBreathTimer[nLocalPlayer]));
    Write(&nPlayerSwear[nLocalPlayer], sizeof(nPlayerSwear[nLocalPlayer]));
    Write(&nPlayerPushSect[nLocalPlayer], sizeof(nPlayerPushSect[nLocalPlayer]));
    Write(&nDeathType[nLocalPlayer], sizeof(nDeathType[nLocalPlayer]));
    Write(&nPlayerScore[nLocalPlayer], sizeof(nPlayerScore[nLocalPlayer]));
    Write(&nPlayerColor[nLocalPlayer], sizeof(nPlayerColor[nLocalPlayer]));
    Write(&nPlayerDY[nLocalPlayer], sizeof(nPlayerDY[nLocalPlayer]));
    Write(&nPlayerDX[nLocalPlayer], sizeof(nPlayerDX[nLocalPlayer]));
    Write(&playerNames[nLocalPlayer], sizeof(playerNames[nLocalPlayer]));
    Write(&nPistolClip[nLocalPlayer], sizeof(nPistolClip[nLocalPlayer]));
    Write(&nXDamage[nLocalPlayer], sizeof(nXDamage[nLocalPlayer]));
    Write(&nYDamage[nLocalPlayer], sizeof(nYDamage[nLocalPlayer]));
    Write(&nDoppleSprite[nLocalPlayer], sizeof(nDoppleSprite[nLocalPlayer]));
    Write(&namelen[nLocalPlayer], sizeof(namelen[nLocalPlayer]));
    Write(&nPlayerOldWeapon[nLocalPlayer], sizeof(nPlayerOldWeapon[nLocalPlayer]));
    Write(&nPlayerClip[nLocalPlayer], sizeof(nPlayerClip[nLocalPlayer]));
    Write(&nPlayerPushSound[nLocalPlayer], sizeof(nPlayerPushSound[nLocalPlayer]));
    Write(&nTauntTimer[nLocalPlayer], sizeof(nTauntTimer[nLocalPlayer]));
    Write(&nPlayerTorch[nLocalPlayer], sizeof(nPlayerTorch[nLocalPlayer]));
    Write(&nPlayerWeapons[nLocalPlayer], sizeof(nPlayerWeapons[nLocalPlayer]));
    Write(&nPlayerLives[nLocalPlayer], sizeof(nPlayerLives[nLocalPlayer]));
    Write(&nPlayerItem[nLocalPlayer], sizeof(nPlayerItem[nLocalPlayer]));
    Write(&PlayerList[nLocalPlayer], sizeof(PlayerList[nLocalPlayer]));
    Write(&nPlayerInvisible[nLocalPlayer], sizeof(nPlayerInvisible[nLocalPlayer]));
    Write(&nPlayerDouble[nLocalPlayer], sizeof(nPlayerDouble[nLocalPlayer]));
    Write(&nPlayerViewSect[nLocalPlayer], sizeof(nPlayerViewSect[nLocalPlayer]));
    Write(&nPlayerFloorSprite[nLocalPlayer], sizeof(nPlayerFloorSprite[nLocalPlayer]));
    Write(&sPlayerSave[nLocalPlayer], sizeof(nPlayerFloorSprite[nLocalPlayer]));
    Write(&totalvel[nLocalPlayer], sizeof(totalvel[nLocalPlayer]));
    Write(&eyelevel[nLocalPlayer], sizeof(eyelevel[nLocalPlayer]));
    Write(&oeyelevel[nLocalPlayer], sizeof(oeyelevel[nLocalPlayer]));
    Write(&nNetStartSprite[nLocalPlayer], sizeof(nNetStartSprite[nLocalPlayer]));

    Write(&nStandHeight, sizeof(nStandHeight));

    Write(&nPlayerGrenade[nLocalPlayer], sizeof(nPlayerGrenade[nLocalPlayer]));
    Write(nGrenadePlayer, sizeof(nGrenadePlayer));

    Write(word_D282A, sizeof(word_D282A));

    Write(&PlayerCount, sizeof(PlayerCount));

    Write(&nNetStartSprites, sizeof(nNetStartSprites));
    Write(&nCurStartSprite, sizeof(nCurStartSprite));

    Write(&nLocalSpr, sizeof(nLocalSpr));
}

static PlayerLoadSave* myLoadSave;

void PlayerLoadSaveConstruct()
{
    myLoadSave = new PlayerLoadSave();
}

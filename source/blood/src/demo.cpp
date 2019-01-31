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
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "common_game.h"
#include "keyboard.h"
#include "control.h"
#include "osd.h"
#include "mmulti.h"
#ifdef WITHKPLIB
#include "kplib.h"
#endif

#include "blood.h"
#include "controls.h"
#include "demo.h"
#include "fire.h"
#include "gamemenu.h"
#include "globals.h"
#include "menu.h"
#include "messages.h"
#include "misc.h"
#include "network.h"
#include "player.h"
#include "screen.h"
#include "view.h"

int nBuild = 0;

void ReadGameOptionsLegacy(GAMEOPTIONS &gameOptions, GAMEOPTIONSLEGACY &gameOptionsLegacy)
{
    gameOptions.nGameType = gameOptionsLegacy.nGameType;
    gameOptions.nDifficulty = gameOptionsLegacy.nDifficulty;
    gameOptions.nEpisode = gameOptionsLegacy.nEpisode;
    gameOptions.nLevel = gameOptionsLegacy.nLevel;
    strcpy(gameOptions.zLevelName, gameOptionsLegacy.zLevelName);
    strcpy(gameOptions.zLevelSong, gameOptionsLegacy.zLevelSong);
    gameOptions.nTrackNumber = gameOptionsLegacy.nTrackNumber;
    strcpy(gameOptions.szSaveGameName, gameOptionsLegacy.szSaveGameName);
    strcpy(gameOptions.szUserGameName, gameOptionsLegacy.szUserGameName);
    gameOptions.nSaveGameSlot = gameOptionsLegacy.nSaveGameSlot;
    gameOptions.picEntry = gameOptionsLegacy.picEntry;
    gameOptions.uMapCRC = gameOptionsLegacy.uMapCRC;
    gameOptions.nMonsterSettings = gameOptionsLegacy.nMonsterSettings;
    gameOptions.uGameFlags = gameOptionsLegacy.uGameFlags;
    gameOptions.uNetGameFlags = gameOptionsLegacy.uNetGameFlags;
    gameOptions.nWeaponSettings = gameOptionsLegacy.nWeaponSettings;
    gameOptions.nItemSettings = gameOptionsLegacy.nItemSettings;
    gameOptions.nRespawnSettings = gameOptionsLegacy.nRespawnSettings;
    gameOptions.nTeamSettings = gameOptionsLegacy.nTeamSettings;
    gameOptions.nMonsterRespawnTime = gameOptionsLegacy.nMonsterRespawnTime;
    gameOptions.nWeaponRespawnTime = gameOptionsLegacy.nWeaponRespawnTime;
    gameOptions.nItemRespawnTime = gameOptionsLegacy.nItemRespawnTime;
    gameOptions.nSpecialRespawnTime = gameOptionsLegacy.nSpecialRespawnTime;
}

CDemo gDemo;

CDemo::CDemo()
{
    nBuild = 4;
    at0 = 0;
    at1 = 0;
    at3 = 0;
    at7 = NULL;
    atb = 0;
    at59ef = 0;
    at59eb = 0;
    at2 = 0;
    memset(&atf, 0, sizeof(atf));
}

CDemo::~CDemo()
{
    at0 = 0;
    at1 = 0;
    at3 = 0;
    atb = 0;
    memset(&atf, 0, sizeof(atf));
    if (at7 != NULL)
    {
        fclose(at7);
        at7 = NULL;
    }
}

bool CDemo::Create(const char *pzFile)
{
    ThrowError("Demo recording is broken :P");
    char buffer[13] = "";
    char vc = 0;
    if (at0 || at1)
        ThrowError("CDemo::Create called during demo record/playback process.");
    if (!pzFile)
    {
        for (int i = 0; i < 8 && !vc; i++)
        {
            sprintf(buffer, "BLOOD0%02d.DEM", i);
            if (!access(buffer, F_OK))
                vc = 1;
        }
        if (vc == 1)
        {
            at7 = fopen(buffer, "wb");
            if (at7 == NULL)
                return false;
        }
    }
    else
    {
        at7 = fopen(pzFile, "wb");
        if (at7 == NULL)
            return false;
    }
    at0 = 1;
    atb = 0;
    return true;
}

void CDemo::Write(GINPUT *pPlayerInputs)
{
    dassert(pPlayerInputs != NULL);
    if (!at0)
        return;
    if (atb == 0)
    {
        atf.signature = '\x1aMED';
        atf.nVersion = BloodVersion;
        atf.nBuild = nBuild;
        atf.nInputCount = 0;
        atf.nNetPlayers = gNetPlayers;
        atf.nMyConnectIndex = myconnectindex;
        atf.nConnectHead = connecthead;
        memcpy(atf.connectPoints, connectpoint2, sizeof(atf.connectPoints));
        memcpy(&atf.gameOptions, &gGameOptions, sizeof(gGameOptions));
        fwrite(&atf, sizeof(DEMOHEADER), 1, at7);
    }
    for (int p = connecthead; p >= 0; p = connectpoint2[p])
    {
        memcpy(&at1aa[atb&1023], &pPlayerInputs[p], sizeof(GINPUT));
        atb++;
        if((atb&(kInputBufferSize-1))==0)
            FlushInput(kInputBufferSize);
    }
}

void CDemo::Close(void)
{
    if (at0)
    {
        if (atb&(kInputBufferSize-1))
            FlushInput(atb&(kInputBufferSize-1));
        atf.nInputCount = atb;
        fseek(at7, 0, SEEK_SET);
        fwrite(&atf, sizeof(DEMOHEADER), 1, at7);
    }
    if (at7 != NULL)
    {
        fclose(at7);
        at7 = NULL;
    }
    at0 = 0;
    at1 = 0;
}

bool CDemo::SetupPlayback(const char *pzFile)
{
    at0 = 0;
    at1 = 0;
    if (pzFile)
    {
        at7 = fopen(pzFile, "rb");
        if (at7 == NULL)
            return false;
    }
    else
    {
        at7 = fopen(at59aa[at59eb], "rb");
        if (at7 == NULL)
            return false;
    }
    fread(&atf, sizeof(DEMOHEADER), 1, at7);
    if (atf.signature != '\x1aMED')
        return 0;
    if (BloodVersion != atf.nVersion && BloodVersion == 0x10b && atf.nVersion != 0x10a)
        return 0;
    if (nBuild != atf.nBuild)
        return 0;
    at0 = 0;
    at1 = 1;
    return 1;
}

void CDemo::ProcessKeys(void)
{
    switch (gInputMode)
    {
    case INPUT_MODE_1:
        gGameMenuMgr.Process();
        break;
    case INPUT_MODE_2:
        gPlayerMsg.ProcessKeys();
        break;
    case INPUT_MODE_0:
    {
        char nKey;
        while ((nKey = keyGetScan()) != 0)
        {
	        char alt = keystatus[0x38] | keystatus[0xb8];
	        char ctrl = keystatus[0x1d] | keystatus[0x9d];
            switch (nKey)
            {
            case 1:
                if (!CGameMenuMgr::m_bActive)
                {
                    gGameMenuMgr.Push(&menuMain, -1);
                    at2 = 1;
                }
                break;
            case 0x58:
                gViewIndex = connectpoint2[gViewIndex];
                if (gViewIndex == -1)
                    gViewIndex = connecthead;
                gView = &gPlayer[gViewIndex];
                break;
            }
        }
        break;
    }
    }
}

void CDemo::Playback(void)
{
    CONTROL_BindsEnabled = false;
    ready2send = 0;
    int v4 = 0;
    if (!CGameMenuMgr::m_bActive)
    {
        gGameMenuMgr.Push(&menuMain, -1);
        at2 = 1;
    }
    gNetFifoClock = gGameClock;
    gViewMode = 3;
_DEMOPLAYBACK:
    while (at1 && !gQuitGame)
    {
        while (gGameClock >= gNetFifoClock && !gQuitGame)
        {
            if (!v4)
            {
                viewResizeView(gViewSize);
                viewSetMessage("");
                gNetPlayers = atf.nNetPlayers;
                atb = atf.nInputCount;
                myconnectindex = atf.nMyConnectIndex;
                connecthead = atf.nConnectHead;
                for (int i = 0; i < 8; i++)
                    connectpoint2[i] = atf.connectPoints[i];
                //memcpy(connectpoint2, atf.connectPoints, sizeof(atf.connectPoints));
                GAMEOPTIONSLEGACY gGameOptionsLegacy;
                memcpy(&gGameOptionsLegacy, &atf.gameOptions, sizeof(GAMEOPTIONSLEGACY));
                gSkill = gGameOptions.nDifficulty;
                for (int i = 0; i < 8; i++)
                    playerInit(i, 0);
                StartLevel(&gGameOptions);
                for (int i = 0; i < 8; i++)
                {
                    gProfile[i].nAutoAim = 1;
                    gProfile[i].nWeaponSwitch = 1;
                }
            }
            ready2send = 0;
            OSD_DispatchQueued();
            ProcessKeys();
            for (int p = connecthead; p >= 0; p = connectpoint2[p])
            {
                if ((v4&1023) == 0)
                {
                    unsigned int nSize = atb-v4;
                    if (nSize > kInputBufferSize)
                        nSize = kInputBufferSize;
                    ReadInput(nSize);
                }
                memcpy(&gFifoInput[gNetFifoHead[p]&255], &at1aa[v4&1023], sizeof(GINPUT));
                gNetFifoHead[p]++;
                v4++;
                if (v4 >= atf.nInputCount)
                {
                    ready2send = 0;
                    if (at59ef != 1)
                    {
                        v4 = 0;
                        Close();
                        NextDemo();
                        gNetFifoClock = gGameClock;
                        goto _DEMOPLAYBACK;
                    }
                    else
                    {
                        fseek(at7, sizeof(DEMOHEADER), SEEK_SET);
                        v4 = 0;
                    }
                }
            }
            gNetFifoClock += 4;
            if (!gQuitGame)
                ProcessFrame();
            ready2send = 0;
        }
        if (viewFPSLimit())
        {
            viewDrawScreen();
            if (gInputMode == INPUT_MODE_1 && CGameMenuMgr::m_bActive)
                gGameMenuMgr.Draw();
        }
        if (TestBitString(gotpic, 2342))
        {
            FireProcess();
            ClearBitString(gotpic, 2342);
        }
    }
    Close();
}

void CDemo::StopPlayback(void)
{
    at1 = 0;
}

void CDemo::LoadDemoInfo(void)
{
    at59ef = 0;
    BDIR *dirr;
    struct Bdirent *dirent;
    dirr = Bopendir("./");
    if (dirr)
    {
        while (dirent = Breaddir(dirr))
        {
            if (!Bwildmatch(dirent->name, "BLOOD*.DEM"))
                continue;
            FILE *pFile = fopen(dirent->name, "rb");
            if (!pFile)
                ThrowError("File error #%d loading demo file header.", errno);
            fread(&atf, 1, sizeof(atf), pFile);
            fclose(pFile);
            if (atf.signature == '\x1aMED' && (atf.nVersion == BloodVersion || BloodVersion != 0x10b || atf.nVersion == 0x10a) && nBuild == atf.nBuild)
            {
                strcpy(at59aa[at59ef], dirent->name);
                at59ef++;
            }
        }
        Bclosedir(dirr);
    }
    // PORT-TODO:
#if 0
    struct find_t find;
    int status = _dos_findfirst("BLOOD*.DEM", &find);
    while (!status && at59ef < 5)
    {
        int hFile2 = open(find.name, 0x200);
        if (hFile2 == -1)
            ThrowError("File error #%d loading demo file header.", errno);
        read(hFile2, &atf, sizeof(atf));
        close(hFile2);
        if (atf.signature == '\x1aMED' && (atf.nVersion == BloodVersion || BloodVersion != 0x10b || atf.nVersion == 0x10a) && nBuild == atf.nBuild)
        {
            strcpy(at59aa[at59ef], find.name);
            at59ef++;
        }
        status = _dos_findnext(&find);
    }
#endif
}

void CDemo::NextDemo(void)
{
    at59eb++;
    if (at59eb >= at59ef)
        at59eb = 0;
    SetupPlayback(NULL);
}

const int nInputSize = 22;

void CDemo::FlushInput(int nCount)
{
    // TODO: Fix q16turn & q16mlook
    char pBuffer[nInputSize*kInputBufferSize];
    BitWriter bitWriter(pBuffer, sizeof(pBuffer));
    for (int i = 0; i < nCount; i++)
    {
        GINPUT *pInput = &at1aa[i];
        bitWriter.writeBit(pInput->syncFlags.buttonChange);
        bitWriter.writeBit(pInput->syncFlags.keyChange);
        bitWriter.writeBit(pInput->syncFlags.useChange);
        bitWriter.writeBit(pInput->syncFlags.weaponChange);
        bitWriter.writeBit(pInput->syncFlags.mlookChange);
        bitWriter.writeBit(pInput->syncFlags.run);
        bitWriter.skipBits(26);
        bitWriter.write(pInput->forward>>8, 8);
        bitWriter.write(fix16_to_int(pInput->q16turn<<2), 16);
        bitWriter.write(pInput->strafe>>8, 8);
        bitWriter.writeBit(pInput->buttonFlags.jump);
        bitWriter.writeBit(pInput->buttonFlags.crouch);
        bitWriter.writeBit(pInput->buttonFlags.shoot);
        bitWriter.writeBit(pInput->buttonFlags.shoot2);
        bitWriter.writeBit(pInput->buttonFlags.lookUp);
        bitWriter.writeBit(pInput->buttonFlags.lookDown);
        bitWriter.skipBits(26);
        bitWriter.writeBit(pInput->keyFlags.action);
        bitWriter.writeBit(pInput->keyFlags.jab);
        bitWriter.writeBit(pInput->keyFlags.prevItem);
        bitWriter.writeBit(pInput->keyFlags.nextItem);
        bitWriter.writeBit(pInput->keyFlags.useItem);
        bitWriter.writeBit(pInput->keyFlags.prevWeapon);
        bitWriter.writeBit(pInput->keyFlags.nextWeapon);
        bitWriter.writeBit(pInput->keyFlags.holsterWeapon);
        bitWriter.writeBit(pInput->keyFlags.lookCenter);
        bitWriter.writeBit(pInput->keyFlags.lookLeft);
        bitWriter.writeBit(pInput->keyFlags.lookRight);
        bitWriter.writeBit(pInput->keyFlags.spin180);
        bitWriter.writeBit(pInput->keyFlags.pause);
        bitWriter.writeBit(pInput->keyFlags.quit);
        bitWriter.writeBit(pInput->keyFlags.restart);
        bitWriter.skipBits(17);
        bitWriter.writeBit(pInput->useFlags.useBeastVision);
        bitWriter.writeBit(pInput->useFlags.useCrystalBall);
        bitWriter.writeBit(pInput->useFlags.useJumpBoots);
        bitWriter.writeBit(pInput->useFlags.useMedKit);
        bitWriter.skipBits(28);
        bitWriter.write(pInput->newWeapon, 8);
        bitWriter.write(fix16_to_int(pInput->q16turn<<2), 8);
    }
    fwrite(pBuffer, 1, nInputSize*nCount, at7);
}

void CDemo::ReadInput(int nCount)
{
    char pBuffer[nInputSize*kInputBufferSize];
    fread(pBuffer, 1, nInputSize*nCount, at7);
    BitReader bitReader(pBuffer, sizeof(pBuffer));
    memset(at1aa, 0, nCount*sizeof(GINPUT));
    for (int i = 0; i < nCount; i++)
    {
        GINPUT *pInput = &at1aa[i];
        pInput->syncFlags.buttonChange = bitReader.readBit();
        pInput->syncFlags.keyChange = bitReader.readBit();
        pInput->syncFlags.useChange = bitReader.readBit();
        pInput->syncFlags.weaponChange = bitReader.readBit();
        pInput->syncFlags.mlookChange = bitReader.readBit();
        pInput->syncFlags.run = bitReader.readBit();
        bitReader.skipBits(26);
        pInput->forward = bitReader.readSigned(8)<<8;
        pInput->q16turn = fix16_from_int(bitReader.readSigned(16)>>2);
        pInput->strafe = bitReader.readSigned(8)<<8;
        pInput->buttonFlags.jump = bitReader.readBit();
        pInput->buttonFlags.crouch = bitReader.readBit();
        pInput->buttonFlags.shoot = bitReader.readBit();
        pInput->buttonFlags.shoot2 = bitReader.readBit();
        pInput->buttonFlags.lookUp = bitReader.readBit();
        pInput->buttonFlags.lookDown = bitReader.readBit();
        bitReader.skipBits(26);
        pInput->keyFlags.action = bitReader.readBit();
        pInput->keyFlags.jab = bitReader.readBit();
        pInput->keyFlags.prevItem = bitReader.readBit();
        pInput->keyFlags.nextItem = bitReader.readBit();
        pInput->keyFlags.useItem = bitReader.readBit();
        pInput->keyFlags.prevWeapon = bitReader.readBit();
        pInput->keyFlags.nextWeapon = bitReader.readBit();
        pInput->keyFlags.holsterWeapon = bitReader.readBit();
        pInput->keyFlags.lookCenter = bitReader.readBit();
        pInput->keyFlags.lookLeft = bitReader.readBit();
        pInput->keyFlags.lookRight = bitReader.readBit();
        pInput->keyFlags.spin180 = bitReader.readBit();
        pInput->keyFlags.pause = bitReader.readBit();
        pInput->keyFlags.quit = bitReader.readBit();
        pInput->keyFlags.restart = bitReader.readBit();
        bitReader.skipBits(17);
        pInput->useFlags.useBeastVision = bitReader.readBit();
        pInput->useFlags.useCrystalBall = bitReader.readBit();
        pInput->useFlags.useJumpBoots = bitReader.readBit();
        pInput->useFlags.useMedKit = bitReader.readBit();
        bitReader.skipBits(28);
        pInput->newWeapon = bitReader.readUnsigned(8);
        int mlook = bitReader.readSigned(8);
        pInput->q16mlook = fix16_from_int(mlook / 4);
    }
}

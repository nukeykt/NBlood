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
#include "compat.h"
#include "mmulti.h"
#include "common_game.h"
#include "blood.h"
#include "demo.h"
#include "config.h"
#include "gamemenu.h"
#include "globals.h"
#include "loadsave.h"
#include "menu.h"
#include "messages.h"
#include "network.h"
#include "screen.h"
#include "sound.h"
#include "view.h"

void SaveGame(CGameMenuItemZEditBitmap *, CGameMenuEvent *);

void SaveGameProcess(CGameMenuItemChain *);
void SetDifficultyAndStart(CGameMenuItemChain *);
void SetDetail(CGameMenuItemSlider *);
void SetGamma(CGameMenuItemSlider *);
void SetMusicVol(CGameMenuItemSlider *);
void SetSoundVol(CGameMenuItemSlider *);
void SetCDVol(CGameMenuItemSlider *);
void SetDoppler(CGameMenuItemZBool *);
void SetCrosshair(CGameMenuItemZBool *);
void SetShowWeapons(CGameMenuItemZBool *);
void SetSlopeTilting(CGameMenuItemZBool *);
void SetViewBobbing(CGameMenuItemZBool *);
void SetViewSwaying(CGameMenuItemZBool *);
void SetMouseSensitivity(CGameMenuItemSliderFloat *);
void SetMouseAimFlipped(CGameMenuItemZBool *);
void SetTurnSpeed(CGameMenuItemSlider *);
void ResetKeys(CGameMenuItemChain *);
void ResetKeysClassic(CGameMenuItemChain *);
void SetMessages(CGameMenuItemZBool *);
void LoadGame(CGameMenuItemZEditBitmap *, CGameMenuEvent *);
void SetupNetLevels(CGameMenuItemZCycle *);
void StartNetGame(CGameMenuItemChain *);
void SetParentalLock(CGameMenuItemZBool *);
void TenProcess(CGameMenuItem7EA1C *);
void SetupLevelMenuItem(int);

char strRestoreGameStrings[][16] = 
{
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
    "<Empty>",
};

const char *zNetGameTypes[] =
{
    "Cooperative",
    "Bloodbath",
    "Teams",
};

const char *zMonsterStrings[] =
{
    "None",
    "Bring 'em on",
    "Respawn",
};

const char *zWeaponStrings[] =
{
    "Do not Respawn",
    "Are Permanent",
    "Respawn",
    "Respawn with Markers",
};

const char *zItemStrings[] =
{
    "Do not Respawn",
    "Respawn",
    "Respawn with Markers",
};

const char *zRespawnStrings[] =
{
    "At Random Locations",
    "Close to Weapons",
    "Away from Enemies",
};

const char *zDiffStrings[] =
{
    "STILL KICKING",
    "PINK ON THE INSIDE",
    "LIGHTLY BROILED",
    "WELL DONE",
    "EXTRA CRISPY",
};

char zUserMapName[16];
char *zEpisodeNames[6];
char *zLevelNames[6][16];

CGameMenu menuMain;
CGameMenu menuMainWithSave;
CGameMenu menuNetMain;
CGameMenu menuNetStart;
CGameMenu menuEpisode;
CGameMenu menuDifficulty;
CGameMenu menuOptions;
CGameMenu menuControls;
CGameMenu menuMessages;
CGameMenu menuKeys;
CGameMenu menuSaveGame;
CGameMenu menuLoadGame;
CGameMenu menuLoading;
CGameMenu menuSounds;
CGameMenu menuQuit;
CGameMenu menuCredits;
CGameMenu menuOrder;
CGameMenu menuPlayOnline;
CGameMenu menuParentalLock;
CGameMenu menuSorry;
CGameMenu menuSorry2;

CGameMenuItemQAV itemBloodQAV("", 3, 160, 100, "BDRIP", true);
CGameMenuItemQAV itemCreditsQAV("", 3, 160, 100, "CREDITS", false, true);
CGameMenuItemQAV itemHelp3QAV("", 3, 160, 100, "HELP3", false, false);
CGameMenuItemQAV itemHelp3BQAV("", 3, 160, 100, "HELP3B", false, false);
CGameMenuItemQAV itemHelp4QAV("", 3, 160, 100, "HELP4", false, true);
CGameMenuItemQAV itemHelp5QAV("", 3, 160, 100, "HELP5", false, true);

CGameMenuItemTitle itemMainTitle("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain itemMain1("NEW GAME", 1, 0, 45, 320, 1, &menuEpisode, -1, NULL, 0);
CGameMenuItemChain itemMain2("PLAY ONLINE", 1, 0, 65, 320, 1, &menuPlayOnline, -1, NULL, 0);
CGameMenuItemChain itemMain3("OPTIONS", 1, 0, 85, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain itemMain4("LOAD GAME", 1, 0, 105, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain itemMain5("HELP", 1, 0, 125, 320, 1, &menuOrder, -1, NULL, 0);
CGameMenuItemChain itemMain6("CREDITS", 1, 0, 145, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain itemMain7("QUIT", 1, 0, 165, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle itemMainSaveTitle("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain itemMainSave1("NEW GAME", 1, 0, 45, 320, 1, &menuEpisode, -1, NULL, 0);
CGameMenuItemChain itemMainSave2("PLAY ONLINE", 1, 0, 60, 320, 1, &menuPlayOnline, -1, NULL, 0);
CGameMenuItemChain itemMainSave3("OPTIONS", 1, 0, 75, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain itemMainSave4("SAVE GAME", 1, 0, 90, 320, 1, &menuSaveGame, -1, SaveGameProcess, 0);
CGameMenuItemChain itemMainSave5("LOAD GAME", 1, 0, 105, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain itemMainSave6("HELP", 1, 0, 120, 320, 1, &menuOrder, -1, NULL, 0);
CGameMenuItemChain itemMainSave7("CREDITS", 1, 0, 135, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain itemMainSave8("QUIT", 1, 0, 150, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle itemEpisodesTitle("EPISODES", 1, 160, 20, 2038);
CGameMenuItemChain7F2F0 itemEpisodes[6];

CGameMenuItemTitle itemDifficultyTitle("DIFFICULTY", 1, 160, 20, 2038);
CGameMenuItemChain itemDifficulty1("STILL KICKING", 1, 0, 60, 320, 1, NULL, -1, SetDifficultyAndStart, 0);
CGameMenuItemChain itemDifficulty2("PINK ON THE INSIDE", 1, 0, 80, 320, 1, NULL, -1, SetDifficultyAndStart, 1);
CGameMenuItemChain itemDifficulty3("LIGHTLY BROILED", 1, 0, 100, 320, 1, NULL, -1, SetDifficultyAndStart, 2);
CGameMenuItemChain itemDifficulty4("WELL DONE", 1, 0, 120, 320, 1, NULL, -1, SetDifficultyAndStart, 3);
CGameMenuItemChain itemDifficulty5("EXTRA CRISPY", 1, 0, 140, 320, 1, 0, -1, SetDifficultyAndStart, 4);

CGameMenuItemTitle itemOptionsTitle("OPTIONS", 1, 160, 20, 2038);
CGameMenuItemChain itemOption1("CONTROLS...", 3, 0, 40, 320, 1, &menuControls, -1, NULL, 0);
CGameMenuItemSlider sliderDetail("DETAIL:", 3, 66, 50, 180, gDetail, 0, 4, 1, SetDetail, -1, -1);
CGameMenuItemSlider sliderGamma("GAMMA:", 3, 66, 60, 180, gGamma, 0, 15, 2, SetGamma, -1, -1);
CGameMenuItemSlider sliderMusic("MUSIC:", 3, 66, 70, 180, MusicVolume, 0, 256, 48, SetMusicVol, -1, -1);
CGameMenuItemSlider sliderSound("SOUND:", 3, 66, 80, 180, FXVolume, 0, 256, 48, SetSoundVol, -1, -1);
CGameMenuItemSlider sliderCDAudio("CD AUDIO:", 3, 66, 90, 180, CDVolume, 0, 256, 48, SetCDVol, -1, -1);
CGameMenuItemZBool bool3DAudio("3D AUDIO:", 3, 66, 100, 180, gDoppler, SetDoppler, NULL, NULL);
CGameMenuItemZBool boolCrosshair("CROSSHAIR:", 3, 66, 110, 180, gAimReticle, SetCrosshair, NULL, NULL);
CGameMenuItemZBool boolShowWeapons("SHOW WEAPONS:", 3, 66, 120, 180, gShowWeapon, SetShowWeapons, NULL, NULL);
CGameMenuItemZBool boolSlopeTilting("SLOPE TILTING:", 3, 66, 130, 180, gSlopeTilting, SetSlopeTilting, NULL, NULL);
CGameMenuItemZBool boolViewBobbing("VIEW BOBBING:", 3, 66, 140, 180, gViewVBobbing, SetViewBobbing, NULL, NULL);
CGameMenuItemZBool boolViewSwaying("VIEW SWAYING:", 3, 66, 150, 180, gViewHBobbing, SetViewSwaying, NULL, NULL);
CGameMenuItem7EE34 itemOption2("VIDEO MODE...", 3, 0, 160, 320, 1);
CGameMenuItemChain itemChainParentalLock("PARENTAL LOCK", 3, 0, 170, 320, 1, &menuParentalLock, -1, NULL, 0);

CGameMenuItemTitle itemControlsTitle("CONTROLS", 1, 160, 20, 2038);
CGameMenuItemSliderFloat sliderMouseSpeed("Mouse Sensitivity:", 1, 10, 70, 300, CONTROL_MouseSensitivity, 0.5f, 16.f, 0.5f, SetMouseSensitivity, -1,-1);
CGameMenuItemZBool boolMouseFlipped("Invert Mouse Aim:", 1, 10, 90, 300, gMouseAimingFlipped, SetMouseAimFlipped, NULL, NULL);
CGameMenuItemSlider sliderTurnSpeed("Key Turn Speed:", 1, 10, 110, 300, gTurnSpeed, 64, 128, 4, SetTurnSpeed, -1, -1);
CGameMenuItemChain itemChainKeyList("Configure Keys...", 1, 0, 130, 320, 1, &menuKeys, -1, NULL, 0);
CGameMenuItemChain itemChainKeyReset("Reset Keys (default)...", 1, 0, 150, 320, 1, &menuKeys, -1, ResetKeys, 0);
CGameMenuItemChain itemChainKeyResetClassic("Reset Keys (classic)...", 1, 0, 170, 320, 1, &menuKeys, -1, ResetKeysClassic, 0);

CGameMenuItemTitle itemMessagesTitle("MESSAGES", 1, 160, 20, 2038);
CGameMenuItemZBool boolMessages("MESSAGES:", 3, 66, 70, 180, 0, SetMessages, NULL, NULL);
CGameMenuItemSlider sliderMsgCount("MESSAGE COUNT:", 3, 66, 80, 180, gMessageCount, 1, 16, 1, NULL, -1, -1);
CGameMenuItemSlider sliderMsgTime("MESSAGE TIME:", 3, 66, 90, 180, gMessageTime, 1, 8, 1, NULL, -1, -1);
CGameMenuItemZBool boolMsgFont("LARGE FONT:", 3, 66, 100, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgIncoming("INCOMING:", 3, 66, 110, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgSelf("SELF PICKUP:", 3, 66, 120, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgOther("OTHER PICKUP:", 3, 66, 130, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool boolMsgRespawn("RESPAWN:", 3, 66, 140, 180, 0, 0, NULL, NULL);

CGameMenuItemTitle itemKeysTitle("KEY SETUP", 1, 160, 20, 2038);
CGameMenuItemKeyList itemKeyList("", 3, 56, 40, 200, 16, NUMGAMEFUNCTIONS, 0);

CGameMenuItemTitle itemSaveTitle("Save Game", 1, 160, 20, 2038);
CGameMenuItemZEditBitmap itemSaveGame1(NULL, 3, 20, 60, 320, strRestoreGameStrings[0], 16, 1, SaveGame, 0);
CGameMenuItemZEditBitmap itemSaveGame2(NULL, 3, 20, 70, 320, strRestoreGameStrings[1], 16, 1, SaveGame, 1);
CGameMenuItemZEditBitmap itemSaveGame3(NULL, 3, 20, 80, 320, strRestoreGameStrings[2], 16, 1, SaveGame, 2);
CGameMenuItemZEditBitmap itemSaveGame4(NULL, 3, 20, 90, 320, strRestoreGameStrings[3], 16, 1, SaveGame, 3);
CGameMenuItemZEditBitmap itemSaveGame5(NULL, 3, 20, 100, 320, strRestoreGameStrings[4], 16, 1, SaveGame, 4);
CGameMenuItemZEditBitmap itemSaveGame6(NULL, 3, 20, 110, 320, strRestoreGameStrings[5], 16, 1, SaveGame, 5);
CGameMenuItemZEditBitmap itemSaveGame7(NULL, 3, 20, 120, 320, strRestoreGameStrings[6], 16, 1, SaveGame, 6);
CGameMenuItemZEditBitmap itemSaveGame8(NULL, 3, 20, 130, 320, strRestoreGameStrings[7], 16, 1, SaveGame, 7);
CGameMenuItemZEditBitmap itemSaveGame9(NULL, 3, 20, 140, 320, strRestoreGameStrings[8], 16, 1, SaveGame, 8);
CGameMenuItemZEditBitmap itemSaveGame10(NULL, 3, 20, 150, 320, strRestoreGameStrings[9], 16, 1, SaveGame, 9);
CGameMenuItemBitmapLS itemSaveGamePic(NULL, 3, 0, 0, 2050);

CGameMenuItemTitle itemLoadTitle("Load Game", 1, 160, 20, 2038);
CGameMenuItemZEditBitmap itemLoadGame1(NULL, 3, 20, 60, 320, strRestoreGameStrings[0], 16, 1, LoadGame, 0);
CGameMenuItemZEditBitmap itemLoadGame2(NULL, 3, 20, 70, 320, strRestoreGameStrings[1], 16, 1, LoadGame, 1);
CGameMenuItemZEditBitmap itemLoadGame3(NULL, 3, 20, 80, 320, strRestoreGameStrings[2], 16, 1, LoadGame, 2);
CGameMenuItemZEditBitmap itemLoadGame4(NULL, 3, 20, 90, 320, strRestoreGameStrings[3], 16, 1, LoadGame, 3);
CGameMenuItemZEditBitmap itemLoadGame5(NULL, 3, 20, 100, 320, strRestoreGameStrings[4], 16, 1, LoadGame, 4);
CGameMenuItemZEditBitmap itemLoadGame6(NULL, 3, 20, 110, 320, strRestoreGameStrings[5], 16, 1, LoadGame, 5);
CGameMenuItemZEditBitmap itemLoadGame7(NULL, 3, 20, 120, 320, strRestoreGameStrings[6], 16, 1, LoadGame, 6);
CGameMenuItemZEditBitmap itemLoadGame8(NULL, 3, 20, 130, 320, strRestoreGameStrings[7], 16, 1, LoadGame, 7);
CGameMenuItemZEditBitmap itemLoadGame9(NULL, 3, 20, 140, 320, strRestoreGameStrings[8], 16, 1, LoadGame, 8);
CGameMenuItemZEditBitmap itemLoadGame10(NULL, 3, 20, 150, 320, strRestoreGameStrings[9], 16, 1, LoadGame, 9);
CGameMenuItemBitmapLS itemLoadGamePic(NULL, 3, 0, 0, 2518);

CGameMenuItemTitle itemNetStartTitle("NETWORK GAME", 1, 160, 20, 2038);
CGameMenuItemZCycle itemNetStart1("GAME", 1, 20, 35, 280, 0, 0, zNetGameTypes, 3, 0);
CGameMenuItemZCycle itemNetStart2("EPISODE", 1, 20, 50, 280, 0, SetupNetLevels, NULL, 0, 0);
CGameMenuItemZCycle itemNetStart3("LEVEL", 1, 20, 65, 280, 0, NULL, NULL, 0, 0);
CGameMenuItemZCycle itemNetStart4("DIFFICULTY", 1, 20, 80, 280, 0, 0, zDiffStrings, 5, 0);
CGameMenuItemZCycle itemNetStart5("MONSTERS", 1, 20, 95, 280, 0, 0, zMonsterStrings, 3, 0);
CGameMenuItemZCycle itemNetStart6("WEAPONS", 1, 20, 110, 280, 0, 0, zWeaponStrings, 4, 0);
CGameMenuItemZCycle itemNetStart7("ITEMS", 1, 20, 125, 280, 0, 0, zItemStrings, 3, 0);
CGameMenuItemZEdit itemNetStart9("USER MAP:", 1, 20, 155, 280, zUserMapName, 13, 0, NULL, 0);
CGameMenuItemChain itemNetStart10("START GAME", 1, 20, 170, 280, 0, 0, -1, StartNetGame, 0);

CGameMenuItemText itemLoadingText("LOADING...", 1, 160, 100, 1);

CGameMenuItemTitle itemSoundsTitle("SOUNDS", 1, 160, 20, 2038);
CGameMenuItemSlider itemSoundsMusic("MUSIC:", 3, 40, 60, 180, MusicVolume, 0, 256, 48, SetMusicVol, -1, -1);
CGameMenuItemSlider itemSoundsSound("SOUND:", 3, 40, 70, 180, FXVolume, 0, 256, 48, SetSoundVol, -1, -1);
CGameMenuItemSlider itemSoundsCDAudio("CD AUDIO:", 3, 40, 80, 180, CDVolume, 0, 256, 48, SetCDVol, -1, -1);
CGameMenuItemZBool itemSounds3DAudio("3D SOUND:", 3, 40, 90, 180, gDoppler, SetDoppler, NULL, NULL);

CGameMenuItemTitle itemQuitTitle("QUIT", 1, 160, 20, 2038);
CGameMenuItemText itemQuitText1("Do you really want to quit?", 0, 160, 100, 1);
CGameMenuItemYesNoQuit itemQuitYesNo("[Y/N]", 0, 20, 110, 280, 1, -1, 0);

CGameMenuItemPicCycle itemCreditsPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemPicCycle itemOrderPicCycle(0, 0, NULL, NULL, 0, 0);

CGameMenuItemTitle itemParentalLockTitle("PARENTAL LOCK", 1, 160, 20, 2038);
CGameMenuItemZBool itemParentalLockToggle("LOCK:", 3, 66, 70, 180, 0, SetParentalLock, NULL, NULL);
CGameMenuItemPassword itemParentalLockPassword("SET PASSWORD:", 3, 160, 80);

CGameMenuItemPicCycle itemSorryPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemText itemSorryText1("Loading and saving games", 0, 160, 90, 1);
CGameMenuItemText itemSorryText2("not supported", 0, 160, 100, 1);
CGameMenuItemText itemSorryText3("in this demo version of Blood.", 0, 160, 110, 1);

CGameMenuItemText itemSorry2Text1("Buy the complete version of", 0, 160, 90, 1);
CGameMenuItemText itemSorry2Text2("Blood for three new episodes", 0, 160, 100, 1);
CGameMenuItemText itemSorry2Text3("plus eight BloodBath-only levels!", 0, 160, 110, 1);

CGameMenuItemTitle unk_26E06C(" ONLINE ", 1, 160, 20, 2038);
CGameMenuItem7EA1C unk_26E090("DWANGO", 1, 0, 45, 320, "matt", "DWANGO", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E0E8("RTIME", 1, 0, 65, 320, "matt", "RTIME", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E140("HEAT", 1, 0, 85, 320, "matt", "HEAT", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E198("KALI", 1, 0, 105, 320, "matt", "KALI", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E1F0("MPATH", 1, 0, 125, 320, "matt", "MPATH", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E248("TEN", 1, 0, 145, 320, "matt", "TEN", 1, -1, TenProcess, NULL);

void SetupLoadingScreen(void)
{
    menuLoading.Add(&itemLoadingText, true);
}

void SetupKeyListMenu(void)
{
    menuKeys.Add(&itemKeysTitle, false);
    menuKeys.Add(&itemKeyList, true);
    menuKeys.Add(&itemBloodQAV, false);
}

void SetupMessagesMenu(void)
{
    menuMessages.Add(&itemMessagesTitle, false);
    menuMessages.Add(&boolMessages, true);
    menuMessages.Add(&sliderMsgCount, false);
    menuMessages.Add(&sliderMsgTime, false);
    menuMessages.Add(&boolMsgFont, false);
    menuMessages.Add(&boolMsgIncoming, false);
    menuMessages.Add(&boolMsgSelf, false);
    menuMessages.Add(&boolMsgOther, false);
    menuMessages.Add(&boolMsgRespawn, false);
    menuMessages.Add(&itemBloodQAV, false);
}

void SetupControlsMenu(void)
{
    sliderMouseSpeed.fValue = ClipRangeF(CONTROL_MouseSensitivity, sliderMouseSpeed.fRangeLow, sliderMouseSpeed.fRangeHigh);
    sliderTurnSpeed.nValue = ClipRange(gTurnSpeed, sliderTurnSpeed.nRangeLow, sliderTurnSpeed.nRangeHigh);
    boolMouseFlipped.at20 = gMouseAimingFlipped;
    menuControls.Add(&itemControlsTitle, false);
    menuControls.Add(&sliderMouseSpeed, true);
    menuControls.Add(&boolMouseFlipped, false);
    menuControls.Add(&sliderTurnSpeed, false);
    menuControls.Add(&itemChainKeyList, false);
    menuControls.Add(&itemChainKeyReset, false);
    menuControls.Add(&itemChainKeyResetClassic, false);
    menuControls.Add(&itemBloodQAV, false);
}

void SetupOptionsMenu(void)
{
    sliderDetail.nValue = ClipRange(gDetail, sliderDetail.nRangeLow, sliderDetail.nRangeHigh);
    sliderGamma.nValue = ClipRange(gGamma, sliderGamma.nRangeLow, sliderGamma.nRangeHigh);
    sliderMusic.nValue = ClipRange(MusicVolume, sliderMusic.nRangeLow, sliderMusic.nRangeHigh);
    sliderSound.nValue = ClipRange(FXVolume, sliderSound.nRangeLow, sliderSound.nRangeHigh);
    // NUKE-TODO: unk_26CB50.at24 = ClipRange(Redbook.GetVolume(), unk_26CB50.at28, unk_26CB50.at2c);
    bool3DAudio.at20 = gDoppler;
    boolCrosshair.at20 = gAimReticle;
    boolShowWeapons.at20 = gShowWeapon;
    boolSlopeTilting.at20 = gSlopeTilting;
    boolViewBobbing.at20 = gViewVBobbing;
    boolViewSwaying.at20 = gViewHBobbing;
    boolMessages.at20 = gGameMessageMgr.at0;
    menuOptions.Add(&itemOptionsTitle, false);
    menuOptions.Add(&itemOption1, true);
    menuOptions.Add(&sliderDetail, false);
    menuOptions.Add(&sliderGamma, false);
    menuOptions.Add(&sliderMusic, false);
    menuOptions.Add(&sliderSound, false);
    menuOptions.Add(&sliderCDAudio, false);
    menuOptions.Add(&bool3DAudio, false);
    menuOptions.Add(&boolCrosshair, false);
    menuOptions.Add(&boolShowWeapons, false);
    menuOptions.Add(&boolSlopeTilting, false);
    menuOptions.Add(&boolViewBobbing, false);
    menuOptions.Add(&boolViewSwaying, false);
    menuOptions.Add(&itemOption2, false);
    menuOptions.Add(&itemChainParentalLock, false);
    menuOptions.Add(&itemBloodQAV, false);
}

void SetupDifficultyMenu(void)
{
    menuDifficulty.Add(&itemDifficultyTitle, false);
    menuDifficulty.Add(&itemDifficulty1, false);
    menuDifficulty.Add(&itemDifficulty2, false);
    menuDifficulty.Add(&itemDifficulty3, true);
    menuDifficulty.Add(&itemDifficulty4, false);
    menuDifficulty.Add(&itemDifficulty5, false);
    menuDifficulty.Add(&itemBloodQAV, false);
}

void SetupEpisodeMenu(void)
{
    menuEpisode.Add(&itemEpisodesTitle, false);
    bool unk = false;
    int height;
    gMenuTextMgr.GetFontInfo(1, NULL, NULL, &height);
    int j = 0;
    for (int i = 0; i < 6; i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (!pEpisode->bloodbath || gGameOptions.nGameType != 0)
        {
            if (j < gEpisodeCount)
            {
                CGameMenuItemChain7F2F0 *pEpisodeItem = &itemEpisodes[j];
                pEpisodeItem->nFont = 1;
                pEpisodeItem->nX = 0;
                pEpisodeItem->nWidth = 320;
                pEpisodeItem->at20 = 1;
                pEpisodeItem->pzText = pEpisode->at0;
                pEpisodeItem->nY = 55+(height+8)*j;
                pEpisodeItem->at34 = i;
                if (!unk || j == 0)
                {
                    pEpisodeItem = &itemEpisodes[j];
                    pEpisodeItem->at24 = &menuDifficulty;
                    pEpisodeItem->at28 = 3;
                }
                else
                {
                    pEpisodeItem->at24 = &menuSorry2;
                    pEpisodeItem->at28 = 1;
                }
                pEpisodeItem = &itemEpisodes[j];
                pEpisodeItem->at18 |= 3;
                bool first = j == 0;
                menuEpisode.Add(&itemEpisodes[j], first);
                if (first)
                    SetupLevelMenuItem(j);
            }
            j++;
        }
    }
    menuEpisode.Add(&itemBloodQAV, false);
}

void SetupMainMenu(void)
{
    menuMain.Add(&itemMainTitle, false);
    menuMain.Add(&itemMain1, true);
    if (gGameOptions.nGameType > 0)
    {
        itemMain1.at24 = &menuNetStart;
        itemMain1.at28 = 2;
    }
    menuMain.Add(&itemMain2, false);
    menuMain.Add(&itemMain3, false);
    menuMain.Add(&itemMain4, false);
    menuMain.Add(&itemMain5, false);
    menuMain.Add(&itemMain6, false);
    menuMain.Add(&itemMain7, false);
    menuMain.Add(&itemBloodQAV, false);
}

void SetupMainMenuWithSave(void)
{
    menuMainWithSave.Add(&itemMainSaveTitle, false);
    menuMainWithSave.Add(&itemMainSave1, true);
    if (gGameOptions.nGameType > 0)
    {
        itemMainSave1.at24 = &menuNetStart;
        itemMainSave1.at28 = 2;
    }
    menuMainWithSave.Add(&itemMainSave2, false);
    menuMainWithSave.Add(&itemMainSave3, false);
    menuMainWithSave.Add(&itemMainSave4, false);
    menuMainWithSave.Add(&itemMainSave5, false);
    menuMainWithSave.Add(&itemMainSave6, false);
    menuMainWithSave.Add(&itemMainSave7, false);
    menuMainWithSave.Add(&itemMainSave8, false);
    menuMainWithSave.Add(&itemBloodQAV, false);
}

void SetupNetStartMenu(void)
{
    bool oneEpisode = false;
    menuNetStart.Add(&itemNetStartTitle, false);
    menuNetStart.Add(&itemNetStart1, false);
    for (int i = 0; i < (oneEpisode ? 1 : 6); i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (i < gEpisodeCount)
            itemNetStart2.Add(pEpisode->at0, i == 0);
    }
    menuNetStart.Add(&itemNetStart2, false);
    menuNetStart.Add(&itemNetStart3, false);
    menuNetStart.Add(&itemNetStart4, false);
    menuNetStart.Add(&itemNetStart5, false);
    menuNetStart.Add(&itemNetStart6, false);
    menuNetStart.Add(&itemNetStart7, false);
    menuNetStart.Add(&itemNetStart9, false);
    menuNetStart.Add(&itemNetStart10, false);
    itemNetStart1.SetTextIndex(1);
    itemNetStart4.SetTextIndex(2);
    itemNetStart5.SetTextIndex(0);
    itemNetStart6.SetTextIndex(1);
    itemNetStart7.SetTextIndex(1);
    menuNetStart.Add(&itemBloodQAV, false);
}

void SetupSaveGameMenu(void)
{
    menuSaveGame.Add(&itemSaveTitle, false);
    menuSaveGame.Add(&itemSaveGame1, true);
    menuSaveGame.Add(&itemSaveGame2, false);
    menuSaveGame.Add(&itemSaveGame3, false);
    menuSaveGame.Add(&itemSaveGame4, false);
    menuSaveGame.Add(&itemSaveGame5, false);
    menuSaveGame.Add(&itemSaveGame6, false);
    menuSaveGame.Add(&itemSaveGame7, false);
    menuSaveGame.Add(&itemSaveGame8, false);
    menuSaveGame.Add(&itemSaveGame9, false);
    menuSaveGame.Add(&itemSaveGame10, false);
    menuSaveGame.Add(&itemSaveGamePic, false);
    menuSaveGame.Add(&itemBloodQAV, false);

    itemSaveGame1.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[0], "<Empty>"))
        itemSaveGame1.at37 = 1;

    itemSaveGame2.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[1], "<Empty>"))
        itemSaveGame2.at37 = 1;

    itemSaveGame3.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[2], "<Empty>"))
        itemSaveGame3.at37 = 1;

    itemSaveGame4.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[3], "<Empty>"))
        itemSaveGame4.at37 = 1;

    itemSaveGame5.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[4], "<Empty>"))
        itemSaveGame5.at37 = 1;

    itemSaveGame6.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[5], "<Empty>"))
        itemSaveGame6.at37 = 1;

    itemSaveGame7.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[6], "<Empty>"))
        itemSaveGame7.at37 = 1;

    itemSaveGame8.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[7], "<Empty>"))
        itemSaveGame8.at37 = 1;

    itemSaveGame9.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[8], "<Empty>"))
        itemSaveGame9.at37 = 1;

    itemSaveGame10.at2c = &itemSaveGamePic;
    if (!strcmp(strRestoreGameStrings[9], "<Empty>"))
        itemSaveGame10.at37 = 1;
}

void SetupLoadGameMenu(void)
{
    menuLoadGame.Add(&itemLoadTitle, false);
    menuLoadGame.Add(&itemLoadGame1, true);
    menuLoadGame.Add(&itemLoadGame2, false);
    menuLoadGame.Add(&itemLoadGame3, false);
    menuLoadGame.Add(&itemLoadGame4, false);
    menuLoadGame.Add(&itemLoadGame5, false);
    menuLoadGame.Add(&itemLoadGame6, false);
    menuLoadGame.Add(&itemLoadGame7, false);
    menuLoadGame.Add(&itemLoadGame8, false);
    menuLoadGame.Add(&itemLoadGame9, false);
    menuLoadGame.Add(&itemLoadGame10, false);
    menuLoadGame.Add(&itemLoadGamePic, false);
    itemLoadGame1.at35 = 0;
    itemLoadGame2.at35 = 0;
    itemLoadGame3.at35 = 0;
    itemLoadGame4.at35 = 0;
    itemLoadGame5.at35 = 0;
    itemLoadGame6.at35 = 0;
    itemLoadGame7.at35 = 0;
    itemLoadGame8.at35 = 0;
    itemLoadGame9.at35 = 0;
    itemLoadGame10.at35 = 0;
    itemLoadGame1.at2c = &itemLoadGamePic;
    itemLoadGame2.at2c = &itemLoadGamePic;
    itemLoadGame3.at2c = &itemLoadGamePic;
    itemLoadGame4.at2c = &itemLoadGamePic;
    itemLoadGame5.at2c = &itemLoadGamePic;
    itemLoadGame6.at2c = &itemLoadGamePic;
    itemLoadGame7.at2c = &itemLoadGamePic;
    itemLoadGame8.at2c = &itemLoadGamePic;
    itemLoadGame9.at2c = &itemLoadGamePic;
    itemLoadGame10.at2c = &itemLoadGamePic;
    menuLoadGame.Add(&itemBloodQAV, false);
}

void SetupSoundsMenu(void)
{
    itemSoundsMusic.nValue = ClipRange(MusicVolume, itemSoundsMusic.nRangeLow, itemSoundsMusic.nRangeHigh);
    itemSoundsSound.nValue = ClipRange(FXVolume, itemSoundsSound.nRangeLow, itemSoundsSound.nRangeHigh);
    // NUKE-TODO: unk_26DB90.at24 = ClipRange(Redbook.GetVolume(), unk_26DB90.at28, unk_26DB90.at2c);
    menuSounds.Add(&itemSoundsTitle, false);
    menuSounds.Add(&itemSoundsMusic, true);
    menuSounds.Add(&itemSoundsSound, false);
    menuSounds.Add(&itemSoundsCDAudio, false);
    menuSounds.Add(&itemSounds3DAudio, false);
    menuSounds.Add(&itemBloodQAV, false);
}

void SetupQuitMenu(void)
{
    menuQuit.Add(&itemQuitTitle, false);
    menuQuit.Add(&itemQuitText1, false);
    menuQuit.Add(&itemQuitYesNo, true);
    menuQuit.Add(&itemBloodQAV, false);
}

void SetupHelpOrderMenu(void)
{
    menuOrder.Add(&itemHelp4QAV, true);
    menuOrder.Add(&itemHelp5QAV, false);
    menuOrder.Add(&itemHelp3QAV, false);
    menuOrder.Add(&itemHelp3BQAV, false);
    itemHelp4QAV.at18 |= 10;
    itemHelp5QAV.at18 |= 10;
    itemHelp3QAV.at18 |= 10;
    itemHelp3BQAV.at18 |= 10;
}

void SetupCreditsMenu(void)
{
    menuCredits.Add(&itemCreditsQAV, true);
    itemCreditsQAV.at18 |= 10;
}

void SetupParentalLockMenu(void)
{
    itemParentalLockToggle.at20 = gbAdultContent;
    strcpy(itemParentalLockPassword.at20, gzAdultPassword);
    menuParentalLock.Add(&itemParentalLockTitle, false);
    menuParentalLock.Add(&itemParentalLockToggle, true);
    menuParentalLock.Add(&itemParentalLockPassword, false);
    menuParentalLock.Add(&itemBloodQAV, false);
}

void SetupSorry3Menu(void)
{
    menuPlayOnline.Add(&unk_26E06C, false);
    menuPlayOnline.Add(&unk_26E090, true);
    menuPlayOnline.Add(&unk_26E0E8, false);
    menuPlayOnline.Add(&unk_26E140, false);
    menuPlayOnline.Add(&unk_26E198, false);
    menuPlayOnline.Add(&unk_26E1F0, false);
    menuPlayOnline.Add(&unk_26E248, false);
    menuPlayOnline.Add(&itemBloodQAV, false);
}

void SetupSorryMenu(void)
{
    menuSorry.Add(&itemSorryPicCycle, true);
    menuSorry.Add(&itemSorryText1, false);
    menuSorry.Add(&itemSorryText3, false);
    menuSorry.Add(&itemBloodQAV, false);
}

void SetupSorry2Menu(void)
{
    menuSorry2.Add(&itemSorryPicCycle, true);
    menuSorry2.Add(&itemSorry2Text1, false);
    menuSorry2.Add(&itemSorry2Text2, false);
    menuSorry2.Add(&itemSorry2Text3, false);
    menuSorry2.Add(&itemBloodQAV, false);
}

void SetupMenus(void)
{
    SetupLoadingScreen();
    SetupKeyListMenu();
    SetupMessagesMenu();
    SetupControlsMenu();
    SetupSaveGameMenu();
    SetupLoadGameMenu();
    SetupOptionsMenu();
    SetupCreditsMenu();
    SetupHelpOrderMenu();
    SetupSoundsMenu();
    SetupDifficultyMenu();
    SetupEpisodeMenu();
    SetupMainMenu();
    SetupMainMenuWithSave();
    SetupNetStartMenu();
    SetupQuitMenu();
    SetupParentalLockMenu();
    SetupSorryMenu();
    SetupSorry2Menu();
    SetupSorry3Menu();
}

void SetDoppler(CGameMenuItemZBool *pItem)
{
    gDoppler = pItem->at20;
}

void SetCrosshair(CGameMenuItemZBool *pItem)
{
    gAimReticle = pItem->at20;
}

void ResetKeys(CGameMenuItemChain *)
{
    CONFIG_SetDefaultKeys(keydefaults);
}

void ResetKeysClassic(CGameMenuItemChain *)
{
    CONFIG_SetDefaultKeys(oldkeydefaults);
}

void SetShowWeapons(CGameMenuItemZBool *pItem)
{
    gShowWeapon = pItem->at20;
}

void SetSlopeTilting(CGameMenuItemZBool *pItem)
{
    gSlopeTilting = pItem->at20;
}

void SetViewBobbing(CGameMenuItemZBool *pItem)
{
    gViewVBobbing = pItem->at20;
}

void SetViewSwaying(CGameMenuItemZBool *pItem)
{
    gViewHBobbing = pItem->at20;
}

void SetDetail(CGameMenuItemSlider *pItem)
{
    gDetail = pItem->nValue;
}

void SetGamma(CGameMenuItemSlider *pItem)
{
    gGamma = pItem->nValue;
    scrSetGamma(gGamma);
}

void SetMusicVol(CGameMenuItemSlider *pItem)
{
    sndSetMusicVolume(pItem->nValue);
}

void SetSoundVol(CGameMenuItemSlider *pItem)
{
    sndSetFXVolume(pItem->nValue);
}

void SetCDVol(CGameMenuItemSlider *pItem)
{
    // NUKE-TODO:
}

void SetMessages(CGameMenuItemZBool *pItem)
{
    gGameMessageMgr.SetState(pItem->at20);
}

void SetMouseSensitivity(CGameMenuItemSliderFloat *pItem)
{
	CONTROL_MouseSensitivity = pItem->fValue;
}

void SetMouseAimFlipped(CGameMenuItemZBool *pItem)
{
    gMouseAimingFlipped = pItem->at20;
}

void SetTurnSpeed(CGameMenuItemSlider *pItem)
{
    gTurnSpeed = pItem->nValue;
}

extern bool gStartNewGame;

void SetDifficultyAndStart(CGameMenuItemChain *pItem)
{
    gGameOptions.nDifficulty = pItem->at30;
    gSkill = pItem->at30;
    gGameOptions.nLevel = 0;
    if (gDemo.at1)
        gDemo.StopPlayback();
    gStartNewGame = true;
    gCheatMgr.sub_5BCF4();
    gGameMenuMgr.Deactivate();
}

void SetVideoMode(CGameMenuItemChain *pItem)
{
    if (pItem->at30 == validmodecnt)
    {
        gSetup.fullscreen = 0;
        gSetup.xdim = 640;
        gSetup.ydim = 480;
    }
    else
    {
        gSetup.fullscreen = 0;
        gSetup.xdim = validmode[pItem->at30].xdim;
        gSetup.ydim = validmode[pItem->at30].ydim;
    }
    scrSetGameMode(gSetup.fullscreen, gSetup.xdim, gSetup.ydim, gSetup.bpp);
    scrSetDac();
    viewResizeView(gViewSize);
}

void SaveGameProcess(CGameMenuItemChain *)
{
}

void TenProcess(CGameMenuItem7EA1C *)
{
}

short gQuickLoadSlot = -1;
short gQuickSaveSlot = -1;

void SaveGame(CGameMenuItemZEditBitmap *pItem, CGameMenuEvent *event)
{
    char strSaveGameName[15] = "";
    int nSlot = pItem->at28;
    if (gGameOptions.nGameType > 0 || !gGameStarted)
        return;
    if (event->at0 != 6 || strSaveGameName[0])
    {
        gGameMenuMgr.Deactivate();
        return;
    }
    sprintf(strSaveGameName, "GAME00%02d.SAV", nSlot);
    strcpy(gGameOptions.szUserGameName, strRestoreGameStrings[nSlot]);
    sprintf(gGameOptions.szSaveGameName, strSaveGameName);
    gGameOptions.nSaveGameSlot = nSlot;
    viewLoadingScreen(2518, "Saving", "Saving Your Game", strRestoreGameStrings[nSlot]);
    gSaveGameNum = nSlot;
    LoadSave::SaveGame(strSaveGameName);
    gQuickSaveSlot = nSlot;
    gGameMenuMgr.Deactivate();
}

void QuickSaveGame(void)
{
    char strSaveGameName[15] = "";
    if (gGameOptions.nGameType > 0 || !gGameStarted)
        return;
    if (strSaveGameName[0])
    {
        gGameMenuMgr.Deactivate();
        return;
    }
    sprintf(strSaveGameName, "GAME00%02d.SAV", gQuickSaveSlot);
    strcpy(gGameOptions.szUserGameName, strRestoreGameStrings[gQuickSaveSlot]);
    sprintf(gGameOptions.szSaveGameName, strSaveGameName);
    gGameOptions.nSaveGameSlot = gQuickSaveSlot;
    viewLoadingScreen(2518, "Saving", "Saving Your Game", strRestoreGameStrings[gQuickSaveSlot]);
    LoadSave::SaveGame(strSaveGameName);
    gGameOptions.picEntry = gSavedOffset;
    gSaveGameOptions[gQuickSaveSlot] = gGameOptions;
    UpdateSavedInfo(gQuickSaveSlot);
    gGameMenuMgr.Deactivate();
}

void LoadGame(CGameMenuItemZEditBitmap *pItem, CGameMenuEvent *event)
{
    char strLoadGameName[15] = "";
    int nSlot = pItem->at28;
    if (gGameOptions.nGameType > 0)
        return;
    sprintf(strLoadGameName, "GAME00%02d.SAV", nSlot);
    if (access(strLoadGameName, 4) == -1)
        return;
    if (gDemo.at1)
        gDemo.Close();
    viewLoadingScreen(2518, "Loading", "Loading Saved Game", strRestoreGameStrings[nSlot]);
    LoadSave::LoadGame(strLoadGameName);
    gGameMenuMgr.Deactivate();
    gQuickLoadSlot = nSlot;
}

void QuickLoadGame(void)
{
    char strLoadGameName[15] = "";
    if (gGameOptions.nGameType > 0)
        return;
    sprintf(strLoadGameName, "GAME00%02d.SAV", gQuickLoadSlot);
    if (access(strLoadGameName, 4) == -1)
        return;
    viewLoadingScreen(2518, "Loading", "Loading Saved Game", strRestoreGameStrings[gQuickLoadSlot]);
    LoadSave::LoadGame(strLoadGameName);
    gGameMenuMgr.Deactivate();
}

void SetupLevelMenuItem(int nEpisode)
{
    dassert(nEpisode >= 0 && nEpisode < gEpisodeCount);
    itemNetStart3.SetTextArray((const char**)zLevelNames[nEpisode], gEpisodeInfo[nEpisode].nLevels, 0);
}

void SetupNetLevels(CGameMenuItemZCycle *pItem)
{
    SetupLevelMenuItem(pItem->at24);
}

void StartNetGame(CGameMenuItemChain *)
{
    gPacketStartGame.gameType = itemNetStart1.at24+1;
    if (gPacketStartGame.gameType == 0)
        gPacketStartGame.gameType = 2;
    gPacketStartGame.episodeId = itemNetStart2.at24;
    gPacketStartGame.levelId = itemNetStart3.at24;
    gPacketStartGame.difficulty = itemNetStart4.at24;
    gPacketStartGame.monsterSettings = itemNetStart5.at24;
    gPacketStartGame.weaponSettings = itemNetStart6.at24;
    gPacketStartGame.itemSettings = itemNetStart7.at24;
    gPacketStartGame.respawnSettings = 0;
    gPacketStartGame.unk = 0;
    gPacketStartGame.userMapName[0] = 0;
    strncpy(gPacketStartGame.userMapName, itemNetStart9.at20, 13);
    gPacketStartGame.userMapName[12] = 0;
    gPacketStartGame.userMap = gPacketStartGame.userMapName[0] != 0;
    netBroadcastNewGame();
    gStartNewGame = 1;
    gGameMenuMgr.Deactivate();
}

void Quit(CGameMenuItemChain *pItem)
{
    if (gGameOptions.nGameType == 0 || numplayers == 1)
        gQuitGame = true;
    else
        gQuitRequest = true;
    gGameMenuMgr.Deactivate();
}

void SetParentalLock(CGameMenuItemZBool *pItem)
{
    if (!pItem->at20)
    {
        pItem->at20 = true;
        pItem->Draw();
        if (strcmp(itemParentalLockPassword.at20, ""))
        {
            itemParentalLockPassword.pMenu->FocusNextItem();
            itemParentalLockPassword.at32 = 0;
            itemParentalLockPassword.at37 = 1;
            itemParentalLockPassword.at5f = pItem;
            itemParentalLockPassword.at29[0] = 0;
            return;
        }
        else
        {
            itemParentalLockPassword.at20[0] = 0;
            pItem->Draw();
            gbAdultContent = false;
        }
    }
    else
        gbAdultContent = true;
    // NUKE-TODO: CONFIG_WriteAdultMode();
}

void MenuSetupEpisodeInfo(void)
{
    memset(zEpisodeNames, 0, sizeof(zEpisodeNames));
    memset(zLevelNames, 0, sizeof(zLevelNames));
    for (int i = 0; i < 6; i++)
    {
        if (i < gEpisodeCount)
        {
            EPISODEINFO *pEpisode = &gEpisodeInfo[i];
            zEpisodeNames[i] = pEpisode->at0;
            for (int j = 0; j < 16; j++)
            {
                if (j < pEpisode->nLevels)
                {
                    zLevelNames[i][j] = pEpisode->at28[j].at90;
                }
            }
        }
    }
}

void sub_5A828(void)
{
    char buffer[80];
    if (gGameOptions.nGameType == 0)
    {
        if (gDemo.at1)
            sprintf(buffer, "Loading Demo");
        else
            sprintf(buffer, "Loading Level");
    }
    else
        sprintf(buffer, "%s", zNetGameTypes[gGameOptions.nGameType-1]);
    viewLoadingScreen(2049, buffer, levelGetTitle(), NULL);
}

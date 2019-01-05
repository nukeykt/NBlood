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
void SetMouseSensitivity(CGameMenuItemSlider *);
void SetMouseAimFlipped(CGameMenuItemZBool *);
void SetTurnSpeed(CGameMenuItemSlider *);
void ResetKeys(CGameMenuItemChain *);
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
CGameMenu unk_26B820;
CGameMenu unk_26B8B0;
CGameMenu unk_26B940;
CGameMenu unk_26B9D0;
CGameMenu menuOptions;
CGameMenu unk_26BAF0;
CGameMenu unk_26BB80;
CGameMenu unk_26BC10;
CGameMenu menuSaveGame;
CGameMenu menuLoadGame;
CGameMenu unk_26BDC0;
CGameMenu menuSounds;
CGameMenu menuQuit;
CGameMenu menuCredits;
CGameMenu menuOrder;
CGameMenu unk_26C090;
CGameMenu unk_26C120;
CGameMenu unk_26C1B0;
CGameMenu unk_26C240;

CGameMenuItemQAV itemBloodQAV("", 3, 160, 100, "BDRIP");
CGameMenuItemQAV unk_26C304("", 3, 160, 100, "CREDITS");
CGameMenuItemQAV unk_26C338("", 3, 160, 100, "HELP3");
CGameMenuItemQAV unk_26C36C("", 3, 160, 100, "HELP3B");
CGameMenuItemQAV unk_26C3A0("", 3, 160, 100, "HELP4");
CGameMenuItemQAV unk_26C3D4("", 3, 160, 100, "HELP5");

CGameMenuItemTitle unk_26C408("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain unk_26C42C("NEW GAME", 1, 0, 45, 320, 1, &unk_26B940, -1, NULL, 0);
CGameMenuItemChain unk_26C460("PLAY ONLINE", 1, 0, 65, 320, 1, &unk_26C090, -1, NULL, 0);
CGameMenuItemChain unk_26C494("OPTIONS", 1, 0, 85, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain unk_26C4C8("LOAD GAME", 1, 0, 105, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain unk_26C4FC("HELP", 1, 0, 125, 320, 1, &menuOrder, -1, NULL, 0);
CGameMenuItemChain unk_26C530("CREDITS", 1, 0, 145, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain unk_26C564("QUIT", 1, 0, 165, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle unk_26C598("BLOOD", 1, 160, 20, 2038);
CGameMenuItemChain unk_26C5BC("NEW GAME", 1, 0, 45, 320, 1, &unk_26B940, -1, NULL, 0);
CGameMenuItemChain unk_26C5F0("PLAY ONLINE", 1, 0, 60, 320, 1, &unk_26C090, -1, NULL, 0);
CGameMenuItemChain unk_26C624("OPTIONS", 1, 0, 75, 320, 1, &menuOptions, -1, NULL, 0);
CGameMenuItemChain unk_26C658("SAVE GAME", 1, 0, 90, 320, 1, &menuSaveGame, -1, SaveGameProcess, 0);
CGameMenuItemChain unk_26C68C("LOAD GAME", 1, 0, 105, 320, 1, &menuLoadGame, -1, NULL, 0);
CGameMenuItemChain unk_26C6C0("HELP", 1, 0, 120, 320, 1, &menuOrder, -1, NULL, 0);
CGameMenuItemChain unk_26C6F4("CREDITS", 1, 0, 135, 320, 1, &menuCredits, -1, NULL, 0);
CGameMenuItemChain unk_26C728("QUIT", 1, 0, 150, 320, 1, &menuQuit, -1, NULL, 0);

CGameMenuItemTitle unk_26C75C("EPISODES", 1, 160, 20, 2038);
CGameMenuItemChain7F2F0 itemEpisodes[6];

CGameMenuItemTitle unk_26C8D0("DIFFICULTY", 1, 160, 20, 2038);
CGameMenuItemChain unk_26C8F4("STILL KICKING", 1, 0, 60, 320, 1, NULL, -1, SetDifficultyAndStart, 0);
CGameMenuItemChain unk_26C928("PINK ON THE INSIDE", 1, 0, 80, 320, 1, NULL, -1, SetDifficultyAndStart, 1);
CGameMenuItemChain unk_26C95C("LIGHTLY BROILED", 1, 0, 100, 320, 1, NULL, -1, SetDifficultyAndStart, 2);
CGameMenuItemChain unk_26C990("WELL DONE", 1, 0, 120, 320, 1, NULL, -1, SetDifficultyAndStart, 3);
CGameMenuItemChain unk_26C9C4("EXTRA CRISPY", 1, 0, 140, 320, 1, 0, -1, SetDifficultyAndStart, 4);

CGameMenuItemTitle unk_26C9F8("OPTIONS", 1, 160, 20, 2038);
CGameMenuItemChain unk_26CA1C("CONTROLS...", 3, 0, 40, 320, 1, &unk_26BAF0, -1, NULL, 0);
CGameMenuItemSlider unk_26CA50("DETAIL:", 3, 66, 50, 180, gDetail, 0, 4, 1, SetDetail, -1, -1);
CGameMenuItemSlider unk_26CA90("GAMMA:", 3, 66, 60, 180, gGamma, 0, 15, 2, SetGamma, -1, -1);
CGameMenuItemSlider unk_26CAD0("MUSIC:", 3, 66, 70, 180, MusicVolume, 0, 256, 48, SetMusicVol, -1, -1);
CGameMenuItemSlider unk_26CB10("SOUND:", 3, 66, 80, 180, FXVolume, 0, 256, 48, SetSoundVol, -1, -1);
CGameMenuItemSlider unk_26CB50("CD AUDIO:", 3, 66, 90, 180, CDVolume, 0, 256, 48, SetCDVol, -1, -1);
CGameMenuItemZBool unk_26CB90("3D AUDIO:", 3, 66, 100, 180, gDoppler, SetDoppler, NULL, NULL);
CGameMenuItemZBool unk_26CBC0("CROSSHAIR:", 3, 66, 110, 180, gAimReticle, SetCrosshair, NULL, NULL);
CGameMenuItemZBool unk_26CBF0("SHOW WEAPONS:", 3, 66, 120, 180, gShowWeapon, SetShowWeapons, NULL, NULL);
CGameMenuItemZBool unk_26CC20("SLOPE TILTING:", 3, 66, 130, 180, gSlopeTilting, SetSlopeTilting, NULL, NULL);
CGameMenuItemZBool unk_26CC50("VIEW BOBBING:", 3, 66, 140, 180, gViewVBobbing, SetViewBobbing, NULL, NULL);
CGameMenuItemZBool unk_26CC80("VIEW SWAYING:", 3, 66, 150, 180, gViewHBobbing, SetViewSwaying, NULL, NULL);
CGameMenuItem7EE34 unk_26CCB0("VIDEO MODE...", 3, 0, 160, 320, 1);
CGameMenuItemChain unk_26CCE0("PARENTAL LOCK", 3, 0, 170, 320, 1, &unk_26C120, -1, NULL, 0);

CGameMenuItemTitle unk_26CD14("CONTROLS", 1, 160, 20, 2038);
CGameMenuItemSlider unk_26CD38("Mouse Sensitivity:", 1, 10, 70, 300, gMouseSensitivity, 0, 0x1000, 0x20000, SetMouseSensitivity, -1,-1);
CGameMenuItemZBool unk_26CD78("Invert Mouse Aim:", 1, 10, 90, 300, gMouseAimingFlipped, SetMouseAimFlipped, NULL, NULL);
CGameMenuItemSlider unk_26CDA8("Key Turn Speed:", 1, 10, 110, 300, gTurnSpeed, 64, 128, 4, SetTurnSpeed, -1, -1);
CGameMenuItemChain unk_26CDE8("Configure Keys...", 1, 0, 130, 320, 1, &unk_26BC10, -1, NULL, 0);
CGameMenuItemChain unk_26CE1C("Reset Keys...", 1, 0, 160, 320, 1, &unk_26BC10, -1, ResetKeys, 0);

CGameMenuItemTitle unk_26CE50("MESSAGES", 1, 160, 20, 2038);
CGameMenuItemZBool unk_26CE74("MESSAGES:", 3, 66, 70, 180, 0, SetMessages, NULL, NULL);
CGameMenuItemSlider unk_26CEA4("MESSAGE COUNT:", 3, 66, 80, 180, gMessageCount, 1, 16, 1, NULL, -1, -1);
CGameMenuItemSlider unk_26CEE4("MESSAGE TIME:", 3, 66, 90, 180, gMessageTime, 1, 8, 1, NULL, -1, -1);
CGameMenuItemZBool unk_26CF24("LARGE FONT:", 3, 66, 100, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool unk_26CF54("INCOMING:", 3, 66, 110, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool unk_26CF84("SELF PICKUP:", 3, 66, 120, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool unk_26CFB4("OTHER PICKUP:", 3, 66, 130, 180, 0, 0, NULL, NULL);
CGameMenuItemZBool unk_26CFE4("RESPAWN:", 3, 66, 140, 180, 0, 0, NULL, NULL);

CGameMenuItemTitle unk_26D014("KEY SETUP", 1, 160, 20, 2038);
CGameMenuItemKeyList unk_26D038("", 3, 56, 40, 200, 16, 54, 0);

CGameMenuItemTitle unk_26D074("Save Game", 1, 160, 20, 2038);
CGameMenuItemZEditBitmap unk_26D098(NULL, 3, 20, 60, 320, strRestoreGameStrings[0], 16, 1, SaveGame, 0);
CGameMenuItemZEditBitmap unk_26D0D0(NULL, 3, 20, 70, 320, strRestoreGameStrings[1], 16, 1, SaveGame, 1);
CGameMenuItemZEditBitmap unk_26D108(NULL, 3, 20, 80, 320, strRestoreGameStrings[2], 16, 1, SaveGame, 2);
CGameMenuItemZEditBitmap unk_26D140(NULL, 3, 20, 90, 320, strRestoreGameStrings[3], 16, 1, SaveGame, 3);
CGameMenuItemZEditBitmap unk_26D178(NULL, 3, 20, 100, 320, strRestoreGameStrings[4], 16, 1, SaveGame, 4);
CGameMenuItemZEditBitmap unk_26D1B0(NULL, 3, 20, 110, 320, strRestoreGameStrings[5], 16, 1, SaveGame, 5);
CGameMenuItemZEditBitmap unk_26D1E8(NULL, 3, 20, 120, 320, strRestoreGameStrings[6], 16, 1, SaveGame, 6);
CGameMenuItemZEditBitmap unk_26D220(NULL, 3, 20, 130, 320, strRestoreGameStrings[7], 16, 1, SaveGame, 7);
CGameMenuItemZEditBitmap unk_26D258(NULL, 3, 20, 140, 320, strRestoreGameStrings[8], 16, 1, SaveGame, 8);
CGameMenuItemZEditBitmap unk_26D290(NULL, 3, 20, 150, 320, strRestoreGameStrings[9], 16, 1, SaveGame, 9);
CGameMenuItemBitmapLS unk_26D2C8(NULL, 3, 0, 0, 2050);

CGameMenuItemTitle unk_26D2F4("Load Game", 1, 160, 20, 2038);
CGameMenuItemZEditBitmap unk_26D318(NULL, 3, 20, 60, 320, strRestoreGameStrings[0], 16, 1, LoadGame, 0);
CGameMenuItemZEditBitmap unk_26D350(NULL, 3, 20, 70, 320, strRestoreGameStrings[1], 16, 1, LoadGame, 1);
CGameMenuItemZEditBitmap unk_26D388(NULL, 3, 20, 80, 320, strRestoreGameStrings[2], 16, 1, LoadGame, 2);
CGameMenuItemZEditBitmap unk_26D3C0(NULL, 3, 20, 90, 320, strRestoreGameStrings[3], 16, 1, LoadGame, 3);
CGameMenuItemZEditBitmap unk_26D3F8(NULL, 3, 20, 100, 320, strRestoreGameStrings[4], 16, 1, LoadGame, 4);
CGameMenuItemZEditBitmap unk_26D430(NULL, 3, 20, 110, 320, strRestoreGameStrings[5], 16, 1, LoadGame, 5);
CGameMenuItemZEditBitmap unk_26D468(NULL, 3, 20, 120, 320, strRestoreGameStrings[6], 16, 1, LoadGame, 6);
CGameMenuItemZEditBitmap unk_26D4A0(NULL, 3, 20, 130, 320, strRestoreGameStrings[7], 16, 1, LoadGame, 7);
CGameMenuItemZEditBitmap unk_26D4D8(NULL, 3, 20, 140, 320, strRestoreGameStrings[8], 16, 1, LoadGame, 8);
CGameMenuItemZEditBitmap unk_26D510(NULL, 3, 20, 150, 320, strRestoreGameStrings[9], 16, 1, LoadGame, 9);
CGameMenuItemBitmapLS unk_26D548(NULL, 3, 0, 0, 2518);

CGameMenuItemTitle unk_26D574("NETWORK GAME", 1, 160, 20, 2038);
CGameMenuItemZCycle unk_26D598("GAME", 1, 20, 35, 280, 0, 0, zNetGameTypes, 3, 0);
CGameMenuItemZCycle unk_26D650("EPISODE", 1, 20, 50, 280, 0, SetupNetLevels, NULL, 0, 0);
CGameMenuItemZCycle unk_26D708("LEVEL", 1, 20, 65, 280, 0, NULL, NULL, 0, 0);
CGameMenuItemZCycle unk_26D7C0("DIFFICULTY", 1, 20, 80, 280, 0, 0, zDiffStrings, 5, 0);
CGameMenuItemZCycle unk_26D878("MONSTERS", 1, 20, 95, 280, 0, 0, zMonsterStrings, 3, 0);
CGameMenuItemZCycle unk_26D930("WEAPONS", 1, 20, 110, 280, 0, 0, zWeaponStrings, 4, 0);
CGameMenuItemZCycle unk_26D9E8("ITEMS", 1, 20, 125, 280, 0, 0, zItemStrings, 3, 0);
CGameMenuItemZEdit unk_26DAA0("USER MAP:", 1, 20, 155, 280, zUserMapName, 13, 0, NULL, 0);
CGameMenuItemChain unk_26DAD4("START GAME", 1, 20, 170, 280, 0, 0, -1, StartNetGame, 0);

CGameMenuItemText unk_26DB08("LOADING...", 1, 160, 100, 1);

CGameMenuItemTitle unk_26DB2C("SOUNDS", 1, 160, 20, 2038);
CGameMenuItemSlider unk_26DB50("MUSIC:", 3, 40, 60, 180, MusicVolume, 0, 256, 48, SetMusicVol, -1, -1);
CGameMenuItemSlider unk_26DB90("SOUND:", 3, 40, 70, 180, FXVolume, 0, 256, 48, SetSoundVol, -1, -1);
CGameMenuItemSlider unk_26DBD0("CD AUDIO:", 3, 40, 80, 180, CDVolume, 0, 256, 48, SetCDVol, -1, -1);
CGameMenuItemZBool unk_26DC10("3D SOUND:", 3, 40, 90, 180, gDoppler, SetDoppler, NULL, NULL);

CGameMenuItemTitle unk_26DC40("QUIT", 1, 160, 20, 2038);
CGameMenuItemText unk_26DC64("Do you really want to quit?", 0, 160, 100, 1);
CGameMenuItemYesNoQuit itemQuitYesNo("[Y/N]", 0, 20, 110, 280, 1, -1, 0);
CGameMenuItemPicCycle itemCreditsPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemPicCycle itemOrderPicCycle(0, 0, NULL, NULL, 0, 0);

CGameMenuItemTitle unk_26DE24("PARENTAL LOCK", 1, 160, 20, 2038);
CGameMenuItemZBool unk_26DE48("LOCK:", 3, 66, 70, 180, 0, SetParentalLock, NULL, NULL);
CGameMenuItemPassword dword_26DE78("SET PASSWORD:", 3, 160, 80);

CGameMenuItemPicCycle itemSorryPicCycle(0, 0, NULL, NULL, 0, 0);
CGameMenuItemText unk_26DF94("Loading and saving games", 0, 160, 90, 1);
CGameMenuItemText unk_26DFB8("not supported", 0, 160, 100, 1);
CGameMenuItemText unk_26DFDC("in this demo version of Blood.", 0, 160, 110, 1);

CGameMenuItemText unk_26E000("Buy the complete version of", 0, 160, 90, 1);
CGameMenuItemText unk_26E024("Blood for three new episodes", 0, 160, 100, 1);
CGameMenuItemText unk_26E048("plus eight BloodBath-only levels!", 0, 160, 110, 1);

CGameMenuItemTitle unk_26E06C(" ONLINE ", 1, 160, 20, 2038);
CGameMenuItem7EA1C unk_26E090("DWANGO", 1, 0, 45, 320, "matt", "DWANGO", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E0E8("RTIME", 1, 0, 65, 320, "matt", "RTIME", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E140("HEAT", 1, 0, 85, 320, "matt", "HEAT", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E198("KALI", 1, 0, 105, 320, "matt", "KALI", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E1F0("MPATH", 1, 0, 125, 320, "matt", "MPATH", 1, -1, NULL, NULL);
CGameMenuItem7EA1C unk_26E248("TEN", 1, 0, 145, 320, "matt", "TEN", 1, -1, TenProcess, NULL);

void SetupLoadingScreen(void)
{
    unk_26BDC0.Add(&unk_26DB08, true);
}

void SetupKeyListMenu(void)
{
    unk_26BC10.Add(&unk_26D014, false);
    unk_26BC10.Add(&unk_26D038, true);
    unk_26BC10.Add(&itemBloodQAV, false);
}

void SetupMessagesMenu(void)
{
    unk_26BB80.Add(&unk_26CE50, false);
    unk_26BB80.Add(&unk_26CE74, true);
    unk_26BB80.Add(&unk_26CEA4, false);
    unk_26BB80.Add(&unk_26CEE4, false);
    unk_26BB80.Add(&unk_26CF24, false);
    unk_26BB80.Add(&unk_26CF54, false);
    unk_26BB80.Add(&unk_26CF84, false);
    unk_26BB80.Add(&unk_26CFB4, false);
    unk_26BB80.Add(&unk_26CFE4, false);
    unk_26BB80.Add(&itemBloodQAV, false);
}

void SetupControlsMenu(void)
{
    unk_26CD38.at24 = ClipRange(gMouseSensitivity, unk_26CD38.at28, unk_26CD38.at2c);
    unk_26CDA8.at24 = ClipRange(gTurnSpeed, unk_26CDA8.at28, unk_26CDA8.at2c);
    unk_26CD78.at20 = gMouseAimingFlipped;
    unk_26BAF0.Add(&unk_26CD14, false);
    unk_26BAF0.Add(&unk_26CD38, true);
    unk_26BAF0.Add(&unk_26CD78, false);
    unk_26BAF0.Add(&unk_26CDA8, false);
    unk_26BAF0.Add(&unk_26CDE8, false);
    unk_26BAF0.Add(&unk_26CE1C, false);
    unk_26BAF0.Add(&itemBloodQAV, false);
}

void SetupOptionsMenu(void)
{
    unk_26CA50.at24 = ClipRange(gDetail, unk_26CA50.at28, unk_26CA50.at2c);
    unk_26CA90.at24 = ClipRange(gGamma, unk_26CA90.at28, unk_26CA90.at2c);
    unk_26CAD0.at24 = ClipRange(MusicVolume, unk_26CAD0.at28, unk_26CAD0.at2c);
    unk_26CB10.at24 = ClipRange(FXVolume, unk_26CB10.at28, unk_26CB10.at2c);
    // NUKE-TODO: unk_26CB50.at24 = ClipRange(Redbook.GetVolume(), unk_26CB50.at28, unk_26CB50.at2c);
    unk_26CB90.at20 = gDoppler;
    unk_26CBC0.at20 = gAimReticle;
    unk_26CBF0.at20 = gShowWeapon;
    unk_26CC20.at20 = gSlopeTilting;
    unk_26CC50.at20 = gViewVBobbing;
    unk_26CC80.at20 = gViewHBobbing;
    unk_26CE74.at20 = gGameMessageMgr.at0;
    menuOptions.Add(&unk_26C9F8, false);
    menuOptions.Add(&unk_26CA1C, true);
    menuOptions.Add(&unk_26CA50, false);
    menuOptions.Add(&unk_26CA90, false);
    menuOptions.Add(&unk_26CAD0, false);
    menuOptions.Add(&unk_26CB10, false);
    menuOptions.Add(&unk_26CB50, false);
    menuOptions.Add(&unk_26CB90, false);
    menuOptions.Add(&unk_26CBC0, false);
    menuOptions.Add(&unk_26CBF0, false);
    menuOptions.Add(&unk_26CC20, false);
    menuOptions.Add(&unk_26CC50, false);
    menuOptions.Add(&unk_26CC80, false);
    menuOptions.Add(&unk_26CCB0, false);
    menuOptions.Add(&unk_26CCE0, false);
    menuOptions.Add(&itemBloodQAV, false);
}

void SetupDifficultyMenu(void)
{
    unk_26B9D0.Add(&unk_26C8D0, false);
    unk_26B9D0.Add(&unk_26C8F4, false);
    unk_26B9D0.Add(&unk_26C928, false);
    unk_26B9D0.Add(&unk_26C95C, true);
    unk_26B9D0.Add(&unk_26C990, false);
    unk_26B9D0.Add(&unk_26C9C4, false);
    unk_26B9D0.Add(&itemBloodQAV, false);
}

void SetupEpisodeMenu(void)
{
    unk_26B940.Add(&unk_26C75C, false);
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
                pEpisodeItem->at8 = 1;
                pEpisodeItem->atc = 0;
                pEpisodeItem->at14 = 320;
                pEpisodeItem->at20 = 1;
                pEpisodeItem->at4 = pEpisode->at0;
                pEpisodeItem->at10 = 55+(height+8)*j;
                pEpisodeItem->at34 = i;
                if (!unk || j == 0)
                {
                    pEpisodeItem = &itemEpisodes[j];
                    pEpisodeItem->at24 = &unk_26B9D0;
                    pEpisodeItem->at28 = 3;
                }
                else
                {
                    pEpisodeItem->at24 = &unk_26C240;
                    pEpisodeItem->at28 = 1;
                }
                pEpisodeItem = &itemEpisodes[j];
                pEpisodeItem->at18 |= 3;
                bool first = j == 0;
                unk_26B940.Add(&itemEpisodes[j], first);
                if (first)
                    SetupLevelMenuItem(j);
            }
            j++;
        }
    }
    unk_26B940.Add(&itemBloodQAV, false);
}

void SetupMainMenu(void)
{
    menuMain.Add(&unk_26C408, false);
    menuMain.Add(&unk_26C42C, true);
    if (gGameOptions.nGameType > 0)
    {
        unk_26C42C.at24 = &unk_26B8B0;
        unk_26C42C.at28 = 2;
    }
    menuMain.Add(&unk_26C460, false);
    menuMain.Add(&unk_26C494, false);
    menuMain.Add(&unk_26C4C8, false);
    menuMain.Add(&unk_26C4FC, false);
    menuMain.Add(&unk_26C530, false);
    menuMain.Add(&unk_26C564, false);
    menuMain.Add(&itemBloodQAV, false);
}

void SetupMainMenuWithSave(void)
{
    menuMainWithSave.Add(&unk_26C598, false);
    menuMainWithSave.Add(&unk_26C5BC, true);
    if (gGameOptions.nGameType > 0)
    {
        unk_26C5BC.at24 = &unk_26B8B0;
        unk_26C5BC.at28 = 2;
    }
    menuMainWithSave.Add(&unk_26C5F0, false);
    menuMainWithSave.Add(&unk_26C624, false);
    menuMainWithSave.Add(&unk_26C658, false);
    menuMainWithSave.Add(&unk_26C68C, false);
    menuMainWithSave.Add(&unk_26C6C0, false);
    menuMainWithSave.Add(&unk_26C6F4, false);
    menuMainWithSave.Add(&unk_26C728, false);
    menuMainWithSave.Add(&itemBloodQAV, false);
}

void SetupNetStartMenu(void)
{
    bool oneEpisode = false;
    unk_26B8B0.Add(&unk_26D574, false);
    unk_26B8B0.Add(&unk_26D598, false);
    for (int i = 0; i < (oneEpisode ? 1 : 6); i++)
    {
        EPISODEINFO *pEpisode = &gEpisodeInfo[i];
        if (i < gEpisodeCount)
            unk_26D650.Add(pEpisode->at0, i == 0);
    }
    unk_26B8B0.Add(&unk_26D650, false);
    unk_26B8B0.Add(&unk_26D708, false);
    unk_26B8B0.Add(&unk_26D7C0, false);
    unk_26B8B0.Add(&unk_26D878, false);
    unk_26B8B0.Add(&unk_26D930, false);
    unk_26B8B0.Add(&unk_26D9E8, false);
    unk_26B8B0.Add(&unk_26DAA0, false);
    unk_26B8B0.Add(&unk_26DAD4, false);
    unk_26D598.SetTextIndex(1);
    unk_26D7C0.SetTextIndex(2);
    unk_26D878.SetTextIndex(0);
    unk_26D930.SetTextIndex(1);
    unk_26D9E8.SetTextIndex(1);
    unk_26B8B0.Add(&itemBloodQAV, false);
}

void SetupSaveGameMenu(void)
{
    menuSaveGame.Add(&unk_26D074, false);
    menuSaveGame.Add(&unk_26D098, true);
    menuSaveGame.Add(&unk_26D0D0, false);
    menuSaveGame.Add(&unk_26D108, false);
    menuSaveGame.Add(&unk_26D140, false);
    menuSaveGame.Add(&unk_26D178, false);
    menuSaveGame.Add(&unk_26D1B0, false);
    menuSaveGame.Add(&unk_26D1E8, false);
    menuSaveGame.Add(&unk_26D220, false);
    menuSaveGame.Add(&unk_26D258, false);
    menuSaveGame.Add(&unk_26D290, false);
    menuSaveGame.Add(&unk_26D2C8, false);
    menuSaveGame.Add(&itemBloodQAV, false);

    unk_26D098.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[0], "<Empty>"))
        unk_26D098.at37 = 1;

    unk_26D0D0.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[1], "<Empty>"))
        unk_26D0D0.at37 = 1;

    unk_26D108.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[2], "<Empty>"))
        unk_26D108.at37 = 1;

    unk_26D140.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[3], "<Empty>"))
        unk_26D140.at37 = 1;

    unk_26D178.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[4], "<Empty>"))
        unk_26D178.at37 = 1;

    unk_26D1B0.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[5], "<Empty>"))
        unk_26D1B0.at37 = 1;

    unk_26D1E8.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[6], "<Empty>"))
        unk_26D1E8.at37 = 1;

    unk_26D220.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[7], "<Empty>"))
        unk_26D220.at37 = 1;

    unk_26D258.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[8], "<Empty>"))
        unk_26D258.at37 = 1;

    unk_26D290.at2c = &unk_26D2C8;
    if (!strcmp(strRestoreGameStrings[9], "<Empty>"))
        unk_26D290.at37 = 1;
}

void SetupLoadGameMenu(void)
{
    menuLoadGame.Add(&unk_26D2F4, false);
    menuLoadGame.Add(&unk_26D318, true);
    menuLoadGame.Add(&unk_26D350, false);
    menuLoadGame.Add(&unk_26D388, false);
    menuLoadGame.Add(&unk_26D3C0, false);
    menuLoadGame.Add(&unk_26D3F8, false);
    menuLoadGame.Add(&unk_26D430, false);
    menuLoadGame.Add(&unk_26D468, false);
    menuLoadGame.Add(&unk_26D4A0, false);
    menuLoadGame.Add(&unk_26D4D8, false);
    menuLoadGame.Add(&unk_26D510, false);
    menuLoadGame.Add(&unk_26D548, false);
    unk_26D318.at35 = 0;
    unk_26D350.at35 = 0;
    unk_26D388.at35 = 0;
    unk_26D3C0.at35 = 0;
    unk_26D3F8.at35 = 0;
    unk_26D430.at35 = 0;
    unk_26D468.at35 = 0;
    unk_26D4A0.at35 = 0;
    unk_26D4D8.at35 = 0;
    unk_26D510.at35 = 0;
    unk_26D318.at2c = &unk_26D548;
    unk_26D350.at2c = &unk_26D548;
    unk_26D388.at2c = &unk_26D548;
    unk_26D3C0.at2c = &unk_26D548;
    unk_26D3F8.at2c = &unk_26D548;
    unk_26D430.at2c = &unk_26D548;
    unk_26D468.at2c = &unk_26D548;
    unk_26D4A0.at2c = &unk_26D548;
    unk_26D4D8.at2c = &unk_26D548;
    unk_26D510.at2c = &unk_26D548;
    menuLoadGame.Add(&itemBloodQAV, false);
}

void SetupSoundsMenu(void)
{
    unk_26DB50.at24 = ClipRange(MusicVolume, unk_26DB50.at28, unk_26DB50.at2c);
    unk_26DB90.at24 = ClipRange(FXVolume, unk_26DB90.at28, unk_26DB90.at2c);
    // NUKE-TODO: unk_26DB90.at24 = ClipRange(Redbook.GetVolume(), unk_26DB90.at28, unk_26DB90.at2c);
    menuSounds.Add(&unk_26DB2C, false);
    menuSounds.Add(&unk_26DB50, true);
    menuSounds.Add(&unk_26DB90, false);
    menuSounds.Add(&unk_26DBD0, false);
    menuSounds.Add(&unk_26DC10, false);
    menuSounds.Add(&itemBloodQAV, false);
}

void SetupQuitMenu(void)
{
    menuQuit.Add(&unk_26DC40, false);
    menuQuit.Add(&unk_26DC64, false);
    menuQuit.Add(&itemQuitYesNo, true);
    menuQuit.Add(&itemBloodQAV, false);
}

void SetupHelpOrderMenu(void)
{
    menuOrder.Add(&unk_26C3A0, true);
    menuOrder.Add(&unk_26C3D4, false);
    menuOrder.Add(&unk_26C338, false);
    menuOrder.Add(&unk_26C36C, false);
    unk_26C3A0.at18 |= 10;
    unk_26C3D4.at18 |= 10;
    unk_26C338.at18 |= 10;
    unk_26C36C.at18 |= 10;
}

void SetupCreditsMenu(void)
{
    menuCredits.Add(&unk_26C304, true);
    unk_26C304.at18 |= 10;
}

void SetupParentalLockMenu(void)
{
    unk_26DE48.at20 = gbAdultContent;
    strcpy(dword_26DE78.at20, gzAdultPassword);
    unk_26C120.Add(&unk_26DE24, false);
    unk_26C120.Add(&unk_26DE48, true);
    unk_26C120.Add(&dword_26DE78, false);
    unk_26C120.Add(&itemBloodQAV, false);
}

void SetupSorry3Menu(void)
{
    unk_26C090.Add(&unk_26E06C, false);
    unk_26C090.Add(&unk_26E090, true);
    unk_26C090.Add(&unk_26E0E8, false);
    unk_26C090.Add(&unk_26E140, false);
    unk_26C090.Add(&unk_26E198, false);
    unk_26C090.Add(&unk_26E1F0, false);
    unk_26C090.Add(&unk_26E248, false);
    unk_26C090.Add(&itemBloodQAV, false);
}

void SetupSorryMenu(void)
{
    unk_26C1B0.Add(&itemSorryPicCycle, true);
    unk_26C1B0.Add(&unk_26DF94, false);
    unk_26C1B0.Add(&unk_26DFDC, false);
    unk_26C1B0.Add(&itemBloodQAV, false);
}

void SetupSorry2Menu(void)
{
    unk_26C240.Add(&itemSorryPicCycle, true);
    unk_26C240.Add(&unk_26E000, false);
    unk_26C240.Add(&unk_26E024, false);
    unk_26C240.Add(&unk_26E048, false);
    unk_26C240.Add(&itemBloodQAV, false);
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
    // NUKE-TODO: CONFIG_ResetKeys();
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
    gDetail = pItem->at24;
}

void SetGamma(CGameMenuItemSlider *pItem)
{
    gGamma = pItem->at24;
    scrSetGamma(gGamma);
}

void SetMusicVol(CGameMenuItemSlider *pItem)
{
    sndSetMusicVolume(pItem->at24);
}

void SetSoundVol(CGameMenuItemSlider *pItem)
{
    sndSetFXVolume(pItem->at24);
}

void SetCDVol(CGameMenuItemSlider *pItem)
{
    // NUKE-TODO:
}

void SetMessages(CGameMenuItemZBool *pItem)
{
    gGameMessageMgr.SetState(pItem->at20);
}

void SetMouseSensitivity(CGameMenuItemSlider *pItem)
{
    // NUKE-TODO:
	CONTROL_MouseSensitivity = pItem->at24;
}

void SetMouseAimFlipped(CGameMenuItemZBool *pItem)
{
    gMouseAimingFlipped = pItem->at20;
}

void SetTurnSpeed(CGameMenuItemSlider *pItem)
{
    gTurnSpeed = pItem->at24;
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
    unk_26D708.SetTextArray((const char**)zLevelNames[nEpisode], gEpisodeInfo[nEpisode].nLevels, 0);
}

void SetupNetLevels(CGameMenuItemZCycle *pItem)
{
    SetupLevelMenuItem(pItem->at24);
}

void StartNetGame(CGameMenuItemChain *)
{
    gPacketStartGame.gameType = unk_26D598.at24+1;
    if (gPacketStartGame.gameType == 0)
        gPacketStartGame.gameType = 2;
    gPacketStartGame.episodeId = unk_26D650.at24;
    gPacketStartGame.levelId = unk_26D708.at24;
    gPacketStartGame.difficulty = unk_26D7C0.at24;
    gPacketStartGame.monsterSettings = unk_26D878.at24;
    gPacketStartGame.weaponSettings = unk_26D930.at24;
    gPacketStartGame.itemSettings = unk_26D9E8.at24;
    gPacketStartGame.respawnSettings = 0;
    gPacketStartGame.unk = 0;
    gPacketStartGame.userMapName[0] = 0;
    strncpy(gPacketStartGame.userMapName, unk_26DAA0.at20, 13);
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
        if (strcmp(dword_26DE78.at20, ""))
        {
            dword_26DE78.pMenu->FocusNextItem();
            dword_26DE78.at32 = 0;
            dword_26DE78.at37 = 1;
            dword_26DE78.at5f = pItem;
            dword_26DE78.at29[0] = 0;
            return;
        }
        else
        {
            dword_26DE78.at20[0] = 0;
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

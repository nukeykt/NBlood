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
#pragma once
#include "common_game.h"
#include "inifile.h"

#define kMaxMessages 32

#pragma pack(push, 1)

struct GAMEOPTIONS {
    char nGameType;
    char nDifficulty;
    int nEpisode;
    int nLevel;
    char zLevelName[144];
    char zLevelSong[144];
    int nTrackNumber; //at12a;
    char szSaveGameName[16];
    char szUserGameName[16];
    short nSaveGameSlot;
    int picEntry;
    unsigned int uMapCRC;
    char nMonsterSettings;
    int uGameFlags;
    int uNetGameFlags;
    char nWeaponSettings;
    char nItemSettings;
    char nRespawnSettings;
    char nTeamSettings;
    int nMonsterRespawnTime;
    int nWeaponRespawnTime;
    int nItemRespawnTime;
    int nSpecialRespawnTime;
};

#pragma pack(pop)

struct LEVELINFO
{
    char at0[144]; // Filename
    char at90[32]; // Title
    char atb0[32]; // Author
    char atd0[16]; // Song;
    int ate0; // SongId
    int ate4; // EndingA
    int ate8; // EndingB
    char atec[kMaxMessages][64]; // Messages
    char at8ec; // Fog
    char at8ed; // Weather
}; // 0x8ee bytes

struct EPISODEINFO
{
    char at0[32];
    int nLevels;
    unsigned int bloodbath : 1;
    unsigned int cutALevel : 4;
    LEVELINFO at28[16];
    char at8f08[144];
    char at8f98[144];
    int at9028;
    int at902c;
    char at9030[144];
    char at90c0[144];
};

extern EPISODEINFO gEpisodeInfo[];
extern GAMEOPTIONS gSingleGameOptions;
extern GAMEOPTIONS gGameOptions;
extern int gSkill;
extern char BloodIniFile[];
extern int gEpisodeCount;
extern int gNextLevel;
extern bool gGameStarted;

void sub_26988(void);
void sub_269D8(const char *pzIni);
void levelPlayIntroScene(int nEpisode);
void levelPlayEndScene(int nEpisode);
void levelClearSecrets(void);
void levelSetupSecret(int nCount);
void levelTriggerSecret(int nSecret);
void CheckSectionAbend(const char *pzSection);
void CheckKeyAbend(const char *pzSection, const char *pzKey);
LEVELINFO * levelGetInfoPtr(int nEpisode, int nLevel);
char * levelGetFilename(int nEpisode, int nLevel);
char * levelGetMessage(int nMessage);
char * levelGetTitle(void);
char * levelGetAuthor(void);
void levelSetupOptions(int nEpisode, int nLevel);
void levelLoadMapInfo(IniFile *pIni, LEVELINFO *pLevelInfo, const char *pzSection);
void levelLoadDefaults(void);
void levelAddUserMap(const char *pzMap);
void levelGetNextLevels(int nEpisode, int nLevel, int *pnEndingA, int *pnEndingB);
void levelEndLevel(int arg);
void levelRestart(void);

//
// Definitions of common game-only data structures/functions
// (and declarations of data appearing in both)
// for EDuke32 and Mapster32
//

#ifndef TEKWAR_COMMON_GAME_H_
#define TEKWAR_COMMON_GAME_H_
#pragma once

#include "build.h"

#include "collections.h"
#include "grpscan.h"

#include "vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

    extern int g_useCwd;

#ifndef APPNAME
#define APPNAME             "ETekWar"
#endif

#ifndef APPBASENAME
#define APPBASENAME         "etekwar"
#endif

#define GAMEFLAG_TEKWAR     0x00000001
#define GAMEFLAGMASK        0x000000FF // flags allowed from grpinfo

extern struct grpfile_t const* g_selectedGrp;

extern int32_t g_gameType;
extern int     g_addonNum;

#define WH1               (g_gameType & GAMEFLAG_WH1)
#define WH2               (g_gameType & GAMEFLAG_WH2)


enum Games_t {
    GAME_TEKWAR = 0,
    GAMECOUNT
};

enum searchpathtypes_t {
    SEARCHPATH_REMOVE = 1 << 0,
    SEARCHPATH_NAM = 1 << 1,
    SEARCHPATH_WW2GI = 1 << 2,
    SEARCHPATH_FURY = 1 << 3,
};

enum basepal_t {
    BASEPAL = 0,
    ANIMPAL,
    BASEPALCOUNT
};

#define OSDTEXT_DEFAULT   "^00"
#define OSDTEXT_DARKRED   "^10"
#define OSDTEXT_GREEN     "^11"
#define OSDTEXT_RED       "^21"
#define OSDTEXT_YELLOW    "^23"

#define OSDTEXT_BRIGHT    "^S0"

#define OSD_ERROR OSDTEXT_DARKRED OSDTEXT_BRIGHT

extern const char* g_gameNamePtr;

extern char* g_grpNamePtr;
extern char* g_scriptNamePtr;
extern char* g_rtsNamePtr;

extern const char* G_DefaultGrpFile(void);
extern const char* G_GrpFile(void);

extern const char* G_DefaultConFile(void);
extern const char* G_ConFile(void);

extern GrowArray<char*> g_scriptModules;

extern void G_AddCon(const char* buffer);
extern void G_AddConModule(const char* buffer);

extern void clearGrpNamePtr(void);
extern void clearScriptNamePtr(void);

extern int loaddefinitions_game(const char* fn, int32_t preload);
extern int32_t g_groupFileHandle;

//////////

extern void G_InitMultiPsky(int CLOUDYOCEAN__DYN, int MOONSKY1__DYN, int BIGORBIT1__DYN, int LA__DYN);
extern void G_SetupGlobalPsky(void);

//////////

extern char g_modDir[BMAX_PATH];
extern buildvfs_kfd kopen4loadfrommod(const char* filename, char searchfirst);
extern void G_AddSearchPaths(void);
extern void G_CleanupSearchPaths(void);

extern void G_ExtPreInit(int32_t argc, char const* const* argv);
extern void G_ExtInit(void);
extern void G_ScanGroups(void);
extern void G_LoadGroups(int32_t autoload);

extern const char* G_GetInstallPath(int32_t insttype);

//////////

void G_LoadGroupsInDir(const char* dirname);
void G_DoAutoload(const char* dirname);

//////////

extern void G_LoadLookups(void);

//////////

#if defined HAVE_FLAC || defined HAVE_VORBIS
# define FORMAT_UPGRADE_ELIGIBLE
    extern int g_maybeUpgradeSoundFormats;
    extern buildvfs_kfd S_OpenAudio(const char* fn, char searchfirst, uint8_t ismusic);
#else
# define S_OpenAudio(fn, searchfirst, ismusic) kopen4loadfrommod(fn, searchfirst)
#endif

#ifdef __cplusplus
}
#endif

#endif

//
// Definitions of common game-only data structures/functions
// (and declarations of data appearing in both)
// for EDuke32 and Mapster32
//

#ifndef EXHUMED_COMMON_GAME_H_
#define EXHUMED_COMMON_GAME_H_
#pragma once

#include "build.h"

#include "collections.h"
#include "grpscan.h"

#include "vfs.h"

#if defined HAVE_FLAC || defined HAVE_VORBIS
# define FORMAT_UPGRADE_ELIGIBLE
extern int32_t S_OpenAudio(const char* fn, char searchfirst, uint8_t ismusic);
#else
# define S_OpenAudio(fn, searchfirst, ismusic) kopen4loadfrommod(fn, searchfirst)
#endif

#define G_ModDirSnprintf(buf, size, basename, ...)                                                                                          \
    (((g_modDir[0] != '/') ? Bsnprintf(buf, size, "%s/" basename, g_modDir, ##__VA_ARGS__) : Bsnprintf(buf, size, basename, ##__VA_ARGS__)) \
     >= ((int32_t)size) - 1)

#define G_ModDirSnprintfLite(buf, size, basename) \
    ((g_modDir[0] != '/') ? Bsnprintf(buf, size, "%s/%s", g_modDir, basename) : Bsnprintf(buf, size, "%s", basename))


#ifdef __cplusplus
extern "C" {
    #endif

    extern int g_useCwd;

    #ifndef APPNAME
    #define APPNAME             "PCExhumed"
    #endif

    #ifndef APPBASENAME
    #define APPBASENAME         "pcexhumed"
    #endif

    extern struct grpfile_t const* g_selectedGrp;

    extern int32_t g_gameType;
    extern int     g_addonNum;

    #define POWERSLAVE  (g_gameType & GAMEFLAG_POWERSLAVE)
    #define EXHUMED     (g_gameType & GAMEFLAG_EXHUMED)
    #define ISDEMOVER   (g_gameType & GAMEFLAG_DEMO)

    enum basepal_t
    {
        BASEPAL = 0,
        ANIMPAL,
        BASEPALCOUNT
    };

    #define OSDTEXT_DEFAULT   "^00"
    #define OSDTEXT_DARKRED   "^00"
    #define OSDTEXT_GREEN     "^00"
    #define OSDTEXT_RED       "^00"
    #define OSDTEXT_YELLOW    "^00"

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

    extern void G_AddSearchPaths(void);
    extern void G_CleanupSearchPaths(void);

    extern int32_t g_groupFileHandle;

    //////////

    extern void G_InitMultiPsky(int CLOUDYOCEAN__DYN, int MOONSKY1__DYN, int BIGORBIT1__DYN, int LA__DYN);
    extern void G_SetupGlobalPsky(void);

    //////////

    extern char g_modDir[BMAX_PATH];
    extern buildvfs_kfd kopen4loadfrommod(const char* filename, char searchfirst);
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
    #ifdef __cplusplus
}
#endif

#endif

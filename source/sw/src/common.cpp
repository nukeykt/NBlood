
#include "build.h"
#include "baselayer.h"

#ifdef _WIN32
# include "windows_inc.h"
# include "winbits.h"
#elif defined __APPLE__
# include "osxbits.h"
#endif

#include "common.h"
#include "common_game.h"
#include "grpscan.h"

uint8_t SW_GameFlags;

static const char *defaultgrpfilename = "SW.GRP";
static const char *defaultdeffilename = "sw.def";

// g_grpNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char *g_grpNamePtr = NULL;

void clearGrpNamePtr(void)
{
    Xfree(g_grpNamePtr);
    // g_grpNamePtr assumed to be assigned to right after
}

const char *G_DefaultGrpFile(void)
{
    return defaultgrpfilename;
}
const char *G_GrpFile(void)
{
    if (g_grpNamePtr == NULL)
        return G_DefaultGrpFile();
    else
        return g_grpNamePtr;
}

const char *G_DefaultDefFile(void)
{
    return defaultdeffilename;
}
const char *G_DefFile(void)
{
    if (g_defNamePtr == NULL)
        return G_DefaultDefFile();
    else
        return g_defNamePtr;
}


void SW_InitMultiPsky(void)
{
    // default
    psky_t * const defaultsky = tileSetupSky(DEFAULTPSKY);
    defaultsky->lognumtiles = 1;
    defaultsky->horizfrac = 8192;
}

int g_useCwd;
static char g_rootDir[BMAX_PATH];

void SW_ExtPreInit(int32_t argc, char const * const * argv)
{
    g_useCwd = G_CheckCmdSwitch(argc, argv, "-usecwd");

#ifdef _WIN32
    GetModuleFileName(NULL,g_rootDir,BMAX_PATH);
    Bcorrectfilename(g_rootDir,1);
    //buildvfs_chdir(g_rootDir);
#else
    buildvfs_getcwd(g_rootDir,BMAX_PATH);
    strcat(g_rootDir,"/");
#endif
}

#ifndef EDUKE32_STANDALONE

#if defined _WIN32 || defined __linux__ || defined EDUKE32_BSD || defined EDUKE32_OSX
static int32_t SW_Add_GOG_SWCR(const char * path)
{
    char buf[BMAX_PATH];

    addsearchpath_user(path, SEARCHPATH_REMOVE);
    Bsnprintf(buf, sizeof(buf), "%s/addons", path);
    addsearchpath_user(buf, SEARCHPATH_REMOVE);
    Bsnprintf(buf, sizeof(buf), "%s/music", path);
    return addsearchpath(buf);
}
static int32_t SW_Add_GOG_SWCC(const char * path)
{
    char buf[BMAX_PATH];

    addsearchpath_user(path, SEARCHPATH_REMOVE);
    Bsnprintf(buf, sizeof(buf), "%s/MUSIC", path);
    return addsearchpath(buf);
}
#endif

#if defined __linux__ || defined EDUKE32_BSD
static void SW_Add_GOG_SWCR_Linux(const char * path)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "%s/game", path);
    SW_Add_GOG_SWCR(buf);
}
static void SW_Add_GOG_SWCC_Linux(const char * path)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "%s/data", path);
    SW_Add_GOG_SWCC(buf);
}
#endif

#ifndef EDUKE32_TOUCH_DEVICES
#if defined EDUKE32_OSX || defined __linux__ || defined EDUKE32_BSD
static void SW_AddSteamPaths(const char *basepath)
{
    char buf[BMAX_PATH];

    // Shadow Warrior Classic Redux - Steam
    static char const s_SWCR_Steam[] = "steamapps/common/Shadow Warrior Classic/gameroot";
    Bsnprintf(buf, sizeof(buf), "%s/%s", basepath, s_SWCR_Steam);
    addsearchpath_user(buf, SEARCHPATH_REMOVE);
    Bsnprintf(buf, sizeof(buf), "%s/%s/addons", basepath, s_SWCR_Steam);
    addsearchpath_user(buf, SEARCHPATH_REMOVE);
    Bsnprintf(buf, sizeof(buf), "%s/%s/classic/MUSIC", basepath, s_SWCR_Steam);
    if (addsearchpath(buf) == 0)
        return;

    // Shadow Warrior Classic (1997) - Steam
    static char const s_SWC_Steam[] = "steamapps/common/Shadow Warrior Original/gameroot";
    Bsnprintf(buf, sizeof(buf), "%s/%s", basepath, s_SWC_Steam);
    addsearchpath_user(buf, SEARCHPATH_REMOVE);
    Bsnprintf(buf, sizeof(buf), "%s/%s/MUSIC", basepath, s_SWC_Steam);
    if (addsearchpath(buf) == 0)
        return;

    // Shadow Warrior (Classic) - 3D Realms Anthology - Steam
#if defined EDUKE32_OSX
    Bsnprintf(buf, sizeof(buf), "%s/steamapps/common/Shadow Warrior DOS/Shadow Warrior.app/Contents/Resources/sw", basepath);
    addsearchpath(buf);
#endif
}
#endif
#endif
#endif

#if defined __linux__ || defined EDUKE32_BSD || defined EDUKE32_OSX
static void SW_AddPathAndMusic(const char *path)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "%s", path);
    addsearchpath(buf);
    Bsnprintf(buf, sizeof(buf), "%s/music", path);
    addsearchpath(buf);
}
#endif

static void SW_AddSearchPaths()
{
#ifndef EDUKE32_STANDALONE
#ifndef EDUKE32_TOUCH_DEVICES
#if defined __linux__ || defined EDUKE32_BSD
    char buf[BMAX_PATH];
    char *homepath = Bgethomedir();
    const char *xdg_docs_path = getenv("XDG_DOCUMENTS_DIR");
    const char *xdg_config_path = getenv("XDG_CONFIG_HOME");

    // Steam
    Bsnprintf(buf, sizeof(buf), "%s/.steam/steam", homepath);
    SW_AddSteamPaths(buf);

    Bsnprintf(buf, sizeof(buf), "%s/.steam/steam/steamapps/libraryfolders.vdf", homepath);
    Paths_ParseSteamLibraryVDF(buf, SW_AddSteamPaths);

    // Steam Flatpak
    Bsnprintf(buf, sizeof(buf), "%s/.var/app/com.valvesoftware.Steam/.steam/steam", homepath);
    SW_AddSteamPaths(buf);

    Bsnprintf(buf, sizeof(buf), "%s/.var/app/com.valvesoftware.Steam/.steam/steam/steamapps/libraryfolders.vdf", homepath);
    Paths_ParseSteamLibraryVDF(buf, SW_AddSteamPaths);

    // Shadow Warrior Classic Redux - GOG.com
    Bsnprintf(buf, sizeof(buf), "%s/GOG Games/Shadow Warrior Classic Redux", homepath);
    SW_Add_GOG_SWCR_Linux(buf);
    Paths_ParseXDGDesktopFilesFromGOG(homepath, "Shadow_Warrior_Classic_Redux", SW_Add_GOG_SWCR_Linux);

    // Shadow Warrior Classic Complete - GOG.com
    Bsnprintf(buf, sizeof(buf), "%s/GOG Games/Shadow Warrior Classic Complete", homepath);
    SW_Add_GOG_SWCC_Linux(buf);
    Paths_ParseXDGDesktopFilesFromGOG(homepath, "Shadow_Warrior_Classic_Complete", SW_Add_GOG_SWCC_Linux);

    if (xdg_config_path) {
        Bsnprintf(buf, sizeof(buf), "%s/" APPBASENAME, xdg_config_path);
        SW_AddPathAndMusic(buf);
    }

    if (xdg_docs_path) {
        Bsnprintf(buf, sizeof(buf), "%s/" APPNAME, xdg_docs_path);
        SW_AddPathAndMusic(buf);
    }
    else {
        Bsnprintf(buf, sizeof(buf), "%s/Documents/" APPNAME, homepath);
        SW_AddPathAndMusic(buf);
    }

    Xfree(homepath);

    SW_AddPathAndMusic("/usr/share/games/jfsw");
    SW_AddPathAndMusic("/usr/local/share/games/jfsw");
    SW_AddPathAndMusic("/usr/share/games/" APPBASENAME);
    SW_AddPathAndMusic("/usr/local/share/games/" APPBASENAME);
    SW_AddPathAndMusic("/app/extensions/extra");
#elif defined EDUKE32_OSX
    char buf[BMAX_PATH];
    int32_t i;
    char *applications[] = { osx_getapplicationsdir(0), osx_getapplicationsdir(1) };
    char *support[] = { osx_getsupportdir(0), osx_getsupportdir(1) };

    for (i = 0; i < 2; i++)
    {
        Bsnprintf(buf, sizeof(buf), "%s/Steam", support[i]);
        SW_AddSteamPaths(buf);

        Bsnprintf(buf, sizeof(buf), "%s/Steam/steamapps/libraryfolders.vdf", support[i]);
        Paths_ParseSteamLibraryVDF(buf, SW_AddSteamPaths);

        // Shadow Warrior Classic Complete - GOG.com
        Bsnprintf(buf, sizeof(buf), "%s/Shadow Warrior.app/Contents/Resources/Shadow Warrior.boxer/C swarrior_files.harddisk", applications[i]);
        SW_Add_GOG_SWCC(buf);
        Bsnprintf(buf, sizeof(buf), "%s/Shadow Warrior Complete/Shadow Warrior.app/Contents/Resources/Shadow Warrior.boxer/C swarrior_files.harddisk", applications[i]);
        SW_Add_GOG_SWCC(buf);
        Bsnprintf(buf, sizeof(buf), "%s/Shadow Warrior Complete.app/Contents/Resources/game/Shadow Warrior.app/Contents/Resources/Shadow Warrior.boxer/C swarrior_files.harddisk", applications[i]);
        SW_Add_GOG_SWCC(buf);

        // Shadow Warrior Classic Redux - GOG.com
        Bsnprintf(buf, sizeof(buf), "%s/Shadow Warrior Classic Redux.app/Contents/Resources/gameroot", applications[i]);
        SW_Add_GOG_SWCR(buf);
        Bsnprintf(buf, sizeof(buf), "%s/Shadow Warrior Classic Redux/Shadow Warrior Classic Redux.app/Contents/Resources/gameroot", applications[i]);
        SW_Add_GOG_SWCR(buf);
        Bsnprintf(buf, sizeof(buf), "%s/Shadow Warrior Classic Redux.app/Contents/Resources/game/Shadow Warrior Classic Redux.app/Contents/Resources/gameroot", applications[i]);
        SW_Add_GOG_SWCR(buf);
    }

    for (i = 0; i < 2; i++)
    {
        Bsnprintf(buf, sizeof(buf), "%s/JFSW", support[i]);
        SW_AddPathAndMusic(buf);
        Bsnprintf(buf, sizeof(buf), "%s/" APPNAME, support[i]);
        SW_AddPathAndMusic(buf);
    }

    for (i = 0; i < 2; i++)
    {
        Xfree(applications[i]);
        Xfree(support[i]);
    }
#elif defined (_WIN32)
    char buf[BMAX_PATH] = {0};
    DWORD bufsize;

    // Shadow Warrior Classic Redux - Steam
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 225160)", "InstallLocation", buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        Bstrncpy(suffix, "/gameroot", remaining);
        addsearchpath_user(buf, SEARCHPATH_REMOVE);
        Bstrncpy(suffix, "/gameroot/addons", remaining);
        addsearchpath_user(buf, SEARCHPATH_REMOVE);
        Bstrncpy(suffix, "/gameroot/classic/MUSIC", remaining);
        if (addsearchpath(buf) == 0)
            return;
    }

    // Shadow Warrior Classic (1997) - Steam
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 238070)", "InstallLocation", buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        Bstrncpy(suffix, "/gameroot", remaining);
        addsearchpath_user(buf, SEARCHPATH_REMOVE);
        Bstrncpy(suffix, "/gameroot/MUSIC", remaining);
        if (addsearchpath(buf) == 0)
            return;
    }

    // Shadow Warrior Classic Redux - GOG.com
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\GOG.com\Games\1618073558)", "PATH", buf, &bufsize))
    {
        if (SW_Add_GOG_SWCR(buf) == 0)
            return;
    }

    // Shadow Warrior Classic Complete - GOG.com
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\GOG.com\Games\1207659142)", "PATH", buf, &bufsize))
    {
        if (SW_Add_GOG_SWCC(buf) == 0)
            return;
    }
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue("SOFTWARE\\GOG.com\\GOGSHADOWARRIOR", "PATH", buf, &bufsize))
    {
        if (SW_Add_GOG_SWCC(buf) == 0)
            return;
    }

    // Shadow Warrior Classic Complete - ZOOM Platform
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\ZOOM PLATFORM\Shadow Warrior Classic Complete)", "InstallPath", buf, &bufsize))
    {
        if (SW_Add_GOG_SWCC(buf) == 0)
            return;
    }

    // Shadow Warrior (Classic) - 3D Realms Anthology - Steam
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 358400)", "InstallLocation", buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        Bstrncpy(suffix, "/Shadow Warrior", remaining);
        addsearchpath(buf);
    }

    // Shadow Warrior - 3D Realms Anthology
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue("SOFTWARE\\3DRealms\\Shadow Warrior", NULL, buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        Bstrncpy(suffix, "/Shadow Warrior", remaining);
        addsearchpath(buf);
    }

    // 3D Realms Anthology
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue("SOFTWARE\\3DRealms\\Anthology", NULL, buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        Bstrncpy(suffix, "/Shadow Warrior", remaining);
        addsearchpath(buf);
    }
#endif
#endif
#endif
}

void SW_CleanupSearchPaths()
{
    removesearchpaths_withuser(SEARCHPATH_REMOVE);
}

static inline void SW_AddMusicFolder()
{
#ifndef _WIN32
    if (addsearchpath("MUSIC") == 0)
        return;
#endif
    addsearchpath("music");
}

void SW_ExtInit()
{
    if (!g_useCwd)
        SW_AddSearchPaths();

#ifdef EDUKE32_OSX
    char *appdir = Bgetappdir();
    addsearchpath(appdir);
    Xfree(appdir);
#endif

    char cwd[BMAX_PATH];
#ifdef USE_PHYSFS
    strncpy(cwd, PHYSFS_getBaseDir(), ARRAY_SIZE(cwd));
    cwd[ARRAY_SIZE(cwd)-1] = '\0';
#else
    if (buildvfs_getcwd(cwd, ARRAY_SIZE(cwd)) && Bstrcmp(cwd, "/") != 0)
#endif
        addsearchpath(cwd);
#if defined(_WIN32) && !defined(EDUKE32_STANDALONE)
    if (buildvfs_exists("user_profiles_enabled"))
#else
    if (g_useCwd == 0 && !buildvfs_exists("user_profiles_disabled"))
#endif
    {
        char *homedir;
        int32_t asperr;

        if ((homedir = Bgethomedir()))
        {
            Bsnprintf(cwd, ARRAY_SIZE(cwd), "%s/"
#if defined(_WIN32)
                      APPNAME
#elif defined(GEKKO)
                      "apps/" APPBASENAME
#else
                      ".config/" APPBASENAME
#endif
                      ,homedir);
            asperr = addsearchpath(cwd);
            if (asperr == -2)
            {
                if (buildvfs_mkdir(cwd,S_IRWXU) == 0) asperr = addsearchpath(cwd);
                else asperr = -1;
            }
            if (asperr == 0)
                buildvfs_chdir(cwd);
            Xfree(homedir);
        }
    }

    SW_AddMusicFolder();

#ifndef EDUKE32_STANDALONE
    if (g_grpNamePtr == NULL)
    {
        const char *cp = getenv("SWGRP");
        if (cp)
        {
            clearGrpNamePtr();
            g_grpNamePtr = dup_filename(cp);
            LOG_F(INFO, "Using \"%s\" as main GRP file", g_grpNamePtr);
        }
    }
#endif
}

struct grpfile const * g_selectedGrp;

void SW_ScanGroups()
{
    ScanGroups();

    g_selectedGrp = NULL;

    char const * const currentGrp = G_GrpFile();

    for (grpfile_t const *fg = foundgrps; fg; fg=fg->next)
    {
        if (!Bstrcasecmp(fg->filename, currentGrp))
        {
            g_selectedGrp = fg;
            break;
        }
    }

    if (g_selectedGrp == NULL)
        g_selectedGrp = foundgrps;
}

static int32_t SW_TryLoadingGrp(char const * const grpfile, internalgrpfile const * type = nullptr)
{
    int32_t i;

    if ((i = initgroupfile(grpfile)) == -1)
        LOG_F(WARNING, "Could not find main data file \"%s\"!", grpfile);
    else
    {
        LOG_F(INFO, "Using \"%s\" as main game data file.", grpfile);
        if (type)
        {
            if (type->postprocessing)
                type->postprocessing(i);
            SW_GameFlags |= type->gameflags;
        }
    }

    return i;
}

static int32_t SW_LoadGrpDependencyChain(grpfile_t const * const grp)
{
    if (!grp)
        return -1;

    if ((grp->type->flags & GRP_HAS_DEPENDENCY) && grp->type->dependency != grp->type->crcval)
        SW_LoadGrpDependencyChain(FindGroup(grp->type->dependency));

    int32_t const i = SW_TryLoadingGrp(grp->filename, grp->type);

    return i;
}

int g_addonNum = -1;

void SW_LoadGroups()
{
    if (g_addonNum != -1)
        SW_LoadAddon();

    if (SW_LoadGrpDependencyChain(g_selectedGrp) != -1)
    {
        clearGrpNamePtr();
        g_grpNamePtr = dup_filename(g_selectedGrp->filename);
    }
    else
    {
        SW_TryLoadingGrp(G_GrpFile());
    }
}

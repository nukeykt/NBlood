
#include "compat.h"
#include "build.h"
#include "common.h"

#ifdef _WIN32
# include "windows_inc.h"
# include "winbits.h"
#elif defined __APPLE__
# include "osxbits.h"
#endif

#include "names.h"
#include "common_game.h"

static const char *defaultgrpfilename = "STUFF.DAT";
static const char *defaultdeffilename = "kenbuild.def";

int g_useCwd;

const char *G_DefaultGrpFile(void)
{
    return defaultgrpfilename;
}
const char *G_GrpFile(void)
{
    return defaultgrpfilename;
}

const char *G_DefaultDefFile(void)
{
    return defaultdeffilename;
}
const char *G_DefFile(void)
{
    return defaultdeffilename;
}

void Ken_AddSearchPaths(void)
{
#ifndef EDUKE32_TOUCH_DEVICES
#if defined __linux__ || defined EDUKE32_BSD
    char buf[BMAX_PATH];
    char *homepath = Bgethomedir();
    const char *xdg_docs_path = getenv("XDG_DOCUMENTS_DIR");
    const char *xdg_config_path = getenv("XDG_CONFIG_HOME");

    if (xdg_config_path) {
        Bsnprintf(buf, sizeof(buf), "%s/" APPBASENAME, xdg_config_path);
        addsearchpath(buf);
    }

    if (xdg_docs_path) {
        Bsnprintf(buf, sizeof(buf), "%s/" APPNAME, xdg_docs_path);
        addsearchpath(buf);
    }
    else {
        Bsnprintf(buf, sizeof(buf), "%s/Documents/" APPNAME, homepath);
        addsearchpath(buf);
    }

    Xfree(homepath);

    addsearchpath("/usr/share/games/" APPBASENAME);
    addsearchpath("/usr/local/share/games/" APPBASENAME);
    addsearchpath("/app/extensions/extra");
#elif defined EDUKE32_OSX
    char buf[BMAX_PATH];
    int32_t i;
    char *support[] = { osx_getsupportdir(0), osx_getsupportdir(1) };

    for (i = 0; i < 2; i++)
    {
        Bsnprintf(buf, sizeof(buf), "%s/" APPNAME, support[i]);
        addsearchpath(buf);
    }

    for (i = 0; i < 2; i++)
    {
        Xfree(support[i]);
    }
#elif defined (_WIN32)

#endif
#endif
}

static char g_rootDir[BMAX_PATH];

void Ken_ExtPreInit(int32_t argc, char const * const * argv)
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

void Ken_ExtInit(void)
{
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
}

static void Ken_InitMultiPsky()
{
    // default
    psky_t * const defaultsky = tileSetupSky(DEFAULTPSKY);
    defaultsky->lognumtiles = 1;
    defaultsky->horizfrac = 65536;

    // DAYSKY
    psky_t * const daysky = tileSetupSky(DAYSKY);
    daysky->lognumtiles = 1;
    daysky->horizfrac = 65536;

    // NIGHTSKY
    psky_t * const nightsky = tileSetupSky(NIGHTSKY);
    nightsky->lognumtiles = 3;
    nightsky->horizfrac = 65536;
}

void Ken_PostStartupWindow()
{
    Ken_InitMultiPsky();

    size_t i;
    char tempbuf[256];

    for (i=0; i<256; i++) tempbuf[i] = i;

    for (i=0; i<32; i++) tempbuf[i+192] = i+128; //green->red
    paletteMakeLookupTable(1,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+32; //green->blue
    paletteMakeLookupTable(2,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+224; //green->pink
    paletteMakeLookupTable(3,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+64; //green->brown
    paletteMakeLookupTable(4,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+96;
    paletteMakeLookupTable(5,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+160;
    paletteMakeLookupTable(6,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+192;
    paletteMakeLookupTable(7,tempbuf,0,0,0,1);

    for (i=0; i<256; i++)
        tempbuf[i] = ((i+32)&255);  //remap colors for screwy palette sectors
    paletteMakeLookupTable(16,tempbuf,0,0,0,1);

    for (i=0; i<256; i++) tempbuf[i] = i;
    paletteMakeLookupTable(17,tempbuf,96,96,96,1);

    for (i=0; i<256; i++) tempbuf[i] = i; //(i&31)+32;
    paletteMakeLookupTable(18,tempbuf,32,32,192,1);
}

int32_t voxid_PLAYER = -1, voxid_BROWNMONSTER = -1;

void Ken_LoadVoxels()
{
    if (!qloadkvx(nextvoxid,"voxel000.kvx"))
        voxid_PLAYER = nextvoxid++;
    if (!qloadkvx(nextvoxid,"voxel001.kvx"))
        voxid_BROWNMONSTER = nextvoxid++;
}

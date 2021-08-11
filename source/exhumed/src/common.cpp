//
// Common non-engine code/data for EDuke32 and Mapster32
//

#include "compat.h"
#include "build.h"
#include "baselayer.h"
#include "palette.h"

#include "grpscan.h"

#include "vfs.h"

#ifdef _WIN32
# include "windows_inc.h"
# include "winbits.h"
#elif defined __APPLE__
# include "osxbits.h"
#endif

#include "common.h"
#include "common_game.h"

struct grpfile_t const* g_selectedGrp;

int32_t g_gameType = GAMEFLAG_POWERSLAVE;

int r_showfps;

// g_gameNamePtr can point to one of: grpfiles[].name (string literal), string
// literal, malloc'd block (XXX: possible leak)
const char* g_gameNamePtr = NULL;

// grp handling

static const char* defaultgamegrp = "STUFF.DAT";
static const char* defaultdeffilename = "exhumed.def";

// g_grpNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char* g_grpNamePtr = NULL;

void clearGrpNamePtr(void)
{
    Xfree(g_grpNamePtr);
    // g_grpNamePtr assumed to be assigned to right after
}

const char* G_DefaultGrpFile(void)
{
    return defaultgamegrp;
}
const char* G_DefaultDefFile(void)
{
    return defaultdeffilename;
}

const char* G_GrpFile(void)
{
    return (g_grpNamePtr == NULL)?G_DefaultGrpFile():g_grpNamePtr;
}

const char* G_DefFile(void)
{
    return (g_defNamePtr == NULL)?G_DefaultDefFile():g_defNamePtr;
}

//////////

static char g_rootDir[BMAX_PATH];

int g_useCwd;
int32_t g_groupFileHandle;

static struct strllist* CommandPaths, * CommandGrps;

void G_ExtPreInit(int32_t argc, char const* const* argv)
{
    g_useCwd = G_CheckCmdSwitch(argc, argv, "-usecwd");

    #ifdef _WIN32
    GetModuleFileName(NULL, g_rootDir, BMAX_PATH);
    Bcorrectfilename(g_rootDir, 1);
    //buildvfs_chdir(g_rootDir);
    #else
    buildvfs_getcwd(g_rootDir, BMAX_PATH);
    strcat(g_rootDir, "/");
    #endif
}

void G_ExtInit(void)
{
    char cwd[BMAX_PATH];

    #ifdef EDUKE32_OSX
    char* appdir = Bgetappdir();
    addsearchpath(appdir);
    Xfree(appdir);
    #endif

    #ifdef USE_PHYSFS
    strncpy(cwd, PHYSFS_getBaseDir(), ARRAY_SIZE(cwd));
    cwd[ARRAY_SIZE(cwd) - 1] = '\0';
    #else
    if (buildvfs_getcwd(cwd, ARRAY_SIZE(cwd)) && Bstrcmp(cwd, "/") != 0)
        #endif
        addsearchpath(cwd);

    // TODO:
    if (CommandPaths)
    {
        int32_t i;
        struct strllist* s;
        while (CommandPaths)
        {
            s = CommandPaths->next;
            i = addsearchpath(CommandPaths->str);
            if (i < 0)
            {
                initprintf("Failed adding %s for game data: %s\n", CommandPaths->str,
                    i == -1?"not a directory":"no such directory");
            }

            Xfree(CommandPaths->str);
            Xfree(CommandPaths);
            CommandPaths = s;
        }
    }

    #if defined(_WIN32) && !defined(EDUKE32_STANDALONE)
    if (buildvfs_exists("user_profiles_enabled"))
        #else
    if (g_useCwd == 0 && !buildvfs_exists("user_profiles_disabled"))
        #endif
    {
        char* homedir;
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
                , homedir);
            asperr = addsearchpath(cwd);
            if (asperr == -2)
            {
                if (buildvfs_mkdir(cwd, S_IRWXU) == 0) asperr = addsearchpath(cwd);
                else asperr = -1;
            }
            if (asperr == 0)
                buildvfs_chdir(cwd);
            Xfree(homedir);
        }
    }
}

void G_ScanGroups(void)
{
    ScanGroups();

    g_selectedGrp = NULL;

    char const* const currentGrp = G_GrpFile();

    for (grpfile_t const* fg = foundgrps; fg; fg = fg->next)
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

static int32_t G_TryLoadingGrp(char const* const grpfile)
{
    int32_t i;

    if ((i = initgroupfile(grpfile)) == -1)
        initprintf("Warning: could not find main data file \"%s\"!\n", grpfile);
    else
        initprintf("Using \"%s\" as main game data file.\n", grpfile);

    return i;
}

static int32_t G_LoadGrpDependencyChain(grpfile_t const* const grp)
{
    if (!grp)
        return -1;

    if (grp->type->dependency && grp->type->dependency != grp->type->crcval)
        G_LoadGrpDependencyChain(FindGroup(grp->type->dependency));

    int32_t const i = G_TryLoadingGrp(grp->filename);

    return i;
}

void G_LoadGroups(int32_t autoload)
{
    if (g_modDir[0] != '/')
    {
        char cwd[BMAX_PATH];

        Bstrcat(g_rootDir, g_modDir);
        addsearchpath(g_rootDir);
        //        addsearchpath(mod_dir);

        char path[BMAX_PATH];

        if (buildvfs_getcwd(cwd, BMAX_PATH))
        {
            Bsnprintf(path, sizeof(path), "%s/%s", cwd, g_modDir);
            if (!Bstrcmp(g_rootDir, path))
            {
                if (addsearchpath(path) == -2)
                    if (buildvfs_mkdir(path, S_IRWXU) == 0)
                        addsearchpath(path);
            }
        }

        #ifdef USE_OPENGL
        Bsnprintf(path, sizeof(path), "%s/%s", g_modDir, TEXCACHEFILE);
        Bstrcpy(TEXCACHEFILE, path);
        #endif
    }

    const char* grpfile;
    int32_t i;

    if ((i = G_LoadGrpDependencyChain(g_selectedGrp)) != -1)
    {
        grpfile = g_selectedGrp->filename;

        clearGrpNamePtr();
        g_grpNamePtr = dup_filename(grpfile);

        grpinfo_t const* const type = g_selectedGrp->type;

        g_gameType = type->game;
        g_gameNamePtr = type->name;

        if (type->defname && g_defNamePtr == NULL)
            g_defNamePtr = dup_filename(type->defname);
    }
    else
    {
        grpfile = G_GrpFile();
        i = G_TryLoadingGrp(grpfile);
    }

    if (autoload)
    {
        G_LoadGroupsInDir("autoload");

        if (i != -1)
            G_DoAutoload(grpfile);
    }

    if (g_modDir[0] != '/')
        G_LoadGroupsInDir(g_modDir);

    loaddefinitions_game(G_DefFile(), TRUE);

    struct strllist* s;

    int const bakpathsearchmode = pathsearchmode;
    pathsearchmode = 1;

    while (CommandGrps)
    {
        int32_t j;

        s = CommandGrps->next;

        if ((j = initgroupfile(CommandGrps->str)) == -1)
            initprintf("Could not find file \"%s\".\n", CommandGrps->str);
        else
        {
            g_groupFileHandle = j;
            initprintf("Using file \"%s\" as game data.\n", CommandGrps->str);
            if (autoload)
                G_DoAutoload(CommandGrps->str);
        }

        Xfree(CommandGrps->str);
        Xfree(CommandGrps);
        CommandGrps = s;
    }
    pathsearchmode = bakpathsearchmode;
}

#ifndef EDUKE32_TOUCH_DEVICES
#if defined __linux__ || defined EDUKE32_BSD
static void Exhumed_Add_GOG_Linux(const char * path)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "%s/game/data", path);
    addsearchpath(buf);
    Bsnprintf(buf, sizeof(buf), "%s/game/data/MUSIC", path);
    addsearchpath(buf);
}
static void Exhumed_Add_Steam_Linux(const char *path)
{
    char buf[BMAX_PATH];

    // PowerSlave (DOS Classic Edition) - Steam
    Bsnprintf(buf, sizeof(buf), "%s/steamapps/common/PowerslaveCE", path);
    addsearchpath(buf);
    Bsnprintf(buf, sizeof(buf), "%s/steamapps/common/PowerslaveCE/PWRSLAVE", path);
    addsearchpath(buf);
    Bsnprintf(buf, sizeof(buf), "%s/steamapps/common/PowerslaveCE/PWRSLAVE/MUSIC", path);
    addsearchpath(buf);
}
#endif
#endif

void G_AddSearchPaths(void)
{
#ifndef EDUKE32_TOUCH_DEVICES
#if defined __linux__ || defined EDUKE32_BSD
    char buf[BMAX_PATH];
    char *homepath = Bgethomedir();

    // PowerSlave (DOS Classic Edition) - Steam
    Bsnprintf(buf, sizeof(buf), "%s/.steam/steam", homepath);
    Exhumed_Add_Steam_Linux(buf);

    Bsnprintf(buf, sizeof(buf), "%s/.steam/steam/steamapps/libraryfolders.vdf", homepath);
    Paths_ParseSteamLibraryVDF(buf, Exhumed_Add_Steam_Linux);

    // Powerslave - GOG.com
    Bsnprintf(buf, sizeof(buf), "%s/GOG Games/Powerslave English", homepath);
    Exhumed_Add_GOG_Linux(buf);
    Paths_ParseXDGDesktopFilesFromGOG(homepath, "Powerslave_English", Exhumed_Add_GOG_Linux);

    Bfree(homepath);

    addsearchpath("/usr/share/games/pcexhumed");
    addsearchpath("/usr/local/share/games/pcexhumed");
#elif defined EDUKE32_OSX
    char buf[BMAX_PATH];
    int32_t i;
    char* applications[] = { osx_getapplicationsdir(0), osx_getapplicationsdir(1) };
    char* support[] = { osx_getsupportdir(0), osx_getsupportdir(1) };
    char* documents[] = { osx_getdocumentsdir(0), osx_getdocumentsdir(1) };

    for (i = 0; i < 2; i++)
    {
        // Powerslave - GOG.com
        Bsnprintf(buf, sizeof(buf), "%s/Powerslave.app/Contents/Resources/game", applications[i]);
        addsearchpath(buf);
        Bsnprintf(buf, sizeof(buf), "%s/Powerslave.app/Contents/Resources/game", documents[i]);
        addsearchpath(buf);
        Bsnprintf(buf, sizeof(buf), "%s/Powerslave.app/Contents/Resources/game/Powerslave.app/Contents/Resources/game", applications[i]);
        addsearchpath(buf);
    }

    for (i = 0; i < 2; i++)
    {
        Bsnprintf(buf, sizeof(buf), "%s/PCExhumed", support[i]);
        addsearchpath(buf);
    }

    for (i = 0; i < 2; i++)
    {
        Bfree(applications[i]);
        Bfree(support[i]);
        Bfree(documents[i]);
    }
#elif defined (_WIN32)
    char buf[BMAX_PATH] = { 0 };
    DWORD bufsize;

    // PowerSlave (DOS Classic Edition) - Steam
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 1260020)", "InstallLocation", buf, &bufsize))
    {
        char* const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        Bstrncpy(suffix, "/PWRSLAVE", remaining);
        addsearchpath(buf);
    }

    // Powerslave - GOG.com
    bufsize = sizeof(buf);
    if (Paths_ReadRegistryValue(R"(SOFTWARE\GOG.com\Games\2132611980)", "path", buf, &bufsize))
    {
        addsearchpath(buf);
    }

#endif
#endif
}

void G_CleanupSearchPaths(void)
{
//    removesearchpaths_withuser(SEARCHPATH_REMOVE);
}

//////////

void G_AddGroup(const char* buffer)
{
    char buf[BMAX_PATH];

    struct strllist* s = (struct strllist*)Xcalloc(1, sizeof(struct strllist));

    Bstrcpy(buf, buffer);

    if (Bstrchr(buf, '.') == 0)
        Bstrcat(buf, ".grp");

    s->str = Xstrdup(buf);

    if (CommandGrps)
    {
        struct strllist* t;
        for (t = CommandGrps; t->next; t = t->next);
        t->next = s;
        return;
    }
    CommandGrps = s;
}

void G_AddPath(const char* buffer)
{
    struct strllist* s = (struct strllist*)Xcalloc(1, sizeof(struct strllist));
    s->str = Xstrdup(buffer);

    if (CommandPaths)
    {
        struct strllist* t;
        for (t = CommandPaths; t->next; t = t->next);
        t->next = s;
        return;
    }
    CommandPaths = s;
}

//////////

// loads all group (grp, zip, pk3/4) files in the given directory
void G_LoadGroupsInDir(const char* dirname)
{
    static const char* extensions[] = {"*.grp", "*.zip", "*.ssi", "*.pk3", "*.pk4"};
    char buf[BMAX_PATH];
    fnlist_t fnlist = FNLIST_INITIALIZER;

    for (auto& extension : extensions)
    {
        BUILDVFS_FIND_REC* rec;

        fnlist_getnames(&fnlist, dirname, extension, -1, 0);

        for (rec = fnlist.findfiles; rec; rec = rec->next)
        {
            Bsnprintf(buf, sizeof(buf), "%s/%s", dirname, rec->name);
            initprintf("Using group file \"%s\".\n", buf);
            initgroupfile(buf);
        }

        fnlist_clearnames(&fnlist);
    }
}

void G_DoAutoload(const char* dirname)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "autoload/%s", dirname);
    G_LoadGroupsInDir(buf);
}

//////////

#ifdef FORMAT_UPGRADE_ELIGIBLE

static int32_t S_TryFormats(char* const testfn, char* const fn_suffix, char const searchfirst)
{
    #ifdef HAVE_FLAC
    {
        Bstrcpy(fn_suffix, ".flac");
        int32_t const fp = kopen4loadfrommod(testfn, searchfirst);
        if (fp >= 0)
            return fp;
    }
    #endif

    #ifdef HAVE_VORBIS
    {
        Bstrcpy(fn_suffix, ".ogg");
        int32_t const fp = kopen4loadfrommod(testfn, searchfirst);
        if (fp >= 0)
            return fp;
    }
    #endif

    return -1;
}

static int32_t S_TryExtensionReplacements(char* const testfn, char const searchfirst, uint8_t const ismusic)
{
    char* extension = Bstrrchr(testfn, '.');
    char* const fn_end = Bstrchr(testfn, '\0');

    // ex: grabbag.voc --> grabbag_voc.*
    if (extension != NULL)
    {
        *extension = '_';

        int32_t const fp = S_TryFormats(testfn, fn_end, searchfirst);
        if (fp >= 0)
            return fp;
    }
    else
    {
        extension = fn_end;
    }

    // ex: grabbag.mid --> grabbag.*
    if (ismusic)
    {
        int32_t const fp = S_TryFormats(testfn, extension, searchfirst);
        if (fp >= 0)
            return fp;
    }

    return -1;
}

int32_t S_OpenAudio(const char* fn, char searchfirst, uint8_t const ismusic)
{
    int32_t const     origfp = kopen4loadfrommod(fn, searchfirst);
    char const* const origparent = origfp != -1 ? kfileparent(origfp) : NULL;
    uint32_t const    parentlength = origparent != NULL ? Bstrlen(origparent) : 0;

    auto testfn = (char*)Xmalloc(Bstrlen(fn) + 12 + parentlength); // "music/" + overestimation of parent minus extension + ".flac" + '\0'

    // look in ./
    // ex: ./grabbag.mid
    Bstrcpy(testfn, fn);
    int32_t fp = S_TryExtensionReplacements(testfn, searchfirst, ismusic);
    if (fp >= 0)
        goto success;

    // look in ./music/<file's parent GRP name>/
    // ex: ./music/duke3d/grabbag.mid
    // ex: ./music/nwinter/grabbag.mid
    if (origparent != NULL)
    {
        char const* const parentextension = Bstrrchr(origparent, '.');
        uint32_t const namelength = parentextension != NULL ? (unsigned)(parentextension - origparent) : parentlength;

        Bsprintf(testfn, "music/%.*s/%s", namelength, origparent, fn);
        fp = S_TryExtensionReplacements(testfn, searchfirst, ismusic);
        if (fp >= 0)
            goto success;
    }

    // look in ./music/
    // ex: ./music/grabbag.mid
    Bsprintf(testfn, "music/%s", fn);
    fp = S_TryExtensionReplacements(testfn, searchfirst, ismusic);
    if (fp >= 0)
        goto success;

    fp = origfp;
success:
    Bfree(testfn);
    if (fp != origfp)
        kclose(origfp);

    return fp;
}

#endif

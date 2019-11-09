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

//
// Common non-engine code/data for EDuke32 and Mapster32
//

#include "compat.h"
#include "build.h"
#include "baselayer.h"
#include "palette.h"

#ifdef _WIN32
# include "windows_inc.h"
# include "winbits.h"
#elif defined __APPLE__
# include "osxbits.h"
#endif

#include "common.h"
#include "common_game.h"

// g_grpNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char *g_grpNamePtr = NULL;

void clearGrpNamePtr(void)
{
    Bfree(g_grpNamePtr);
    // g_grpNamePtr assumed to be assigned to right after
}

const char *G_DefaultGrpFile(void)
{
    return "nblood.pk3";
}

const char *G_DefaultDefFile(void)
{
    return "blood.def";
}

const char *G_GrpFile(void)
{
    return (g_grpNamePtr == NULL) ? G_DefaultGrpFile() : g_grpNamePtr;
}

const char *G_DefFile(void)
{
    return (g_defNamePtr == NULL) ? G_DefaultDefFile() : g_defNamePtr;
}


void G_SetupGlobalPsky(void)
{
    int skyIdx = 0;

    // NOTE: Loop must be running backwards for the same behavior as the game
    // (greatest sector index with matching parallaxed sky takes precedence).
    for (bssize_t i = numsectors - 1; i >= 0; i--)
    {
        if (sector[i].ceilingstat & 1)
        {
            skyIdx = getpskyidx(sector[i].ceilingpicnum);
            if (skyIdx > 0)
                break;
        }
    }

    g_pskyidx = skyIdx;
}

static char g_rootDir[BMAX_PATH];

int g_useCwd;
int32_t g_groupFileHandle;

static struct strllist *CommandPaths, *CommandGrps;

void G_ExtPreInit(int32_t argc,char const * const * argv)
{
    g_useCwd = G_CheckCmdSwitch(argc, argv, "-usecwd");

#ifdef _WIN32
    GetModuleFileName(NULL,g_rootDir,BMAX_PATH);
    Bcorrectfilename(g_rootDir,1);
    //chdir(g_rootDir);
#else
    getcwd(g_rootDir,BMAX_PATH);
    strcat(g_rootDir,"/");
#endif
}

void G_ExtInit(void)
{
    char cwd[BMAX_PATH];

#ifdef EDUKE32_OSX
    char *appdir = Bgetappdir();
    addsearchpath(appdir);
    Bfree(appdir);
#endif

    if (getcwd(cwd,BMAX_PATH) && Bstrcmp(cwd,"/") != 0)
        addsearchpath(cwd);

    if (CommandPaths)
    {
        int32_t i;
        struct strllist *s;
        while (CommandPaths)
        {
            s = CommandPaths->next;
            i = addsearchpath(CommandPaths->str);
            if (i < 0)
            {
                initprintf("Failed adding %s for game data: %s\n", CommandPaths->str,
                           i==-1 ? "not a directory" : "no such directory");
            }

            Bfree(CommandPaths->str);
            Bfree(CommandPaths);
            CommandPaths = s;
        }
    }

#if defined(_WIN32)
    if (!access("user_profiles_enabled", F_OK))
#else
    if (g_useCwd == 0 && access("user_profiles_disabled", F_OK))
#endif
    {
        char *homedir;
        int32_t asperr;

        if ((homedir = Bgethomedir()))
        {
            Bsnprintf(cwd,sizeof(cwd),"%s/"
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
                if (Bmkdir(cwd,S_IRWXU) == 0) asperr = addsearchpath(cwd);
                else asperr = -1;
            }
            if (asperr == 0)
                Bchdir(cwd);
            Bfree(homedir);
        }
    }
}

static int32_t G_TryLoadingGrp(char const * const grpfile)
{
    int32_t i;

    if ((i = initgroupfile(grpfile)) == -1)
        initprintf("Warning: could not find main data file \"%s\"!\n", grpfile);
    else
        initprintf("Using \"%s\" as main game data file.\n", grpfile);

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

        if (getcwd(cwd, BMAX_PATH))
        {
            Bsnprintf(path, sizeof(path), "%s/%s", cwd, g_modDir);
            if (!Bstrcmp(g_rootDir, path))
            {
                if (addsearchpath(path) == -2)
                    if (Bmkdir(path, S_IRWXU) == 0)
                        addsearchpath(path);
            }
        }

#ifdef USE_OPENGL
        Bsnprintf(path, sizeof(path), "%s/%s", g_modDir, TEXCACHEFILE);
        Bstrcpy(TEXCACHEFILE, path);
#endif
    }
    const char *grpfile = G_GrpFile();
    G_TryLoadingGrp(grpfile);

    if (autoload)
    {
        G_LoadGroupsInDir("autoload");

        //if (i != -1)
        //    G_DoAutoload(grpfile);
    }

    if (g_modDir[0] != '/')
        G_LoadGroupsInDir(g_modDir);

    if (g_defNamePtr == NULL)
    {
        const char *tmpptr = getenv("BLOODDEF");
        if (tmpptr)
        {
            clearDefNamePtr();
            g_defNamePtr = dup_filename(tmpptr);
            initprintf("Using \"%s\" as definitions file\n", g_defNamePtr);
        }
    }

    loaddefinitions_game(BLOODWIDESCREENDEF, TRUE);
    loaddefinitions_game(G_DefFile(), TRUE);

    struct strllist *s;

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

        Bfree(CommandGrps->str);
        Bfree(CommandGrps);
        CommandGrps = s;
    }
    pathsearchmode = bakpathsearchmode;
}

#ifndef EDUKE32_TOUCH_DEVICES
#if defined __linux__ || defined EDUKE32_BSD
static void Blood_Add_GOG_OUWB_Linux(const char * path)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "%s/data", path);
    addsearchpath(buf);
}
#endif

#if defined EDUKE32_OSX || defined __linux__ || defined EDUKE32_BSD
static void Blood_AddSteamPaths(const char *basepath)
{
    char buf[BMAX_PATH];

    // Blood: Fresh Supply - Steam
    Bsnprintf(buf, sizeof(buf), "%s/steamapps/common/Blood", basepath);
    addsearchpath(buf);

    // Blood: One Unit Whole Blood - Steam
    Bsnprintf(buf, sizeof(buf), "%s/steamapps/common/One Unit Whole Blood", basepath);
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

    Bsnprintf(buf, sizeof(buf), "%s/.steam/steam", homepath);
    Blood_AddSteamPaths(buf);

    Bsnprintf(buf, sizeof(buf), "%s/.steam/steam/steamapps/libraryfolders.vdf", homepath);
    Paths_ParseSteamLibraryVDF(buf, Blood_AddSteamPaths);

    // Blood: One Unit Whole Blood - GOG.com
    Bsnprintf(buf, sizeof(buf), "%s/GOG Games/Blood One Unit Whole Blood", homepath);
    Blood_Add_GOG_OUWB_Linux(buf);
    Paths_ParseXDGDesktopFilesFromGOG(homepath, "Blood_One_Unit_Whole_Blood", Blood_Add_GOG_OUWB_Linux);

    Bfree(homepath);

    addsearchpath("/usr/share/games/nblood");
    addsearchpath("/usr/local/share/games/nblood");
#elif defined EDUKE32_OSX
    char buf[BMAX_PATH];
    int32_t i;
    char *applications[] = { osx_getapplicationsdir(0), osx_getapplicationsdir(1) };
    char *support[] = { osx_getsupportdir(0), osx_getsupportdir(1) };

    for (i = 0; i < 2; i++)
    {
        Bsnprintf(buf, sizeof(buf), "%s/Steam", support[i]);
        Blood_AddSteamPaths(buf);

        Bsnprintf(buf, sizeof(buf), "%s/Steam/steamapps/libraryfolders.vdf", support[i]);
        Paths_ParseSteamLibraryVDF(buf, Blood_AddSteamPaths);
    }

    for (i = 0; i < 2; i++)
    {
        Bsnprintf(buf, sizeof(buf), "%s/NBlood", support[i]);
        addsearchpath(buf);
    }

    for (i = 0; i < 2; i++)
    {
        Bfree(applications[i]);
        Bfree(support[i]);
    }
#elif defined (_WIN32)
    char buf[BMAX_PATH] = {0};
    DWORD bufsize;
    bool found = false;

    // Blood: One Unit Whole Blood - Steam
    bufsize = sizeof(buf);
    if (!found && Paths_ReadRegistryValue(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 299030)", "InstallLocation", buf, &bufsize))
    {
        addsearchpath(buf);
        found = true;
    }

    // Blood: One Unit Whole Blood - GOG.com
    bufsize = sizeof(buf);
    if (!found && Paths_ReadRegistryValue("SOFTWARE\\GOG.com\\GOGONEUNITONEBLOOD", "PATH", buf, &bufsize))
    {
        addsearchpath(buf);
        found = true;
    }

    // Blood: Fresh Supply - Steam
    bufsize = sizeof(buf);
    if (!found && Paths_ReadRegistryValue(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 1010750)", "InstallLocation", buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        addsearchpath(buf);
        strncpy(suffix, "/addons/Cryptic Passage", remaining);
        addsearchpath(buf);
        found = true;
    }

    // Blood: Fresh Supply - GOG.com
    bufsize = sizeof(buf);
    if (!found && Paths_ReadRegistryValue(R"(SOFTWARE\GOG.com\Games\1374469660)", "path", buf, &bufsize))
    {
        char * const suffix = buf + bufsize - 1;
        DWORD const remaining = sizeof(buf) - bufsize;

        addsearchpath(buf);
        strncpy(suffix, "/addons/Cryptic Passage", remaining);
        addsearchpath(buf);
        found = true;
    }
#endif
#endif
}

void G_CleanupSearchPaths(void)
{
    removesearchpaths_withuser(SEARCHPATH_REMOVE);
}

//////////

void G_AddGroup(const char *buffer)
{
    char buf[BMAX_PATH];

    struct strllist *s = (struct strllist *)Xcalloc(1,sizeof(struct strllist));

    Bstrcpy(buf, buffer);

    if (Bstrchr(buf,'.') == 0)
        Bstrcat(buf,".grp");

    s->str = Xstrdup(buf);

    if (CommandGrps)
    {
        struct strllist *t;
        for (t = CommandGrps; t->next; t=t->next) ;
        t->next = s;
        return;
    }
    CommandGrps = s;
}

void G_AddPath(const char *buffer)
{
    struct strllist *s = (struct strllist *)Xcalloc(1,sizeof(struct strllist));
    s->str = Xstrdup(buffer);

    if (CommandPaths)
    {
        struct strllist *t;
        for (t = CommandPaths; t->next; t=t->next) ;
        t->next = s;
        return;
    }
    CommandPaths = s;
}

//////////

// loads all group (grp, zip, pk3/4) files in the given directory
void G_LoadGroupsInDir(const char *dirname)
{
    static const char *extensions[] = { "*.grp", "*.zip", "*.ssi", "*.pk3", "*.pk4" };
    char buf[BMAX_PATH];
    fnlist_t fnlist = FNLIST_INITIALIZER;

    for (auto & extension : extensions)
    {
        BUILDVFS_FIND_REC *rec;

        fnlist_getnames(&fnlist, dirname, extension, -1, 0);

        for (rec=fnlist.findfiles; rec; rec=rec->next)
        {
            Bsnprintf(buf, sizeof(buf), "%s/%s", dirname, rec->name);
            initprintf("Using group file \"%s\".\n", buf);
            initgroupfile(buf);
        }

        fnlist_clearnames(&fnlist);
    }
}

void G_DoAutoload(const char *dirname)
{
    char buf[BMAX_PATH];

    Bsnprintf(buf, sizeof(buf), "autoload/%s", dirname);
    G_LoadGroupsInDir(buf);
}

//////////

#ifdef FORMAT_UPGRADE_ELIGIBLE

static int32_t S_TryFormats(char * const testfn, char * const fn_suffix, char const searchfirst)
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

static int32_t S_TryExtensionReplacements(char * const testfn, char const searchfirst, uint8_t const ismusic)
{
    char * extension = Bstrrchr(testfn, '.');
    char * const fn_end = Bstrchr(testfn, '\0');

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

int32_t S_OpenAudio(const char *fn, char searchfirst, uint8_t const ismusic)
{
    int32_t const     origfp       = kopen4loadfrommod(fn, searchfirst);
    char const *const origparent   = origfp != -1 ? kfileparent(origfp) : NULL;
    uint32_t const    parentlength = origparent != NULL ? Bstrlen(origparent) : 0;

    auto testfn = (char *)Xmalloc(Bstrlen(fn) + 12 + parentlength); // "music/" + overestimation of parent minus extension + ".flac" + '\0'

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
        char const * const parentextension = Bstrrchr(origparent, '.');
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


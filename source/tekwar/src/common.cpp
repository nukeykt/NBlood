//
// Common non-engine code/data for EDuke32 and Mapster32
//

#include "compat.h"
#include "build.h"
#include "baselayer.h"
#include "palette.h"
#include "grpscan.h"
#include "vfs.h"
#include "texcache.h"

#ifdef _WIN32
# include "windows_inc.h"
# include "winbits.h"
#elif defined __APPLE__
# include "osxbits.h"
#endif

#include "common.h"
#include "common_game.h"

struct grpfile_t const* g_selectedGrp;

int32_t g_gameType = GAMEFLAG_TEKWAR;
int     g_addonNum = 0;

int r_showfps;

// g_gameNamePtr can point to one of: grpfiles[].name (string literal), string
// literal, malloc'd block (XXX: possible leak)
const char* g_gameNamePtr = NULL;

// grp/con handling

static const char* defaultgamegrp = "tekwar.pk3";
static const char* defaultdeffilename = "tekwar.def";

// g_grpNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char* g_grpNamePtr = NULL;
// g_scriptNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char* g_scriptNamePtr = NULL;
// g_rtsNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char* g_rtsNamePtr = NULL;

void clearGrpNamePtr(void)
{
    Xfree(g_grpNamePtr);
    // g_grpNamePtr assumed to be assigned to right after
}

void clearScriptNamePtr(void)
{
    Xfree(g_scriptNamePtr);
    // g_scriptNamePtr assumed to be assigned to right after
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
    return (g_grpNamePtr == NULL) ? G_DefaultGrpFile() : g_grpNamePtr;
}

const char* G_DefFile(void)
{
    return (g_defNamePtr == NULL) ? G_DefaultDefFile() : g_defNamePtr;
}

//////////

// Set up new-style multi-psky handling.
void G_InitMultiPsky(int CLOUDYOCEAN__DYN, int MOONSKY1__DYN, int BIGORBIT1__DYN, int LA__DYN)
{
    // When adding other multi-skies, take care that the tileofs[] values are
    // <= PSKYOFF_MAX. (It can be increased up to MAXPSKYTILES, but should be
    // set as tight as possible.)

    // The default sky properties (all others are implicitly zero):
    psky_t* sky = tileSetupSky(DEFAULTPSKY);
    sky->lognumtiles = 3;
    sky->horizfrac = 32768;

    // CLOUDYOCEAN
    // Aligns with the drawn scene horizon because it has one itself.
    sky = tileSetupSky(CLOUDYOCEAN__DYN);
    sky->lognumtiles = 3;
    sky->horizfrac = 65536;

    // MOONSKY1
    //        earth          mountain   mountain         sun
    sky = tileSetupSky(MOONSKY1__DYN);
    sky->lognumtiles = 3;
    sky->horizfrac = 32768;
    sky->tileofs[6] = 1;
    sky->tileofs[1] = 2;
    sky->tileofs[4] = 2;
    sky->tileofs[2] = 3;

    // BIGORBIT1   // orbit
    //       earth1         2           3           moon/sun
    sky = tileSetupSky(BIGORBIT1__DYN);
    sky->lognumtiles = 3;
    sky->horizfrac = 32768;
    sky->tileofs[5] = 1;
    sky->tileofs[6] = 2;
    sky->tileofs[7] = 3;
    sky->tileofs[2] = 4;

    // LA // la city
    //       earth1         2           3           moon/sun
    sky = tileSetupSky(LA__DYN);
    sky->lognumtiles = 3;
    sky->horizfrac = 16384 + 1024;
    sky->tileofs[0] = 1;
    sky->tileofs[1] = 2;
    sky->tileofs[2] = 1;
    sky->tileofs[3] = 3;
    sky->tileofs[4] = 4;
    sky->tileofs[5] = 0;
    sky->tileofs[6] = 2;
    sky->tileofs[7] = 3;

#if 0
    // This assertion should hold. See note above.
    for (bssize_t i = 0; i < pskynummultis; ++i)
        for (bssize_t j = 0; j < (1 << multipsky[i].lognumtiles); ++j)
            Bassert(multipsky[i].tileofs[j] <= PSKYOFF_MAX);
#endif
}

void G_SetupGlobalPsky(void)
{
    int skyIdx = 0;

    // NOTE: Loop must be running backwards for the same behavior as the game
    // (greatest sector index with matching parallaxed sky takes precedence).
    for (int i = numsectors - 1; i >= 0; i--)
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
                    i == -1 ? "not a directory" : "no such directory");
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

void G_AddSearchPaths(void)
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

void G_CleanupSearchPaths(void)
{
    removesearchpaths_withuser(SEARCHPATH_REMOVE);
}

//////////

GrowArray<char*> g_scriptModules;

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

void G_AddCon(const char* buffer)
{
    clearScriptNamePtr();
    g_scriptNamePtr = dup_filename(buffer);
    initprintf("Using CON file \"%s\".\n", g_scriptNamePtr);
}

void G_AddConModule(const char* buffer)
{
    g_scriptModules.append(Xstrdup(buffer));
}

//////////

// loads all group (grp, zip, pk3/4) files in the given directory
void G_LoadGroupsInDir(const char* dirname)
{
    static const char* extensions[] = { "*.grp", "*.zip", "*.ssi", "*.pk3", "*.pk4" };
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

void G_LoadLookups(void)
{
    int32_t j;
    buildvfs_kfd fp;

    if ((fp = kopen4loadfrommod("lookup.dat", 0)) == buildvfs_kfd_invalid)
        if ((fp = kopen4loadfrommod("lookup.dat", 1)) == buildvfs_kfd_invalid)
            return;

    j = paletteLoadLookupTable(fp);

    if (j < 0)
    {
        if (j == -1)
            initprintf("ERROR loading \"lookup.dat\": failed reading enough data.\n");

        return kclose(fp);
    }

    uint8_t paldata[768];

    for (j = 1; j <= 5; j++)
    {
        // Account for TITLE and REALMS swap between basepal number and on-disk order.
        int32_t basepalnum = (j == 3 || j == 4) ? 4 + 3 - j : j;

        if (kread_and_test(fp, paldata, 768))
            return kclose(fp);

        for (unsigned char& k : paldata)
            k <<= 2;

        paletteSetColorTable(basepalnum, paldata);
    }

    kclose(fp);
}

//////////

#ifdef FORMAT_UPGRADE_ELIGIBLE
int g_maybeUpgradeSoundFormats = 1;

static buildvfs_kfd S_TryFormats(char* const testfn, char* const fn_suffix, char const searchfirst)
{
    if (!g_maybeUpgradeSoundFormats)
        return buildvfs_kfd_invalid;

    static char const* extensions[] =
    {
#ifdef HAVE_FLAC
        ".flac",
#endif
#ifdef HAVE_VORBIS
        ".ogg",
#endif
    };

    for (char const* ext : extensions)
    {
        Bstrcpy(fn_suffix, ext);
        buildvfs_kfd const fp = kopen4loadfrommod(testfn, searchfirst);
        if (fp != buildvfs_kfd_invalid)
            return fp;
    }

    return buildvfs_kfd_invalid;
}

static buildvfs_kfd S_TryExtensionReplacements(char* const testfn, char const searchfirst, uint8_t const ismusic)
{
    char* extension = Bstrrchr(testfn, '.');
    char* const fn_end = Bstrchr(testfn, '\0');

    // ex: grabbag.voc --> grabbag_voc.*
    if (extension != NULL)
    {
        *extension = '_';

        buildvfs_kfd const fp = S_TryFormats(testfn, fn_end, searchfirst);
        if (fp != buildvfs_kfd_invalid)
            return fp;
    }
    else
    {
        extension = fn_end;
    }

    // ex: grabbag.mid --> grabbag.*
    if (ismusic)
    {
        buildvfs_kfd const fp = S_TryFormats(testfn, extension, searchfirst);
        if (fp != buildvfs_kfd_invalid)
            return fp;
    }

    return buildvfs_kfd_invalid;
}

buildvfs_kfd S_OpenAudio(const char* fn, char searchfirst, uint8_t const ismusic)
{
    buildvfs_kfd const origfp = kopen4loadfrommod(fn, searchfirst);
#ifndef USE_PHYSFS
    char const* const origparent = origfp != buildvfs_kfd_invalid ? kfileparent(origfp) : NULL;
    uint32_t const    parentlength = origparent != NULL ? Bstrlen(origparent) : 0;

    auto testfn = (char*)Xmalloc(Bstrlen(fn) + 12 + parentlength); // "music/" + overestimation of parent minus extension + ".flac" + '\0'
#else
    auto testfn = (char*)Xmalloc(Bstrlen(fn) + 12);
#endif

    // look in ./
    // ex: ./grabbag.mid
    Bstrcpy(testfn, fn);
    buildvfs_kfd fp = S_TryExtensionReplacements(testfn, searchfirst, ismusic);
    if (fp != buildvfs_kfd_invalid)
        goto success;

#ifndef USE_PHYSFS
    // look in ./music/<file's parent GRP name>/
    // ex: ./music/duke3d/grabbag.mid
    // ex: ./music/nwinter/grabbag.mid
    if (origparent != NULL)
    {
        char const* const parentextension = Bstrrchr(origparent, '.');
        uint32_t const namelength = parentextension != NULL ? (unsigned)(parentextension - origparent) : parentlength;

        Bsprintf(testfn, "music/%.*s/%s", namelength, origparent, fn);
        fp = S_TryExtensionReplacements(testfn, searchfirst, ismusic);
        if (fp != buildvfs_kfd_invalid)
            goto success;
    }

    // look in ./music/
    // ex: ./music/grabbag.mid
    Bsprintf(testfn, "music/%s", fn);
    fp = S_TryExtensionReplacements(testfn, searchfirst, ismusic);
    if (fp != buildvfs_kfd_invalid)
        goto success;
#endif

    Xfree(testfn);
    return origfp;

success:
    Xfree(testfn);
    kclose(origfp);
    return fp;
}

void G_Polymer_UnInit(void) { }

static inline int32_t calc_smoothratio(ClockTicks totalclk, ClockTicks ototalclk)
{
    // if (!((ud.show_help == 0 && (!g_netServer && ud.multimode < 2) && ((g_player[myconnectindex].ps->gm & MODE_MENU) == 0)) ||
    //       (g_netServer || ud.multimode > 1) ||
    //       ud.recstat == 2) ||
    //     ud.pause_on)
    // {
    //     return 65536;
    // }

/* TODO
    if (bRecord || bPlayback || nFreeze != 0 || bCamera || bPause)
        return 65536;
*/

    int32_t rfreq = (refreshfreq != -1 ? refreshfreq : 60);
    uint64_t elapsedFrames = tabledivide64(((uint64_t)(totalclk - ototalclk).toScale16()) * rfreq, 65536 * 120);
#if 0
    //POGO: additional debug info for testing purposes
    OSD_Printf("Elapsed frames: %" PRIu64 ", smoothratio: %" PRIu64 "\n", elapsedFrames, tabledivide64(65536 * elapsedFrames * 30, rfreq));
#endif
    return clamp(tabledivide64(65536 * elapsedFrames * 30, rfreq), 0, 65536);
}

#endif


#include "compat.h"
#include "build.h"
#include "scriptfile.h"
#include "cache1d.h"
#include "kplib.h"
#include "baselayer.h"

#include "common.h"

#include "vfs.h"

void PrintBuildInfo(void)
{
    buildprint(
        "Built ", s_buildTimestamp, ", "

#if defined __INTEL_COMPILER
        "ICC ", __INTEL_COMPILER / 100, ".", __INTEL_COMPILER % 100, " " __INTEL_COMPILER_BUILD_DATE " (" __VERSION__ ")"
#elif defined __clang__
        "clang "
# ifdef DEBUGGINGAIDS
        __clang_version__
# else
        , __clang_major__, ".", __clang_minor__, ".", __clang_patchlevel__,
# endif
#elif defined _MSC_VER
        "MSVC ",
# if defined _MSC_FULL_VER
            _MSC_FULL_VER / 10000000, ".", _MSC_FULL_VER % 10000000 / 100000, ".", _MSC_FULL_VER % 100000, ".", _MSC_BUILD,
# else
            _MSC_VER / 100, ".", _MSC_VER % 100,
# endif
#elif defined __GNUC__
        "GCC "
# ifdef DEBUGGINGAIDS
            __VERSION__
# else
            , __GNUC__, ".", __GNUC_MINOR__,
#  if defined __GNUC_PATCHLEVEL__
            ".", __GNUC_PATCHLEVEL__,
#  endif
# endif
#else
        "Unknown"
#endif
        ", "
#ifdef BITNESS64
        "64"
#else
        "32"
#endif
        "-bit "
#if B_BIG_ENDIAN == 1
        "big-endian"
#endif
        "\n");

    // TODO: architecture, OS, maybe build and feature settings
}

// def/clipmap handling

// g_defNamePtr can ONLY point to a malloc'd block (length BMAX_PATH)
char *g_defNamePtr = NULL;

void clearDefNamePtr(void)
{
    Xfree(g_defNamePtr);
    // g_defNamePtr assumed to be assigned to right after
}

GrowArray<char *> g_defModules;

#ifdef HAVE_CLIPSHAPE_FEATURE
GrowArray<char *> g_clipMapFiles;
#endif

void G_AddDef(const char *buffer)
{
    clearDefNamePtr();
    g_defNamePtr = dup_filename(buffer);
    initprintf("Using DEF file \"%s\".\n",g_defNamePtr);
}

void G_AddDefModule(const char *buffer)
{
    g_defModules.append(Xstrdup(buffer));
}

#ifdef HAVE_CLIPSHAPE_FEATURE
void G_AddClipMap(const char *buffer)
{
    g_clipMapFiles.append(Xstrdup(buffer));
}
#endif

//////////

int32_t getatoken(scriptfile *sf, const tokenlist *tl, int32_t ntokens)
{
    char *tok;
    int32_t i;

    if (!sf) return T_ERROR;
    tok = scriptfile_gettoken(sf);
    if (!tok) return T_EOF;

    for (i=ntokens-1; i>=0; i--)
    {
        if (!Bstrcasecmp(tok, tl[i].text))
            return tl[i].tokenid;
    }
    return T_ERROR;
}

//////////

int32_t G_CheckCmdSwitch(int32_t argc, char const * const * argv, const char *str)
{
    int32_t i;
    for (i=0; i<argc; i++)
    {
        if (str && !Bstrcasecmp(argv[i], str))
            return 1;
    }

    return 0;
}

// returns: 1 if file could be opened, 0 else
int32_t testkopen(const char *filename, char searchfirst)
{
    buildvfs_kfd fd = kopen4load(filename, searchfirst);
    if (fd != buildvfs_kfd_invalid)
        kclose(fd);
    return (fd != buildvfs_kfd_invalid);
}

// checks from path and in ZIPs, returns 1 if NOT found
int32_t check_file_exist(const char *fn)
{
#ifdef USE_PHYSFS
    return !PHYSFS_exists(fn);
#else
    int32_t opsm = pathsearchmode;
    char *tfn;

    pathsearchmode = 1;
    if (findfrompath(fn,&tfn) < 0)
    {
        char buf[BMAX_PATH];

        Bstrcpy(buf,fn);
        kzfindfilestart(buf);
        if (!kzfindfile(buf))
        {
            initprintf("Error: file \"%s\" does not exist\n",fn);
            pathsearchmode = opsm;
            return 1;
        }
    }
    else Xfree(tfn);
    pathsearchmode = opsm;

    return 0;
#endif
}


//// FILE NAME / DIRECTORY LISTS ////
void fnlist_clearnames(fnlist_t *fnl)
{
    klistfree(fnl->finddirs);
    klistfree(fnl->findfiles);

    fnl->finddirs = fnl->findfiles = NULL;
    fnl->numfiles = fnl->numdirs = 0;
}

// dirflags, fileflags:
//  -1 means "don't get dirs/files",
//  otherwise ORed to flags for respective klistpath
int32_t fnlist_getnames(fnlist_t *fnl, const char *dirname, const char *pattern,
                        int32_t dirflags, int32_t fileflags)
{
    BUILDVFS_FIND_REC *r;

    fnlist_clearnames(fnl);

    if (dirflags != -1)
        fnl->finddirs = klistpath(dirname, "*", BUILDVFS_FIND_DIR|dirflags);
    if (fileflags != -1)
        fnl->findfiles = klistpath(dirname, pattern, BUILDVFS_FIND_FILE|fileflags);

    for (r=fnl->finddirs; r; r=r->next)
        fnl->numdirs++;
    for (r=fnl->findfiles; r; r=r->next)
        fnl->numfiles++;

    return 0;
}


////

// Copy FN to WBUF and append an extension if it's not there, which is checked
// case-insensitively.
// Returns: 1 if not all characters could be written to WBUF, 0 else.
int32_t maybe_append_ext(char *wbuf, int32_t wbufsiz, const char *fn, const char *ext)
{
    const int32_t slen=Bstrlen(fn), extslen=Bstrlen(ext);
    const int32_t haveext = (slen>=extslen && Bstrcasecmp(&fn[slen-extslen], ext)==0);

    Bassert((intptr_t)wbuf != (intptr_t)fn);  // no aliasing

    // If 'fn' has no extension suffixed, append one.
    return (Bsnprintf(wbuf, wbufsiz, "%s%s", fn, haveext ? "" : ext) >= wbufsiz);
}


int32_t ldist(const void *s1, const void *s2)
{
    auto sp1 = (vec2_t const *)s1;
    auto sp2 = (vec2_t const *)s2;
    return sepldist(sp1->x - sp2->x, sp1->y - sp2->y)
        + (enginecompatibilitymode != ENGINE_EDUKE32);
}

int32_t dist(const void *s1, const void *s2)
{
    auto sp1 = (vec3_t const *)s1;
    auto sp2 = (vec3_t const *)s2;
    return sepdist(sp1->x - sp2->x, sp1->y - sp2->y, sp1->z - sp2->z);
}

int32_t FindDistance2D(int32_t x, int32_t y)
{
    return sepldist(x, y);
}

int32_t FindDistance3D(int32_t x, int32_t y, int32_t z)
{
    return sepdist(x, y, z);
}


// Clear OSD background
void COMMON_clearbackground(int numcols, int numrows)
{
    UNREFERENCED_PARAMETER(numcols);

# ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST && in3dmode())
    {
//        glPushAttrib(GL_FOG_BIT);
        polymost_setFogEnabled(false);
        polymost_useColorOnly(true);

        polymostSet2dView();
        glColor4f(0.f, 0.f, 0.f, 0.67f);
        buildgl_setEnabled(GL_BLEND);
        glRecti(0, 0, xdim, 8*numrows+8);
        glColor4f(0.f, 0.f, 0.f, 1.f);
        glRecti(0, 8*numrows+4, xdim, 8*numrows+8);

//        glPopAttrib();
        polymost_useColorOnly(false);
        polymost_setFogEnabled(true);

        return;
    }
# endif

    CLEARLINES2D(0, min(ydim, numrows*8+8), blackcol*0x01010101);
}

#if defined _WIN32 && !defined EDUKE32_STANDALONE
# define NEED_SHLWAPI_H
# include "windows_inc.h"
# ifndef KEY_WOW64_64KEY
#  define KEY_WOW64_64KEY 0x0100
# endif
# ifndef KEY_WOW64_32KEY
#  define KEY_WOW64_32KEY 0x0200
# endif

int Paths_ReadRegistryValue(char const * const SubKey, char const * const Value, char * const Output, DWORD * OutputSize)
{
    // KEY_WOW64_32KEY gets us around Wow6432Node on 64-bit builds
    REGSAM const wow64keys[] = { KEY_WOW64_32KEY, KEY_WOW64_64KEY };

    for (auto &wow64key : wow64keys)
    {
        HKEY hkey;
        LONG keygood = RegOpenKeyEx(HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ | wow64key, &hkey);

        if (keygood != ERROR_SUCCESS)
            continue;

        LONG retval = SHGetValueA(hkey, SubKey, Value, NULL, Output, OutputSize);

        RegCloseKey(hkey);

        if (retval == ERROR_SUCCESS)
            return 1;
    }

    return 0;
}
#endif

// A bare-bones "parser" for Valve's KeyValues VDF format.
// There is no guarantee this will function properly with ill-formed files (i.e. unbalanced or improper).
static void KeyValues_SkipWhitespace(char *& buf, char * const bufend)
{
    while (1)
    {
        while (buf < bufend && (buf[0] == ' ' || buf[0] == '\n' || buf[0] == '\r' || buf[0] == '\t'))
            ++buf;

        // comments
        if (buf + 2 < bufend && buf[0] == '/' && buf[1] == '/')
        {
            buf += 2;

            while (buf < bufend && buf[0] != '\n' && buf[0] != '\r')
                ++buf;

            continue;
        }

        break;
    }
}
static void KeyValues_SkipToEndOfNormalizedToken(char *& buf, char * const bufend)
{
    ++buf;
    while (buf < bufend && buf[0] != '\0')
        ++buf;
    while (buf < bufend && buf[0] == '\0')
        ++buf;
}
static void KeyValues_SkipToEndOfQuotedToken(char *& buf, char * const bufend)
{
    ++buf;
    while (buf < bufend)
    {
        if (buf + 2 < bufend && buf[0] == '\\' && buf[1] == '"')
            buf += 2;
        else if (buf[0] == '"')
        {
            ++buf;
            break;
        }
        else
            ++buf;
    }
}
static void KeyValues_SkipToEndOfUnquotedToken(char *& buf, char * const bufend)
{
    while (buf < bufend && buf[0] != ' ' && buf[0] != '\n' && buf[0] != '\r' && buf[0] != '\t' && buf[0] != '\0')
        ++buf;
}
static void KeyValues_SkipNextWhatever(char *& buf, char * const bufend)
{
    KeyValues_SkipWhitespace(buf, bufend);

    if (buf >= bufend)
        return;

    if (buf[0] == '{')
    {
        ++buf;

        do
            KeyValues_SkipNextWhatever(buf, bufend);
        while (buf < bufend && buf[0] != '}');

        if (buf < bufend)
            ++buf;
    }
    else if (buf[0] == '\0')
        KeyValues_SkipToEndOfNormalizedToken(buf, bufend);
    else if (buf[0] == '"')
        KeyValues_SkipToEndOfQuotedToken(buf, bufend);
    else if (buf[0] != '}')
        KeyValues_SkipToEndOfUnquotedToken(buf, bufend);

    KeyValues_SkipWhitespace(buf, bufend);
}
static char * KeyValues_NormalizeToken(char *& buf, char * const bufend)
{
    if (buf >= bufend)
        return bufend;

    char * token = buf;

    if (buf[0] == '"')
    {
        buf[0] = '\0';
        KeyValues_SkipToEndOfQuotedToken(buf, bufend);
        buf[-1] = '\0';

        ++token;

        // account for escape sequences
        const char * readseeker = token;
        char * writeseeker = token;
        while (readseeker < buf)
        {
            if (readseeker[0] == '\\')
                ++readseeker;

            writeseeker[0] = readseeker[0];

            ++writeseeker;
            ++readseeker;
        }
        if (writeseeker < buf)
            memset(writeseeker, '\0', buf - writeseeker);
    }
    else if (buf[0] == '\0')
    {
        KeyValues_SkipToEndOfNormalizedToken(buf, bufend);
        ++token;
    }
    else
    {
        KeyValues_SkipToEndOfUnquotedToken(buf, bufend);
        buf[0] = '\0';
    }

    return token;
}
static void KeyValues_SeekToKey(char *& buf, char * const bufend, const char * token)
{
    KeyValues_SkipWhitespace(buf, bufend);

    if (buf >= bufend || buf[0] == '}') // end of scope
        return;

    char * ParentKey = KeyValues_NormalizeToken(buf, bufend);
    while (buf < bufend && Bstrcasecmp(ParentKey, token) != 0)
    {
        KeyValues_SkipNextWhatever(buf, bufend);
        ParentKey = KeyValues_NormalizeToken(buf, bufend);
    }

    KeyValues_SkipWhitespace(buf, bufend);
}
static char * KeyValues_GetValue(char *& buf, char * const bufend)
{
    KeyValues_SkipWhitespace(buf, bufend);

    if (buf >= bufend)
        return nullptr;

    return KeyValues_NormalizeToken(buf, bufend);
}

void Paths_ParseSteamLibraryVDF(const char * fn, PathsParseFunc func)
{
    buildvfs_fd const fd = buildvfs_open_read(fn);
    if (fd == buildvfs_fd_invalid)
        return;

    int32_t size = buildvfs_length(fd);
    if (size <= 0)
        return;

    auto const bufstart = (char *)Xmalloc(size+1);
    char * buf = bufstart;
    auto readsize = (int32_t)buildvfs_read(fd, buf, size);
    buildvfs_close(fd);
    if (readsize != size)
        buildprint("Warning: Read ", readsize, " bytes from \"", fn, "\", expected ", size, "\n");

    char * const bufend = buf + readsize;
    bufend[0] = '\0';

    KeyValues_SeekToKey(buf, bufend, "LibraryFolders");
    if (buf < bufend && buf[0] == '{')
    {
        ++buf;

        for (int i = 1; ; ++i)
        {
            char * seeker = buf;
            char key[24];
            snprintf(key, ARRAY_SIZE(key), "%d", i);
            KeyValues_SeekToKey(seeker, bufend, key);
            if (seeker >= bufend || seeker[0] == '}')
                break;

            if (seeker[0] == '{')
            {
                ++seeker;
                KeyValues_SeekToKey(seeker, bufend, "path");
                if (seeker >= bufend || seeker[0] == '}' || seeker[0] == '{')
                    break;
            }

            char * value = KeyValues_GetValue(seeker, bufend);
            if (value == nullptr)
                break;

            func(value);
        }
    }

    Xfree(bufstart);
}

void Paths_ParseXDGDesktopFile(const char * fn, PathsParseFunc func)
{
    buildvfs_fd const fd = buildvfs_open_read(fn);
    if (fd == buildvfs_fd_invalid)
        return;

    int32_t size = buildvfs_length(fd);
    if (size <= 0)
        return;

    auto const bufstart = (char *)Xmalloc(size+1);
    char * buf = bufstart;
    auto readsize = (int32_t)buildvfs_read(fd, buf, size);
    buildvfs_close(fd);
    if (readsize != size)
        buildprint("Warning: Read ", readsize, " bytes from \"", fn, "\", expected ", size, "\n");

    char * const bufend = buf + readsize;
    bufend[0] = '\0';

    static char const s_PathEquals[] = "Path=";

    while (buf < bufend)
    {
        if (Bstrncmp(buf, s_PathEquals, ARRAY_SIZE(s_PathEquals)-1) == 0)
        {
            const char * path = buf += ARRAY_SIZE(s_PathEquals)-1;

            while (buf < bufend && *buf != '\n')
                ++buf;
            *buf = '\0';

            func(path);

            break;
        }

        while (buf < bufend && *buf++ != '\n') { }
    }

    Xfree(bufstart);
}

void Paths_ParseXDGDesktopFilesFromGOG(const char * homepath, const char * game, PathsParseFunc func)
{
    static char const * const locations[] = { ".local/share/applications", "Desktop" };
    char buf[BMAX_PATH];

    for (char const * location : locations)
    {
        Bsnprintf(buf, sizeof(buf), "%s/%s/gog_com-%s_1.desktop", homepath, location, game);
        Paths_ParseXDGDesktopFile(buf, func);
    }
}

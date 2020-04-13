#include "baselayer.h"

#include "a.h"
#include "build.h"
#include "cache1d.h"
#include "communityapi.h"
#include "compat.h"
#include "osd.h"
#include "polymost.h"
#include "renderlayer.h"

// video
#ifdef _WIN32
#include "winbits.h"
extern "C"
{
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
    __declspec(dllexport) DWORD NvOptimusEnablement                = 0x00000001;
}
#endif // _WIN32

int32_t g_borderless=2;

// input
char    inputdevices = 0;

char    keystatus[NUMKEYS];
char    g_keyFIFO[KEYFIFOSIZ];
char    g_keyAsciiFIFO[KEYFIFOSIZ];
uint8_t g_keyFIFOpos;
uint8_t g_keyFIFOend;
uint8_t g_keyAsciiPos;
uint8_t g_keyAsciiEnd;
char    g_keyRemapTable[NUMKEYS];
char    g_keyNameTable[NUMKEYS][24];

int32_t r_maxfps = -1;
int32_t r_maxfpsoffset;
uint64_t g_frameDelay;

void (*keypresscallback)(int32_t, int32_t);

void keySetCallback(void (*callback)(int32_t, int32_t)) { keypresscallback = callback; }

int32_t keyGetState(int32_t key) { return keystatus[g_keyRemapTable[key]]; }

void keySetState(int32_t key, int32_t state)
{
    keystatus[g_keyRemapTable[key]] = state;

    if (state)
    {
        g_keyFIFO[g_keyFIFOend] = g_keyRemapTable[key];
        g_keyFIFO[(g_keyFIFOend+1)&(KEYFIFOSIZ-1)] = state;
        g_keyFIFOend = ((g_keyFIFOend+2)&(KEYFIFOSIZ-1));
    }
}

char keyGetScan(void)
{
    if (g_keyFIFOpos == g_keyFIFOend)
        return 0;

    char const c    = g_keyFIFO[g_keyFIFOpos];
    g_keyFIFOpos = ((g_keyFIFOpos + 2) & (KEYFIFOSIZ - 1));

    return c;
}

void keyFlushScans(void)
{
    Bmemset(&g_keyFIFO,0,sizeof(g_keyFIFO));
    g_keyFIFOpos = g_keyFIFOend = 0;
}

//
// character-based input functions
//
char keyGetChar(void)
{
    if (g_keyAsciiPos == g_keyAsciiEnd)
        return 0;

    char const c    = g_keyAsciiFIFO[g_keyAsciiPos];
    g_keyAsciiPos = ((g_keyAsciiPos + 1) & (KEYFIFOSIZ - 1));

    return c;
}

void keyFlushChars(void)
{
    Bmemset(&g_keyAsciiFIFO,0,sizeof(g_keyAsciiFIFO));
    g_keyAsciiPos = g_keyAsciiEnd = 0;
}

const char *keyGetName(int32_t num) { return ((unsigned)num >= NUMKEYS) ? NULL : g_keyNameTable[num]; }

vec2_t  g_mousePos;
vec2_t  g_mouseAbs;
int32_t g_mouseBits;
uint8_t g_mouseClickState;

bool g_mouseEnabled;
bool g_mouseGrabbed;
bool g_mouseInsideWindow   = 1;
bool g_mouseLockedToWindow = 1;

void (*g_mouseCallback)(int32_t, int32_t);
void mouseSetCallback(void(*callback)(int32_t, int32_t)) { g_mouseCallback = callback; }

int32_t mouseAdvanceClickState(void)
{
    switch (g_mouseClickState)
    {
        case MOUSE_PRESSED: g_mouseClickState  = MOUSE_HELD; return 1;
        case MOUSE_RELEASED: g_mouseClickState = MOUSE_IDLE; return 1;
        case MOUSE_HELD: return 1;
    }
    return 0;
}

void mouseReadPos(int32_t *x, int32_t *y)
{
    if (!g_mouseEnabled || !g_mouseGrabbed || !appactive)
    {
        *x = *y = 0;
        return;
    }

    *x = g_mousePos.x;
    *y = g_mousePos.y;
    g_mousePos.x = g_mousePos.y = 0;
}

int32_t mouseReadAbs(vec2_t * const pResult, vec2_t const * const pInput)
{
    if (!g_mouseEnabled || !appactive || !g_mouseInsideWindow || (osd && osd->flags & OSD_CAPTURE))
        return 0;

    int32_t const xwidth = max(scale(240<<16, xdim, ydim), 320<<16);

    pResult->x = scale(pInput->x, xwidth, xres) - ((xwidth>>1) - (320<<15));
    pResult->y = scale(pInput->y, 200<<16, yres);

    pResult->y = divscale16(pResult->y - (200<<15), rotatesprite_yxaspect) + (200<<15) - rotatesprite_y_offset;

    return 1;
}

int32_t mouseReadButtons(void)
{
    return (!g_mouseEnabled || !appactive || !g_mouseInsideWindow || (osd && osd->flags & OSD_CAPTURE)) ? 0 : g_mouseBits;
}

controllerinput_t joystick;

void joySetCallback(void (*callback)(int32_t, int32_t)) { joystick.pCallback = callback; }
void joyReadButtons(int32_t *pResult) { *pResult = appactive ? joystick.bits : 0; }

#if defined __linux || defined EDUKE32_BSD || defined __APPLE__
# include <sys/mman.h>
#endif

#if !defined(NOASM) && !defined(GEKKO) && !defined(__ANDROID__)
#ifdef __cplusplus
extern "C" {
#endif
    extern intptr_t dep_begin, dep_end;
#ifdef __cplusplus
}
#endif
#endif

#if !defined(NOASM) && !defined(GEKKO) && !defined(__ANDROID__)
static int32_t nx_unprotect(intptr_t beg, intptr_t end, int prot)
{
# if defined _WIN32
#  define B_PROT_RW PAGE_READWRITE
#  define B_PROT_RX PAGE_EXECUTE_READ
#  define B_PROT_RWX PAGE_EXECUTE_READWRITE

    DWORD oldprot;

    if (!VirtualProtect((LPVOID) beg, (SIZE_T)end - (SIZE_T)beg, prot, &oldprot))
    {
        initprintf("VirtualProtect() error! Crashing in 3... 2... 1...\n");
        return 1;
    }
# elif defined __linux || defined EDUKE32_BSD || defined __APPLE__
#  define B_PROT_RW (PROT_READ|PROT_WRITE)
#  define B_PROT_RX (PROT_READ|PROT_EXEC)
#  define B_PROT_RWX (PROT_READ|PROT_WRITE|PROT_EXEC)

    int32_t pagesize;
    size_t dep_begin_page;
    pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1)
    {
        initprintf("Error getting system page size\n");
        return 1;
    }
    dep_begin_page = ((size_t)beg) & ~(pagesize-1);
    if (mprotect((void *) dep_begin_page, (size_t)end - dep_begin_page, prot) < 0)
    {
        initprintf("Error making code writeable (errno=%d)\n", errno);
        return 1;
    }
# else
#  error "Don't know how to unprotect the self-modifying assembly on this platform!"
# endif

    return 0;
}
#endif


// Calculate ylookup[] and call setvlinebpl()
void calc_ylookup(int32_t bpl, int32_t lastyidx)
{
    int32_t i, j=0;
    static int32_t ylookupsiz;

    Bassert(lastyidx <= MAXYDIM);

    lastyidx++;

    if (lastyidx > ylookupsiz)
    {
        Xaligned_free(ylookup);

        ylookup = (intptr_t *)Xaligned_alloc(16, lastyidx * sizeof(intptr_t));
        ylookupsiz = lastyidx;
    }

    for (i=0; i<=lastyidx-4; i+=4)
    {
        ylookup[i] = j;
        ylookup[i + 1] = j + bpl;
        ylookup[i + 2] = j + (bpl << 1);
        ylookup[i + 3] = j + (bpl * 3);
        j += (bpl << 2);
    }

    for (; i<lastyidx; i++)
    {
        ylookup[i] = j;
        j += bpl;
    }

    setvlinebpl(bpl);
}


void makeasmwriteable(void)
{
#if !defined(NOASM) && !defined(GEKKO) && !defined(__ANDROID__)
    nx_unprotect((intptr_t)&dep_begin, (intptr_t)&dep_end, B_PROT_RWX);
#endif
}

int32_t vsync=0;
int32_t r_finishbeforeswap=0;
int32_t r_glfinish=0;
int32_t g_logFlushWindow = 1;

#ifdef USE_OPENGL
struct glinfo_t glinfo =
{
    "Unknown",  // vendor
    "Unknown",  // renderer
    "0.0.0",    // version
    "",         // extensions

    1.0,        // max anisotropy
    0,          // structure filled
    0,          // supported extensions
};

void fill_glinfo(void)
{
    glinfo.extensions = (const char *)glGetString(GL_EXTENSIONS);
    glinfo.renderer   = (const char *)glGetString(GL_RENDERER);
    glinfo.vendor     = (const char *)glGetString(GL_VENDOR);
    glinfo.version    = (const char *)glGetString(GL_VERSION);

#ifdef POLYMER
    if (!Bstrcmp(glinfo.vendor, "ATI Technologies Inc."))
    {
        pr_ati_fboworkaround = 1;
        initprintf("Enabling ATI FBO color attachment workaround.\n");

        if (Bstrstr(glinfo.renderer, "Radeon X1"))
        {
            pr_ati_nodepthoffset = 1;
            initprintf("Enabling ATI R520 polygon offset workaround.\n");
        }
# ifdef __APPLE__
        // See bug description at http://lists.apple.com/archives/mac-opengl/2005/Oct/msg00169.html
        if (!Bstrncmp(glinfo.renderer, "ATI Radeon 9600", 15))
        {
            pr_ati_textureformat_one = 1;
            initprintf("Enabling ATI Radeon 9600 texture format workaround.\n");
        }
# endif
    }
#endif  // defined POLYMER

    // process the extensions string and flag stuff we recognize
    glinfo.depthtex = !!Bstrstr(glinfo.extensions, "GL_ARB_depth_texture");
    glinfo.fbos     = !!Bstrstr(glinfo.extensions, "GL_EXT_framebuffer_object") || !!Bstrstr(glinfo.extensions, "GL_OES_framebuffer_object");
    glinfo.shadow   = !!Bstrstr(glinfo.extensions, "GL_ARB_shadow");
    glinfo.texnpot  = !!Bstrstr(glinfo.extensions, "GL_ARB_texture_non_power_of_two") || !!Bstrstr(glinfo.extensions, "GL_OES_texture_npot");

#if !defined EDUKE32_GLES
    glinfo.bgra             = !!Bstrstr(glinfo.extensions, "GL_EXT_bgra");
    glinfo.bufferstorage    = !!Bstrstr(glinfo.extensions, "GL_ARB_buffer_storage");
    glinfo.clamptoedge      = !!Bstrstr(glinfo.extensions, "GL_EXT_texture_edge_clamp") || !!Bstrstr(glinfo.extensions, "GL_SGIS_texture_edge_clamp");
    glinfo.debugoutput      = !!Bstrstr(glinfo.extensions, "GL_ARB_debug_output");
    glinfo.depthclamp       = !!Bstrstr(glinfo.extensions, "GL_ARB_depth_clamp");
    glinfo.glsl             = !!Bstrstr(glinfo.extensions, "GL_ARB_shader_objects");
    glinfo.multitex         = !!Bstrstr(glinfo.extensions, "GL_ARB_multitexture");
    glinfo.occlusionqueries = !!Bstrstr(glinfo.extensions, "GL_ARB_occlusion_query");
    glinfo.rect             = !!Bstrstr(glinfo.extensions, "GL_NV_texture_rectangle") || !!Bstrstr(glinfo.extensions, "GL_EXT_texture_rectangle");
    glinfo.sync             = !!Bstrstr(glinfo.extensions, "GL_ARB_sync");
    glinfo.texcompr         = !!Bstrstr(glinfo.extensions, "GL_ARB_texture_compression") && Bstrcmp(glinfo.vendor, "ATI Technologies Inc.");
    glinfo.vbos             = !!Bstrstr(glinfo.extensions, "GL_ARB_vertex_buffer_object");
    glinfo.vsync            = !!Bstrstr(glinfo.extensions, "WGL_EXT_swap_control") || !!Bstrstr(glinfo.extensions, "GLX_EXT_swap_control");

# ifdef DYNAMIC_GLEXT
    if (glinfo.texcompr && (!glCompressedTexImage2D || !glGetCompressedTexImage))
    {
        // lacking the necessary extensions to do this
        initprintf("Warning: the GL driver lacks necessary functions to use caching\n");
        glinfo.texcompr = 0;
    }
# endif
#else
    // don't bother checking because ETC2 et al. are not listed in extensions anyway
    glinfo.texcompr = 1; // !!Bstrstr(glinfo.extensions, "GL_OES_compressed_ETC1_RGB8_texture");
#endif

    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glinfo.maxanisotropy);

    if (!glinfo.filled)
    {
        int32_t oldbpp = bpp;
        bpp = 32;
        osdcmd_glinfo(NULL);
        glinfo.filled = 1;
        bpp = oldbpp;
    }
}

// Used to register the game's / editor's osdcmd_vidmode() functions here.
int32_t (*baselayer_osdcmd_vidmode_func)(osdcmdptr_t parm);

static int osdfunc_setrendermode(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    int32_t m = Bstrtol(parm->parms[0], NULL, 10);

    if (m != REND_CLASSIC && m != REND_POLYMOST && m != REND_POLYMER)
        return OSDCMD_SHOWHELP;

    if ((m==REND_CLASSIC) != (bpp==8) && baselayer_osdcmd_vidmode_func)
    {
        // Mismatch between video mode and requested renderer, do auto switch.
        osdfuncparm_t parm;
        char arg[4];

        const char *ptrptr[1];
        ptrptr[0] = arg;

        Bmemset(&parm, 0, sizeof(parm));

        if (m==REND_CLASSIC)
            Bmemcpy(&arg, "8", 2);
        else
            Bmemcpy(&arg, "32", 3);

        // CAUTION: we assume that the osdcmd_vidmode function doesn't use any
        // other member!
        parm.numparms = 1;
        parm.parms = ptrptr;

        baselayer_osdcmd_vidmode_func(&parm);
    }

    videoSetRenderMode(m);

    char const *renderer = "other";

    switch (videoGetRenderMode())
    {
    case REND_CLASSIC:
#ifdef NOASM
        renderer = "classic software (C)";
#else
        renderer = "classic software (ASM)";
#endif
        break;
    case REND_POLYMOST:
        renderer = "polygonal OpenGL";
        break;
#ifdef POLYMER
    case REND_POLYMER:
        renderer = "great justice (Polymer)";
        break;
#endif
    }

    OSD_Printf("Rendering method changed to %s\n", renderer);

    return OSDCMD_OK;
}

#ifdef DEBUGGINGAIDS
static int osdcmd_hicsetpalettetint(osdcmdptr_t parm)
{
    int32_t parms[8];

    if (parm->numparms < 1 || (int32_t)ARRAY_SIZE(parms) < parm->numparms) return OSDCMD_SHOWHELP;

    size_t i;
    for (i = 0; (int32_t)i < parm->numparms; ++i)
        parms[i] = Batol(parm->parms[i]);
    for (; i < ARRAY_SIZE(parms); ++i)
        parms[i] = 0;

    // order is intentional
    hicsetpalettetint(parms[0],parms[1],parms[2],parms[3],parms[5],parms[6],parms[7],parms[4]);

    return OSDCMD_OK;
}
#endif

int osdcmd_glinfo(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    if (bpp == 8)
    {
        initprintf("glinfo: not in OpenGL mode!\n");
        return OSDCMD_OK;
    }

    initprintf("OpenGL information\n %s %s %s\n",
               glinfo.vendor, glinfo.renderer, glinfo.version);

    if (!glinfo.filled)
        return OSDCMD_OK;

    char const *s[] = { "supported", "not supported" };

#define SUPPORTED(x) (x ? s[0] : s[1])

    initprintf(" BGRA textures:           %s\n", SUPPORTED(glinfo.bgra));
    initprintf(" Clamp-to-edge:           %s\n", SUPPORTED(glinfo.clamptoedge));
    initprintf(" Framebuffer objects:     %s\n", SUPPORTED(glinfo.fbos));
    initprintf(" Multi-texturing:         %s\n", SUPPORTED(glinfo.multitex));
    initprintf(" Non-power-of-2 textures: %s\n", SUPPORTED(glinfo.texnpot));
#ifndef EDUKE32_GLES
    initprintf(" Buffer storage:          %s\n", SUPPORTED(glinfo.bufferstorage));
    initprintf(" Debug output:            %s\n", SUPPORTED(glinfo.debugoutput));
    initprintf(" Depth textures:          %s\n", SUPPORTED(glinfo.depthtex));
    initprintf(" GLSL:                    %s\n", SUPPORTED(glinfo.glsl));
    initprintf(" Occlusion queries:       %s\n", SUPPORTED(glinfo.occlusionqueries));
    initprintf(" Rectangle textures:      %s\n", SUPPORTED(glinfo.rect));
    initprintf(" Shadow textures:         %s\n", SUPPORTED(glinfo.shadow));
    initprintf(" Sync:                    %s\n", SUPPORTED(glinfo.sync));
    initprintf(" Texture compression:     %s\n", SUPPORTED(glinfo.texcompr));
    initprintf(" Vertex buffer objects:   %s\n", SUPPORTED(glinfo.vbos));
#endif
    initprintf(" Maximum anisotropy:      %.1f%s\n", glinfo.maxanisotropy, glinfo.maxanisotropy > 1.0 ? "" : " (no anisotropic filtering)");

#undef SUPPORTED

    initprintf(" Extensions:\n%s", glinfo.extensions);

    return OSDCMD_OK;
}
#endif

static int osdcmd_cvar_set_baselayer(osdcmdptr_t parm)
{
    int32_t r = osdcmd_cvar_set(parm);

    if (r != OSDCMD_OK) return r;

    if (!Bstrcasecmp(parm->name, "vid_gamma") || !Bstrcasecmp(parm->name, "vid_brightness") || !Bstrcasecmp(parm->name, "vid_contrast"))
    {
        videoSetPalette(GAMMA_CALC,0,0);
        return r;
    }
    else if (!Bstrcasecmp(parm->name, "r_maxfps") || !Bstrcasecmp(parm->name, "r_maxfpsoffset"))
    {
        if (r_maxfps > 0) r_maxfps = clamp(r_maxfps, 30, 1000);
        g_frameDelay = calcFrameDelay(r_maxfps, r_maxfpsoffset);
    }
    return r;
}

int32_t baselayer_init(void)
{
#ifdef _WIN32
// on Windows, don't save the "r_screenaspect" cvar because the physical screen size is
// determined at startup
# define SCREENASPECT_CVAR_TYPE (CVAR_UINT|CVAR_NOSAVE)
#else
# define SCREENASPECT_CVAR_TYPE (CVAR_UINT)
#endif
    static osdcvardata_t cvars_engine[] =
    {
        { "lz4compressionlevel","adjust LZ4 compression level used for savegames",(void *) &lz4CompressionLevel, CVAR_INT, 1, 32 },
        { "r_borderless", "borderless windowed mode: 0: never  1: always  2: if resolution matches desktop", (void *) &r_borderless, CVAR_INT|CVAR_RESTARTVID, 0, 2 },
        { "r_displayindex","index of output display",(void *)&r_displayindex, CVAR_INT|CVAR_RESTARTVID, 0, 10 },
        { "r_usenewaspect","enable/disable new screen aspect ratio determination code",(void *) &r_usenewaspect, CVAR_BOOL, 0, 1 },
        { "r_screenaspect","if using r_usenewaspect and in fullscreen, screen aspect ratio in the form XXYY, e.g. 1609 for 16:9",
          (void *) &r_screenxy, SCREENASPECT_CVAR_TYPE, 0, 9999 },
        { "r_fpgrouscan","use floating-point numbers for slope rendering",(void *) &r_fpgrouscan, CVAR_BOOL, 0, 1 },
        { "r_novoxmips","turn off/on the use of mipmaps when rendering 8-bit voxels",(void *) &novoxmips, CVAR_BOOL, 0, 1 },
        { "r_voxels","enable/disable automatic sprite->voxel rendering",(void *) &usevoxels, CVAR_BOOL, 0, 1 },
        { "r_maxfps", "limit the frame rate", (void *)&r_maxfps, CVAR_INT | CVAR_FUNCPTR, -1, 1000 },
        { "r_maxfpsoffset", "menu-controlled offset for r_maxfps", (void *)&r_maxfpsoffset, CVAR_INT | CVAR_FUNCPTR, -10, 10 },
#ifdef YAX_ENABLE
        { "r_tror_nomaskpass", "enable/disable additional pass in TROR software rendering", (void *)&r_tror_nomaskpass, CVAR_BOOL, 0, 1 },
#endif
        { "r_windowpositioning", "enable/disable window position memory", (void *) &windowpos, CVAR_BOOL, 0, 1 },
        { "vid_gamma","adjusts gamma component of gamma ramp",(void *) &g_videoGamma, CVAR_FLOAT|CVAR_FUNCPTR, 0, 10 },
        { "vid_contrast","adjusts contrast component of gamma ramp",(void *) &g_videoContrast, CVAR_FLOAT|CVAR_FUNCPTR, 0, 10 },
        { "vid_brightness","adjusts brightness component of gamma ramp",(void *) &g_videoBrightness, CVAR_FLOAT|CVAR_FUNCPTR, 0, 10 },
#ifdef DEBUGGINGAIDS
        { "debug1","debug counter",(void *) &debug1, CVAR_FLOAT, -100000, 100000 },
        { "debug2","debug counter",(void *) &debug2, CVAR_FLOAT, -100000, 100000 },
#endif
#ifdef DEBUG_MASK_DRAWING
        { "debug_maskdrawmode", "Show mask draw orders", (void *)&g_maskDrawMode, CVAR_BOOL, 0, 1 },
#endif
    };

    for (auto & i : cvars_engine)
        OSD_RegisterCvar(&i, (i.flags & CVAR_FUNCPTR) ? osdcmd_cvar_set_baselayer : osdcmd_cvar_set);

#ifdef USE_OPENGL
    OSD_RegisterFunction("setrendermode","setrendermode <number>: sets the engine's rendering mode.\n"
                         "Mode numbers are:\n"
                         "   0 - Classic Build software\n"
                         "   3 - Polygonal OpenGL\n"
#ifdef POLYMER
                         "   4 - Great justice renderer (Polymer)\n"
#endif
                         ,
                         osdfunc_setrendermode);

# ifdef DEBUGGINGAIDS
    OSD_RegisterFunction("hicsetpalettetint","hicsetpalettetint: sets palette tinting values",osdcmd_hicsetpalettetint);
# endif

    OSD_RegisterFunction("glinfo","glinfo: shows OpenGL information about the current OpenGL mode",osdcmd_glinfo);

    polymost_initosdfuncs();
#endif

    for (native_t i = 0; i < NUMKEYS; i++) g_keyRemapTable[i] = i;

    return 0;
}

void maybe_redirect_outputs(void)
{
#if !(defined __APPLE__ && defined __BIG_ENDIAN__)
    char *argp;

    // pipe standard outputs to files
    if ((argp = Bgetenv("EDUKE32_LOGSTDOUT")) == NULL || Bstrcasecmp(argp, "TRUE"))
        return;

    FILE *fp = freopen("stdout.txt", "w", stdout);

    if (!fp)
        fp = fopen("stdout.txt", "w");

    if (fp)
    {
        setvbuf(fp, 0, _IONBF, 0);
        *stdout = *fp;
        *stderr = *fp;
    }
#endif
}

int engineFPSLimit(void)
{
    if (!r_maxfps)
        return true;

    g_frameDelay = calcFrameDelay(r_maxfps, r_maxfpsoffset);

    uint64_t const  frameJitter = timerGetPerformanceFrequency() / 1000ull;
    uint64_t        frameTicks;
    static uint64_t nextFrameTicks;
    static uint64_t frameDelay;

    if (g_frameDelay != frameDelay)
    {
        nextFrameTicks = timerGetPerformanceCounter() + g_frameDelay;
        frameDelay = g_frameDelay;
    }

    do { handleevents(); } while (nextFrameTicks - (frameTicks = timerGetPerformanceCounter()) < frameJitter);

    if (nextFrameTicks - frameTicks > g_frameDelay)
    {
        while (nextFrameTicks - frameTicks > g_frameDelay)
            nextFrameTicks += g_frameDelay;

        return true;
    }

    return false;
}

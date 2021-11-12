// Base services interface declaration
// for the Build Engine
// by Jonathon Fowler (jf@jonof.id.au)

#pragma once

#ifndef baselayer_h_
#define baselayer_h_

#include "compat.h"
#include "osd.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int app_main(int argc, char const * const * argv);
extern const char* AppProperName;
extern const char* AppTechnicalName;

void engineCreateAllocator(void);
void engineDestroyAllocator(void);

#ifdef DEBUGGINGAIDS
# define DEBUG_MASK_DRAWING
extern int32_t g_maskDrawMode;
#endif

#define PRINTF_INITIAL_BUFFER_SIZE 32
#define MSGBOX_PRINTF_MAX          1024

extern char quitevent, appactive;
extern char modechange;
extern char nogl;

extern int32_t vsync;
extern int32_t r_finishbeforeswap;
extern int32_t r_glfinish;
extern int32_t r_borderless;
extern int32_t r_windowpositioning;
extern int32_t r_displayindex;

extern void app_crashhandler(void);

// NOTE: these are implemented in game-land so they may be overridden in game specific ways
extern int32_t startwin_open(void);
extern int32_t startwin_close(void);
extern int32_t startwin_puts(const char *);
extern int32_t startwin_settitle(const char *);
extern int32_t startwin_idle(void *);
extern int32_t startwin_run(void);

// video
extern int32_t r_rotatespriteinterp;
extern int32_t r_usenewaspect, newaspect_enable;
extern int32_t r_fpgrouscan;
extern int32_t setaspect_new_use_dimen;
extern uint32_t r_screenxy;
extern int32_t xres, yres, bpp, fullscreen, bytesperline;
extern double refreshfreq;
extern intptr_t frameplace;
extern char offscreenrendering;
extern int32_t nofog;

extern int32_t r_maxfps;
extern int32_t g_numdisplays;
extern int32_t g_displayindex;

void calc_ylookup(int32_t bpl, int32_t lastyidx);

int32_t videoCheckMode(int32_t *x, int32_t *y, int32_t c, int32_t fs, int32_t forced);
int32_t videoSetMode(int32_t x, int32_t y, int32_t c, int32_t fs);
void    videoGetModes(int display = -1);
void    videoResetMode(void);
void    videoEndDrawing(void);
void    videoShowFrame(int32_t);
int32_t videoUpdatePalette(int32_t start, int32_t num);
int32_t videoSetGamma(void);
int32_t videoSetVsync(int32_t newSync);
char const* videoGetDisplayName(int display);
//#define DEBUG_FRAME_LOCKING
#if !defined DEBUG_FRAME_LOCKING
void videoBeginDrawing(void);
#else
void begindrawing_real(void);
# define BEGINDRAWING_SIZE 256
extern uint32_t begindrawing_line[BEGINDRAWING_SIZE];
extern const char *begindrawing_file[BEGINDRAWING_SIZE];
extern int32_t lockcount;
# define videoBeginDrawing() do {                     \
    if (lockcount < BEGINDRAWING_SIZE) {         \
        begindrawing_line[lockcount] = __LINE__; \
        begindrawing_file[lockcount] = __FILE__; \
    }                                            \
    begindrawing_real();                         \
} while(0)
#endif

extern float g_videoGamma, g_videoContrast, g_videoBrightness;

#define DEFAULT_GAMMA 1.0f
#define DEFAULT_CONTRAST 1.0f
#define DEFAULT_BRIGHTNESS 0.0f

#define GAMMA_CALC ((int32_t)(min(max((float)((g_videoGamma - 1.0f) * 10.0f), 0.f), 15.f)))

#ifdef USE_OPENGL
extern int32_t (*baselayer_osdcmd_vidmode_func)(osdcmdptr_t parm);
extern int osdcmd_glinfo(osdcmdptr_t parm);

struct glinfo_t {
    const char *vendor;
    const char *renderer;
    const char *version;
    const char *extensions;

    float maxanisotropy;

    int filled;

    union {
        uint32_t features;
        struct
        {
            int bgra             : 1;
            int bufferstorage    : 1;
            int debugoutput      : 1;
            int depthclamp       : 1;
            int depthtex         : 1;
            int fbos             : 1;
            int glsl             : 1;
            int multitex         : 1;
            int occlusionqueries : 1;
            int rect             : 1;
            int reset_notification : 1;
            int samplerobjects   : 1;
            int shadow           : 1;
            int sync             : 1;
            int texcompr         : 1;
            int texnpot          : 1;
            int vsync            : 1;
        };
    };
};

extern struct glinfo_t glinfo;

extern void fill_glinfo(void);
#endif

vec2_t CONSTEXPR const g_defaultVideoModes []
= { { 2560, 1440 }, { 2560, 1200 }, { 2560, 1080 }, { 1920, 1440 }, { 1920, 1200 }, { 1920, 1080 }, { 1680, 1050 },
    { 1600, 1200 }, { 1600, 900 },  { 1366, 768 },  { 1280, 1024 }, { 1280, 960 },  { 1280, 720 },  { 1152, 864 },
    { 1024, 768 },  { 1024, 600 },  { 800, 600 },   { 640, 480 },   { 640, 400 },   { 512, 384 },   { 480, 360 },
    { 400, 300 },   { 320, 240 },   { 320, 200 },   { 0, 0 } };

extern char inputdevices;

#define DEV_KEYBOARD 0x1
#define DEV_MOUSE    0x2
#define DEV_JOYSTICK 0x4
#define DEV_HAPTIC   0x8

// keys
#define NUMKEYS 256
#define KEYFIFOSIZ 64

char CONSTEXPR const g_keyAsciiTable[128] = {
    0  ,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,  0,   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']', 0,   0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`', 0,   92,  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/', 0,   '*', 0,   32,  0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6',
    '+', '1', '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0  ,   0,   0,   0,   0,   0, 0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,
};

char CONSTEXPR const g_keyAsciiTableShift[128] = {
    0  ,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,  0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}', 0,   0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?', 0,   '*', 0,   32,  0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6',
    '+', '1', '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0  ,   0,   0,   0,   0,   0, 0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,
};

extern char    keystatus[NUMKEYS];
extern char    g_keyFIFO[KEYFIFOSIZ];
extern char    g_keyAsciiFIFO[KEYFIFOSIZ];
extern uint8_t g_keyAsciiPos;
extern uint8_t g_keyAsciiEnd;
extern uint8_t g_keyFIFOend;
extern char    g_keyRemapTable[NUMKEYS];
extern char    g_keyNameTable[NUMKEYS][24];

extern int32_t keyGetState(int32_t key);
extern void keySetState(int32_t key, int32_t state);

// mouse
extern vec2_t  g_mousePos;
extern vec2_t  g_mouseAbs;
extern int32_t g_mouseBits;
extern uint8_t g_mouseClickState;
extern bool    g_mouseGrabbed;
extern bool    g_mouseEnabled;
extern bool    g_mouseInsideWindow;
extern bool    g_mouseLockedToWindow;

enum
{
    MOUSE_IDLE = 0,
    MOUSE_PRESSED,
    MOUSE_HELD,
    MOUSE_RELEASED,
};
extern int32_t mouseAdvanceClickState(void);

// joystick

typedef struct
{
    int32_t *pAxis;
    int32_t *pHat;
    void (*pCallback)(int32_t, int32_t);
    int32_t  bits;
    int32_t  numAxes;
    int32_t  numBalls;
    int32_t  numButtons;
    int32_t  numHats;
    int32_t  isGameController;
    uint32_t validButtons;
} controllerinput_t;

extern controllerinput_t joystick;

extern int32_t qsetmode;

#define in3dmode() (qsetmode==200)

int32_t initsystem(void);
void uninitsystem(void);
void system_getcvars(void);

extern int32_t g_logFlushWindow;
void initputs(const char *);
#define buildputs initputs
int initprintf(const char *, ...) ATTRIBUTE((format(printf,1,2)));
#define buildprintf initprintf
int debugprintf(const char *,...) ATTRIBUTE((format(printf,1,2)));

int32_t handleevents(void);
int32_t handleevents_peekkeys(void);

extern void (*keypresscallback)(int32_t,int32_t);
extern void (*g_mouseCallback)(int32_t,int32_t);
extern void (*g_controllerHotplugCallback)(void);

int32_t initinput(void(*hotplugCallback)(void) = NULL);
void uninitinput(void);
void keySetCallback(void (*callback)(int32_t,int32_t));
void mouseSetCallback(void (*callback)(int32_t,int32_t));
void joySetCallback(void (*callback)(int32_t,int32_t));
const char *keyGetName(int32_t num);
const char *joyGetName(int32_t what, int32_t num); // what: 0=axis, 1=button, 2=hat
void joyScanDevices(void);

char keyGetScan(void);
char keyGetChar(void);
#define keyBufferWaiting() (g_keyAsciiPos != g_keyAsciiEnd)

static FORCE_INLINE int keyBufferFull(void)
{
    return ((g_keyAsciiEnd+1)&(KEYFIFOSIZ-1)) == g_keyAsciiPos;
}

static FORCE_INLINE void keyBufferInsert(char code)
{
    g_keyAsciiFIFO[g_keyAsciiEnd] = code;
    g_keyAsciiEnd = ((g_keyAsciiEnd+1)&(KEYFIFOSIZ-1));
}

void keyFlushScans(void);
void keyFlushChars(void);

void mouseInit(void);
void mouseUninit(void);
int32_t mouseReadAbs(vec2_t *pResult, vec2_t const *pInput);
void mouseGrabInput(bool grab);
void mouseLockToWindow(char a);
int32_t mouseReadButtons(void);
void mouseReadPos(int32_t *x, int32_t *y);

bool joyHasButton(int button);
void joyReadButtons(int32_t *pResult);
extern int32_t inputchecked;

int32_t wm_msgbox(const char *name, const char *fmt, ...) ATTRIBUTE((format(printf,2,3)));
int32_t wm_ynbox(const char *name, const char *fmt, ...) ATTRIBUTE((format(printf,2,3)));
void wm_setapptitle(const char *name);

// baselayer.c
int32_t baselayer_init();

void makeasmwriteable(void);
void maybe_redirect_outputs(void);

extern uint64_t g_frameDelay;
static inline uint64_t calcFrameDelay(int maxFPS)
{
    uint64_t perfFreq = timerGetNanoTickRate();

    switch (maxFPS)
    {
        case -1: maxFPS = refreshfreq; break;
        case 0: perfFreq = 0; break;
    }

    return tabledivide64(perfFreq, maxFPS);
}
extern int engineFPSLimit(void);
#ifdef __cplusplus
}
#endif

static inline int32_t calc_smoothratio(ClockTicks const totalclk, ClockTicks const ototalclk, int gameTicRate)
{
    int const   tfreq = (int)floorf(refreshfreq * 120 / timerGetClockRate());
    int const   clk   = (totalclk - ototalclk).toScale16();
    float const tics  = clk * tfreq * (1.f / (65536.f * 120));
    int const   ratio = tabledivide32_noinline((int)(65536 * tics * gameTicRate), tfreq);

#if 0 //ndef NDEBUG
    if ((unsigned)ratio > 66048)
        OSD_Printf("calc_smoothratio: ratio: %d\n", ratio);
#endif
    return clamp(ratio, 0, 65536);
}

#include "print.h"

#endif // baselayer_h_


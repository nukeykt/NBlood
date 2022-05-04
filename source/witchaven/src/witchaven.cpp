/***************************************************************************
 *    WITCHAVEN.C  - main game code for Witchaven game                     *
 *                                                                         *
 ***************************************************************************/

#define WHAVEN
#define GAME
#define SVGA

#include "witchaven.h"

#include "keyboard.h"
#include "control.h"
#include "config.h"
#include "compat.h"
#include "baselayer.h"
#include "renderlayer.h"
#include "build.h"
#include "common.h"
#include "objects.h"
#include "player.h"
#include "effects.h"
#include "menu.h"
#include "input.h"
#include "animation.h"
#include "tags.h"
#include "network.h"
#include "sound.h"
#include "grpscan.h"
#include "osdcmds.h"
#include "common_game.h"

#ifdef _WIN32
# include "winbits.h"
#endif /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif
    extern const char* s_buildRev;
    extern const char* s_buildTimestamp;
#ifdef __cplusplus
}
#endif

const char* AppProperName = APPNAME;
const char* AppTechnicalName = APPBASENAME;

static char g_rootDir[BMAX_PATH];

int mouseaiming, aimmode, mouseflip;
int runkey_mode, auto_run;


#if defined GEKKO
# define FPS_YOFFSET 16
#else
# define FPS_YOFFSET 0
#endif

extern "C" void M32RunScript(const char* s);
void M32RunScript(const char* s) { UNREFERENCED_PARAMETER(s); }
void app_crashhandler(void)
{
    shutdown();
}

void InstallEngine();

int32_t synctics;
int32_t globhiz, globloz, globhihit, globlohit;

char option[NUMOPTIONS] = { 0,0,0,0,0,0,1,0 };

// TODO / FIXME / CHECKME - old Build visibility variable. What happened it?
int32_t visibility;

int svga = 0; // REVERT / CHECKME - was originally 0
int delaycnt, engineinitflag, timerinitflag, videoinitflag;
int netgame = 0;
int gametype;
int godmode = 0; // WH2

char boardname[BMAX_PATH];
char tempboard[BMAX_PATH];
char loadgamename[BMAX_PATH];

int32_t* animateptr[MAXANIMATES];
int32_t animategoal[MAXANIMATES];
int32_t animatevel[MAXANIMATES];
int32_t animatecnt = 0;

// WH2 variables
int treasurescnt = 0;
int treasuresfound = 0;
int killcnt = 0;
int kills = 0;
int expgained = 0;
int bonus = 0;
int overtheshoulder;
static char detailmode = 0;
static char ready2send = 0;
int32_t screentilt = 0;
extern int32_t cachecount, transarea;
extern char chainstat;


uint32_t flags32[32]={
    0x80000000,0x40000000,0x20000000,0x10000000,
    0x08000000,0x04000000,0x02000000,0x01000000,
    0x00800000,0x00400000,0x00200000,0x00100000,
    0x00080000,0x00040000,0x00020000,0x00010000,
    0x00008000,0x00004000,0x00002000,0x00001000,
    0x00000800,0x00000400,0x00000200,0x00000100,
    0x00000080,0x00000040,0x00000020,0x00000010,
    0x00000008,0x00000004,0x00000002,0x00000001
};

extern short cddrive;

int32_t followx, followy;

int ratcnt = 0;
int gameactivated = 0;
int escapetomenu = 0;

int32_t difficulty = 2;
int32_t goreon = 1;

int32_t totsynctics, frames;

extern bool followmode;
extern int loadedgame;
extern char tempbuf[50];
extern int musiclevel;
extern int digilevel;

struct Delayitem delayitem[MAXSECTORS];

int32_t brightness  = 0;
int32_t gbrightness = 0;

int swingcnt;
short spikecnt;
short spikesector[64];

short goblinspritelist[100];
short goblinwarcnt;

short lavadrylandsector[32];
short lavadrylandcnt;

short xpanningsectorlist[64], xpanningsectorcnt;
short ypanningwalllist[64], ypanningwallcnt;
short floorpanninglist[64], floorpanningcnt;
short skypanlist[64], skypancnt;

short crushsectorlist[32], crushsectorcnt;
short crushsectoranim[32], crushsectordone[32];

short revolvesector[16], revolveang[16], revolveclip[16], revolvecnt;
int revolvex[16][16], revolvey[16][16];
int revolvepivotx[16], revolvepivoty[16];

short dragsectorlist[16], dragxdir[16], dragydir[16], dragsectorcnt;
int dragx1[16], dragy1[16], dragx2[16], dragy2[16], dragfloorz[16];

short warpsectorlist[16], warpsectorcnt;

short bobbingsectorlist[16], bobbingsectorcnt;

//JSA ends

int justteleported = 0;

short ironbarsector[16],ironbarscnt;
int ironbarsgoal1[16],ironbarsgoal2[16];
short ironbarsdone[16], ironbarsanim[16];
int ironbarsgoal[16];

extern int mapon;
int mapflag;

extern char option2[];
extern short mousekeys[];

static int32_t nonsharedtimer;

int adjusthp(int hp);
void playloop();
void readpalettetable();
void drawoverheadmap(Player *plr);

int32_t g_commandSetup = 0;
int32_t g_noSetup = 0;
int32_t g_noAutoLoad = 0;

//////////

enum gametokens
{
    T_INCLUDE = 0,
    T_INTERFACE = 0,
    T_LOADGRP = 1,
    T_MODE = 1,
    T_CACHESIZE = 2,
    T_ALLOW = 2,
    T_NOAUTOLOAD,
    T_INCLUDEDEFAULT,
    T_MUSIC,
    T_SOUND,
    T_FILE,
    T_CUTSCENE,
    T_ANIMSOUNDS,
    T_NOFLOORPALRANGE,
    T_ID,
    T_MINPITCH,
    T_MAXPITCH,
    T_PRIORITY,
    T_TYPE,
    T_DISTANCE,
    T_VOLUME,
    T_DELAY,
    T_RENAMEFILE,
    T_GLOBALGAMEFLAGS,
    T_ASPECT,
    T_FORCEFILTER,
    T_FORCENOFILTER,
    T_TEXTUREFILTER,
    T_NEWGAMECHOICES,
    T_CHOICE,
    T_NAME,
    T_LOCKED,
    T_HIDDEN,
    T_USERCONTENT,
};

int witchaven_globalflags;

static int parsedefinitions_game(scriptfile*, int);

static void parsedefinitions_game_include(const char* fileName, scriptfile* pScript, const char* cmdtokptr, int const firstPass)
{
    scriptfile* included = scriptfile_fromfile(fileName);

    if (!included)
    {
        if (!Bstrcasecmp(cmdtokptr, "null") || pScript == NULL) // this is a bit overboard to prevent unused parameter warnings
        {
            // initprintf("Warning: Failed including %s as module\n", fn);
        }
        /*
                else
                    {
                    initprintf("Warning: Failed including %s on line %s:%d\n",
                               fn, script->filename,scriptfile_getlinum(script,cmdtokptr));
                    }
        */
    }
    else
    {
        parsedefinitions_game(included, firstPass);
        scriptfile_close(included);
    }
}

static int parsedefinitions_game(scriptfile* pScript, int firstPass)
{
    int   token;
    char* pToken;

    static const tokenlist tokens[] =
    {
        { "include",         T_INCLUDE          },
        { "#include",        T_INCLUDE          },
        { "includedefault",  T_INCLUDEDEFAULT   },
        { "#includedefault", T_INCLUDEDEFAULT   },
        { "loadgrp",         T_LOADGRP          },
        { "cachesize",       T_CACHESIZE        },
        { "noautoload",      T_NOAUTOLOAD       },
        { "renamefile",      T_RENAMEFILE       },
        { "globalgameflags", T_GLOBALGAMEFLAGS  },
    };

    do
    {
        token = getatoken(pScript, tokens, ARRAY_SIZE(tokens));
        pToken = pScript->ltextptr;

        switch (token)
        {
            case T_LOADGRP:
            {
                char* fileName;

                pathsearchmode = 1;
                if (!scriptfile_getstring(pScript, &fileName) && firstPass)
                {
                    if (initgroupfile(fileName) == -1)
                        initprintf("Could not find file \"%s\".\n", fileName);
                    else
                    {
                        initprintf("Using file \"%s\" as game data.\n", fileName);
                        if (!g_noAutoLoad && !gSetup.noautoload)
                            G_DoAutoload(fileName);
                    }
                }

                pathsearchmode = 0;
            }
            break;
            case T_CACHESIZE:
            {
                int32_t cacheSize;

                if (scriptfile_getnumber(pScript, &cacheSize) || !firstPass)
                    break;

                if (cacheSize > 0)
                    MAXCACHE1DSIZE = cacheSize << 10;
            }
            break;
            case T_INCLUDE:
            {
                char* fileName;

                if (!scriptfile_getstring(pScript, &fileName))
                    parsedefinitions_game_include(fileName, pScript, pToken, firstPass);

                break;
            }
            case T_INCLUDEDEFAULT:
            {
                parsedefinitions_game_include(G_DefaultDefFile(), pScript, pToken, firstPass);
                break;
            }
            case T_NOAUTOLOAD:
            if (firstPass)
                g_noAutoLoad = 1;
            break;
            case T_GLOBALGAMEFLAGS: scriptfile_getnumber(pScript, &witchaven_globalflags); break;
            case T_EOF: return 0;
            default: break;
        }
    } while (1);

    return 0;
}

int loaddefinitions_game(const char* fileName, int32_t firstPass)
{
    scriptfile* pScript = scriptfile_fromfile(fileName);

    if (pScript)
        parsedefinitions_game(pScript, firstPass);

    for (char const* m : g_defModules)
        parsedefinitions_game_include(m, NULL, "null", firstPass);

    if (pScript)
        scriptfile_close(pScript);

    scriptfile_clearsymbols();

    return 0;
}

#define FPS_COLOR(x) ((x) ? COLOR_RED : COLOR_WHITE)

#define COLOR_RED redcol
#define COLOR_WHITE whitecol

#define LOW_FPS ((videoGetRenderMode() == REND_CLASSIC) ? 35 : 50)
#define SLOW_FRAME_TIME 20

static void G_PrintFPS(void)
{
    static char tempbuf[256];
    static int32_t frameCount;
    static double cumulativeFrameDelay;
    static double lastFrameTime;
    static float lastFPS; // , minFPS = std::numeric_limits<float>::max(), maxFPS;
    //static double minGameUpdate = std::numeric_limits<double>::max(), maxGameUpdate;

    double frameTime = timerGetFractionalTicks();
    double frameDelay = frameTime - lastFrameTime;
    cumulativeFrameDelay += frameDelay;

    if (frameDelay >= 0)
    {
        int32_t x = (xdim <= 640);

        if (r_showfps)
        {
            int32_t chars = Bsprintf(tempbuf, "%.1f ms, %5.1f fps", frameDelay, lastFPS);

            printext256(windowxy2.x - (chars << (3 - x)) + 1, windowxy1.y + 2 + FPS_YOFFSET, blackcol, -1, tempbuf, x);
            printext256(windowxy2.x - (chars << (3 - x)), windowxy1.y + 1 + FPS_YOFFSET,
                FPS_COLOR(lastFPS < LOW_FPS), -1, tempbuf, x);
        }

        if (cumulativeFrameDelay >= 1000.0)
        {
            lastFPS = 1000.f * frameCount / cumulativeFrameDelay;
            // g_frameRate = Blrintf(lastFPS);
            frameCount = 0;
            cumulativeFrameDelay = 0.0;
        }
        frameCount++;
    }
    lastFrameTime = frameTime;
}

void faketimerhandler(void)
{
    return;
}

//
//   basic text functions
//

void cls80x25(int top,int mid,int bot)
{
}

void tprintf(int x, int y, const char *fmt,...)
{
}

void rp_delay(int goal)
{
    bool bExit = false;

    int32_t dagoal = (int)totalclock+goal;

    while (!bExit)
    {
        handleevents();
        if (totalclock == dagoal) {
            bExit = true;
        }
    }
}

//
//   game code
//

void showadditionalinfo()
{
    printf("average synctics = %ld\n", totsynctics / frames);
}

int crashflag;

void shutdown()
{
    CONFIG_WriteSetup(0);

#if 0
    int fil = open("pref.dat", O_BINARY|O_TRUNC|O_CREAT|O_WRONLY, S_IREAD | S_IWRITE);
    if (fil != NULL)
    {
        write(fil, &goreon, 2);
        write(fil, &brightness, 2);
        write(fil, &gbrightness, 2);
        write(fil, &digilevel, 2);
        write(fil, &musiclevel, 2);
        write(fil, &difficulty, 2);
        close(fil);
    }
#endif

    SND_Shutdown();

//TODO   netshutdown();

//    if (engineinitflag) {
        engineUnInit();
//    }

    if (SoundMode) {
//TODO        SND_UnDoBuffers();
    }

    if (videoinitflag) {
// TODO        setvmode(oldvmode);
    }

    //CHECKME uninitkeys();
    uninitgroupfile();

    if (crashflag) {
        return;
    }

    exit(EXIT_SUCCESS);
}

void crash(const char* fmt)
{
    crashflag = 1;
    shutdown();

    printf("\n%s\n", fmt);
    printf("\n\n\n\n\n\n\n\n");
}

void doanimations(int32_t numtics)
{
    for (int i = animatecnt - 1; i >= 0; i--)
    {
        int animval = *animateptr[i];
        if (animval < animategoal[i])
        {
            animval += (numtics * animatevel[i]);
            if (animval > animategoal[i]) {
                animval = animategoal[i];
            }
        }
        if (animval > animategoal[i])
        {
            animval -= (numtics * animatevel[i]);
            if (animval < animategoal[i]) {
                animval = animategoal[i];
            }
        }
        *animateptr[i] = animval;
        if (animval == animategoal[i])
        {
            animatecnt--;
            animateptr[i] = animateptr[animatecnt];
            animategoal[i] = animategoal[animatecnt];
            animatevel[i] = animatevel[animatecnt];
        }
    }
}

int32_t getanimationgoal(int32_t *animptr)
{
    int j = -1;
    for (int i = 0; i < animatecnt; i++)
    {
        if (animptr == animateptr[i])
        {
            j = i;
            break;
        }
    }

    return j;
}

int32_t setanimation(int32_t*animptr, int32_t thegoal, int32_t thevel)
{
    if (animatecnt >= MAXANIMATES-1) {
        return(-1);
    }

    int j = animatecnt;

    for (int i = 0; i < animatecnt; i++)
    {
        if (animptr == animateptr[i])
        {
            j = i;
            break;
        }
    }

    animateptr[j]  = animptr;
    animategoal[j] = thegoal;
    animatevel[j]  = thevel;

    if (j == animatecnt) {
        animatecnt++;
    }

    return animatecnt - 1;
}

void setdelayfunc(void (*func)(int),int item,int delay)
{
    for (int i = 0; i < delaycnt; i++)
    {
        if (delayitem[i].func == func && delayitem[i].item == item)
        {
            if (delay == 0) {
                delayitem[i].func = NULL;
            }
            delayitem[i].timer = delay;
            return;
        }
    }

    if (delay > 0)
    {
        delayitem[delaycnt].func = func;
        delayitem[delaycnt].item = item;
        delayitem[delaycnt].timer = delay;
        delaycnt++;
    }
}

void dodelayitems(int tics)
{
    int cnt = delaycnt;

    for (int i = 0; i < cnt; i++)
    {
        if (delayitem[i].func == NULL)
        {
            int j = delaycnt - 1;
            memmove(&delayitem[i], &delayitem[j], sizeof(Delayitem));
            delaycnt = j;
        }

        if (delayitem[i].timer > 0)
        {
            if ((delayitem[i].timer -= tics) <= 0)
            {
                delayitem[i].timer = 0;
                (*delayitem[i].func)(delayitem[i].item);
                delayitem[i].func = NULL;
            }
        }
    }
}

void setup3dscreen()
{
    int32_t dax, day, dax2, day2;

    Player *plr = &player[pyrn];

    videoSetGameMode(gSetup.fullscreen, gSetup.xdim, gSetup.ydim, gSetup.bpp, 0);

    videoinitflag = 1;

#if 0 // TODO
    switch (option[0])
    {
        case 1:
        {
            if (svga == 0)
            {
                dax = 160 - (plr->screensize >> 1);
                dax2 = dax + plr->screensize - 1;
                day = 84 - (((plr->screensize * 168) / 320) >> 1);
                day2 = STATUSSCREEN - 1;
                videoSetViewableArea(dax, day, dax2, day2);
            }
            else {
                videoSetViewableArea(0, 0, vesares[option[6] & 15][0] - 1, vesares[option[6] & 15][1] - 1);
            }
            break;
        }
        default:
            videoSetViewableArea(0, 0, 319, 199);
        break;
    }

    if (svga == 0)
    {
        if(plr->screensize <= 320)
            permanentwritesprite(0,0,BACKGROUND,0,0,0,319,199,0);
        if(plr->screensize <= 320)
            permanentwritesprite(0,200-46,NEWSTATUSBAR,0,0,0,319,199,0);
    }
    else if(svga == 1)
    {
        permanentwritesprite(0,0,SVGAMENU,0,0,0,639,239,0);
        permanentwritesprite(0,240,SVGAMENU2,0,0,240,639,479,0);
    }
#endif
}

void setupboard(char* fname)
{
    int endwall, i, j, k, startwall;
    int32_t dax, day;
    short treesize;
    int32_t dasector;
    int32_t dax2, day2;
    int16_t daang;

    Player* plr = &player[pyrn];
    randomseed = 17;

    vec3_t startPos;
    int status = engineLoadBoard(fname, 0, &startPos, &daang, &plr->sector);
    if (status == -2)
        status = engineLoadBoardV5V6(fname, 0, &startPos, &daang, &plr->sector);

    if (status == -1) {
        crash("Board not found");
    }

    plr->x = startPos.x;
    plr->y = startPos.y;
    plr->z = startPos.z;

    plr->ang = daang;

#ifdef YAX_ENABLE
    yax_update(1);
#endif

    psky_t* pSky = tileSetupSky(0);

    pSky->tileofs[0] = 0;
    pSky->tileofs[1] = 0;
    pSky->tileofs[2] = 0;
    pSky->tileofs[3] = 0;
    pSky->yoffs = 256;
    pSky->lognumtiles = 2;
    pSky->horizfrac = 65536;
    pSky->yscale = 65536;
    parallaxtype = 2;
    g_visibility = 2048;

    // TODO    precache();

    ratcnt = 0;
    swingcnt = 0;
    xpanningsectorcnt = 0;
    ypanningwallcnt = 0;
    floorpanningcnt = 0;
    crushsectorcnt = 0;
    revolvecnt = 0;
    warpsectorcnt = 0;
    dragsectorcnt = 0;
    ironbarscnt = 0;
    spikecnt = 0;
    bobbingsectorcnt = 0;
    goblinwarcnt = 0;
    lavadrylandcnt = 0;

    for (i = 0; i < MAXSPRITES; i++) // setup sector effect options
    {
        if (sprite[i].picnum == FRED && sprite[i].pal == 1) {
            deletesprite(i);
        }

        if (sprite[i].picnum == RAT) {
            ratcnt++;
            if (ratcnt > 10) {
                deletesprite(i);
            }
        }

        if (sprite[i].picnum == SPAWN) {
            deletesprite(i);
        }

        if (sprite[i].picnum == TORCH) {
            sprite[i].cstat &= ~3;
            changespritestat(i, TORCHLIGHT);
        }

        if (sprite[i].picnum == STANDINTORCH ||
            sprite[i].picnum == BOWLOFFIRE) {
            changespritestat(i, TORCHLIGHT);
        }

        if (sprite[i].picnum == GLOW) {
            changespritestat(i, GLOWLIGHT);
        }

        if (sprite[i].picnum == SNDEFFECT) {
            sector[sprite[i].sectnum].extra = sprite[i].lotag;
            deletesprite(i);
        }

        if (sprite[i].picnum == SNDLOOP) {      //loop on
            sector[sprite[i].sectnum].extra = (32768 | (sprite[i].lotag << 1) | 1);
            deletesprite(i);
        }

        if (sprite[i].picnum == SNDLOOPOFF) {   //loop off
            sector[sprite[i].sectnum].extra = (32768 | (sprite[i].lotag << 1));
            deletesprite(i);
        }

        if (sprite[i].lotag == 80)
        {
            ironbarsector[ironbarscnt] = sprite[i].sectnum;
            ironbarsdone[ironbarscnt] = 0;
            ironbarsanim[ironbarscnt] = i;
            ironbarsgoal[ironbarscnt] = 0;
            ironbarscnt++;
        }

        if (sprite[i].statnum < MAXSTATUS)
        {
            switch (sprite[i].picnum)
            {
                case GRONHAL:
                sprite[i].xrepeat = 30;
                sprite[i].yrepeat = 30;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(300);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                sprite[i].extra = 4;
                break;
                case GRONMU:
                sprite[i].xrepeat = 30;
                sprite[i].yrepeat = 30;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(300);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                sprite[i].extra = 2;
                break;
                case GRONSW:
                sprite[i].xrepeat = 30;
                sprite[i].yrepeat = 30;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(300);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                sprite[i].extra = 0;
                break;
                case RAT:
                sprite[i].xrepeat = 32;
                sprite[i].yrepeat = 32;
                sprite[i].shade = 12;
                sprite[i].pal = 5;
                changespritestat(i, FACE);
                sprite[i].hitag = 10;
                sprite[i].lotag = 100;
                sprite[i].cstat = 0x101;
                break;
                case FISH:
                sprite[i].clipdist = 32;
                changespritestat(i, FACE);
                sprite[i].hitag = 10;
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case WILLOW:
                sprite[i].xrepeat = 32;
                sprite[i].yrepeat = 32;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(400);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case DRAGON:
                sprite[i].xrepeat = 54;
                sprite[i].yrepeat = 54;
                sprite[i].clipdist = 128;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(900);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case DEVIL:
                case DEVILSTAND:
                sprite[i].xrepeat = 36;
                sprite[i].yrepeat = 36;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                if (sprite[i].pal == 2)
                    sprite[i].hitag = adjusthp(60);
                else
                    sprite[i].hitag = adjusthp(50);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case HANGMAN + 1:
                sprite[i].xrepeat = 28;
                sprite[i].yrepeat = 28;
                break;
                case HANGMAN:
                sprite[i].xrepeat = 28;
                sprite[i].yrepeat = 28;
                sprite[i].clipdist = 64;
                changespritestat(i, STAND);
                sprite[i].hitag = adjusthp(30);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case SKELETON:
                sprite[i].xrepeat = 24;
                sprite[i].yrepeat = 24;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(30);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case GOBLINDEAD:
                sprite[i].xrepeat = 36;
                sprite[i].yrepeat = 36;
                break;
                case GOBLIN:
                case GOBLINSTAND:
                if (sprite[i].hitag >= 90 && sprite[i].hitag <= 99)
                {
                    if (sprite[i].pal == 0) {
                        sprite[i].xrepeat = 36;
                        sprite[i].yrepeat = 36;
                    }
                    else {
                        sprite[i].xrepeat = 30;
                        sprite[i].yrepeat = 36;
                    }
                    sprite[i].extra = 0;
                    sprite[i].owner = 0;
                    sprite[i].lotag = 100;
                    sprite[i].cstat |= 0x101;
                    sprite[i].clipdist = 64;
                    goblinspritelist[goblinwarcnt] = i;
                    goblinwarcnt++;
                }
                else
                {
                    sprite[i].xrepeat = 36;
                    sprite[i].yrepeat = 36;
                    changespritestat(i, FACE);
                    if (sprite[i].pal == 5)
                        sprite[i].hitag = adjusthp(35);
                    else if (sprite[i].pal == 4)
                        sprite[i].hitag = adjusthp(25);
                    else
                        sprite[i].hitag = adjusthp(15);
                    sprite[i].lotag = 100;
                    sprite[i].cstat |= 0x101;
                    if (rand() % 100 > 50)
                        sprite[i].extra = 1;
                    sprite[i].clipdist = 64;
                }
                break;
                case GOBLINCHILL:
                sprite[i].xrepeat = 36;
                sprite[i].yrepeat = 36;
                changespritestat(i, STAND);
                sprite[i].hitag = adjusthp(15);
                sprite[i].lotag = 30;
                sprite[i].cstat |= 0x101;
                if (rand() % 100 > 50)
                    sprite[i].extra = 1;
                sprite[i].clipdist = 64;
                break;
                case FATWITCH:
                sprite[i].clipdist = 64;
                sprite[i].xrepeat = 32;
                sprite[i].yrepeat = 32;
                changespritestat(i, FACE);
                if (sprite[i].pal == 7)
                    sprite[i].hitag = adjusthp(290);
                else
                    sprite[i].hitag = adjusthp(280);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                if (rand() % 100 > 50)
                    sprite[i].extra = 1;
                break;
                case SKULLY:
                sprite[i].clipdist = 64;
                sprite[i].xrepeat = 32;
                sprite[i].yrepeat = 32;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(300);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                if (rand() % 100 > 50)
                    sprite[i].extra = 1;
                break;
                case MINOTAUR:
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(100);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                if (rand() % 100 > 50)
                    sprite[i].extra = 1;
                break;
                case FRED:
                case FREDSTAND:
                if (sprite[i].pal == 1)
                    break;
                sprite[i].xrepeat = 48;
                sprite[i].yrepeat = 48;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(40);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                sprite[i].clipdist = 64;
                if (rand() % 100 > 50)
                    sprite[i].extra = 1;
                break;
                case KOBOLD:
                // gave kobold x2 hp
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                if (sprite[i].pal == 8)
                    sprite[i].hitag = adjusthp(40);
                else if (sprite[i].pal == 7)
                    sprite[i].hitag = adjusthp(60);
                else
                    sprite[i].hitag = adjusthp(20);
                sprite[i].lotag = 100;
                sprite[i].xrepeat = sprite[i].yrepeat = 54;
                sprite[i].cstat |= 0x101;
                break;
                case SPIDER:
                sprite[i].xrepeat = 24;
                sprite[i].yrepeat = 18;
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(5);
                sprite[i].lotag = 100;
                sprite[i].cstat |= 0x101;
                break;
                case GUARDIAN:
                sprite[i].clipdist = 64;
                changespritestat(i, FACE);
                sprite[i].hitag = adjusthp(200);
                sprite[i].lotag = 100;
                sprite[i].xrepeat = sprite[i].yrepeat = 32;
                sprite[i].cstat |= 0x101;
                break;
                case JUDYSIT:
                changespritestat(i, WITCHSIT);
                if (mapon > 24)
                    sprite[i].hitag = adjusthp(700);
                else
                    sprite[i].hitag = adjusthp(500);
                sprite[i].lotag = 100;
                sprite[i].xrepeat = sprite[i].yrepeat = 32;
                sprite[i].cstat |= 0x101;
                sprite[i].extra = 1200;
                break;
                case JUDY:
                changespritestat(i, FACE);
                if (mapon > 24)
                    sprite[i].hitag = adjusthp(700);
                else
                    sprite[i].hitag = adjusthp(500);
                sprite[i].lotag = 100;
                sprite[i].xrepeat = sprite[i].yrepeat = 32;
                sprite[i].cstat |= 0x101;
                break;
                case GOBWEAPON:
                case WEAPON1:
                sprite[i].xrepeat = 34;
                sprite[i].yrepeat = 21;
                break;
                case WEAPON2:
                sprite[i].xrepeat = 26;
                sprite[i].yrepeat = 26;
                break;
                case WEAPON3:
                sprite[i].xrepeat = 44;
                sprite[i].yrepeat = 39;
                break;
                case WEAPON4:
                sprite[i].xrepeat = 25;
                sprite[i].yrepeat = 20;
                break;
                case WEAPON6:
                sprite[i].xrepeat = 20;
                sprite[i].yrepeat = 15;
                sprite[i].cstat &= ~3;
                break;
                case WEAPON7:
                sprite[i].xrepeat = 41;
                sprite[i].yrepeat = 36;
                sprite[i].cstat &= ~3;
                break;
                case QUIVER:
                sprite[i].xrepeat = 27;
                sprite[i].yrepeat = 27;
                break;
                case LEATHERARMOR:
                sprite[i].xrepeat = 47;
                sprite[i].yrepeat = 50;
                break;
                case CHAINMAIL:
                case PLATEARMOR:
                case SHIELD:
                sprite[i].xrepeat = sprite[i].yrepeat = 26;
                break;
                case HELMET:
                sprite[i].xrepeat = 27;
                sprite[i].yrepeat = 28;
                break;
                case SCROLLSCARE:
                case SCROLLNUKE:
                case SCROLLFLY:
                case SCROLLFIREBALL:
                case SCROLLFREEZE:
                case SCROLLNIGHT:
                case SCROLLMAGIC:
                sprite[i].xrepeat = 35;
                sprite[i].yrepeat = 36;
                sprite[i].cstat &= ~3;
                break;
                case DIAMONDRING:
                case ADAMANTINERING:
                sprite[i].xrepeat = 14;
                sprite[i].yrepeat = 14;
                break;
                case SHADOWAMULET:
                sprite[i].xrepeat = 30;
                sprite[i].yrepeat = 23;
                break;
                case GLASSSKULL:
                sprite[i].xrepeat = 22;
                sprite[i].yrepeat = 22;
                break;
                case AHNK:
                sprite[i].xrepeat = 51;
                sprite[i].yrepeat = 54;
                break;
                case BLUESCEPTER:
                case YELLOWSCEPTER:
                sprite[i].xrepeat = 32;
                sprite[i].yrepeat = 32;
                break;
                case ONYXRING:
                sprite[i].xrepeat = 42;
                sprite[i].yrepeat = 28;
                break;
                case HORNEDSKULL:
                case CRYSTALSTAFF:
                sprite[i].xrepeat = 64;
                sprite[i].yrepeat = 64;
                break;
                case AMULETOFTHEMIST:
                sprite[i].xrepeat = 26;
                sprite[i].yrepeat = 28;
                break;
                case SAPHIRERING:
                sprite[i].xrepeat = 30;
                sprite[i].yrepeat = 20;
                break;
                case PINE:
                sprite[i].xrepeat = treesize = ((krand() % 5) + 3) << 4;
                sprite[i].yrepeat = treesize;
                break;
                case GIFTBOX:
                sprite[i].xrepeat = 56;
                sprite[i].yrepeat = 49;
                break;
                case GYSER:
                sprite[i].xrepeat = 32;
                sprite[i].yrepeat = 18;
                sprite[i].shade = -17;
                sprite[i].pal = 0;
                //changespritestat(i,DORMANT);
                break;
                case BARREL:
                case VASEA:
                case VASEB:
                case VASEC:
                sprite[i].cstat |= 0x101;
                sprite[i].clipdist = 64;
                break;
                case BRASSKEY:
                sprite[i].xrepeat = sprite[i].yrepeat = 24;
                break;
                case BLACKKEY:
                sprite[i].xrepeat = sprite[i].yrepeat = 24;
                break;
                case GLASSKEY:
                sprite[i].xrepeat = sprite[i].yrepeat = 24;
                break;
                case IVORYKEY:
                sprite[i].xrepeat = sprite[i].yrepeat = 24;
                break;
                case THEHORN:
                sprite[i].xrepeat = sprite[i].yrepeat = 32;
                break;
                case 2233:     // team flags
// TODO                         netmarkflag(i);
                break;
            }
        }
    }

    for (i = 0; i < numsectors; i++)
    {
        if (sector[i].lotag == 100) {
            spikesector[spikecnt++] = i;
        }

        if (sector[i].lotag == 70) {
            skypanlist[skypancnt++] = i;
        }

        if (sector[i].lotag >= 80 && sector[i].lotag <= 89)
        {
            floorpanninglist[floorpanningcnt++] = i;
        }

        if (sector[i].lotag >= 900 && sector[i].lotag <= 999)
        {
            lavadrylandsector[lavadrylandcnt] = i;
            lavadrylandcnt++;
        }

        if (sector[i].lotag >= 2100 && sector[i].lotag <= 2199)
        {
            startwall = sector[i].wallptr;
            endwall = startwall + sector[i].wallnum - 1;
            dax = 0;
            day = 0;
            for (j = startwall; j <= endwall; j++) {
                dax += wall[j].x;
                day += wall[j].y;
            }
            revolvepivotx[revolvecnt] = dax / (endwall - startwall + 1);
            revolvepivoty[revolvecnt] = day / (endwall - startwall + 1);

            k = 0;
            for (j = startwall; j <= endwall; j++) {
                revolvex[revolvecnt][k] = wall[j].x;
                revolvey[revolvecnt][k] = wall[j].y;
                k++;
            }
            revolvesector[revolvecnt] = i;
            revolveang[revolvecnt] = 0;

            revolveclip[revolvecnt] = 1;
            if (sector[i].ceilingz == sector[wall[startwall].nextsector].ceilingz) {
                revolveclip[revolvecnt] = 0;
            }
            revolvecnt++;
        }

        switch (sector[i].lotag) 
        {
            case DOORSWINGTAG:
            startwall = sector[i].wallptr;
            endwall = startwall + sector[i].wallnum - 1;
            for (j = startwall; j <= endwall; j++)
            {
                if (wall[j].lotag == 4) 
                {
                    k = wall[wall[wall[wall[j].point2].point2].point2].point2;
                    if ((wall[j].x == wall[k].x) && (wall[j].y == wall[k].y)) {
                        swingdoor[swingcnt].wall[0] = j;
                        swingdoor[swingcnt].wall[1] = wall[j].point2;
                        swingdoor[swingcnt].wall[2] = wall[wall[j].point2].point2;
                        swingdoor[swingcnt].wall[3] = wall[wall[wall[j].point2].point2].point2;
                        swingdoor[swingcnt].angopen = 1536;
                        swingdoor[swingcnt].angclosed = 0;
                        swingdoor[swingcnt].angopendir = -1;
                    }
                    else {
                        swingdoor[swingcnt].wall[0] = wall[j].point2;
                        swingdoor[swingcnt].wall[1] = j;
                        swingdoor[swingcnt].wall[2] = lastwall(j);
                        swingdoor[swingcnt].wall[3] = lastwall(swingdoor[swingcnt].wall[2]);
                        swingdoor[swingcnt].angopen = 512;
                        swingdoor[swingcnt].angclosed = 0;
                        swingdoor[swingcnt].angopendir = 1;
                    }
                    for (k = 0; k < 4; k++) {
                        swingdoor[swingcnt].x[k] = wall[swingdoor[swingcnt].wall[k]].x;
                        swingdoor[swingcnt].y[k] = wall[swingdoor[swingcnt].wall[k]].y;
                    }
                    swingdoor[swingcnt].sector = i;
                    swingdoor[swingcnt].ang = swingdoor[swingcnt].angclosed;
                    swingdoor[swingcnt].anginc = 0;
                    swingcnt++;
                }
            }
            break;
            case 11:
            xpanningsectorlist[xpanningsectorcnt++] = i;
            break;
            case 12:
            dasector = i;
            dax = 0x7fffffff;
            day = 0x7fffffff;
            dax2 = 0x80000000;
            day2 = 0x80000000;
            startwall = sector[i].wallptr;
            endwall = startwall + sector[i].wallnum - 1;

            for (j = startwall; j <= endwall; j++)
            {
                if (wall[j].x < dax)
                    dax = wall[j].x;
                if (wall[j].y < day)
                    day = wall[j].y;
                if (wall[j].x > dax2)
                    dax2 = wall[j].x;
                if (wall[j].y > day2)
                    day2 = wall[j].y;
                if (wall[j].lotag == 3)
                    k = j;
            }
            if (wall[k].x == dax)
                dragxdir[dragsectorcnt] = -16;
            if (wall[k].y == day)
                dragydir[dragsectorcnt] = -16;
            if (wall[k].x == dax2)
                dragxdir[dragsectorcnt] = 16;
            if (wall[k].y == day2)
                dragydir[dragsectorcnt] = 16;

            dasector = wall[startwall].nextsector;
            dragx1[dragsectorcnt] = 0x7fffffff;
            dragy1[dragsectorcnt] = 0x7fffffff;
            dragx2[dragsectorcnt] = 0x80000000;
            dragy2[dragsectorcnt] = 0x80000000;
            startwall = sector[dasector].wallptr;
            endwall = startwall + sector[dasector].wallnum - 1;

            for (j = startwall; j <= endwall; j++)
            {
                if (wall[j].x < dragx1[dragsectorcnt])
                    dragx1[dragsectorcnt] = wall[j].x;
                if (wall[j].y < dragy1[dragsectorcnt])
                    dragy1[dragsectorcnt] = wall[j].y;
                if (wall[j].x > dragx2[dragsectorcnt])
                    dragx2[dragsectorcnt] = wall[j].x;
                if (wall[j].y > dragy2[dragsectorcnt])
                    dragy2[dragsectorcnt] = wall[j].y;
            }

            dragx1[dragsectorcnt] += (wall[sector[i].wallptr].x - dax);
            dragy1[dragsectorcnt] += (wall[sector[i].wallptr].y - day);
            dragx2[dragsectorcnt] -= (dax2 - wall[sector[i].wallptr].x);
            dragy2[dragsectorcnt] -= (day2 - wall[sector[i].wallptr].y);

            dragfloorz[dragsectorcnt] = sector[i].floorz;

            dragsectorlist[dragsectorcnt++] = i;
            break;
            case 10:
            case 14:
            case 4002:
            warpsectorlist[warpsectorcnt++] = i;
            break;
            case 10000:
            bobbingsectorlist[bobbingsectorcnt++] = i;
        }
        if (sector[i].floorpicnum == TELEPAD && sector[i].lotag == 0) {
            warpsectorlist[warpsectorcnt++] = i;
        }
    }

    ypanningwallcnt = 0;
    for (i = 0; i < numwalls; i++)
    {
        if (wall[i].lotag == 1) {
            ypanningwalllist[ypanningwallcnt++] = i;
        }
    }

    automapping = 1;

    if (justteleported == 1 || loadedgame == 1)
    {
        plr->hvel = 0;
        angvel = 0;
        svel = 0;
        vel = 0;

        plr->spritenum = insertsprite(plr->sector, 0);
        plr->oldsector = plr->sector;

        sprite[plr->spritenum].x = plr->x;
        sprite[plr->spritenum].y = plr->y;
        sprite[plr->spritenum].z = sector[plr->sector].floorz;
        sprite[plr->spritenum].cstat = 1 + 256;
        sprite[plr->spritenum].picnum = FRED;
        sprite[plr->spritenum].shade = 0;
        sprite[plr->spritenum].xrepeat = 36;
        sprite[plr->spritenum].yrepeat = 36;
        sprite[plr->spritenum].ang = plr->ang;
        sprite[plr->spritenum].xvel = 0;
        sprite[plr->spritenum].yvel = 0;
        sprite[plr->spritenum].zvel = 0;
        sprite[plr->spritenum].owner = 4096;
        sprite[plr->spritenum].lotag = 0;
        sprite[plr->spritenum].hitag = 0;
        sprite[plr->spritenum].pal = 1;

        vec3_t pos;
        pos.x = plr->x;
        pos.y = plr->y;
        pos.z = plr->z + (PLAYERHEIGHT << 8);

        setsprite(plr->spritenum, &pos);

        justteleported = 0;
        loadedgame = 0;
    }
}

void DrawHUD()
{
    overwritesprite(0, 200 - 46, NEWSTATUSBAR, 0, 0x2, 0);

    healthpic();
    drawarmor();
    drawpotionpic();
    levelpic();
    drawscore();
    draworbpic();
}

void drawscreen(Player* plr)
{
    if (plr->dimension == 3 || plr->dimension == 2)
    {
        drawrooms(plr->x, plr->y, plr->z, plr->ang, plr->horiz, plr->sector);
        transformactors(plr);

        renderDrawMasks();

        if (!plr->playerdie) {
            drawweapons(plr);
        }

        if (plr->spiked) {
            spikeheart(plr);
        }

        dofx();
    }

    if (plr->dimension == 2) {
        drawoverheadmap(plr);
    }

    monitor();
    screenfx();

    plr->oldsector = plr->sector;

    if (netgame) {
        whnetmon();
    }

    DrawHUD();
    
    //updatepics();

    G_PrintFPS();

    videoNextPage();
}

void timerhandler()
{
    // TODO    totalclock++;

    OSD_DispatchQueued();
}

void InitTimer()
{
    timerInit(kTimerTicks);
    timerSetCallback(timerhandler);
}

int app_main(int argc, char const* const* argv)
{
    char temp1[10] = { "DEMO" };

    Player* plr = &player[pyrn];

    char tempbuf[256];

#ifdef _WIN32
#ifndef DEBUGGINGAIDS
    if (!G_CheckCmdSwitch(argc, argv, "-noinstancechecking") && !windowsCheckAlreadyRunning())
    {
#ifdef EDUKE32_STANDALONE
        if (!wm_ynbox(APPNAME, "It looks like " APPNAME " is already running.\n\n"
#else
        if (!wm_ynbox(APPNAME, "It looks like the game is already running.\n\n"
#endif
            "Are you sure you want to start another copy?"))
            return 3;
    }
#endif
#endif

    G_ExtPreInit(argc, argv);

    OSD_SetLogFile(APPBASENAME ".log");

    OSD_SetFunctions(NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        GAME_clearbackground,
        BGetTime,
        GAME_onshowosd);

    wm_setapptitle(APPNAME);

    initprintf("Witchaven %s\n", s_buildRev);
    PrintBuildInfo();

    // This needs to happen afterwards, as G_CheckCommandLine() is where we set
    // up the command-line-provided search paths (duh).
    G_ExtInit();

    if (!g_useCwd)
        G_AddSearchPaths();

#if defined(RENDERTYPEWIN) && defined(USE_OPENGL)
    if (forcegl) initprintf("GL driver blacklist disabled.\n");
#endif

    // used with binds for fast function lookup
    hash_init(&h_gamefuncs);
    for (bssize_t i = kMaxGameFunctions - 1; i >= 0; i--)
    {
        if (gamefunctions[i][0] == '\0')
            continue;

        hash_add(&h_gamefuncs, gamefunctions[i], i, 0);
    }

#ifdef STARTUP_SETUP_WINDOW
    int const readSetup =
#endif
        CONFIG_ReadSetup();

    if (enginePreInit())
    {
        wm_msgbox("Build Engine Initialization Error",
            "There was a problem initializing the Build engine: %s", engineerrstr);
        ERRprintf("app_main: There was a problem initializing the Build engine: %s\n", engineerrstr);
        Bexit(2);
    }

    if (Bstrcmp(setupfilename, kSetupFilename))
        initprintf("Using config file \"%s\".\n", setupfilename);

    G_ScanGroups();

    wm_msgbox("Pre-Release Software Warning", "%s is not ready for public use. Proceed with caution!", AppProperName);

#ifdef STARTUP_SETUP_WINDOW
    if (readSetup < 0 || (!g_noSetup && gSetup.forcesetup) || g_commandSetup)
    {
        if (quitevent || !startwin_run())
        {
            engineUnInit();
            Bexit(0);
        }
    }
#endif

    G_LoadGroups(!g_noAutoLoad && !gSetup.noautoload);

    CONFIG_WriteSetup(1);
    CONFIG_ReadSetup();

    initprintf("Initializing OSD...\n");

    Bsprintf(tempbuf, "Witchaven %s", s_buildRev);
    OSD_SetVersion(tempbuf, 10, 0);
    OSD_SetParameters(0, 0, 0, 0, 0, 0, OSD_ERROR, OSDTEXT_RED, OSDTEXT_DARKRED, gamefunctions[gamefunc_Show_Console][0] == '\0' ? OSD_PROTECTED : 0);
    registerosdcommands();

    SetupInput();

/*
    char* const setupFileName = Xstrdup(setupfilename);
    char* const p = strtok(setupFileName, ".");

    if (!p || !Bstrcmp(setupfilename, kSetupFilename))
        Bsprintf(tempbuf, "settings.cfg");
    else
        Bsprintf(tempbuf, "%s_settings.cfg", p);

    Xfree(setupFileName);
*/
    OSD_Exec("ewitchaven_cvars.cfg");
    OSD_Exec("ewitchaven_autoexec.cfg");

    CONFIG_SetDefaultKeys(keydefaults, true);

    system_getcvars();

    InstallEngine();

    const char* defsfile = G_DefFile();
    uint32_t stime = timerGetTicks();
    if (!loaddefinitionsfile(defsfile))
    {
        uint32_t etime = timerGetTicks();
        initprintf("Definitions file \"%s\" loaded in %d ms.\n", defsfile, etime - stime);
    }
    loaddefinitions_game(defsfile, FALSE);

    if (enginePostInit())
        shutdown();

    g_frameDelay = calcFrameDelay(r_maxfps);

    KB_Startup();
    InitTimer();

    //TODO    netcheckargs(argc,argv);

    tprintf(-1, 0, "WITCHAVEN Copyright (C) 1995 IntraCorp, Inc. ver 1.1", temp1);
    tprintf(2, 2, " map name: level%d", mapon);
    tprintf(2, 3, " initengine()");

#if 0
    int fil;
    if ((fil = open("setup.dat", O_BINARY | O_RDWR, S_IREAD)) != -1) {
        read(fil, &option, NUMOPTIONS);
        read(fil, keys, WHNUMKEYS);
        read(fil, option2, 7);
        close(fil);
    }

    if (access("pref.dat", F_OK) != 0) {
        goreon = 1;
        brightness = gbrightness = 0;
        digilevel = 11;
        musiclevel = 11;
        fil = open("pref.dat", O_BINARY | O_TRUNC | O_CREAT | O_WRONLY, S_IREAD | S_IWRITE);
        if (fil != NULL) {
            write(fil, &goreon, 2);
            write(fil, &brightness, 2);
            write(fil, &gbrightness, 2);
            write(fil, &digilevel, 2);
            write(fil, &musiclevel, 2);
            write(fil, &difficulty, 2);
            close(fil);
        }
    }
    else {
        fil = open("pref.dat", O_RDONLY | O_BINARY);
        if (fil != NULL) {
            read(fil, &goreon, 2);
            read(fil, &brightness, 2);
            read(fil, &gbrightness, 2);
            read(fil, &digilevel, 2);
            read(fil, &musiclevel, 2);
            read(fil, &difficulty, 2);
            close(fil);
        }
    }
#endif

    if (option[8] == 1 || option[9] == 1)
        MusicMode = 1;
    else
        MusicMode = 0;

    if (option[10] == 1 || option[11] == 1)
        SoundMode = 1;
    else
        SoundMode = 0;

    // TEMP
    MusicMode = 1;
    SoundMode = 1;
    musiclevel = 15;

    //JSA_SPOOGE
    if (SoundMode) {
        //TODO        SND_DoBuffers();
    }

#if 0 // TODO
    if (option[3] != 0) {
        initmouse();
        mousekeys[0] = option2[0];
        mousekeys[1] = option2[1];
    }

    if (option2[2] != 0) {
        initjstick();
    }
#endif
/*
    switch (option[0])
    {
        case 1:
        {
            initengine(1, vesares[option[6] & 15][0], vesares[option[6] & 15][1]);
            if (vesares[option[6] & 15][0] == 640)
                svga = 1;
        }
        break;
        default:
            initengine(1,320,200);
        break;
    }
*/

    /* TODO
        pskyoff[0] = 0;
        pskyoff[1] = 0;
        pskybits = 1;
    */

    SND_Startup();

//    tprintf(2, 4, " loadpics()");
//    tprintf(2, 5, " tiles000.art");

    //** Les END   - 09/06/95
#if 0
    if (vixen = ismscdex()) {
        sprintf(tempbuf, "%c:\\whaven\\intro.smk", vixen);
        if (access(tempbuf, F_OK) != 0) {
            crash("\nCD-ROM NOT DETECTED  \n");
        }
    }
#endif

    initlava();
    initwater();

    tprintf(2, 6, " setupboard()");

    pyrn = 0;
    tprintf(2, 7, " initplayersprite(%d)", pyrn);
    tprintf(2, 8, " resettiming()");
    totalclock = 0;

    //JSA_DEMO start menu song here use
    //flc_init();
    //flc_playseq(plr,0,FT_FULLSCREEN);

    SND_MenuMusic(MENUSONG);

    //precache();

    plr->playerdie = false;
    plr->oldsector = plr->sector;
    plr->horiz = 100;
    plr->zoom = 256;
    if (svga == 1) {
        plr->screensize = 328;
    }
    else {
        plr->screensize = 320;
    }

    plr->dimension = 3;
    plr->height = PLAYERHEIGHT;
    plr->z = sector[plr->sector].floorz - (plr->height << 8);

    //TODO    visibility = 1024;

    parallaxtype = 1;

    setup3dscreen();

    // intro();

    /*
        initpaletteshifts();
        readpalettetable();

        gameactivated=0;
        escapetomenu=0;
    */


    /*
         if (option[MULTIOPT] != 0) {
              initmultiplayers(MAXPLAYERS);
         }
    */

    playloop();

    return 0;
}

void intro()
{
    //loadtile(CAPSTONE);

    //for(i=0;i<32;i++) {
    //   overwritesprite(80,40,CAPSTONE,0,0,0);
    //   videoNextPage();
    //}

    tileLoad(TITLEPIC);

    for (int i = 0; i < 32; i++)
    {
        overwritesprite(0,0,TITLEPIC,32-i,2,0);
        videoNextPage();
    }

    rp_delay(120);

    for (int i = 0; i < 32; i++)
    {
        overwritesprite(0,0,TITLEPIC,i,2,0);
        videoNextPage();
    }
}

ClockTicks ototalclock;

void playloop()
{
    Player *plr = &player[pyrn];

    if (netgame) {
// TODO    initmulti(MAXPLAYERS);
    }

    int exit = 0;

    CONTROL_BindsEnabled = 1;

    ototalclock = totalclock;

    while (!exit)
    {
        static bool frameJustDrawn;
        bool gameUpdate = false;
        double gameUpdateStartTime = timerGetFractionalTicks();

        if (gameactivated == 0 || escapetomenu == 1)
        {
            exit = menuscreen();
        }

        if (totalclock >= (ototalclock + kTicksPerFrame))
        {
            do // while(0)
            {
                if (!frameJustDrawn)
                    break;

                frameJustDrawn = false;

                int32_t oldsynctics = synctics;
                synctics = 4;
                processinput(plr);
                synctics = oldsynctics;

                do
                {
                    ototalclock += kTicksPerFrame;
                    auto const moveClock = totalclock;
    /*
                    if (gameactivated == 0 || escapetomenu == 1)
                    {
                        exit = menuscreen(plr);
                    }
                    else
    */
                    {
                        int32_t oldsynctics = synctics;
                        // temp
                        synctics = kTicksPerFrame;
                        processobjs(plr);
                        animateobjs(plr);
                        animatetags(plr);
                        doanimations(kTicksPerFrame);
                        dodelayitems(kTicksPerFrame);

                        synctics = oldsynctics;
                    }

                    if ((int)(totalclock - moveClock) >= (kTicksPerFrame >> 1))
                        break;
                } 
                while ((totalclock - ototalclock) >= kTicksPerFrame);

                gameUpdate = true;

            } while (0);
        }

		keytimerstuff();

		handleevents();

        drawscreen(plr);

        frameJustDrawn = true;

        #if 0

        handleevents();

        if (gameactivated == 0 || escapetomenu == 1)
        {
            exit = menuscreen(plr);
            #if 0
            if (svga == 0)
            {
                permanentwritesprite(0, 0, BACKGROUND, 0, 0, 0, 319, 199, 0);
                permanentwritesprite(0, 200 - 46, STATUSBAR, 0, 0, 0, 319, 199, 0);
                //updatepics();
            }

            if (svga == 1)
            {
                if (plr->screensize == 320) {
                    overwritesprite(0, 372, SSTATUSBAR, 0, 0, 0);
                    //updatepics();
                }
            }
            #endif

            plr->z = sector[plr->sector].floorz - (PLAYERHEIGHT << 8);
        }

        //        drawscreen(plr);
        processinput(plr);

        if (netgame)
        {
            netgetmove();
            netsendmove();
        }

        processobjs(plr);
        animateobjs(plr);
        animatetags(plr);
        doanimations(synctics);
        dodelayitems(synctics);

        //if (engineFPSLimit()) {
            drawscreen(plr);
        //}
            #endif
    }
}

void drawoverheadmap(Player *plr)
{
#if 0 // TODO
    int i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
    int dax, day, cosang, sinang, xspan, yspan, sprx, spry;
    int xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
    int xvect, yvect, xvect2, yvect2;
    char col;
    walltype *wal, *wal2;
    spritetype *spr;
    short cang;
    int czoom;

    cang=(short)plr->ang;
    czoom=plr->zoom;

    xvect = sintable[(2048-cang)& kAngleMask] * czoom;
    yvect = sintable[(1536-cang)& kAngleMask] * czoom;
    xvect2 = mulscale(xvect,yxaspect,16);
    yvect2 = mulscale(yvect,yxaspect,16);

        //Draw red lines
    for (i=0;i<numsectors;i++)
    {
        startwall = sector[i].wallptr;
        endwall = sector[i].wallptr + sector[i].wallnum - 1;

        z1 = sector[i].ceilingz;
        z2 = sector[i].floorz;

        for(j=startwall,wal=&wall[startwall];j<=endwall;j++,wal++)
        {
            k = wal->nextwall;
            if (k < 0)
                 continue;

            if ((show2dwall[j>>3]&(1<<(j&7))) == 0) {
                 continue;
            }
            if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0))
                 continue;

            if (sector[wal->nextsector].ceilingz == z1)
                if (sector[wal->nextsector].floorz == z2)
                    if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0)
                          continue;

            col = 152;

            if (plr->dimension == 2)
            {
                if (sector[i].floorz != sector[i].ceilingz)
                    if (sector[wal->nextsector].floorz != sector[wal->nextsector].ceilingz)
                        if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0)
                            if (sector[i].floorz == sector[wal->nextsector].floorz)
                                 continue;
                if (sector[i].floorpicnum != sector[wal->nextsector].floorpicnum)
                     continue;
                if (sector[i].floorshade != sector[wal->nextsector].floorshade)
                     continue;
                col = 12;
            }

            ox = wal->x-plr->x; oy = wal->y-plr->y;
            x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
            y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

            wal2 = &wall[wal->point2];
            ox = wal2->x-plr->x; oy = wal2->y-plr->y;
            x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
            y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

            drawline256(x1+(xdim<<11),y1+(ydim<<11),x2+(xdim<<11),y2+(ydim<<11),col);
        }
    }

        //Draw sprites
    k = plr->spritenum;
    for(i=0;i<numsectors;i++)
        for(j=headspritesect[i];j>=0;j=nextspritesect[j])
            if ((show2dsprite[j>>3]&(1<<(j&7))) > 0)
            {
                spr = &sprite[j];
                col = 56;
                if ((spr->cstat&1) > 0)
                     col = 248;
                if (j == k)
                      col = 31;

                sprx = spr->x;
                spry = spr->y;

                k = spr->statnum;
                if ((k >= 1) && (k <= 8) && (k != 2))  //Interpolate moving sprite
                {
                    sprx = osprite[j].x+mulscale(sprx-osprite[j].x,smoothratio,16);
                    spry = osprite[j].y+mulscale(spry-osprite[j].y,smoothratio,16);
                }

                switch (spr->cstat&48)
                {
                    case 0:
                        ox = sprx-plr->x; oy = spry-plr->y;
                        x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                        y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                        if (plr->dimension == 1)
                        {
                            ox = (sintable[(spr->ang+512)& kAngleMask]>>7);
                            oy = (sintable[(spr->ang)& kAngleMask]>>7);
                            x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                            y2 = mulscale(oy,xvect,16) + mulscale(ox,yvect,16);

                            if (j == plr->spritenum)
                            {
                                x2 = 0L;
                                y2 = -(czoom<<5);
                            }

                            x3 = mulscale(x2,yxaspect,16);
                            y3 = mulscale(y2,yxaspect,16);

                            drawline256(x1-x2+(xdim<<11),y1-y3+(ydim<<11),
                                            x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                            drawline256(x1-y2+(xdim<<11),y1+x3+(ydim<<11),
                                            x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                            drawline256(x1+y2+(xdim<<11),y1-x3+(ydim<<11),
                                            x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                        }
                        else
                        {
                            if (((gotsector[i>>3]&(1<<(i&7))) > 0) && (czoom > 192))
                            {
                                daang = (spr->ang-cang)& kAngleMask;
                                if (j == plr->spritenum )
                                    { x1 = 0; y1 = (yxaspect<<2); daang = 0; }
                                rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),mulscale(czoom*spr->yrepeat,yxaspect,16),daang,spr->picnum,spr->shade,spr->pal,(spr->cstat&2)>>1);
                            }
                        }
                        break;
                    case 16:
                        x1 = sprx;
                        y1 = spry;
                        tilenum = spr->picnum;
                        xoff = (int)((signed char)((picanm[tilenum]>>8)&255))+((int)spr->xoffset);
                        if ((spr->cstat&4) > 0)
                             xoff = -xoff;
                        k = spr->ang;
                        l = spr->xrepeat;
                        dax = sintable[k& kAngleMask]*l;
                        day = sintable[(k+1536)& kAngleMask]*l;
                        l = tilesizx[tilenum];
                        k = (l>>1)+xoff;
                        x1 -= mulscale(dax,k,16);
                        x2 = x1+mulscale(dax,l,16);
                        y1 -= mulscale(day,k,16);
                        y2 = y1+mulscale(day,l,16);

                        ox = x1-plr->x;
                        oy = y1-plr->y;
                        x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                        y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                        ox = x2-plr->x;
                        oy = y2-plr->y;
                        x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                        y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                        drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                        x2+(xdim<<11),y2+(ydim<<11),col);

                        break;
                    case 32:
                        if (plr->dimension == 1)
                        {
                            tilenum = spr->picnum;
                            xoff = (int)((signed char)((picanm[tilenum]>>8)&255))+((int)spr->xoffset);
                            yoff = (int)((signed char)((picanm[tilenum]>>16)&255))+((int)spr->yoffset);
                            if ((spr->cstat&4) > 0)
                                 xoff = -xoff;
                            if ((spr->cstat&8) > 0)
                                 yoff = -yoff;

                            k = spr->ang;
                            cosang = sintable[(k+512)& kAngleMask];
                            sinang = sintable[k];
                            xspan = tilesizx[tilenum];
                            xrepeat = spr->xrepeat;
                            yspan = tilesizy[tilenum];
                            yrepeat = spr->yrepeat;

                            dax = ((xspan>>1)+xoff)*xrepeat;
                            day = ((yspan>>1)+yoff)*yrepeat;
                            x1 = sprx + mulscale(sinang,dax,16) + mulscale(cosang,day,16);
                            y1 = spry + mulscale(sinang,day,16) - mulscale(cosang,dax,16);
                            l = xspan*xrepeat;
                            x2 = x1 - mulscale(sinang,l,16);
                            y2 = y1 + mulscale(cosang,l,16);
                            l = yspan*yrepeat;
                            k = -mulscale(cosang,l,16);
                            x3 = x2+k;
                            x4 = x1+k;
                            k = -mulscale(sinang,l,16);
                            y3 = y2+k;
                            y4 = y1+k;

                            ox = x1-plr->x;
                            oy = y1-plr->y;
                            x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                            y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                            ox = x2-plr->x;
                            oy = y2-plr->y;
                            x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                            y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                            ox = x3-plr->x;
                            oy = y3-plr->y;
                            x3 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                            y3 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                            ox = x4-plr->x;
                            oy = y4-plr->y;
                            x4 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                            y4 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                            drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                            x2+(xdim<<11),y2+(ydim<<11),col);

                            drawline256(x2+(xdim<<11),y2+(ydim<<11),
                                            x3+(xdim<<11),y3+(ydim<<11),col);

                            drawline256(x3+(xdim<<11),y3+(ydim<<11),
                                            x4+(xdim<<11),y4+(ydim<<11),col);

                            drawline256(x4+(xdim<<11),y4+(ydim<<11),
                                            x1+(xdim<<11),y1+(ydim<<11),col);

                        }
                        break;
                }
            }

        //Draw white lines
    for(i=0;i<numsectors;i++)
    {
        startwall = sector[i].wallptr;
        endwall = sector[i].wallptr + sector[i].wallnum - 1;

        for(j=startwall,wal=&wall[startwall];j<=endwall;j++,wal++)
        {
            if (wal->nextwall >= 0)
                  continue;

            if ((show2dwall[j>>3]&(1<<(j&7))) == 0) {
                 continue;
            }

            if (tilesizx[wal->picnum] == 0)
                 continue;
            if (tilesizy[wal->picnum] == 0)
                 continue;

            ox = wal->x-plr->x;
            oy = wal->y-plr->y;
            x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
            y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

            wal2 = &wall[wal->point2];
            ox = wal2->x-plr->x;
            oy = wal2->y-plr->y;
            x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
            y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

            drawline256(x1+(xdim<<11),y1+(ydim<<11),x2+(xdim<<11),y2+(ydim<<11),24);
        }
    }
#endif
}

void readpalettetable()
{
    uint8_t palbuf[256];

    int hFile = kopen4loadfrommod("lookup.dat", 0);
    if (hFile == -1)
    {
        // TODO - error
        return;
    }

    uint8_t num_tables = 0;
    kread(hFile, &num_tables, 1);

    for (int j = 0; j < num_tables; j++)
    {
        char lookup_num;
        kread(hFile, &lookup_num, 1);
        kread(hFile, palbuf, 256);

        paletteMakeLookupTable(lookup_num, (const char*)palbuf, 0, 0, 0, 1);
    }

    kclose(hFile);
}

int adjusthp(int hp)
{
    float factor = (krand() % 20) / 100;
    int howhard = difficulty;

    if (krand() % 100 > 50) {
        return((hp * (factor + 1)) * howhard);
    }
    else {
        return((hp - (hp * (factor))) * howhard);
    }
}

void InstallEngine()
{
    // initgroupfile("stuff.dat");

    char* cwd;

    if (g_modDir[0] != '/' && (cwd = buildvfs_getcwd(NULL, 0)))
    {
        buildvfs_chdir(g_modDir);
        if (artLoadFiles("tiles%03i.art", MAXCACHE1DSIZE) < 0)
        {
            buildvfs_chdir(cwd);
            if (artLoadFiles("tiles%03i.art", MAXCACHE1DSIZE) < 0)
                crash("Failed loading art.");
        }
        buildvfs_chdir(cwd);
#ifndef __ANDROID__ //This crashes on *some* Android devices. Small onetime memory leak. TODO fix above function
        Xfree(cwd);
#endif
    }
    else if (artLoadFiles("tiles%03i.art", MAXCACHE1DSIZE) < 0)
        crash("Failed loading art.");

    if (engineInit())
    {
        wm_msgbox("Fatal Engine Initialization Error",
            "There was a problem initializing the engine: %s\n\nThe application will now close.", engineerrstr);
        //TODO:
        //G_Cleanup();
        ERRprintf("G_Startup: There was a problem initializing the engine: %s\n", engineerrstr);
        exit(6);
    }
    if (videoSetGameMode(gSetup.fullscreen, gSetup.xdim, gSetup.ydim, gSetup.bpp, 0) < 0)
    {
        initprintf("Failure setting video mode %dx%dx%d %s! Trying next mode...\n", gSetup.xdim, gSetup.ydim,
            gSetup.bpp, gSetup.fullscreen ? "fullscreen" : "windowed");

        int resIdx = 0;

        for (int i = 0; i < validmodecnt; i++)
        {
            if (validmode[i].xdim == gSetup.xdim && validmode[i].ydim == gSetup.ydim)
            {
                resIdx = i;
                break;
            }
        }

        int const savedIdx = resIdx;
        int bpp = gSetup.bpp;

        while (videoSetGameMode(0, validmode[resIdx].xdim, validmode[resIdx].ydim, bpp, 0) < 0)
        {
            initprintf("Failure setting video mode %dx%dx%d windowed! Trying next mode...\n",
                validmode[resIdx].xdim, validmode[resIdx].ydim, bpp);

            if (++resIdx == validmodecnt)
            {
                if (bpp == 8)
                    crash("Fatal error: unable to set any video mode!");

                resIdx = savedIdx;
                bpp = 8;
            }
        }

        gSetup.xdim = validmode[resIdx].xdim;
        gSetup.ydim = validmode[resIdx].ydim;
        gSetup.bpp = bpp;
    }

    initpaletteshifts();
    readpalettetable();

    // build.obj is dated 9th July 1995
    enginecompatibilitymode = ENGINE_19950829;

    engineinitflag = 1;
}

void RemoveEngine()
{
    engineUnInit();
    uninitgroupfile();
}

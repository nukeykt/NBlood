//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
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

#include "compat.h"
#include "baselayer.h"
#include "renderlayer.h"
#include "typedefs.h"
#include "common.h"
#include "keyboard.h"
#include "control.h"
#include "engine.h"
#include "exhumed.h"
#include "osdcmds.h"
#include "anims.h"
#include "map.h"
#include "sequence.h"
#include "movie.h"
#include "names.h"
#include "menu.h"
#include "player.h"
#include "network.h"
#include "input.h"
#include "sound.h"
#include "cd.h"
#include "view.h"
#include "status.h"
#include "config.h"
#include "init.h"
#include "ra.h"
#include "version.h"
#include "timer.h"
#include "runlist.h"
#include "anubis.h"
#include "spider.h"
#include "mummy.h"
#include "fish.h"
#include "lion.h"
#include "move.h"
#include "lavadude.h"
#include "rex.h"
#include "set.h"
#include "queen.h"
#include "roach.h"
#include "wasp.h"
#include "scorp.h"
#include "rat.h"
#include "serial.h"
#include "network.h"
#include "random.h"
#include "items.h"
#include "trigdat.h"
#include "record.h"
#include "light.h"
#include "lighting.h"
#include "grpscan.h"
#include "save.h"
#include <string.h>
#include <cstdio> // for printf
#include <cstdlib>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

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
const char* defaultpk3filename = "pcexhumed.pk3";

void FinishLevel();
void PrintHelp();

int htimer = 0;

/* these are XORed in the original game executable then XORed back to normal when the game first starts. Here they are normally */
const char *gString[] =
{
    "CINEMAS",
    "THE ANCIENT EGYPTIAN CITY",
    "OF KARNAK HAS BEEN SEIZED",
    "BY UNKNOWN POWERS, AND GREAT",
    "TURMOIL IS SPREADING INTO",
    "NEIGHBORING LANDS, POSING",
    "A GRAVE THREAT TO PLANET",
    "EARTH. MILITANT FORCES FROM",
    "ALL PARTS OF THE GLOBE HAVE",
    "ENTERED THE KARNAK VALLEY,",
    "BUT NONE HAVE RETURNED. THE",
    "ONLY KNOWN INFORMATION",
    "REGARDING THIS CRISIS CAME",
    "FROM A DYING KARNAK VILLAGER",
    "WHO MANAGED TO WANDER OUT OF",
    "THE VALLEY TO SAFETY.",
    "'THEY'VE STOLEN THE GREAT",
    "KING'S MUMMY...', MURMURED",
    "THE DYING VILLAGER, BUT THE",
    "VILLAGER DIED BEFORE HE",
    "COULD SAY MORE. WITH NO",
    "OTHER OPTIONS, WORLD",
    "LEADERS HAVE CHOSEN TO DROP",
    "YOU INTO THE VALLEY VIA",
    "HELICOPTER IN AN ATTEMPT",
    "TO FIND AND DESTROY THE",
    "THREATENING FORCES AND",
    "RESOLVE THE MYSTERY THAT",
    "HAS ENGULFED THIS ONCE",
    "PEACEFUL LAND. FLYING AT",
    "HIGH ALTITUDE TO AVOID",
    "BEING SHOT DOWN LIKE OTHERS",
    "BEFORE YOU, YOUR COPTER",
    "MYSTERIOUSLY EXPLODES IN THE",
    "AIR AS YOU BARELY ESCAPE,",
    "WITH NO POSSIBLE CONTACT",
    "WITH THE OUTSIDE WORLD.",
    "SCARED AS HELL, YOU DESCEND",
    "INTO THE HEART OF KARNAK...",
    "HOME TO THE CELEBRATED",
    "BURIAL CRYPT OF THE GREAT",
    "KING RAMSES.",
    "END",
    "AN EVIL FORCE KNOWN AS THE",
    "KILMAAT HAS BESIEGED THE",
    "SANCTITY OF MY PALACE AND",
    "IS PULLING AT THE VERY",
    "TENDRILS OF MY EXISTENCE.",
    "THESE FORCES INTEND TO",
    "ENSLAVE ME BY REANIMATING",
    "MY PRESERVED CORPSE. I HAVE",
    "PROTECTED MY CORPSE WITH A",
    "GENETIC KEY.  IF YOU ARE",
    "UNSUCCESSFUL I CANNOT",
    "PROTECT CIVILIZATION, AND",
    "CHAOS WILL PREVAIL. I AM",
    "BEING TORN BETWEEN WORLDS",
    "AND THIS INSIDIOUS",
    "EXPERIMENT MUST BE STOPPED.",
    "END",
    "I HAVE HIDDEN A MYSTICAL",
    "GAUNTLET AT EL KAB THAT WILL",
    "CHANNEL MY ENERGY THROUGH",
    "YOUR HANDS. FIND THE",
    "GAUNTLET AND CROSS THE ASWAN",
    "HIGH DAM TO DEFEAT THE EVIL",
    "BEAST SET.",
    "END",
    "SET WAS A FORMIDABLE FOE.",
    "NO MORTAL HAS EVEN CONQUERED",
    "THEIR OWN FEAR, MUCH LESS",
    "ENTERED MORTAL BATTLE AND",
    "TAKEN HIS LIFE.",
    "END",
    "YOU'VE MADE IT HALFWAY TOWARD",
    "FULLFILLING YOUR DESTINY.",
    "THE KILMAAT ARE GROWING",
    "RESTLESS WITH YOUR PROGRESS.",
    "SEEK OUT A TEMPLE IN THIS",
    "CITADEL WHERE I WILL PROVIDE",
    "MORE INFORMATION",
    "END",
    "THE KILMAAT RACE HAS",
    "CONTINUED THEIR MONSTEROUS",
    "ANIMAL-HUMAN EXPERIMENTS IN",
    "AN EFFORT TO SOLVE THE KEY OF",
    "ANIMATING MY CORPSE. THE",
    "VICTORY DEFEATING SET DIDN'T",
    "SLOW YOU DOWN AS MUCH AS",
    "THEY HAD PLANNED. THEY ARE",
    "ACTIVELY ROBBING A SLAVE",
    "GIRL OF HER LIFE TO CREATE",
    "ANOTHER MONSTEROUS",
    "ABOMINATION, COMBINING HUMAN",
    "AND INSECT INTENT ON SLAYING",
    "YOU.  PREPARE YOURSELF FOR",
    "BATTLE AS SHE WILL BE WAITING",
    "FOR YOU AT THE LUXOR TEMPLE. ",
    "END",
    "YOU'VE DONE WELL TO DEFEAT",
    "SELKIS. YOU HAVE DISTRACTED",
    "THE KILMAAT WITH YOUR",
    "PRESENCE AND THEY HAVE",
    "TEMPORARILY ABANDONED",
    "ANIMATION OF MY CORPSE.",
    "THE ALIEN QUEEN KILMAATIKHAN",
    "HAS A PERSONAL VENDETTA",
    "AGAINST YOU. ARROGANCE IS",
    "HER WEAKNESS, AND IF YOU CAN",
    "DEFEAT KILMAATIKHAN, THE",
    "BATTLE WILL BE WON.",
    "END",
    "THE KILMAAT HAVE BEEN",
    "DESTROYED. UNFORTUNATELY,",
    "YOUR RECKLESSNESS HAS",
    "DESTROYED THE EARTH AND ALL",
    "OF ITS INHABITANTS.  ALL THAT",
    "REMAINS IS A SMOLDERING HUNK",
    "OF ROCK.",
    "END",
    "THE KILMAAT HAVE BEEN",
    "DEFEATED AND YOU SINGLE",
    "HANDEDLY SAVED THE EARTH",
    "FROM DESTRUCTION.",
    " ",
    " ",
    " ",
    "YOUR BRAVERY AND HEROISM",
    "ARE LEGENDARY.",
    "END",
    "ITEMS",
    "LIFE BLOOD",
    "LIFE",
    "VENOM",
    "YOU'RE LOSING YOUR GRIP",
    "FULL LIFE",
    "INVINCIBILITY",
    "INVISIBILITY",
    "TORCH",
    "SOBEK MASK",
    "INCREASED WEAPON POWER!",
    "THE MAP!",
    "AN EXTRA LIFE!",
    ".357 MAGNUM!",
    "GRENADE",
    "M-60",
    "FLAME THROWER!",
    "COBRA STAFF!",
    "THE EYE OF RAH GAUNTLET!",
    "SPEED LOADER",
    "AMMO",
    "FUEL",
    "COBRA!",
    "RAW ENERGY",
    "POWER KEY",
    "TIME KEY",
    "WAR KEY",
    "EARTH KEY",
    "MAGIC",
    "LOCATION PRESERVED",
    "COPYRIGHT",
    "LOBOTOMY SOFTWARE, INC.",
    "3D ENGINE BY 3D REALMS",
    "",
    "LASTLEVEL",
    "INCOMING MESSAGE",
    "",
    "OUR LATEST SCANS SHOW",
    "THAT THE ALIEN CRAFT IS",
    "POWERING UP, APPARENTLY",
    "IN AN EFFORT TO LEAVE.",
    "THE BAD NEWS IS THAT THEY",
    "SEEM TO HAVE LEFT A DEVICE",
    "BEHIND, AND ALL EVIDENCE",
    "SAYS ITS GOING TO BLOW A",
    "BIG HOLE IN OUR FINE PLANET.",
    "A SQUAD IS TRYING TO DISMANTLE",
    "IT RIGHT NOW, BUT NO LUCK SO",
    "FAR, AND TIME IS RUNNING OUT.",
    "",
    "GET ABOARD THAT CRAFT NOW",
    "BEFORE IT LEAVES, THEN FIND",
    "AND SHOOT ALL THE ENERGY",
    "TOWERS TO GAIN ACCESS TO THE",
    "CONTROL ROOM. THERE YOU NEED TO",
    "TAKE OUT THE CONTROL PANELS AND",
    "THE CENTRAL POWER SOURCE.  THIS",
    "IS THE BIG ONE BUDDY, BEST OF",
    "LUCK... FOR ALL OF US.",
    "",
    "",
    "CREDITS",
    "EXHUMED",
    "",
    "EXECUTIVE PRODUCERS",
    " ",
    "BRIAN MCNEELY",
    "PAUL LANGE",
    "",
    "GAME CONCEPT",
    " ",
    "PAUL LANGE",
    "",
    "GAME DESIGN",
    " ",
    "BRIAN MCNEELY",
    "",
    "ADDITIONAL DESIGN",
    " ",
    "PAUL KNUTZEN",
    "PAUL LANGE",
    "JOHN VAN DEUSEN",
    "KURT PFEIFER",
    "DOMINICK MEISSNER",
    "DANE EMERSON",
    "",
    "GAME PROGRAMMING",
    " ",
    "KURT PFEIFER",
    "JOHN YUILL",
    "",
    "ADDITIONAL PROGRAMMING",
    " ",
    "PAUL HAUGERUD",
    "",
    "ADDITIONAL TECHNICAL SUPPORT",
    " ",
    "JOHN YUILL",
    "PAUL HAUGERUD",
    "JEFF BLAZIER",
    "",
    "LEVEL DESIGN",
    " ",
    "PAUL KNUTZEN",
    "",
    "ADDITIONAL LEVELS",
    " ",
    "BRIAN MCNEELY",
    "",
    "MONSTERS AND WEAPONS ",
    " ",
    "JOHN VAN DEUSEN",
    "",
    "ARTISTS",
    " ",
    "BRIAN MCNEELY",
    "PAUL KNUTZEN",
    "JOHN VAN DEUSEN",
    "TROY JACOBSON",
    "KEVIN CHUNG",
    "ERIC KLOKSTAD",
    "RICHARD NICHOLS",
    "JOE KRESOJA",
    "JASON WIGGIN",
    "",
    "MUSIC AND SOUND EFFECTS",
    " ",
    "SCOTT BRANSTON",
    "",
    "PRODUCT TESTING",
    " ",
    "DOMINICK MEISSNER",
    "TOM KRISTENSEN",
    "JASON WIGGIN",
    "MARK COATES",
    "",
    "INSTRUCTION MANUAL",
    " ",
    "TOM KRISTENSEN",
    "",
    "SPECIAL THANKS",
    " ",
    "JACQUI LYONS",
    "MARJACQ MICRO, LTD.",
    "MIKE BROWN",
    "IAN MATHIAS",
    "CHERYL LUSCHEI",
    "3D REALMS",
    "KENNETH SILVERMAN",
    "GREG MALONE",
    "MILES DESIGN",
    "REDMOND AM/PM MINI MART",
    "7-11 DOUBLE GULP",
    "",
    "THANKS FOR PLAYING",
    "",
    "THE END",
    "",
    "GUESS YOURE STUCK HERE",
    "UNTIL THE SONG ENDS",
    "",
    "MAYBE THIS IS A GOOD",
    "TIME TO THINK ABOUT ALL",
    "THE THINGS YOU CAN DO",
    "AFTER THE MUSIC IS OVER.",
    "",
    "OR YOU COULD JUST STARE",
    "AT THIS SCREEN",
    "",
    "AND WATCH THESE MESSAGES",
    "GO BY...",
    "",
    "...AND WONDER JUST HOW LONG",
    "WE WILL DRAG THIS OUT...",
    "",
    "AND BELIEVE ME, WE CAN DRAG",
    "IT OUT FOR QUITE A WHILE.",
    "",
    "SHOULD BE OVER SOON...",
    "",
    "ANY MOMENT NOW...",
    "",
    " ",
    "",
    "SEE YA",
    "",
    "END",
    "PASSWORDS",
    "HOLLY",
    "KIMBERLY",
    "LOBOCOP",
    "LOBODEITY",
    "LOBOLITE",
    "LOBOPICK",
    "LOBOSLIP",
    "LOBOSNAKE",
    "LOBOSPHERE",
    "LOBOSWAG",
    "LOBOXY",
    "",
    "PASSINFO",
    "",
    "HI SWEETIE, I LOVE YOU",
    "",
    "",
    "FLASHES TOGGLED",
    "",
    "",
    "",
    "FULL MAP",
    "",
    "",
    "",
    "EOF",
    "",
};

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

int exhumed_globalflags;

static int parsedefinitions_game(scriptfile *, int);

static void parsedefinitions_game_include(const char *fileName, scriptfile *pScript, const char *cmdtokptr, int const firstPass)
{
    scriptfile *included = scriptfile_fromfile(fileName);

    if (!included)
    {
        if (!Bstrcasecmp(cmdtokptr,"null") || pScript == NULL) // this is a bit overboard to prevent unused parameter warnings
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

static int parsedefinitions_game(scriptfile *pScript, int firstPass)
{
    int   token;
    char *pToken;

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
        token  = getatoken(pScript, tokens, ARRAY_SIZE(tokens));
        pToken = pScript->ltextptr;

        switch (token)
        {
        case T_LOADGRP:
        {
            char *fileName;

            pathsearchmode = 1;
            if (!scriptfile_getstring(pScript,&fileName) && firstPass)
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
            char *fileName;

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
        case T_GLOBALGAMEFLAGS: scriptfile_getnumber(pScript, &exhumed_globalflags); break;
        case T_EOF: return 0;
        default: break;
        }
    }
    while (1);

    return 0;
}

int loaddefinitions_game(const char *fileName, int32_t firstPass)
{
    scriptfile *pScript = scriptfile_fromfile(fileName);

    if (pScript)
        parsedefinitions_game(pScript, firstPass);

    for (char const * m : g_defModules)
        parsedefinitions_game_include(m, NULL, "null", firstPass);

    if (pScript)
        scriptfile_close(pScript);

    scriptfile_clearsymbols();

    return 0;
}

////////

void CopyTileToBitmap(short nSrcTile, short nDestTile, int xPos, int yPos);
void DoTitle();

// void TestSaveLoad();
void LoadStatus();
int FindGString(const char *str);
void MySetView(int x1, int y1, int x2, int y2);
void mysetbrightness(char al);

char sHollyStr[40];

short nFontFirstChar;
short nBackgroundPic;
short nShadowPic;

short nCreaturesLeft = 0;
short bNoSound = kFalse;

short nFreeze;

short nSnakeCam = -1;

short nBestLevel;
short levelnew = 1;

int nNetPlayerCount = 0;

short nClockVal;
short fps;
short nRedTicks;
short lastlevel;
volatile short bInMove;
short nAlarmTicks;
short nButtonColor;
short nEnergyChan;


short bSerialPlay = kFalse;
short bModemPlay = kFalse;
int lCountDown = 0;
short nEnergyTowers = 0;


short nCfgNetPlayers = 0;
FILE *vcrfpwrite = NULL;
buildvfs_kfd hVCRRead = buildvfs_kfd_invalid;

short forcelevel = -1;

int lLocalButtons = 0;
int lLocalCodes = 0;

short bHiRes = kFalse;
short bCoordinates = kFalse;

int nNetTime = -1;

short nCodeMin = 0;
short nCodeMax = 0;
short nCodeIndex = 0;

short levelnum = -1;

int moveframes;
int flash;
int localclock;
int totalmoves;

short nCurBodyNum = 0;
short nBodyTotal = 0;

short textpages;
short lastfps;

short nMapMode = 0;
short bNoCreatures = kFalse;

short nTotalPlayers = 1;
// TODO: Rename this (or make it static) so it doesn't conflict with library function
//short socket = 0;

short nFirstPassword = 0;
short nFirstPassInfo = 0;
short nPasswordCount = 0;

// short word_964B0 = 0;
// short word_9AC30 = 0;
// short word_96E3C = 0;
// short word_96E3E = -1;
// short word_96E40 = 0;
// short word_CB326;

int32_t nGamma = 0;
int32_t screensize = 0;
int32_t bFullScreen = 0;
short bSnakeCam = kFalse;
short bRecord = kFalse;
short bPlayback = kFalse;
short bPause = kFalse;
short bInDemo = kFalse;
short bSlipMode = kFalse;
short bDoFlashes = kTrue;
short bHolly = kFalse;

int doTitle = kTrue;

short nItemTextIndex;

short scan_char = 0;

// int nStartLevel;
// int nTimeLimit;

int bVanilla = 0;

char debugBuffer[256];
char gUserMapFilename[BMAX_PATH];
bool bUserMap = false;

short wConsoleNode; // TODO - move me into network file

int mouseaiming, aimmode, mouseflip;
int runkey_mode, auto_run;

ClockTicks tclocks, tclocks2;

void DebugOut(const char *fmt, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, fmt);

    debugBuffer[0] = '\0';

    vsprintf(debugBuffer, fmt, args);

    initprintf("%s", debugBuffer);
    fflush(stdout);

    va_end(args);
#endif
}

void ShutDown(void)
{
    CONFIG_WriteSetup(0);
    StopCD();
    if (bSerialPlay)
    {
        if (bModemPlay) {
            HangUp();
        }
        UnInitSerial();
    }

    KB_Shutdown();
    RemoveEngine();
    UnInitNet();
    UnInitFX();

    exit(EXIT_SUCCESS);
}

void bail2dos(const char *fmt, ...)
{
    char buf[256];

#ifdef __WATCOMC__
    setvmode(3);
#endif

    initputs("bailed to dos\n");

    va_list args;
    va_start(args, fmt);

    vsprintf(buf, fmt, args);

    va_end(args);

    initputs(buf);

    if (*buf != 0)
    {
        if (!(buf[0] == ' ' && buf[1] == 0))
        {
            char titlebuf[256];
            Bsprintf(titlebuf,APPNAME " %s",s_buildRev);
            wm_msgbox(titlebuf, "%s", buf);
        }
    }

    exit(0);
}

void faketimerhandler()
{
    if ((totalclock < ototalclock + 1) || bInMove)
        return;
    ototalclock = ototalclock + 1;

    if (!((int)ototalclock&3) && moveframes < 4)
        moveframes++;

    PlayerInterruptKeys();
}

void timerhandler()
{
    scan_char++;
    if (scan_char == kTimerTicks)
    {
        scan_char = 0;
        lastfps = fps;
        fps = 0;
    }

    if (!bInMove) {
        OSD_DispatchQueued();
    }
}

void HandleAsync()
{
    handleevents();
}

int MyGetStringWidth(const char *str)
{
    int nLen = strlen(str);

    int nWidth = 0;

    for (int i = 0; i < nLen; i++)
    {
        int nPic = seq_GetSeqPicnum(kSeqFont2, 0, str[i] - 32);
        nWidth += tilesiz[nPic].x + 1;
    }

    return nWidth;
}

void UpdateScreenSize()
{
    int xsize = xdim - scale(screensize*16, xdim, 320);
    int ysize = scale(ydim, xsize, xdim);
    int y1 = ((ydim >> 1) - (ysize >> 1));

    MySetView(
        (xdim >> 1) - (xsize >> 1),
        y1,
        (xdim >> 1) - (xsize >> 1) + xsize - 1,
        (y1 + ysize - 1));

    RefreshStatus();
}

void ResetPassword()
{
    nCodeMin = nFirstPassword;
    nCodeIndex = 0;

    nCodeMax = (nFirstPassword + nPasswordCount) - 1;
}

void DoPassword(int nPassword)
{
    if (nNetPlayerCount) {
        return;
    }

    const char *str = gString[nFirstPassInfo + nPassword];

    if (str[0] != '\0') {
        StatusMessage(750, str);
    }

    switch (nPassword)
    {
        case 0:
        {
            if (!nNetPlayerCount) {
                bHolly = kTrue;
            }
            break;
        }

        case 1: // KIMBERLY
        {
            break;
        }

        case 2: // LOBOCOP
        {
            lLocalCodes |= kButtonCheatGuns;
            break;
        }

        case 3: // LOBODEITY
        {
            lLocalCodes |= kButtonCheatGodMode;
            break;
        }

        case 4: // LOBOLITE
        {
            if (bDoFlashes == kFalse)
            {
                bDoFlashes = kTrue;
            }
            else {
                bDoFlashes = kFalse;
            }
            break;
        }

        case 5: // LOBOPICK
        {
            lLocalCodes |= kButtonCheatKeys;
            break;
        }

        case 6: // LOBOSLIP
        {
            if (!nNetPlayerCount)
            {
                if (bSlipMode == kFalse)
                {
                    bSlipMode = kTrue;
                    StatusMessage(300, "Slip mode ON");
                }
                else {
                    bSlipMode = kFalse;
                    StatusMessage(300, "Slip mode OFF");
                }
            }
            break;
        }

        case 7: // LOBOSNAKE
        {
            if (!nNetPlayerCount)
            {
                if (bSnakeCam == kFalse)
                {
                    bSnakeCam = kTrue;
                    StatusMessage(750, "SNAKE CAM ENABLED");
                }
                else
                {
                    bSnakeCam = kFalse;
                    StatusMessage(750, "SNAKE CAM DISABLED");
                }
            }
            break;
        }

        case 8: // LOBOSPHERE
        {
            GrabMap();
            bShowTowers = kTrue;
            break;
        }

        case 9: // LOBOSWAG
        {
            lLocalCodes |= kButtonCheatItems;
            break;
        }

        case 10: // LOBOXY
        {
            if (bCoordinates == kFalse) {
                bCoordinates = kTrue;
            }
            else {
                bCoordinates = kFalse;
            }
            break;
        }

        default:
            return;
    }
}

void mysetbrightness(char nBrightness)
{
    g_visibility = 2048 - (nBrightness << 9);
}

// Replicate original DOS EXE behaviour when pointer is null
static const char *safeStrtok(char *s, const char *d)
{
    const char *r = strtok(s, d);
    return r ? r : "";
}

void CheckKeys()
{
    if (BUTTON(gamefunc_Next_Weapon))
    {
        CONTROL_ClearButton(gamefunc_Next_Weapon);
        SelectNextWeapon();
    }
    else if (BUTTON(gamefunc_Previous_Weapon))
    {
        CONTROL_ClearButton(gamefunc_Previous_Weapon);
        SelectPreviousWeapon();
    }

    if (BUTTON(gamefunc_Enlarge_Screen))
    {
        if (nMapMode == 0)
        {
            if (screensize == 0)
            {
                if (!bFullScreen)
                {
                    bFullScreen = kTrue;
                    UnMaskStatus();
                }
            }
            else
            {
                screensize--;
                if (screensize < 0) {
                    screensize = 0;
                }

                UpdateScreenSize();
            }
            CONTROL_ClearButton(gamefunc_Enlarge_Screen);
        }
    }

    // F11?
    if (BUTTON(gamefunc_Gamma_Correction))
    {
        nGamma++;

        if (nGamma > 4)
            nGamma = 0;

        mysetbrightness((uint8_t)nGamma);
        CONTROL_ClearButton(gamefunc_Gamma_Correction);
        StatusMessage(150, "Gamma level %d", nGamma);
    }

    if (BUTTON(gamefunc_Shrink_Screen))
    {
        if (nMapMode == 0)
        {
            if (bFullScreen)
            {
                bFullScreen = kFalse;
            }
            else
            {
                if ((screensize + 1) < 15)
                    screensize++;
            }

            UpdateScreenSize();
            CONTROL_ClearButton(gamefunc_Shrink_Screen);
        }
    }

    // print version string
    if (KB_KeyDown[sc_V] && KB_KeyDown[sc_LeftAlt])
    {
        KB_KeyDown[sc_V] = 0;
        StatusMessage(300, versionstr);
        return;
    }

    // go to 3rd person view
    if ((KB_KeyDown[sc_C] && KB_KeyDown[sc_LeftAlt]) || BUTTON(gamefunc_Third_Person_View))
    {
        if (!nFreeze)
        {
            if (bCamera) {
                bCamera = kFalse;
            }
            else 
            {
                bCamera = kTrue;
                nCameraDist = 0;
                nCameraClock = (int32_t)totalclock;
            }

            if (bCamera)
                GrabPalette();
        }
        KB_KeyDown[sc_C] = 0;
        CONTROL_ClearButton(gamefunc_Third_Person_View);
        return;
    }

    if (KB_KeyDown[sc_Pause])
    {
        if (!nNetPlayerCount)
        {
            if (bPause)
            {
                ototalclock = totalclock = tclocks;
                bPause = kFalse;
            }
            else
            {
                bPause = kTrue;
                // NoClip();
                // int nLen = MyGetStringWidth("PAUSED");
                // myprintext((320 - nLen) / 2, 100, "PAUSED", 0);
                // Clip();
                // videoNextPage();
            }
            KB_KeyDown[sc_Pause] = 0;
        }
        return;
    }

    // Handle cheat codes
    if (!bInDemo && KB_KeyWaiting())
    {
        char ch = KB_GetCh();

        if (bHolly)
        {
            if (ch)
            {
                size_t nStringLen = strlen(sHollyStr);

                if (ch == asc_Enter)
                {
                    const char *pToken = safeStrtok(sHollyStr, " ");

                    if (!strcmp(pToken, "GOTO"))
                    {
                        // move player to X, Y coordinates
                        int nSprite = PlayerList[0].nSprite;

                        pToken = safeStrtok(NULL, ",");
                        sprite[nSprite].x = atoi(pToken);
                        pToken = safeStrtok(NULL, ",");
                        sprite[nSprite].y = atoi(pToken);

                        setsprite(nSprite, &sprite[nSprite].xyz);
                        sprite[nSprite].z = sector[sprite[nSprite].sectnum].floorz;
                    }
                    else if (!strcmp(pToken, "LEVEL"))
                    {
                        pToken = safeStrtok(NULL, " ");
                        levelnew = atoi(pToken);
                    }
                    else if (!strcmp(pToken, "DOORS"))
                    {
                        for (int i = 0; i < kMaxChannels; i++)
                        {
                            // CHECKME - does this toggle?
                            if (sRunChannels[i].c == 0) {
                                runlist_ChangeChannel(i, 1);
                            }
                            else {
                                runlist_ChangeChannel(i, 0);
                            }
                        }
                    }
                    else if (!strcmp(pToken, "EXIT"))
                    {
                        FinishLevel();
                    }
                    else if (!strcmp(pToken, "CREATURE"))
                    {
                        // i = nNetPlayerCount;
                        if (!nNetPlayerCount)
                        {
                            pToken = safeStrtok(NULL, " ");
                            switch (atoi(pToken))
                            {
                                // TODO - enums?
                                case 0:
                                    BuildAnubis(-1, initx, inity, sector[initsect].floorz, initsect, inita, kFalse);
                                    break;
                                case 1:
                                    BuildSpider(-1, initx, inity, sector[initsect].floorz, initsect, inita);
                                    break;
                                case 2:
                                    BuildMummy(-1, initx, inity, sector[initsect].floorz, initsect, inita);
                                    break;
                                case 3:
                                    BuildFish(-1, initx, inity, initz + eyelevel[nLocalPlayer], initsect, inita);
                                    break;
                                case 4:
                                    BuildLion(-1, initx, inity, sector[initsect].floorz, initsect, inita);
                                    break;
                                case 5:
                                    BuildLava(-1, initx, inity, sector[initsect].floorz, initsect, inita, nNetPlayerCount);
                                    break;
                                case 6:
                                    BuildRex(-1, initx, inity, sector[initsect].floorz, initsect, inita, nNetPlayerCount);
                                    break;
                                case 7:
                                    BuildSet(-1, initx, inity, sector[initsect].floorz, initsect, inita, nNetPlayerCount);
                                    break;
                                case 8:
                                    BuildQueen(-1, initx, inity, sector[initsect].floorz, initsect, inita, nNetPlayerCount);
                                    break;
                                case 9:
                                    BuildRoach(0, -1, initx, inity, sector[initsect].floorz, initsect, inita);
                                    break;
                                case 10:
                                    BuildRoach(1, -1, initx, inity, sector[initsect].floorz, initsect, inita);
                                    break;
                                case 11:
                                    BuildWasp(-1, initx, inity, sector[initsect].floorz - 25600, initsect, inita);
                                    break;
                                case 12:
                                    BuildScorp(-1, initx, inity, sector[initsect].floorz, initsect, inita, nNetPlayerCount);
                                    break;
                                case 13:
                                    BuildRat(-1, initx, inity, sector[initsect].floorz, initsect, inita);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    else
                    {
                        if (nStringLen == 0)
                        {
                            bHolly = kFalse;
                            StatusMessage(1, " ");
                        }
                        else
                        {
                            for (int i = 0; i < nPasswordCount; ++i)
                            {
                                if (!strcmp(sHollyStr, gString[i + nFirstPassword]))
                                {
                                    DoPassword(i);
                                    break;
                                }
                            }
                        }
                    }
                    sHollyStr[0] = '\0';
                }
                else if (ch == asc_BackSpace)
                {
                    if (nStringLen != 0) {
                        sHollyStr[nStringLen - 1] = '\0';
                    }
                }
                else if (nStringLen < (sizeof(sHollyStr) - 1)) // do we have room to add a char and null terminator?
                {
                    sHollyStr[nStringLen] = toupper(ch);
                    sHollyStr[nStringLen + 1] = '\0';
                }
            }
            else
            {
                KB_GetCh();
            }
        }

        if (isalpha(ch))
        {
            ch = toupper(ch);

            int ecx = nCodeMin;

            int ebx = nCodeMin;
            int edx = nCodeMin - 1;

            while (ebx <= nCodeMax)
            {
                if (ch == gString[ecx][nCodeIndex])
                {
                    nCodeMin = ebx;
                    nCodeIndex++;

                    if (gString[ecx][nCodeIndex] == 0)
                    {
                        ebx -= nFirstPassword;

                        DoPassword(ebx);
                        ResetPassword();
                    }

                    break;
                }
                else if (gString[ecx][nCodeIndex] < ch)
                {
                    nCodeMin = ebx + 1;
                }
                else if (gString[ecx][nCodeIndex] > ch)
                {
                    nCodeMax = edx;
                }

                ecx++;
                edx++;
                ebx++;
            }

            if (nCodeMin > nCodeMax) {
                ResetPassword();
            }
        }
    }
}

void DoCredits()
{
    NoClip();

    playCDtrack(19, false);

    int nSecretSkipKeyCount = 0;

    if (videoGetRenderMode() == REND_CLASSIC)
        FadeOut(0);

    int nCreditsIndex = FindGString("CREDITS");

    while (strcmp(gString[nCreditsIndex], "END") != 0)
    {
        EraseScreen(overscanindex);

        int nStart = nCreditsIndex;

        // skip blanks
        while (strlen(gString[nCreditsIndex]) != 0) {
            nCreditsIndex++;
        }

        int y = 100 - ((10 * (nCreditsIndex - nStart - 1)) / 2);

        for (int i = nStart; i < nCreditsIndex; i++)
        {
            int nWidth = MyGetStringWidth(gString[i]);
            myprintext((320 - nWidth) / 2, y, gString[i], 0);
            y += 10;
        }

        videoNextPage();

        nCreditsIndex++;

        if (videoGetRenderMode() == REND_CLASSIC)
            FadeIn();

        int nDuration = (int)totalclock + 600;

        while ((int)totalclock <= nDuration)
        {
            HandleAsync();

            if (KB_KeyDown[sc_F12])
            {
                nSecretSkipKeyCount++;

                KB_KeyDown[sc_F12] = 0;

                if (nSecretSkipKeyCount > 5) {
                    return;
                }
            }
        }

        if (videoGetRenderMode() == REND_CLASSIC)
            FadeOut(0);
    }

    while (CDplaying())
    {
        HandleAsync();

        if (KB_KeyWaiting()) {
            KB_GetCh();
        }
    }

    Clip();
}

void FinishLevel()
{
    if (levelnum > nBestLevel) {
        nBestLevel = levelnum - 1;
    }

    levelnew = levelnum + 1;

    StopAllSounds();

    bCamera = kFalse;
    nMapMode = 0;

    if (levelnum != kMap20)
    {
        EraseScreen(4);
        SetLocalChan(1);
        PlayLocalSound(StaticSound[59], 0);
        videoNextPage();
        WaitTicks(12);
        WaitVBL();
        RefreshBackground();
        RefreshStatus();
        DrawView(65536);
        videoNextPage();
    }

    FadeOut(1);
    EraseScreen(overscanindex);

    if (levelnum == 0)
    {
        nPlayerLives[0] = 0;
        levelnew = 100;
    }
    else
    {
        DoAfterCinemaScene(levelnum);
        if (levelnum == kMap20)
        {
            DoCredits();
            nPlayerLives[0] = 0;
        }
    }
}

EDUKE32_STATIC_ASSERT(sizeof(demo_header) == 75);
EDUKE32_STATIC_ASSERT(sizeof(demo_input) == 36);


void WritePlaybackInputs()
{
    demo_input output;
    output.moveframes = B_LITTLE32(moveframes);
    output.xVel = B_LITTLE32(sPlayerInput[nLocalPlayer].xVel);
    output.yVel = B_LITTLE32(sPlayerInput[nLocalPlayer].yVel);
    output.nAngle  = B_LITTLE16(fix16_to_int(sPlayerInput[nLocalPlayer].nAngle >> 2));
    output.buttons = B_LITTLE16(sPlayerInput[nLocalPlayer].buttons);
    output.nTarget = B_LITTLE16(sPlayerInput[nLocalPlayer].nTarget);
    output.horizon = fix16_to_int(sPlayerInput[nLocalPlayer].horizon);
    output.nItem = sPlayerInput[nLocalPlayer].nItem;
    output.h = B_LITTLE32(sPlayerInput[nLocalPlayer].h);
    output.i = sPlayerInput[nLocalPlayer].i;

    if (!fwrite(&output, 1, sizeof(output), vcrfpwrite))
    {
        fclose(vcrfpwrite);
        vcrfpwrite = NULL;
        bRecord = kFalse;
        return;
    }
}

uint8_t ReadPlaybackInputs()
{
    demo_input input;
    if (kread(hVCRRead, &input, sizeof(input)))
    {
        moveframes = B_LITTLE32(input.moveframes);
        sPlayerInput[nLocalPlayer].xVel = B_LITTLE32(input.xVel);
        sPlayerInput[nLocalPlayer].yVel = B_LITTLE32(input.yVel);
        sPlayerInput[nLocalPlayer].nAngle = fix16_from_int(B_LITTLE16(input.nAngle) << 2);
        sPlayerInput[nLocalPlayer].buttons = B_LITTLE16(input.buttons);
        sPlayerInput[nLocalPlayer].nTarget = B_LITTLE16(input.nTarget);
        sPlayerInput[nLocalPlayer].horizon = fix16_from_int(input.horizon);
        sPlayerInput[nLocalPlayer].nItem = input.nItem;
        sPlayerInput[nLocalPlayer].h = B_LITTLE32(input.h);
        sPlayerInput[nLocalPlayer].i = input.i;

        besttarget = sPlayerInput[nLocalPlayer].nTarget;
        Ra[nLocalPlayer].nTarget = besttarget;
        return kTrue;
    }
    else
    {
        kclose(hVCRRead);
        hVCRRead = buildvfs_kfd_invalid;
        bPlayback = kFalse;
        return kFalse;
    }
}

void SetHiRes()
{
    //nScreenWidth  = 640;
    //nScreenHeight = 400;
    bHiRes = kTrue;
}

void DoClockBeep()
{
    for (int i = headspritestat[407]; i != -1; i = nextspritestat[i]) {
        PlayFX2(StaticSound[kSoundTick1], i);
    }
}

void DoRedAlert(int nVal)
{
    if (nVal)
    {
        nAlarmTicks = 69;
        nRedTicks = 30;
    }

    for (int i = headspritestat[405]; i != -1; i = nextspritestat[i])
    {
        if (nVal)
        {
            PlayFXAtXYZ(StaticSound[kSoundAlarm], sprite[i].x, sprite[i].y, sprite[i].z, sprite[i].sectnum);
            AddFlash(sprite[i].sectnum, sprite[i].x, sprite[i].y, sprite[i].z, 192);
        }
    }
}

/*
void LockEnergyTiles()
{
    // old	loadtilelockmode = 1;
    tileLoad(kTile3603);
    tileLoad(kEnergy1);
    tileLoad(kEnergy2);
    // old  loadtilelockmode = 0;
}
*/

void DrawClock()
{
    int ebp = 49;

    if (!waloff[kTile3603]) tileLoad(kTile3603);

    memset((void*)waloff[kTile3603], -1, 4096);

    if (lCountDown / 30 != nClockVal)
    {
        nClockVal = lCountDown / 30;
        DoClockBeep();
    }

    int nVal = nClockVal;

    while (nVal)
    {
        int v2 = nVal & 0xF;
        int yPos = 32 - tilesiz[v2 + kClockSymbol1].y / 2;

        CopyTileToBitmap(v2 + kClockSymbol1, kTile3603, ebp - tilesiz[v2 + kClockSymbol1].x / 2, yPos);

        ebp -= 15;

        nVal /= 16;
    }

    DoEnergyTile();
}

extern "C" void M32RunScript(const char* s);
void M32RunScript(const char* s) { UNREFERENCED_PARAMETER(s); }
void app_crashhandler(void)
{
    ShutDown();
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
    if (bRecord || bPlayback || nFreeze != 0 || bCamera || bPause)
        return 65536;

    return calc_smoothratio(totalclk, ototalclk, REALGAMETICSPERSEC);
}

#define COLOR_RED redcol
#define COLOR_WHITE whitecol

#define LOW_FPS ((videoGetRenderMode() == REND_CLASSIC) ? 35 : 50)
#define SLOW_FRAME_TIME 20

#if defined GEKKO
# define FPS_YOFFSET 16
#else
# define FPS_YOFFSET 0
#endif

#define FPS_COLOR(x) ((x) ? COLOR_RED : COLOR_WHITE)

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

            printext256(windowxy2.x-(chars<<(3-x))+1, windowxy1.y+2+FPS_YOFFSET, blackcol, -1, tempbuf, x);
            printext256(windowxy2.x-(chars<<(3-x)), windowxy1.y+1+FPS_YOFFSET,
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

static void GameDisplay(void)
{
    // End Section B

    SetView1();

    if (levelnum == kMap20)
    {
        //LockEnergyTiles();
        DoEnergyTile();
        DrawClock();
    }

    auto smoothRatio = calc_smoothratio(totalclock, tclocks);

    DrawView(smoothRatio);

    if (bPause)
    {
        int nLen = MyGetStringWidth("PAUSED");
        myprintext((320 - nLen) / 2, 100, "PAUSED", 0);
    }

    G_PrintFPS();

    videoNextPage();
}

static void GameMove(void)
{
    FixPalette();

    if (levelnum == kMap20)
    {
        if (lCountDown <= 0)
        {
            for (int i = 0; i < nTotalPlayers; i++) {
                nPlayerLives[i] = 0;
            }

            DoFailedFinalScene();
            levelnew = 100;

            return;
        }
        // Pink section
        lCountDown--;
        DrawClock();

        if (nRedTicks)
        {
            nRedTicks--;

            if (nRedTicks <= 0) {
                DoRedAlert(0);
            }
        }

        nAlarmTicks--;
        nButtonColor--;

        if (nAlarmTicks <= 0) {
            DoRedAlert(1);
        }
    }

    // YELLOW SECTION
    MoveThings();

    obobangle = bobangle;

    if (totalvel[nLocalPlayer] == 0)
    {
        bobangle = 0;
    }
    else
    {
        bobangle += 56;
        bobangle &= kAngleMask;
    }

    UpdateCreepySounds();

    // loc_120E9:
    totalmoves++;
    moveframes--;
}

#if defined(_WIN32) && defined(DEBUGGINGAIDS)
// See FILENAME_CASE_CHECK in cache1d.c
static int32_t check_filename_casing(void)
{
    return 1;
}
#endif

void PatchDemoStrings()
{
    if (!ISDEMOVER)
        return;

    if (EXHUMED) {
        gString[60] = "PICK UP A COPY OF EXHUMED";
        nBeforeScene[4] = 3;
    }
    else {
        gString[60] = "PICK UP A COPY OF POWERSLAVE";
    }

    gString[61] = "TODAY TO CONTINUE THE ADVENTURE!";
    gString[62] = "MORE LEVELS, NASTIER CREATURES";
    gString[63] = "AND THE EVIL DOINGS OF THE";
    gString[64] = "KILMAAT AWAIT YOU IN THE FULL";
    gString[65] = "VERSION OF THE GAME.";
    gString[66] = "TWENTY LEVELS, PLUS 12 NETWORK";
    gString[67] = "PLAY LEVELS CAN BE YOURS!";
    gString[68] = "END";
}

void ExitGame()
{
    if (bRecord) {
        fclose(vcrfpwrite);
    }

    FadeSong();
    if (CDplaying()) {
        fadecdaudio();
    }

    StopAllSounds();
    StopLocalSound();

    if (bSerialPlay)
    {
        if (nNetPlayerCount != 0) {
            bSendBye = kTrue;
            UpdateSerialInputs();
        }
    }
    else
    {
        if (nNetPlayerCount != 0) {
            SendGoodbye();
        }
    }

    ShutDown();
}

static int32_t nonsharedtimer;

int app_main(int argc, char const* const* argv)
{
    char buffer[BMAX_PATH];
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

#ifndef USE_PHYSFS
#ifdef DEBUGGINGAIDS
    extern int32_t (*check_filename_casing_fn)(void);
    check_filename_casing_fn = check_filename_casing;
#endif
#endif
#endif

    G_ExtPreInit(argc, argv);

#ifdef __APPLE__
    if (!g_useCwd)
    {
        char cwd[BMAX_PATH];
        char *homedir = Bgethomedir();
        if (homedir)
            Bsnprintf(cwd, sizeof(cwd), "%s/Library/Logs/" APPBASENAME ".log", homedir);
        else
            Bstrcpy(cwd, APPBASENAME ".log");
        OSD_SetLogFile(cwd);
        Xfree(homedir);
    }
    else
#endif
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

    initprintf("Exhumed %s\n", s_buildRev);
    PrintBuildInfo();

    int i;

    int stopTitle = kFalse;
    levelnew = 1;

    wConsoleNode = 0;

    int nMenu = 0; // TEMP

    // Check for any command line arguments
    for (i = 1; i < argc; i++)
    {
        const char *pChar = argv[i];

        if ((*pChar == '-') 
#ifdef _WIN32            
            || (*pChar == '/')
#endif
            )
        {
            pChar++;
            //strlwr(pChar);

            if (!Bstrcasecmp(pChar, "?") || !Bstrcasecmp(pChar, "help") || !Bstrcasecmp(pChar, "-help")) {
                PrintHelp();
            }
            else if (Bstrcasecmp(pChar, "nocreatures") == 0) {
                bNoCreatures = kTrue;
            }
            else if (Bstrcasecmp(pChar, "nosound") == 0) {
                bNoSound = kTrue;
            }
            else if (Bstrcasecmp(pChar, "record") == 0)
            {
                if (!bPlayback)
                {
                    G_ModDirSnprintfLite(buffer, sizeof(buffer), "DATA.VCR");
                    vcrfpwrite = fopen(buffer, "wb+");
                    if (vcrfpwrite != NULL) {
                        bRecord = kTrue;
                    }
                    else {
                        initprintf("Can't open demo file DATA.VCR for recording\n");
                    }
                }
            }
            else if (Bstrcasecmp(pChar, "playback") == 0)
            {
                if (!bRecord)
                {
                    hVCRRead = kopen4loadfrommod("DATA.VCR", 0);
                    if (hVCRRead >= 0) {
                        bPlayback = kTrue;
                        doTitle = kFalse;
                    }
                    else {
                        initprintf("Can't open demo file DATA.VCR for playback\n");
                    }
                }
            }
            else if (Bstrcasecmp(pChar, "map") == 0)
            {
                pChar += 4;
                strcpy(gUserMapFilename, pChar);

                bUserMap = true;
                doTitle = kFalse;
            }
            else if (Bstrncasecmp(pChar, "null", 4) == 0)
            {
                pChar += 4;

                bSerialPlay = kTrue;
                nNetPlayerCount = 1;
                nTotalPlayers = 2;

                doTitle = kFalse;

                char ch = *pChar;

                // bjd - unused/unfished code in original .exe?
                switch (ch - 1)
                {
                    default:
                        break;

                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        break;
                }

                if (forcelevel < 0)
                {
                    forcelevel = levelnew;
                }
            }
            else if (Bstrcasecmp(pChar, "modem") == 0)
            {
                bModemPlay = kTrue;
                bSerialPlay = kTrue;
                nNetPlayerCount = 1;
                nTotalPlayers = 2;

                doTitle = kFalse;

                if (forcelevel < 0) {
                    forcelevel = levelnew;
                }
            }
            else if (Bstrcasecmp(pChar, "network") == 0)
            {
                nNetPlayerCount = -1;
                forcelevel = levelnew;
                bModemPlay = kFalse;
                bSerialPlay = kFalse;

                doTitle = kFalse;
            }
            else if (Bstrcasecmp(pChar, "setup") == 0) {
                g_commandSetup = 1;
            }
            else if (Bstrcasecmp(pChar, "nosetup") == 0) {
                g_noSetup = 1;
                g_commandSetup = 0;
            }
            else if (Bstrcasecmp(pChar, "quick") == 0) {
                doTitle = kFalse;
            }
            else if (Bstrcasecmp(pChar, "noautoload") == 0)
            {
                initprintf("Autoload disabled\n");
                g_noAutoLoad = 1;
            }
            else if (Bstrcasecmp(pChar, "cachesize") == 0)
            {
                if (argc > i + 1)
                {
                    uint32_t j = Batol(argv[i + 1]);
                    MAXCACHE1DSIZE = j << 10;
                    initprintf("Cache size: %dkB\n", j);
                    i++;
                }
            }
            else if (Bstrcasecmp(pChar, "g") == 0)
            {
                if (argc > i + 1)
                {
                    G_AddGroup(argv[i + 1]);
                    i++;
                }
            }
            else if (Bstrcasecmp(pChar, "h") == 0)
            {
                if (argc > i + 1)
                {
                    G_AddDef(argv[i + 1]);
                    i++;
                }
            }
            else if (Bstrcasecmp(pChar, "j") == 0)
            {
                if (argc > i + 1)
                {
                    G_AddPath(argv[i + 1]);
                    i++;
                }
            }
            else
            {
                char c = tolower(*pChar);

                switch (c)
                {
                    case 'h':
                        SetHiRes();
                        break;
#if 0
                    case 's':
                        socket = atoi(pChar + 1);
                        break;
#endif
                    case 't':
                        nNetTime = atoi(pChar + 1);
                        if (nNetTime < 0) {
                            nNetTime = 0;
                        }
                        else {
                            nNetTime = nNetTime * 1800;
                        }
                        break;
                    default:
                    {
                        if (isdigit(c))
                        {
                            levelnew = atoi(pChar);
                            forcelevel = levelnew;

                            doTitle = kFalse;

                            initprintf("Jumping to level %d...\n", levelnew);
                        }
                        break;
                    }
                }
            }
        }
    }

    if (nNetPlayerCount && forcelevel == -1) {
        forcelevel = 1;
    }

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
    for (bssize_t i=kMaxGameFunctions-1; i>=0; i--)
    {
        if (gamefunctions[i][0] == '\0')
            continue;

        hash_add(&h_gamefuncs,gamefunctions[i],i,0);
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
        initprintf("Using config file \"%s\".\n",setupfilename);

    G_ScanGroups();

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

    initgroupfile(defaultpk3filename);
    G_LoadGroups(!g_noAutoLoad && !gSetup.noautoload);

    PatchDemoStrings();

    // Decrypt strings code would normally be here
#if 0

    for (int i = 0; ; i++)
    {
        int j = i - 1;

        while (j >= 0)
        {
            if (gString_Enc[i] == gString_Enc[j]) {
                break;
            }

            j--;
        }

        if (j < 0)
        {
            int k = 0;

            while (1)
            {
                uint8_t v27 = gString_Enc[i + k];
                if (v27)
                {
                    gString_Enc[i + k] = v27 ^ 0xFF;

                    k++;
                }
                else {
                    break;
                }


            }

//			strupr(gString[j]);

            int blah = 123;
//			if (!strcmp(*(char **)((char *)gString + v29), "EOF", v27, v30))
//				break;
        }
    }
#endif

    // loc_115F5:
    nItemTextIndex = FindGString("ITEMS");
    nFirstPassword = FindGString("PASSWORDS");
    nFirstPassInfo = FindGString("PASSINFO");

    // count the number of passwords available
    for (nPasswordCount = 0; strlen(gString[nFirstPassword+nPasswordCount]) != 0; nPasswordCount++)
    {
    }

    ResetPassword();

    // GetCurPal(NULL);

    CONFIG_WriteSetup(1);
    CONFIG_ReadSetup();

    initprintf("Initializing OSD...\n");

    Bsprintf(buffer, "Exhumed %s", s_buildRev);
    OSD_SetVersion(buffer, 10,0);
    OSD_SetParameters(0, 0, 0, 0, 0, 0, OSD_ERROR, OSDTEXT_RED, OSDTEXT_DARKRED, gamefunctions[gamefunc_Show_Console][0] == '\0' ? OSD_PROTECTED : 0);
    registerosdcommands();

    SetupInput();

	/*
    char *const setupFileName = Xstrdup(setupfilename);
    char *const p = strtok(setupFileName, ".");

    if (!p || !Bstrcmp(setupfilename, kSetupFilename))
        Bsprintf(buffer, "settings.cfg");
    else
        Bsprintf(buffer, "%s_settings.cfg", p);

    Xfree(setupFileName);
	*/

    OSD_Exec("pcexhumed_cvars.cfg");
    OSD_Exec("pcexhumed_autoexec.cfg");

    CONFIG_SetDefaultKeys(keydefaults, true);

    system_getcvars();

    if (nNetPlayerCount == -1)
    {
        nNetPlayerCount = nCfgNetPlayers - 1;
        nTotalPlayers += nNetPlayerCount;
    }

    // loc_116A5:

#if 0
    if (nNetPlayerCount)
    {
        InitInput();
        forcelevel = nStartLevel;
        nNetTime = 1800 * nTimeLimit;

        if (nNetTime == 0) {
            nNetTime = -1;
        }

        int nWaitTicks = 0;

        if (!bSerialPlay)
        {
            if (InitNet(socket, nTotalPlayers))
            {
                DebugOut("Found network players!\n");
                nWaitTicks = 30;
            }
            else
            {
                AbortNetworkPlay();
                DebugOut("Network play aborted\n");
                initprintf("Network play aborted\n");
                nWaitTicks = 60;
            }

            WaitTicks(nWaitTicks);
        }
    }
#endif

    // temp - moving InstallEngine(); before FadeOut as we use nextpage() in FadeOut
    InstallEngine();

    const char *defsfile = G_DefFile();
    uint32_t stime = timerGetTicks();
    if (!loaddefinitionsfile(defsfile))
    {
        uint32_t etime = timerGetTicks();
        initprintf("Definitions file \"%s\" loaded in %d ms.\n", defsfile, etime-stime);
    }
    loaddefinitions_game(defsfile, FALSE);


    if (enginePostInit())
        ShutDown();

    g_frameDelay = calcFrameDelay(r_maxfps);

    // loc_11745:
//    FadeOut(0);
//	InstallEngine();
    KB_Startup();
    InitView();
    InitFX();
    LoadFX();
    setCDaudiovolume(MusicVolume);
    seq_LoadSequences();
    InitStatus();
    InitTimer();

    for (i = 0; i < kMaxPlayers; i++) {
        nPlayerLives[i] = kDefaultLives;
    }

    nBestLevel = 0;

    LoadSaveSetup();
    UpdateScreenSize();

    EraseScreen(overscanindex);
    ResetEngine();
    EraseScreen(overscanindex);

    ResetView();
    GrabPalette();

    if (bSerialPlay && !InitSerial()) {
        bail2dos("Unable to connect");
    }

    if (doTitle)
    {
        while (!stopTitle)
        {
            DoTitle();
            stopTitle = kTrue;
        }
    }
    // loc_11811:
    if (forcelevel > -1)
    {
        levelnew = forcelevel;
        goto STARTGAME1;
    }
    // no forcelevel specified...
    if (bPlayback || bUserMap)
    {
        goto STARTGAME1;
    }
MENU:
    SavePosition = -1;
    nMenu = menu_Menu(0);
    switch (nMenu)
    {
    case -1:
        goto MENU;
    case 0:
        goto EXITGAME;
    case 3:
        forcelevel = 0;
        goto STARTGAME2;
    case 9:
        hVCRRead = kopen4loadfrommod("DEMO.VCR", 0);
        if (hVCRRead < 0) {
            goto MENU;
        }

        InitRandom();
        bInDemo = kTrue;
        bPlayback = kTrue;

        KB_FlushKeyboardQueue();
        KB_ClearKeysDown();
        break;
    }
STARTGAME1:
    levelnew = 1;
    levelnum = 1;
    if (!nNetPlayerCount) {
        FadeOut(0);
    }
STARTGAME2:
    bool cdSuccess;
    bCamera = kFalse;
    ClearCinemaSeen();
    PlayerCount = 0;
    lastlevel = -1;

    for (i = 0; i < nTotalPlayers; i++)
    {
        int nPlayer = GrabPlayer();
        if (nPlayer < 0) {
            bail2dos("Can't create local player\n");
        }

        InitPlayerInventory(nPlayer);

        if (i == wConsoleNode) {
            PlayerList[nPlayer].someNetVal = -3;
        }
        else {
            PlayerList[nPlayer].someNetVal = -4;
        }
    }

    nNetMoves = 0;

    if (bPlayback)
    {
        menu_GameLoad2(hVCRRead, true);
        levelnew = GameStats.nMap;
        levelnum = GameStats.nMap;
        forcelevel = GameStats.nMap;
    }

    if (forcelevel > -1)
    {
        // YELLOW SECTION
        levelnew = forcelevel;
        UpdateInputs();
        forcelevel = -1;

        if (bRecord && !bInDemo) {
            menu_DemoGameSave(vcrfpwrite);
        }
        goto LOOP3;
    }

    // PINK SECTION
    UpdateInputs();
    nNetMoves = 1;

    if (nMenu == 2)
    {
        levelnew = 1;
        levelnum = 1;
        levelnew = menu_GameLoad(SavePosition);
        lastlevel = -1;
    }

    if (bRecord && !bInDemo) {
        menu_DemoGameSave(vcrfpwrite);
    }

    nBestLevel = levelnew - 1;
LOOP1:

    if (nPlayerLives[nLocalPlayer] <= 0) {
        goto MENU;
    }
    if (levelnew > 99) {
        goto EXITGAME;
    }
    if (!bInDemo && levelnew > nBestLevel && levelnew != 0 && levelnew <= kMap20 && SavePosition > -1) {
        menu_GameSave(SavePosition);
    }
LOOP2:
    if (!nNetPlayerCount && !bPlayback && levelnew > 0 && levelnew <= kMap20) {
        levelnew = showmap(levelnum, levelnew, nBestLevel);
    }

    if (levelnew > nBestLevel) {
        nBestLevel = levelnew;
    }
LOOP3:
    while (levelnew != -1)
    {
        // BLUE
        if (CDplaying()) {
            fadecdaudio();
        }

        if (levelnew == kMap20)
        {
            lCountDown = 81000;
            nAlarmTicks = 30;
            nRedTicks = 0;
            nClockVal = 0;
            nEnergyTowers = 0;
        }

        if (!LoadLevel(levelnew)) {
            // TODO "Can't load level %d...\n", nMap;
            goto EXITGAME;
        }
        levelnew = -1;
    }
    /* don't restore mid level savepoint if re-entering just completed level
    if (nNetPlayerCount == 0 && lastlevel == levelnum)
    {
        RestoreSavePoint(nLocalPlayer, &initx, &inity, &initz, &initsect, &inita);
    }
    */
    lastlevel = levelnum;

    for (i = 0; i < nTotalPlayers; i++)
    {
        SetSavePoint(i, initx, inity, initz, initsect, inita);
        RestartPlayer(i);
        InitPlayerKeys(i);
    }

    UpdateScreenSize();
    fps = 0;
    lastfps = 0;
    InitStatus();
    ResetView();
    ResetEngine();
    totalmoves = 0;
    GrabPalette();
    ResetMoveFifo();
    moveframes = 0;
    bInMove = kFalse;
    tclocks = totalclock;
    nPlayerDAng = 0;
    lPlayerXVel = 0;
    lPlayerYVel = 0;
    movefifopos = movefifoend;

    RefreshStatus();

    if (bSerialPlay) {
        ClearSerialInbuf();
    }

    mysetbrightness((uint8_t)nGamma);
    tclocks2 = totalclock;
    CONTROL_BindsEnabled = 1;

    cdSuccess = true;
    // Game Loop
    while (1)
    {
        if (levelnew >= 0)
        {
            CONTROL_BindsEnabled = 0;
            goto LOOP1;
        }

        HandleAsync();
        OSD_DispatchQueued();

        // Section B
        if (cdSuccess && !CDplaying() && !nFreeze && !nNetPlayerCount)
        {
            int nTrack = levelnum;
            if (nTrack != 0) {
                nTrack--;
            }

            cdSuccess = playCDtrack((nTrack % 8) + 11, true);
        }

// TODO		CONTROL_GetButtonInput();
        CheckKeys();
        UpdateSounds();

        if (bRecord || bPlayback)
        {
            bInMove = kTrue;

            moveframes = ((int)totalclock - (int)tclocks2) / 4;

            if (moveframes > 4)
                moveframes = 4;

            if (moveframes != 0)
                tclocks2 = totalclock;

            if (bPlayback)
            {
                // YELLOW
                if ((bInDemo && KB_KeyWaiting() || !ReadPlaybackInputs()) && (bDemoPlayerFinishedLevel || KB_GetCh()))
                {
                    bDemoPlayerFinishedLevel = false;
                    KB_FlushKeyboardQueue();
                    KB_ClearKeysDown();

                    bPlayback = kFalse;
                    bInDemo = kFalse;

                    if (hVCRRead) {
                        kclose(hVCRRead);
                    }

                    CONTROL_BindsEnabled = 0;
                    goto MENU;
                }
            }
            else if (bRecord || moveframes)
            {
                GetLocalInput();

                sPlayerInput[nLocalPlayer].xVel = lPlayerXVel;
                sPlayerInput[nLocalPlayer].yVel = lPlayerYVel;
                sPlayerInput[nLocalPlayer].buttons = lLocalButtons | lLocalCodes;
                sPlayerInput[nLocalPlayer].nAngle = nPlayerDAng;
                sPlayerInput[nLocalPlayer].nTarget = besttarget;
                sPlayerInput[nLocalPlayer].horizon = nVertPan[nLocalPlayer];

                Ra[nLocalPlayer].nTarget = besttarget;

                lLocalCodes = 0;
                nPlayerDAng = 0; 
            }

            // loc_11F72:
            /*
            if (bRecord && !bInDemo) {
                WritePlaybackInputs();
            }
            */

            if (nNetPlayerCount)
            {
                if (moveframes)
                {
                    UpdateInputs();
                    moveframes = nNetMoveFrames;
                }
            }
            else
            {
                // loc_11FBC:
                while (bPause)
                {
                    ClearAllKeys();
                    if (WaitAnyKey(-1) != sc_Pause)
                    {
                        bPause = kFalse;
                    }
                }
            }

            // loc_11FEE:
            tclocks += moveframes * 4;
            while (moveframes && levelnew < 0)
            {
                // TEMP - try this here...
                if (bRecord && !bInDemo) {
                    WritePlaybackInputs();
                }

                GameMove();
                // if (nNetTime > 0)
                // {
                //     nNetTime--;
                //
                //     if (!nNetTime) {
                //         nFreeze = 3;
                //     }
                // }
                // else if (nNetTime == 0)
                // {
                //     if (BUTTON(gamefunc_Open))
                //     {
                //         CONTROL_ClearButton(gamefunc_Open);
                //         goto MENU2;
                //     }
                // }
            }

            bInMove = kFalse;

            // END YELLOW SECTION

            // loc_12149:
            if (bInDemo || bPlayback)
            {
                while (tclocks > totalclock) { HandleAsync(); }
                tclocks = totalclock;
            }

            if (engineFPSLimit())
            {
                GameDisplay();
            }
        }
        else
        {
            static bool frameJustDrawn = false;
            bInMove = kTrue;
            if (!bPause && totalclock >= tclocks + 4)
            {
                do
                {
                    if (!frameJustDrawn)
                        break;

                    frameJustDrawn = false;

                    GetLocalInput();

                    sPlayerInput[nLocalPlayer].xVel = lPlayerXVel;
                    sPlayerInput[nLocalPlayer].yVel = lPlayerYVel;
                    sPlayerInput[nLocalPlayer].buttons = lLocalButtons | lLocalCodes;
                    sPlayerInput[nLocalPlayer].nAngle = nPlayerDAng;
                    sPlayerInput[nLocalPlayer].nTarget = besttarget;
                    sPlayerInput[nLocalPlayer].horizon = nVertPan[nLocalPlayer];

                    Ra[nLocalPlayer].nTarget = besttarget;

                    lLocalCodes = 0;
                    nPlayerDAng = 0;

                    do
                    {
                        // timerUpdate();
                        tclocks += 4;
                        GameMove();
                        // timerUpdate();
                    } while (levelnew < 0 && totalclock >= tclocks + 4);
                } while (0);
            }
            bInMove = kFalse;

            faketimerhandler();

            if (engineFPSLimit())
            {
                GameDisplay();
                frameJustDrawn = true;
            }
        }
        if (!bInDemo)
        {
            if (BUTTON(gamefunc_Escape))
            {
                CONTROL_ClearButton(gamefunc_Escape);
// MENU2:
                CONTROL_BindsEnabled = 0;
                bInMove = kTrue;
                nMenu = menu_Menu(1);

                switch (nMenu)
                {
                    case 0:
                        goto EXITGAME;

                    case 1:
                        goto STARTGAME1;

                    case 2:
                        levelnum = levelnew = menu_GameLoad(SavePosition);
                        lastlevel = -1;
                        nBestLevel = levelnew - 1;
                        goto LOOP2;

                    case 3: // When selecting 'Training' from ingame menu when already playing a level
                        if (levelnum == 0 || !Query(2, 4, "Quit current game?", "Y/N", 'Y', 13, 'N', 27))
                        {
                            levelnew = 0;
                            levelnum = 0;

                            goto STARTGAME2;
                        }
                }

                totalclock = ototalclock = tclocks;
                bInMove = kFalse;
                CONTROL_BindsEnabled = 1;
                RefreshStatus();
            }
            else if (KB_UnBoundKeyPressed(sc_F12))
            {
                KB_ClearKeyDown(sc_F12);
                videoCaptureScreen("captxxxx.png", 0);
            }
            else if (BUTTON(gamefunc_Map)) // e.g. TAB (to show 2D map)
            {
                CONTROL_ClearButton(gamefunc_Map);

                if (!nFreeze) {
                    nMapMode = (nMapMode+1)%3;
                }
            }
            else if (BUTTON(gamefunc_Map_Follow_Mode))
            {
                CONTROL_ClearButton(gamefunc_Map_Follow_Mode);
                bFollowMode = !bFollowMode;
            }
            else if (BUTTON(gamefunc_Toggle_Crosshair))
            {
                CONTROL_ClearButton(gamefunc_Toggle_Crosshair);
                gShowCrosshair = !gShowCrosshair;
            }

            if (nMapMode != 0)
            {
                int const timerOffset = ((int) totalclock - nonsharedtimer);
                nonsharedtimer += timerOffset;

                if (BUTTON(gamefunc_Zoom_In) || KB_KeyPressed(sc_kpad_Plus))
                    lMapZoom += mulscale6(timerOffset, max<int>(lMapZoom, 256));

                if (BUTTON(gamefunc_Zoom_Out) || KB_KeyPressed(sc_kpad_Minus))
                    lMapZoom -= mulscale6(timerOffset, max<int>(lMapZoom, 256));

                lMapZoom = clamp(lMapZoom, 48, 2048);
            }

            if (PlayerList[nLocalPlayer].nHealth > 0)
            {
                if (BUTTON(gamefunc_Inventory_Left))
                {
                    SetPrevItem(nLocalPlayer);
                    CONTROL_ClearButton(gamefunc_Inventory_Left);
                }
                if (BUTTON(gamefunc_Inventory_Right))
                {
                    SetNextItem(nLocalPlayer);
                    CONTROL_ClearButton(gamefunc_Inventory_Right);
                }
                if (BUTTON(gamefunc_Inventory))
                {
                    UseCurItem(nLocalPlayer);
                    CONTROL_ClearButton(gamefunc_Inventory);
                }
            }
            else {
                SetAirFrame();
            }
        }
        if (record_mode == 3 && movefifopos == movefifoend) {
            levelnew = 0;
        }
        fps++;
    }
EXITGAME:

    ExitGame();
    return 0;
}

void mychangespritesect(int nSprite, int nSector)
{
    DoKenTest();
    changespritesect(nSprite, nSector);
    DoKenTest();
}

void mydeletesprite(int nSprite)
{
    if (nSprite < 0 || nSprite > kMaxSprites) {
        bail2dos("bad sprite value %d handed to mydeletesprite", nSprite);
    }

    UnlinkIgnitedAnim(nSprite);
    deletesprite(nSprite);

    if (nSprite == besttarget) {
        besttarget = -1;
    }
}

void KeyFn1()
{
    menu_DoPlasma();
    overwritesprite(160, 100, kSkullHead, 0, 3, kPalNormal);
    overwritesprite(161, 130, kSkullJaw, 0, 3, kPalNormal);
    videoNextPage();
}

void DoGameOverScene()
{
    FadeOut(0);
    ClearAllKeys();

    if (LoadCinemaPalette(16) < 0) {
        return;
    }

    SetOverscan(ANIMPAL);
    NoClip();
    overwritesprite(0, 0, kTile3591, 0, 2, kPalNormal);
    Clip();
    videoNextPage();
    CinemaFadeIn();
    PlayGameOverSound();
    WaitAnyKey(3);
    FadeOut(0);
    SetOverscan(BASEPAL);
}

#ifdef USE_OPENGL
void FadeOutScreen(int nTile)
{
    int f = 0;

    while (f <= 255)
    {
        HandleAsync();

        overwritesprite(0, 0, nTile, 0, 2, kPalNormal);

        fullscreen_tint_gl(0, 0, 0, f);
        f += 4;

        WaitTicks(2);

        videoNextPage();
    }

    EraseScreen(overscanindex);
}

void FadeInScreen(int nTile)
{
    int f = 255;

    while (f >= 0)
    {
        HandleAsync();

        overwritesprite(0, 0, nTile, 0, 2, kPalNormal);

        fullscreen_tint_gl(0, 0, 0, f);
        f -= 4;

        WaitTicks(2);

        videoNextPage();
    }
}
#endif

void DoTitle()
{
    short skullDurations[] = { 6, 25, 43, 50, 68, 78, 101, 111, 134, 158, 173, 230, 6000 };

    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);

//  if (videoGetRenderMode() == REND_CLASSIC)
//     BlackOut();

#ifdef USE_OPENGL
    // for OpenGL, fade from black to the publisher logo
    if (videoGetRenderMode() > REND_CLASSIC)
        FadeInScreen(EXHUMED ? kTileBMGLogo : kTilePIELogo);
#endif

    overwritesprite(0, 0, EXHUMED ? kTileBMGLogo : kTilePIELogo, 0, 2, kPalNormal);
    videoNextPage();

    if (videoGetRenderMode() == REND_CLASSIC)
        FadeIn();

    ClearAllKeys();

    WaitAnyKey(2);

    if (videoGetRenderMode() == REND_CLASSIC)
        FadeOut(0);
#ifdef USE_OPENGL
    else
        FadeOutScreen(EXHUMED ? kTileBMGLogo : kTilePIELogo);
#endif

    SetOverscan(BASEPAL);

    int nScreenTile = seq_GetSeqPicnum(kSeqScreens, 0, 0);

#ifdef USE_OPENGL
    if (videoGetRenderMode() > REND_CLASSIC)
        FadeInScreen(nScreenTile);
#endif

    overwritesprite(0, 0, nScreenTile, 0, 2, kPalNormal);
    videoNextPage();

    if (videoGetRenderMode() == REND_CLASSIC)
        FadeIn();

    PlayLogoSound();

    WaitAnyKey(2);

    if (videoGetRenderMode() == REND_CLASSIC)
        FadeOut(0);
#ifdef USE_OPENGL
    else
        FadeOutScreen(nScreenTile);
#endif

    ClearAllKeys();

    PlayMovie("book.mov");

    if (videoGetRenderMode() == REND_CLASSIC)
        FadeOut(0);

    SetOverscan(BASEPAL);
    GrabPalette();

    SetLocalChan(1);
    PlayLocalSound(StaticSound[59], 0);

    SetLocalChan(0);

    EraseScreen(4);

    playCDtrack(19, true);

    videoNextPage();
    FadeIn();
    WaitVBL();

    int String_Copyright = FindGString("COPYRIGHT");

    const char *a = gString[String_Copyright];
    const char *b = gString[String_Copyright + 1];

    menu_DoPlasma();

    int nTile = kSkullHead;

    overwritesprite(160, 100, kSkullHead, 0, 3, kPalNormal);
    overwritesprite(161, 130, kSkullJaw, 0, 3, kPalNormal);
    videoNextPage();

    WaitNoKey(2, KeyFn1);

    if (time(0) & 0xF) {
        PlayGameOverSound();
    }
    else {
        PlayLocalSound(StaticSound[61], 0);
    }

    int nStartTime = (int)totalclock;
    int nCount = 0;
    int var_18 = (int)totalclock + skullDurations[0];
    int var_4 = 0;

    int esi = 130;

    while (LocalSoundPlaying() && !KB_KeyWaiting())
    {
        HandleAsync();

        menu_DoPlasma();
        overwritesprite(160, 100, nTile, 0, 3, kPalNormal);

        int nStringWidth = MyGetStringWidth(a);

        int y = 200 - 24;
        myprintext((320 / 2 - nStringWidth / 2), y, a, 0);

        nStringWidth = MyGetStringWidth(b);

        y = 200 - 16;
        myprintext((320 / 2 - nStringWidth / 2), y, b, 0);

        if ((int)totalclock > var_18)
        {
            nCount++;

            assert(nCount <= 12);
            var_18 = nStartTime + skullDurations[nCount];
            var_4 = var_4 == 0;
        }

        short nTile = kSkullJaw;

        if (var_4)
        {
            if (esi >= 135) {
                nTile = kTile3583;
            }
            else {
                esi += 5;
            }
        }
        else if (esi <= 130)
        {
            esi = 130;
        }
        else
        {
            esi -= 2;
        }

        y = 0;

        if (nTile == kTile3583)
        {
            y = 131;
        }
        else
        {
            y = esi;

            if (y > 135) {
                y = 135;
            }
        }

        overwritesprite(161, y, nTile, 0, 3, kPalNormal);
        videoNextPage();
    }

    WaitNoKey(1, KeyFn1);

    videoSetViewableArea(nViewLeft, nViewTop, nViewRight, nViewBottom);
}

void CopyTileToBitmap(short nSrcTile,  short nDestTile, int xPos, int yPos)
{
    int nOffs = tilesiz[nDestTile].y * xPos;

    uint8_t *pDest = (uint8_t*)waloff[nDestTile] + nOffs + yPos;
    uint8_t *pDestB = pDest;

    if (!waloff[nSrcTile]) tileLoad(nSrcTile);

    int destYSize = tilesiz[nDestTile].y;
    int srcYSize = tilesiz[nSrcTile].y;

    uint8_t *pSrc = (uint8_t*)waloff[nSrcTile];

    for (int x = 0; x < tilesiz[nSrcTile].x; x++)
    {
        pDest += destYSize;

        for (int y = 0; y < srcYSize; y++)
        {
            uint8_t val = *pSrc;
            if (val != 0xFF) {
                *pDestB = val;
            }

            pDestB++;
            pSrc++;
        }

        // reset pDestB
        pDestB = pDest;
    }

    tileInvalidate(nDestTile, -1, -1);
}

int CopyCharToBitmap(char nChar, int nTile, int xPos, int yPos)
{
    if (nChar == ' ') {
        return 4;
    }

    nChar = toupper(nChar);
    int nFontTile = seq_GetSeqPicnum(kSeqFont2, 0, nChar - 32) + 102;
    CopyTileToBitmap(nFontTile, nTile, xPos, yPos);

    return tilesiz[nFontTile].x + 1;
}

// Note: strings passed should be uppercase
int myprintext(int x, int y, const char *str, int shade)
{
    if (y < -15 || y >= 200)
        return x;

    const char *c = str;

    while (*c != '\0')
    {
        int nTile = seq_GetSeqPicnum(kSeqFont2, 0, (*c) - 32);
        overwritesprite(x, y, nTile, shade, 2, kPalNormal);

        int tileWidth = tilesiz[nTile].x;

        x += tileWidth + 1;
        c++;
    }

    return x;
}

void EraseScreen(int nClearColour)
{
    if (nClearColour == -1) {
        nClearColour = overscanindex;
    }

    videoClearScreen(nClearColour);
#if 0
    for (int i = 0; i < numpages; i++)
    {
        videoClearScreen(nClearColour);
        videoNextPage();
    }
#endif
}

int Query(short nLines, short nKeys, ...)
{
    short i;

    char strings[20][80];
    char keys[20];

    va_list args;

    short bPrevClip = bClip;
    NoClip();

    short nWidth = 0;

    va_start(args, nKeys);

    for (i = 0; i < nLines; i++)
    {
        char *str = va_arg(args, char*);
        strcpy(strings[i], str);
        Bstrupr(strings[i]);

        int strWidth = MyGetStringWidth(strings[i]);

        if (strWidth > nWidth) {
            nWidth = strWidth;
        }
    }

    for (i = 0; i < nKeys; i++) {
        keys[i] = (char)va_arg(args, int);
    }

    va_end(args);

    int nHeight = (nLines + 1) * 10;
    int y1 = (200 - nHeight) / 2;
    int yB = y1;

    // add some padding to left and right sides of text in the box
    nWidth += 30;

    int x1 = (320 - nWidth) / 2;
    int x2 = x1 + nWidth;

    // if (bHiRes)
    // {
    //     x1 *= 2;
    //     y1 *= 2;
    //     nHeight *= 2;
    //     x2 *= 2;
    // }

    y1 = scale(y1, ydim, 200);
    nHeight = scale(nHeight, ydim, 200);
    x1 = scale(x1-160, ydim*4/3, 320)+xdim/2;
    x2 = scale(x2-160, ydim*4/3, 320)+xdim/2;

    // draw background box that the text sits in
    for (i = 0; i < nHeight; i++) {
        renderDrawLine(x1 << 12, (y1 + i) << 12, x2 << 12, (y1 + i) << 12, overscanindex);
    }

    y1 = yB + 5;

    // draw each line of text
    for (i = 0; i < nLines; i++) {
        myprintext((320 - MyGetStringWidth(strings[i])) / 2, (y1 + i*10) , strings[i], 0);
    }

    videoNextPage();

    if (bPrevClip) {
        Clip();
    }

    i = 0;

    if (nKeys)
    {
        KB_FlushKeyboardQueue();

        while (1)
        {
            HandleAsync();

            char key = toupper(KB_GetCh());

            for (i = 0; i < nKeys; i++)
            {
                if (keys[i] == 0 || key == keys[i])
                {
                    RefreshStatus();
                    ClearAllKeys();
                    return i;
                }
            }
        }
    }

    RefreshStatus();
    ClearAllKeys();

    return i;
}

void PrintHelp()
{
    char tempbuf[128];
    static char const s[] = "Usage: " APPBASENAME " [files] [options]\n"
        //"Example: " APPBASENAME " -usecwd -cfg myconfig.cfg -map nukeland.map\n\n"
        "Example: " APPBASENAME " -g ruins.grp -quick -nomonsters -2\n\n"
        "Files can be of type [grp|zip|def]\n"
        "\n"
        //"-art [file.art]\tSpecify an art base file name\n"
        "-cachesize #\tSet cache size in kB\n"
        //"-cfg [file.cfg]\tUse an alternate configuration file\n"
        //"-client [host]\tConnect to a multiplayer game\n"
        //"-game_dir [dir]\tSpecify game data directory\n"
        "-g [file.grp]\tLoad additional game data\n"
        "-h [file.def]\tLoad an alternate definitions file\n"
        "-j [dir]\t\tAdd a directory to " APPNAME "'s search list\n"
        "-map [file.map]\tLoad an external map file\n"
        "-mh [file.def]\tInclude an additional definitions module\n"
        "-noautoload\tDisable loading from autoload directory\n"
        //"-nodemo\t\tNo Demos\n"
        "-nocreatures\tNo enemy creatures\n"
        "-nosound\tNo sound\n"
        "-record\t\tRecord demo to file data.vcr\n"
        "-playback\tPlay back a demo from file data.vcr\n"
        "-quick\t\tSkip intro splash screens and movie\n"
        "-#\t\tImmediately jump to level number specified\n"

#ifdef STARTUP_SETUP_WINDOW
        "-setup/nosetup\tEnable or disable startup window\n"
#endif
        "-usecwd\t\tRead data and configuration from current directory\n"
        ;
#ifdef WM_MSGBOX_WINDOW
    Bsnprintf(tempbuf, sizeof(tempbuf), APPNAME " %s", s_buildRev);
    wm_msgbox(tempbuf, s);
#else
    initprintf("%s\n", s);
#endif
}

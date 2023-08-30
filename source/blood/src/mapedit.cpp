//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
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
#include "build.h"
#include "editor.h"
#include "pragmas.h"

#include "baselayer.h"
#include "renderlayer.h"

#include "osd.h"
#include "cache1d.h"

//#include "osdfuncs.h"
//#include "names.h"
//
//#include "grpscan.h"

#include "common.h"
#include "common_game.h"
#include "db.h"
#include "globals.h"
#include "gui.h"
#include "qheap.h"
#include "replace.h"
#include "resource.h"
#include "screen.h"
#include "sectorfx.h"
#include "sound.h"
#include "tile.h"
#include "trig.h"
#include "mapster32.h"
#include "keys.h"

#include "keyboard.h"
#include "scriptfile.h"
#include "xxhash.h"

//#include "sounds_mapster32.h"
#include "fx_man.h"

#include "macros.h"
#include "lz4.h"
#include "colmatch.h"
#include "palette.h"

#include "m32script.h"
#include "m32def.h"

#include <signal.h>

// Workaround for namespace pollution in <sys/stat.h> introduced in MinGW 4.8.
#ifdef stat
# undef stat
#endif

const char* AppProperName = "NMapedit";
const char* AppTechnicalName = "nmapedit";

#if defined(_WIN32)
#define DEFAULT_GAME_EXEC APPBASENAME ".exe"
#define DEFAULT_GAME_LOCAL_EXEC APPBASENAME ".exe"
#elif defined(__APPLE__)
#define DEFAULT_GAME_EXEC APPNAME ".app/Contents/MacOS/" APPBASENAME
#define DEFAULT_GAME_LOCAL_EXEC APPNAME ".app/Contents/MacOS/" APPBASENAME
#else
#define DEFAULT_GAME_EXEC APPBASENAME
#define DEFAULT_GAME_LOCAL_EXEC "./" APPBASENAME
#endif

const char* DefaultGameExec = DEFAULT_GAME_EXEC;
const char* DefaultGameLocalExec = DEFAULT_GAME_LOCAL_EXEC;

static int32_t floor_over_floor;
static int32_t g_fillCurSector = 0;

const char* defaultsetupfilename = SETUPFILENAME;
char setupfilename[BMAX_PATH] = SETUPFILENAME;

static int32_t fixmaponsave_walls = 0;
static int32_t lastsave = -180*60;
static int32_t NoAutoLoad = 0;
static int32_t spnoclip=1;

static char const *default_tiles_cfg = "tiles.cfg";
static int32_t pathsearchmode_oninit;

#ifdef LUNATIC
static L_State g_EmState;
#endif

//#pragma pack(push,1)
//sound_t g_sounds[MAXSOUNDS];
//#pragma pack(pop)

//static int16_t g_definedsndnum[MAXSOUNDS];  // maps parse order index to g_sounds index
//static int16_t g_sndnum[MAXSOUNDS];  // maps current order index to g_sounds index
//int32_t g_numsounds = 0;
static int32_t mousecol, bstatus;

static int32_t corruptchecktimer;
static int32_t curcorruptthing=-1;

static uint32_t templenrepquot=1;

static int32_t duke3d_m32_globalflags;

// KEEPINSYNC global.h (used values only)
enum DUKE3D_GLOBALFLAGS {
    DUKE3D_NO_HARDCODED_FOGPALS = 1<<1,
    DUKE3D_NO_PALETTE_CHANGES = 1<<2,
};

//////////////////// Key stuff ////////////////////

#define eitherALT   (keystatus[KEYSC_LALT] || keystatus[KEYSC_RALT])
#define eitherCTRL  (keystatus[KEYSC_LCTRL] || keystatus[KEYSC_RCTRL])
#define eitherSHIFT (keystatus[KEYSC_LSHIFT] || keystatus[KEYSC_RSHIFT])

#define PRESSED_KEYSC(Key) (keystatus[KEYSC_##Key] && !(keystatus[KEYSC_##Key]=0))


//////////////////// Aiming ////////////////////
static const char *Typestr[] = { "Wall", "Ceiling", "Floor", "Sprite", "Wall" };
static const char *typestr[] = { "wall", "ceiling", "floor", "sprite", "wall" };
static const char *Typestr_wss[] = { "Wall", "Sector", "Sector", "Sprite", "Wall" };
/*static const char *typestr_wss[] = { "wall", "sector", "sector", "sprite", "wall" };*/

/** The following macros multiplex between identically named fields of sector/wall/sprite,
 * based on a macro parameter or the currently aimed at object (AIMED_ versions).
 * They can be used on either side of an assignment. */

// select wall, only makes a difference with walls that have 'swap bottom of walls' bit set
#define SELECT_WALL() (AIMING_AT_WALL ? searchbottomwall : searchwall)

#define SECFLD(i, Field)  (sector[i].Field)
#define WALFLD(i, Field)  (wall[i].Field)
#define SPRFLD(i, Field)  (sprite[i].Field)

// valid fields: z, stat, picnum, heinum, shade, pal, xpanning, ypanning
#define CEILINGFLOOR(iSec, Field) (*(AIMING_AT_CEILING ? &(sector[iSec].ceiling##Field) : &(sector[iSec].floor##Field)))
#define AIMED_CEILINGFLOOR(Field) CEILINGFLOOR(searchsector, Field)

#define AIMED_SEL_WALL(Field) WALFLD(SELECT_WALL(), Field)

// selects from wall proper or its mask
#define OVR_WALL(iWal, Field) (*(AIMING_AT_WALL ? &WALFLD(iWal, Field) : &(wall[iWal].over##Field)))
#define AIMED_SELOVR_WALL(Field) OVR_WALL(SELECT_WALL(), Field)

// the base macro to construct field multiplexing macros: wall and sector cases undetermined
#define MUXBASE(Field, SectorCase, WallCase) (*(AIMING_AT_CEILING_OR_FLOOR ? (SectorCase) : \
                                               (AIMING_AT_WALL_OR_MASK ? (WallCase) : \
                                                &SPRFLD(searchwall, Field))))

#define SFBASE_CF(Field, WallCase) MUXBASE(Field, &AIMED_CEILINGFLOOR(Field), WallCase)

#define SFBASE_(Field, WallCase) MUXBASE(Field, &SECFLD(searchsector,Field), WallCase)

#define AIMED(Field) SFBASE_(Field, &WALFLD(searchwall, Field))
#define AIMED_SEL(Field) SFBASE_(Field, &AIMED_SEL_WALL(Field))
//#define AIMED_CF(Field) SFBASE_CF(Field, &WALFLD(searchwall,Field))
#define AIMED_CF_SEL(Field) SFBASE_CF(Field, &AIMED_SEL_WALL(Field))

// OVR makes sense only with picnum
//#define AIMED_OVR_PICNUM  SFBASE_CF(picnum, &OVR_WALL(searchwall, picnum))
#define AIMED_SELOVR_PICNUM SFBASE_CF(picnum, &AIMED_SELOVR_WALL(picnum))

static const char *ONOFF_[] = {"OFF","ON"};
#define ONOFF(b) (ONOFF_[!!(b)])

static int32_t tsign, mouseax=0, mouseay=0;
static int32_t repeatcountx, repeatcounty;
static int32_t infobox=3; // bit0: current window, bit1: mouse pointer, the variable should be renamed

static char wallshades[MAXWALLS];
static char sectorshades[MAXSECTORS][2];
static char spriteshades[MAXSPRITES];
static char wallpals[MAXWALLS];
static char sectorpals[MAXSECTORS][2];
static char spritepals[MAXSPRITES];
static uint8_t wallflag[bitmap_size(MAXWALLS)];

#ifdef YAX_ENABLE
static uint8_t havebunch[YAX_MAXBUNCHES];
static int32_t *tempzar[YAX_MAXBUNCHES];

static int32_t yax_invalidop()
{
    silentmessage("Operation forbidden on extended sector.");
    return 0;
}

static int32_t yax_invalidslope()
{
    silentmessage("Firstwalls must coincide for changing slope.");
    return 0;
}

// 1: ok
static int32_t yax_checkslope(int16_t sectnum, int32_t othersectnum)
{
    int16_t w1 = sector[sectnum].wallptr, w2 = wall[w1].point2;
    int16_t nw1 = sector[othersectnum].wallptr, nw2 = wall[nw1].point2;

    if (nw1 < 0)
        return 0;  // error

    nw2 = wall[nw1].point2;
    if (wall[w1].x != wall[nw1].x || wall[w1].y != wall[nw1].y ||
            wall[w2].x != wall[nw2].x || wall[w2].y != wall[nw2].y)
        return 0;

    return 1;
}

# define YAXSLOPECHK(s,os)  (yax_checkslope(s,os) || yax_invalidslope())
# define YAXCHK(p) ((p) || yax_invalidop())
#endif

// tile marking in tile selector for custom creation of tile groups
static uint8_t tilemarked[bitmap_size(MAXTILES)];

#ifdef POLYMER
static int16_t spritelightid[MAXSPRITES];
_prlight *spritelightptr[MAXSPRITES];

static int32_t check_prlight_colors(int32_t i)
{
    return (sprite[i].xvel != spritelightptr[i]->color[0]) ||
        (sprite[i].yvel != spritelightptr[i]->color[1]) ||
        (sprite[i].zvel != spritelightptr[i]->color[2]);
}

static void copy_prlight_colors(_prlight *mylightptr, int32_t i)
{
    mylightptr->color[0] = sprite[i].xvel;
    mylightptr->color[1] = sprite[i].yvel;
    mylightptr->color[2] = sprite[i].zvel;
}

static void addprlight_common1(_prlight *mylightptr, int32_t i)
{
    mylightptr->sector = SECT(i);
    Bmemcpy(mylightptr, &sprite[i], sizeof(vec3_t));
    mylightptr->range = SHT(i);
    copy_prlight_colors(mylightptr, i);
    mylightptr->angle = SA(i);
    mylightptr->horiz = SH(i);
    mylightptr->minshade = sprite[i].xoffset;
    mylightptr->maxshade = sprite[i].yoffset;

    // overridden for spot lights
    mylightptr->radius = mylightptr->faderadius = mylightptr->tilenum = 0;

    if (CS(i) & 2)
    {
        if (CS(i) & 512)
            mylightptr->priority = PR_LIGHT_PRIO_LOW;
        else
            mylightptr->priority = PR_LIGHT_PRIO_HIGH;
    }
    else
        mylightptr->priority = PR_LIGHT_PRIO_MAX;

    mylightptr->publicflags.negative = !!(CS(i) & 128);

    spritelightid[i] = polymer_addlight(mylightptr);
    if (spritelightid[i] >= 0)
        spritelightptr[i] = &prlights[spritelightid[i]];
}

static void DeletePolymerLights(void)
{
    int32_t i;
    for (i=0; i<MAXSPRITES; i++)
        if (spritelightptr[i] != NULL)
        {
            polymer_deletelight(spritelightid[i]);
            spritelightid[i] = -1;
            spritelightptr[i] = NULL;
        }
}

void G_Polymer_UnInit(void)
{
    DeletePolymerLights();
}
#endif

extern int32_t mskip;

//extern int32_t fillsector(int16_t sectnum, char fillcolor);

static int osdcmd_quit(osdcmdptr_t parm);


#define M32_NUM_SPRITE_MODES (signed)ARRAY_SIZE(SpriteMode)
static const char *SpriteMode[]=
{
    "NONE",
    "SECTORS",
    "WALLS",
    "SPRITES",
    "ALL",
    "ITEMS ONLY",
    "CURRENT SPRITE ONLY",
    //"ONLY SECTOREFFECTORS AND SECTORS",
    //"NO SECTOREFFECTORS OR SECTORS"
};

#define MAXSKILL 5
static const char *SKILLMODE[MAXSKILL]=
{
    "Actor skill display: PIECE OF CAKE",
    "Actor skill display: LET'S ROCK",
    "Actor skill display: COME GET SOME",
    "Actor skill display: DAMN I'M GOOD",
    "Actor skill display: ALL SKILL LEVELS"
};

#define MAXNOSPRITES 4
static const char *SPRDSPMODE[MAXNOSPRITES]=
{
    "Sprite display: DISPLAY ALL SPRITES",
    "Sprite display: NO EFFECTORS",
    "Sprite display: NO ACTORS",
    "Sprite display: NO EFFECTORS OR ACTORS"
};

#define MAXHELP3D (signed)ARRAY_SIZE(Help3d)
static const char *Help3d[]=
{
    "NMapedit 3D mode help",
    " ",
    " F2 = TOGGLE CLIPBOARD",
    " F3 = TOGGLE MOUSELOOK",
    " F4 = TOGGLE AMBIENT SOUNDS",
    " F6 = AUTOMATIC SECTOREFFECTOR HELP",
    " F7 = AUTOMATIC SECTOR TAG HELP",
    "",
    " ' A = TOGGLE AUTOSAVE",
    " ' D = CYCLE SPRITE SKILL DISPLAY",
    " ' R = TOGGLE FRAMERATE DISPLAY",
    " ' W = TOGGLE SPRITE DISPLAY",
    " ' X = MAP SHADE PREVIEW",
    " ' I = TOGGLE INVISIBLE SPRITES",
    "",
    " ' T = CHANGE LOTAG",
    " ' H = CHANGE HITAG",
    " ' S = CHANGE SHADE",
    " ' M = CHANGE EXTRA",
    " ' V = CHANGE VISIBILITY",
    " ' L = CHANGE OBJECT COORDINATES",
    " ' C = CHANGE GLOBAL SHADE",
    "",
    " ' ENTER = PASTE GRAPHIC ONLY",
    " ' P & ; P = PASTE PALETTE TO ALL SELECTED SECTORS",
    " ; V = SET VISIBILITY ON ALL SELECTED SECTORS",
    " ' DEL = CSTAT=0",
    " CTRL-S = SAVE BOARD",
    " HOME = PGUP/PGDN MODIFIER (256 UNITS)",
    " END = PGUP/PGDN MODIFIER (512 UNITS)",
};

const char *ExtGetVer(void)
{
    return s_buildRev;
}

void ExtSetupMapFilename(const char *mapname)
{
    UNREFERENCED_PARAMETER(mapname);
}

void ExtLoadMap(const char *mapname)
{
    UNREFERENCED_PARAMETER(mapname);

    getmessageleng = 0;
    getmessagetimeoff = 0;

    // Old-fashioned multi-psky handling setup.
    G_SetupGlobalPsky();

    //parallaxtype = 0;

    //////////
#if M32_UNDO
    map_undoredo_free();
#endif
}

void ExtSaveMap(const char *mapname)
{
    UNREFERENCED_PARAMETER(mapname);
}


////////// tag labeling system //////////

#define TLCHAR "+"
#define TLCHR(Cond) ((Cond)?TLCHAR:"")

int32_t taglab_linktags(int32_t, int32_t)
{
    return 0;
}

// The return value is an OR of the following:
//  1: rxID has linking semantics
//  2: txID
int32_t taglab_linkxtags(int32_t stat, int32_t num)
{
    // NUKE-TODO: Adjust link value depending on object type
    int32_t link = 0;
    switch (stat)
    {
    case 0:
    case 4:
    {
        switch (wall[num].lotag)
        {
        default:
            link = 1+2;
            break;
        }
        break;
    }
    case 1:
    case 2:
    {
        switch (sector[num].lotag)
        {
        default:
            link = 1+2;
            break;
        }
        break;
    }
    case 3:
    {
        switch (sprite[num].type)
        {
        default:
            link = 1+2;
            break;
        }
        break;
    }
    }

    g_iReturnVar = link;
    VM_OnEvent(EVENT_LINKTAGS, stat==3?num:-1);
    link = g_iReturnVar;

    return link;
}

// <duetoptr>: if non-NULL, a value will be written denoting the object with
// the currently greatest tag:
//  32768 + spritenum, or
//  16384 + sectnum, or
//  wallnum, or
//  -1 (the return value i.e. no more tags left OR there are no tagged objects)
int32_t taglab_getnextfreetag(int32_t *duetoptr)
{
    int32_t i, nextfreetag=100;
    int32_t obj = -1;

    for (i=0; i<MAXSPRITES; i++)
    {
        int nXSprite = sprite[i].extra;
        if (nXSprite > 0)
        {
            if (nextfreetag <= xsprite[nXSprite].rxID)
            {
                nextfreetag = xsprite[nXSprite].rxID+1;
                obj = 32768 + i;
            }
            if (nextfreetag <= xsprite[nXSprite].txID)
            {
                nextfreetag = xsprite[nXSprite].txID+1;
                obj = 32768 + i;
            }
        }
    }

    for (i=0; i<numwalls; i++)
    {
        int nXWall = wall[i].extra;
        if (nXWall > 0)
        {
            if (nextfreetag <= xwall[nXWall].rxID)
                nextfreetag = xwall[nXWall].rxID+1, obj = i;
            if (nextfreetag <= xwall[nXWall].txID)
                nextfreetag = xwall[nXWall].txID+1, obj = i;
        }
    }

    for (i=0; i<numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector > 0)
        {
            if (nextfreetag <= xsector[nXSector].rxID)
                nextfreetag = xsector[nXSector].rxID+1, obj = 16384 + i;
            if (nextfreetag <= xsector[nXSector].txID)
                nextfreetag = xsector[nXSector].txID+1, obj = 16384 + i;
        }
    }

    if (duetoptr != NULL)
        *duetoptr = obj;

    if (nextfreetag < 1024)
        return nextfreetag;

    return 0;
}


static void taglab_handle1(int32_t linktagp, int32_t tagnum, char *buf)
{
    char const * const label = (linktagp && showtags==2) ? taglab_getlabel(tagnum) : NULL;

    if (label)
        Bsprintf(buf, "%d<%s>", tagnum, label);
    else
        Bsprintf(buf, "%d%s", tagnum, TLCHR(linktagp));
}
////////// end tag labeling system //////////

////////// blood stuff //////////


class CXTracker {
public:
    char m_type;
    int m_nSector;
    int m_nWall;
    int m_nSprite;
    sectortype *m_pSector;
    walltype *m_pWall;
    spritetype *m_pSprite;
    XSECTOR *m_pXSector;
    XWALL *m_pXWall;
    XSPRITE *m_pXSprite;
    CXTracker();
    void TrackClear();
    void TrackSector(int a1, char a2);
    void TrackWall(int a1, char a2);
    void TrackSprite(int a1, char a2);
    void Draw(void);
};

CXTracker gXTracker;

extern void drawlinebetween(const vec3_t *v1, const vec3_t *v2, int32_t col, uint32_t pat);

CXTracker::CXTracker()
{
    //dprintf("CXTracker::CXTracker()\n");
    TrackClear();
}

void CXTracker::TrackClear()
{
    //dprintf("CXTracker::TrackClear()\n");
    m_type = 0;
    m_nSector = m_nWall = m_nSprite = -1;
    m_pSector = 0;
    m_pWall = 0;
    m_pSprite = 0;
    m_pXSector = 0;
    m_pXWall = 0;
    m_pXSprite = 0;
}

void CXTracker::TrackSector(int nSector, char a2)
{
    //dprintf("CXTracker::TrackSector( %d, %s )\n", nSector, a2 ? "TRUE" : "FALSE");
    TrackClear();
    if (nSector < 0 || nSector >= kMaxSectors)
        return;
    //dprintf("sector is valid\n");
    sectortype* pSector = &sector[nSector];
    int nXSector = pSector->extra;
    if (nXSector <= 0 || nXSector >= kMaxXSprites) // ???
        return;
    //dprintf("xsector is valid\n");
    XSECTOR* pXSector = &xsector[nXSector];
    if ((a2 && !pXSector->txID) || (!a2 && !pXSector->rxID))
        return;
    //dprintf("txID = %d,  rxID = %d\n", pXSector->at6_0, pXSector->at8_0);
    m_nSector = nSector;
    m_pSector = pSector;
    m_pXSector = pXSector;
    m_type = a2;
}

void CXTracker::TrackWall(int nWall, char a2)
{
    //dprintf("CXTracker::TrackWall( %d, %s )\n", nWall, a2 ? "TRUE" : "FALSE");
    TrackClear();
    if (nWall < 0 || nWall >= kMaxWalls)
        return;
    //dprintf("wall is valid\n");
    walltype* pWall = &wall[nWall];
    int nXWall = pWall->extra;
    if (nXWall <= 0 || nXWall >= kMaxXSprites) // ???
        return;
    //dprintf("xwall is valid\n");
    XWALL* pXWall = &xwall[nXWall];
    if ((a2 && !pXWall->txID) || (!a2 && !pXWall->rxID))
        return;
    //dprintf("txID = %d,  rxID = %d\n", pXWall->at6_0, pXWall->at8_0);
    m_nWall = nWall;
    m_pWall = pWall;
    m_pXWall = pXWall;
    m_type = a2;
}

void CXTracker::TrackSprite(int nSprite, char a2)
{
    //dprintf("CXTracker::TrackSprite( %d, %s )\n", nWall, a2 ? "TRUE" : "FALSE");
    TrackClear();
    if (nSprite < 0 || nSprite >= kMaxSprites)
        return;
    //dprintf("sprite is valid\n");
    spritetype* pSprite = &sprite[nSprite];
    int nXSprite = pSprite->extra;
    if (nXSprite <= 0 || nXSprite >= kMaxXSprites) // ???
        return;
    //dprintf("xsprite is valid\n");
    XSPRITE* pXSprite = &xsprite[nXSprite];
    if ((a2 && !pXSprite->txID) || (!a2 && !pXSprite->rxID))
        return;
    //dprintf("txID = %d,  rxID = %d\n", pXSprite->at4_0, pXSprite->at5_2);
    m_nSprite = nSprite;
    m_pSprite = pSprite;
    m_pXSprite = pXSprite;
    m_type = a2;
}

void CXTracker::Draw(void)
{
    int txId, rxId;
    vec3_t v1, v2;
    if (m_type)
    {
        if (m_pXSector)
        {
            txId = m_pXSector->txID;
            walltype* pWall2 = &wall[m_pSector->wallptr];
            v1.x = pWall2->x;
            v1.y = pWall2->y;
            v1.z = getflorzofslope(m_nSector, v1.x, v1.y);
        }
        else if (m_pXWall)
        {
            txId = m_pXWall->txID;
            walltype* pWall2 = &wall[m_pWall->point2];
            int dx = (pWall2->x-m_pWall->x)/2; 
            int dy = (pWall2->y-m_pWall->y)/2; 
            v1.x = m_pWall->x + dx;
            v1.y = m_pWall->y + dy;
            v1.z = getflorzofslope(sectorofwall(m_nWall), v1.x, v1.y);
        }
        else if (m_pXSprite)
        {
            txId = m_pXSprite->txID;
            v1.x = m_pSprite->x;
            v1.y = m_pSprite->y;
            v1.z = m_pSprite->z;
        }
        else
        {
            return;
        }
        if (txId == 0)
            return;
        for (int i = 0; i < kMaxSprites; i++)
        {
            spritetype* pSprite = &sprite[i];
            if (pSprite->statnum < 0 || pSprite->statnum >= kMaxStatus)
                continue;
            int nXSprite = pSprite->extra;
            if (nXSprite <= 0)
                continue;
            XSPRITE* pXSprite = &xsprite[nXSprite];
            if (pXSprite->rxID == txId)
            {
                v2.x = pSprite->x;
                v2.y = pSprite->y;
                v2.z = pSprite->z;
                drawlinebetween(&v1, &v2, editorcolors[3]+(M32_THROB>>2), 0x33333333);
            }
        }
        for (int i = 0; i < numwalls; i++)
        {
            walltype* pWall = &wall[i];
            int nXWall = pWall->extra;
            if (nXWall <= 0)
                continue;
            XWALL* pXWall = &xwall[nXWall];
            if (pXWall->rxID == txId)
            {
                walltype* pWall2 = &wall[pWall->point2];
                int dx = (pWall2->x-pWall->x)/2;
                int dy = (pWall2->y-pWall->y)/2; 
                v2.x = pWall->x + dx;
                v2.y = pWall->y + dy;
                v2.z = getflorzofslope(sectorofwall(i), v2.x, v2.y);
                drawlinebetween(&v1, &v2, editorcolors[4]+(M32_THROB>>2), 0x33333333);
            }
        }
        for (int i = 0; i < numsectors; i++)
        {
            sectortype* pSector = &sector[i];
            int nXSector = pSector->extra;
            if (nXSector <= 0)
                continue;
            XSECTOR* pXSector = &xsector[nXSector];
            if (pXSector->rxID == txId)
            {
                walltype* pWall = &wall[pSector->wallptr];
                v2.x = pWall->x;
                v2.y = pWall->y;
                v2.z = getflorzofslope(i, v2.x, v2.y);
                drawlinebetween(&v1, &v2, editorcolors[6]+(M32_THROB>>2), 0x33333333);
            }
        }
    }
    else
    {
        if (m_pXSector)
        {
            rxId = m_pXSector->rxID;
            walltype* pWall2 = &wall[m_pSector->wallptr];
            v1.x = pWall2->x;
            v1.y = pWall2->y;
            v1.z = getflorzofslope(m_nSector, v1.x, v1.y);
        }
        else if (m_pXWall)
        {
            rxId = m_pXWall->rxID;
            walltype* pWall2 = &wall[m_pWall->point2];
            int dx = (pWall2->x-m_pWall->x)/2; 
            int dy = (pWall2->y-m_pWall->y)/2; 
            v1.x = m_pWall->x + dx;
            v1.y = m_pWall->y + dy;
            v1.z = getflorzofslope(sectorofwall(m_nWall), v1.x, v1.y);
        }
        else if (m_pXSprite)
        {
            rxId = m_pXSprite->rxID;
            v1.x = m_pSprite->x;
            v1.y = m_pSprite->y;
            v1.z = m_pSprite->z;
        }
        else
        {
            return;
        }
        if (rxId == 0)
            return;
        for (int i = 0; i < kMaxSprites; i++)
        {
            spritetype* pSprite = &sprite[i];
            if (pSprite->statnum < 0 || pSprite->statnum >= kMaxStatus)
                continue;
            int nXSprite = pSprite->extra;
            if (nXSprite <= 0)
                continue;
            XSPRITE* pXSprite = &xsprite[nXSprite];
            if (pXSprite->txID == rxId)
            {
                v2.x = pSprite->x;
                v2.y = pSprite->y;
                v2.z = pSprite->z;
                drawlinebetween(&v1, &v2, editorcolors[11]-(M32_THROB>>2), 0x33333333);
            }
        }
        for (int i = 0; i < numwalls; i++)
        {
            walltype* pWall = &wall[i];
            int nXWall = pWall->extra;
            if (nXWall <= 0)
                continue;
            XWALL* pXWall = &xwall[nXWall];
            if (pXWall->txID == rxId)
            {
                walltype* pWall2 = &wall[pWall->point2];
                int dx = (pWall2->x-pWall->x)/2;
                int dy = (pWall2->y-pWall->y)/2; 
                v2.x = pWall->x + dx;
                v2.y = pWall->y + dy;
                v2.z = getflorzofslope(sectorofwall(i), v2.x, v2.y);
                drawlinebetween(&v1, &v2, editorcolors[12]-(M32_THROB>>2), 0x33333333);
            }
        }
        for (int i = 0; i < numsectors; i++)
        {
            sectortype* pSector = &sector[i];
            int nXSector = pSector->extra;
            if (nXSector <= 0)
                continue;
            XSECTOR* pXSector = &xsector[nXSector];
            if (pXSector->txID == rxId)
            {
                walltype* pWall = &wall[pSector->wallptr];
                v2.x = pWall->x;
                v2.y = pWall->y;
                v2.z = getflorzofslope(i, v2.x, v2.y);
                drawlinebetween(&v1, &v2, editorcolors[14]-(M32_THROB>>2), 0x33333333);
            }
        }
    }
}

struct TextList {
    short id;
    const char* text;
};

TextList tOffOn[] = {
    { 0, "Off" },
    { 1, "On" },
};

TextList tSpriteType[] = {
    { 0, "Decoration" },
    { 1, "Player Start" },
    { 2, "Bloodbath Start" },
    { 3, "Off marker" },
    { 4, "On marker" },
    { 5, "Axis marker" },
    { 6, "Lower link" },
    { 7, "Upper link" },
    { 8, "Teleport target" },
    { 10, "Lower water" },
    { 9, "Upper water" },
    { 12, "Lower stack" },
    { 11, "Upper stack" },
    { 14, "Lower goo" },
    { 13, "Upper goo" },
    { 15, "Path marker" },
    //{ 16, "Alignable Region" },
    //{ 17, "Base Region" },
    { 18, "Dude Spawn" },
    { 19, "Earthquake" },
    { 20, "Toggle switch" },
    { 21, "1-Way switch" },
    { 22, "Combination switch" },
    { 23, "Padlock (1-shot)" },
    { 30, "Torch" },
    { 32, "Candle" },
    { 40, gWeaponText[0] },
    //{ 47, gWeaponText[7] },
    { 43, gWeaponText[3] },
    { 41, gWeaponText[1] },
    { 42, gWeaponText[2] },
    { 46, gWeaponText[6] },
    //{ 49, gWeaponText[9] },
    //{ 48, gWeaponText[8] },
    { 45, gWeaponText[5] },
    { 50, gWeaponText[10] },
    { 44, gWeaponText[4] },
    { 60, gAmmoText[0] },
    { 62, gAmmoText[2] },
    { 63, gAmmoText[3] },
    { 64, gAmmoText[4] },
    { 65, gAmmoText[5] },
    { 67, gAmmoText[7] },
    { 68, gAmmoText[8] },
    { 69, gAmmoText[9] },
    { 70, gAmmoText[10] },
    { 72, gAmmoText[12] },
    { 73, gAmmoText[13] },
    { 76, gAmmoText[16] },
    { 79, gAmmoText[19] },
    { 66, gAmmoText[6] },
    { 80, "Random Ammo" },
    { 100, gItemText[0] },
    { 101, gItemText[1] },
    { 102, gItemText[2] },
    { 103, gItemText[3] },
    { 104, gItemText[4] },
    { 105, gItemText[5] },
    { 106, gItemText[6] },
    { 107, gItemText[7] },
    { 108, gItemText[8] },
    { 109, gItemText[9] },
    { 110, gItemText[10] },
    { 111, gItemText[11] },
    { 112, gItemText[12] },
    { 113, gItemText[13] },
    { 114, gItemText[14] },
    { 115, gItemText[15] },
    //{ 116, gItemText[16] },
    { 117, gItemText[17] },
    { 118, gItemText[18] },
    { 119, gItemText[19] },
    //{ 120, gItemText[20] },
    { 121, gItemText[21] },
    //{ 122, gItemText[22] },
    { 123, gItemText[23] },
    { 124, gItemText[24] },
    { 125, gItemText[25] },
    //{ 126, gItemText[26] },
    { 127, gItemText[27] },
    { 128, gItemText[28] },
    { 129, gItemText[29] },
    { 130, gItemText[30] },
    //{ 131, gItemText[31] },
    //{ 132, gItemText[32] },
    //{ 133, gItemText[33] },
    //{ 134, gItemText[34] },
    //{ 135, gItemText[35] },
    { 136, gItemText[36] },
    { 137, gItemText[37] },
    { 138, gItemText[38] },
    { 139, gItemText[39] },
    { 140, gItemText[40] },
    { 141, gItemText[41] },
    { 142, gItemText[42] },
    { 143, gItemText[43] },
    { 144, gItemText[44] },
    { 145, gItemText[45] },
    { 146, gItemText[46] },
    { 201, "Cultist w/Tommy" },
    { 202, "Cultist w/Shotgun" },
    { 247, "Cultist w/Tesla" },
    { 248, "Cultist w/Dynamite" },
    { 249, "Beast Cultist" },
    { 250, "Tiny Caleb" },
    { 251, "Beast" },
    { 203, "Axe Zombie" },
    { 204, "Fat Zombie" },
    { 205, "Earth Zombie" },
    { 244, "Sleep Zombie" },
    { 245, "Innocent" },
    { 206, "Flesh Gargoyle" },
    { 207, "Stone Gargoyle" },
    { 208, "Flesh Statue" },
    { 209, "Stone Statue" },
    { 210, "Phantasm" },
    { 211, "Hound" },
    { 212, "Hand" },
    { 213, "Brown Spider" },
    { 214, "Red Spider" },
    { 216, "Mother Spider" },
    { 215, "Black Spider" },
    { 217, "GillBeast" },
    { 218, "Eel" },
    { 219, "Bat" },
    { 220, "Rat" },
    { 221, "Green Pod" },
    { 222, "Green Tentacle" },
    { 223, "Fire Pod" },
    { 224, "Fire Tentacle" },
    { 227, "Cerberus" },
    { 228, "Cerberus (1 Dead Head)" },
    { 229, "Tchernobog" },
    { 230, "TCultist prone" },
    { 246, "SCultist prone" },
    { 400, "TNT Barrel" },
    { 401, "Armed Prox Bomb" },
    { 402, "Armed Remote" },
    //{ 403, "Blue Vase" },
    //{ 404, "Brown Vase" },
    { 405, "Crate Face" },
    //{ 406, "Glass Window" },
    { 407, "Fluorescent Light" },
    { 408, "Wall Crack" },
    { 409, "Wood Beam" },
    { 410, "Spider's Web" },
    { 411, "MetalGrate1" },
    { 412, "FlammableTree" },
    { 413, "Machine Gun" },
    { 414, "Falling Rock" },
    { 415, "Kickable Pail" },
    { 416, "Gib Object" },
    { 417, "Explode Object" },
    { 427, "Zombie Head" },
   // { 450, "Spike Trap" },
   // { 451, "Rock Trap" },
    { 452, "Flame Trap" },
    { 454, "Saw Blade" },
    { 455, "Electric Zap" },
    { 456, "Switched Zap" },
    { 457, "Pendulum" },
    { 458, "Guillotine" },
    { 459, "Hidden Exploder" },
    { 700, "Trigger Gen" },
    { 701, "WaterDrip Gen" },
    { 702, "BloodDrip Gen" },
    { 703, "Fireball Gen" },
    { 704, "EctoSkull Gen" },
    //{ 705, "Dart Gen" },
    { 706, "Bubble Gen" },
    { 707, "Multi-Bubble Gen" },
    { 708, "SFX Gen" },
    { 709, "Sector SFX" },
    { 710, "Ambient SFX" },
    { 711, "Player SFX" },
};

TextList tWallType[] = {
    { 0, "Normal" },
    { 20, "Toggle switch" },
    { 21, "1-Way switch" },
    { 500, "Wall Link" },
    { 501, "Wall Stack" },
    { 511, "Gib Wall" },
};

TextList tSectType[] = {
    { 0, "Normal" },
    { 600, "Z Motion" },
    { 602, "Z Motion SPRITE" },
    { 603, "Warp" },
    { 604, "Teleporter" },
    { 614, "Slide Marked" },
    { 615, "Rotate Marked" },
    { 616, "Slide" },
    { 617, "Rotate" },
    { 613, "Step Rotate" },
    { 612, "Path Sector" },
    { 618, "Damage Sector" },
    { 619, "Counter Sector" },
};

TextList tWaveForm[] = {
    { 1, "Linear" },
    { 0, "Sine" },
    { 2, "SlowOff"},
    { 3, "SlowOn"},
};

TextList tLighting[] = {
    { 0, "None" },
    { 1, "Square" },
    { 2, "Saw" },
    { 3, "Ramp up" },
    { 4, "Ramp down" },
    { 5, "Sine" },
    { 6, "Flicker1" },
    { 7, "Flicker2" },
    { 8, "Flicker3" },
    { 9, "Flicker4" },
    { 10, "Strobe" },
    { 11, "Search" },
};

TextList tCmd[] = {
    { 0, "OFF" },
    { 1, "ON" },
    { 2, "State" },
    { 3, "Toggle" },
    { 4, "!State" },
    { 5, "Link" },
    { 6, "Lock" },
    { 7, "Unlock" },
    { 8, "Toggle Lock" },
    { 9, "Stop OFF" },
    { 10, "Stop ON" },
    { 11, "Stop Next" },
};

TextList tRespawn[] = {
    { 0, "Optional" },
    { 1, "Never" },
    { 2, "Always" },
    { 3, "Permanent" },
};

const char* pzOffOn[2];
const char* pzSpriteType[1024];
const char* pzWallType[1024];
const char* pzSectType[1024];
const char* pzWaveForm[8];
const char* pzLighting[20];
const char* pzCmd[64];
const char* pzNumber[192]; // UNUSED?
const char* pzRespawn[4];
char sNumberBuffer[192][4];

void FillList(const char **a1, TextList *a2, int a3)
{
    for (int i = 0; i < a3; i++)
    {
        a1[a2[i].id] = a2[i].text;
    }
}

void FillStringLists()
{
    FillList(pzOffOn, tOffOn, ARRAY_SIZE(tOffOn));
    FillList(pzSpriteType, tSpriteType, ARRAY_SIZE(tSpriteType));
    FillList(pzWallType, tWallType , ARRAY_SIZE(tWallType));
    FillList(pzSectType, tSectType, ARRAY_SIZE(tSectType));
    FillList(pzWaveForm, tWaveForm, ARRAY_SIZE(tWaveForm));
    FillList(pzLighting, tLighting, ARRAY_SIZE(tLighting));
    FillList(pzCmd, tCmd, ARRAY_SIZE(tCmd));
    for (int i = 0; i < 192; i++)
    {
        sprintf(sNumberBuffer[i], "%d", i);
        pzNumber[i] = sNumberBuffer[i];
    }
    FillList(pzRespawn, tRespawn, ARRAY_SIZE(tRespawn));
}

void cleanUp()
{
    dbXSectorClean();
#ifdef YAX_ENABLE
    for (int i = 0; i < numwalls; i++)
    {
        if (yax_hasnextwall(i))
            wall[i].extra = -1;
    }
#endif
    dbXWallClean();
    dbXSpriteClean();
    InitSectorFX();
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector > 0)
        {
            switch (sector[i].lotag)
            {
            case 604:
            {
                int nSprite = xsector[nXSector].marker0;
                if (nSprite >= 0)
                {
                    if (nSprite < kMaxSprites && sprite[nSprite].statnum == 10 && sprite[nSprite].type == 8)
                        sprite[nSprite].owner = i;
                    else
                        xsector[nXSector].marker0 = -1;
                }
                break;
            }
            case 614:
            case 616:
            {
                int nSprite = xsector[nXSector].marker0;
                if (nSprite >= 0)
                {
                    if (nSprite < kMaxSprites && sprite[nSprite].statnum == 10 && sprite[nSprite].type == 3)
                        sprite[nSprite].owner = i;
                    else
                        xsector[nXSector].marker0 = -1;
                }
                nSprite = xsector[nXSector].marker1;
                if (nSprite >= 0)
                {
                    if (nSprite < kMaxSprites && sprite[nSprite].statnum == 10 && sprite[nSprite].type == 4)
                        sprite[nSprite].owner = i;
                    else
                        xsector[nXSector].marker1 = -1;
                }
                break;
            }
            case 613:
            case 615:
            case 617:
            {
                int nSprite = xsector[nXSector].marker0;
                if (nSprite >= 0)
                {
                    if (nSprite < kMaxSprites && sprite[nSprite].statnum == 10 && sprite[nSprite].type == 5)
                        sprite[nSprite].owner = i;
                    else
                        xsector[nXSector].marker0 = -1;
                }
                break;
            }
            }
        }
    }
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector > 0)
        {
            switch (sector[i].lotag)
            {
            case 604:
            {
                int nSprite = xsector[nXSector].marker0;
                if (nSprite >= 0 && i != sprite[nSprite].owner)
                {
                    int vd = InsertSprite(sprite[nSprite].sectnum, 10);
                    sprite[vd] = sprite[nSprite];
                    sprite[vd].owner = i;
                    xsector[nXSector].marker0 = vd;
                }
                if (xsector[nXSector].marker0 < 0)
                {
                    int vd = InsertSprite(i, 10);
                    sprite[vd].x = wall[sector[i].wallptr].x;
                    sprite[vd].y = wall[sector[i].wallptr].y;
                    sprite[vd].owner = i;
                    sprite[vd].type = 8;
                    xsector[nXSector].marker0 = vd;
                }
                break;
            }
            case 614:
            case 616:
            {
                int nSprite = xsector[nXSector].marker0;
                if (nSprite >= 0 && i != sprite[nSprite].owner)
                {
                    int vd = InsertSprite(sprite[nSprite].sectnum, 10);
                    sprite[vd] = sprite[nSprite];
                    sprite[vd].owner = i;
                    xsector[nXSector].marker0 = vd;
                }
                if (xsector[nXSector].marker0 < 0)
                {
                    int vd = InsertSprite(i, 10);
                    sprite[vd].x = wall[sector[i].wallptr].x;
                    sprite[vd].y = wall[sector[i].wallptr].y;
                    sprite[vd].owner = i;
                    sprite[vd].type = 3;
                    xsector[nXSector].marker0 = vd;
                }
                nSprite = xsector[nXSector].marker1;
                if (nSprite >= 0 && i != sprite[nSprite].owner)
                {
                    int vd = InsertSprite(sprite[nSprite].sectnum, 10);
                    sprite[vd] = sprite[nSprite];
                    sprite[vd].owner = i;
                    xsector[nXSector].marker1 = vd;
                }
                if (xsector[nXSector].marker1 < 0)
                {
                    int vd = InsertSprite(i, 10);
                    sprite[vd].x = wall[sector[i].wallptr].x;
                    sprite[vd].y = wall[sector[i].wallptr].y;
                    sprite[vd].owner = i;
                    sprite[vd].type = 4;
                    xsector[nXSector].marker1 = vd;
                }
                break;
            }
            case 613:
            case 615:
            case 617:
            {
                int nSprite = xsector[nXSector].marker0;
                if (nSprite >= 0 && i != sprite[nSprite].owner)
                {
                    int vd = InsertSprite(sprite[nSprite].sectnum, 10);
                    sprite[vd] = sprite[nSprite];
                    sprite[vd].owner = i;
                    xsector[nXSector].marker0 = vd;
                }
                if (xsector[nXSector].marker0 < 0)
                {
                    int vd = InsertSprite(i, 10);
                    sprite[vd].x = wall[sector[i].wallptr].x;
                    sprite[vd].y = wall[sector[i].wallptr].y;
                    sprite[vd].owner = i;
                    sprite[vd].type = 5;
                    xsector[nXSector].marker0 = vd;
                }
                break;
            }
            default:
                xsector[nXSector].marker0 = -1;
                xsector[nXSector].marker1 = -1;
                break;
            }
        }
    }
    int nNextSprite;
    for (int nSprite = headspritestat[10]; nSprite != -1; nSprite = nNextSprite)
    {
        sprite[nSprite].extra = -1;
        sprite[nSprite].cstat |= 0x8000;
        sprite[nSprite].cstat &= ~0x101;
        nNextSprite = nextspritestat[nSprite];
        int nSector = sprite[nSprite].owner;
        int nXSector = sector[nSector].extra;
        if (nSector >= 0 && nSector < numsectors)
        {
            if (nXSector > 0 && nXSector < kMaxXSectors)
            {
                switch (sprite[nSprite].type)
                {
                case 4:
                    sprite[nSprite].picnum = 3997;
                    if (nSprite == xsector[nXSector].marker1)
                        continue;
                    break;
                case 3:
                case 5:
                    sprite[nSprite].picnum = 3997;
                case 8:
                    if (nSprite == xsector[nXSector].marker0)
                        continue;
                    break;
                }
            }
        }
        DeleteSprite(nSprite);
    }
}

int sub_10DBC(int nSector)
{
    dassert(nSector >= 0 && nSector < kMaxSectors);
    int nXSector = sector[nSector].extra;
    if (nXSector <= 0)
        nXSector = dbInsertXSector(nSector);
    return nXSector;
}

int sub_10E08(int nWall)
{
    dassert(nWall >= 0 && nWall < kMaxWalls);
    int nXWall = wall[nWall].extra;
    if (nXWall <= 0)
        nXWall = dbInsertXWall(nWall);
    return nXWall;
}

int getXSprite(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nXSprite = sprite[nSprite].extra;
    if (nXSprite <= 0)
        nXSprite = dbInsertXSprite(nSprite);
    return nXSprite;
}

struct ADJUST_DATA {
    short nType;
    short nTile;
    short xr;
    short yr;
    signed char hitscan;
    short nPlu;
};

ADJUST_DATA gAdjustData[] = {
    { 1, -1, 64, 64, 0, 0 },        // Single Player start
    { 2, -1, 64, 64, 0, 5 },        // Multiplayer start
    { 6, 2331, 80, 80, 0, 6 },      // Stacks and Links
    { 7, 2332, 80, 80, 0, 6 },      // ...
    { 10, 2331, 80, 80, 0, 0 },     // ...
    { 9, 2332, 80, 80, 0, 0 },      // ...
    { 12, 2331, 80, 80, 0, 0 },     // ...
    { 11, 2332, 80, 80, 0, 2 },     // ...
    { 14, 2331, 80, 80, 0, 0 },     // ...
    { 13, 2332, 80, 80, 0, 5 },     // ...
    { 15, 2319, 144, 144, 0, 1 },   // Path Marker
    { 18, 2077, 48, 48, 0, -1 },    // DudeSpawn
    { 19, 2072, 48, 48, 0, -1 },    // Earthquake
    { 20, -1, -1, -1, 1, -1 },      // Switches
    { 21, -1, -1, -1, 1, -1 },      // ...
    { 22, -1, -1, -1, 1, -1 },      // ...
    { 23, 948, -1, -1, 1, -1 },     // ...
    { 30, 550, -1, -1, 1, -1 },     // Torches
    { 30, 572, -1, -1, 1, -1 },     // ...
    { 30, 560, -1, -1, 1, -1 },     // ...
    { 30, 564, -1, -1, 1, -1 },     // ...
    { 30, 570, -1, -1, 1, -1 },     // ...
    { 30, 554, -1, -1, 1, -1 },     // ...
    { 32, 938, -1, -1, 1, -1 },     // Candle
    { 40, 832, 48, 48, 0, 0 },      // RANDOM
    { 43, 524, 48, 48, 0, 0 },      // Flare Pistol
    { 41, 559, 48, 48, 0, 0 },      // Sawed-off
    { 42, 558, 48, 48, 0, 0 },      // Tommy Gun
    { 46, 526, 48, 48, 0, 0 },      // Napalm Launcher
    { 45, 539, 48, 48, 0, 0 },      // Tesla Cannon
    { 50, 800, 48, 48, 0, 0 },      // Life Leech
    { 60, 618, 40, 40, 1, 0 },      // Spray Can
    { 62, 589, 48, 48, 1, 0 },      // Bundle of TNT
    { 63, 809, 48, 48, 1, 0 },      // Case of TNT
    { 64, 811, 40, 40, 0, 0 },      // Proximity Detonator
    { 65, 810, 40, 40, 0, 0 },      // Remote Detonator
    { 66, 820, 24, 24, 0, 0 },      // Trapped Soul
    { 67, 619, 48, 48, 0, 0 },      // 4 shotgun shells
    { 68, 812, 48, 48, 0, 0 },      // Box of shotgun shells
    { 69, 813, 48, 48, 0, 0 },      // A few bullets
    { 70, 525, 48, 48, 0, 0 },      // Voodoo doll
    { 72, 817, 48, 48, 0, 0 },      // Full drum of bullets
    { 73, 548, 24, 24, 0, 0 },      // Tesla Charge
    { 76, 816, 48, 48, 0, 0 },      // Flares
    { 79, 801, 48, 48, 0, 0 },      // Gasoline Can
    { 100, 2552, 32, 32, 0, 0 },    // Skull Key
    { 101, 2553, 32, 32, 0, 0 },    // Eye key
    { 102, 2554, 32, 32, 0, 0 },    // Fire key
    { 103, 2555, 32, 32, 0, 0 },    // Dagger key
    { 104, 2556, 32, 32, 0, 0 },    // Spider key
    { 105, 2557, 32, 32, 0, 0 },    // Moon key
    { 106, -1, -1, -1, 0, 0 },      // Key 7
    { 107, 519, 48, 48, 0, 0 },     // Doctor's Bag
    { 108, 822, 40, 40, 0, 0 },     // Medicine pouch
    { 109, 2169, 40, 40, 0, 0 },    // Life essence
    { 110, 2433, 40, 40, 0, 0 },    // Life Seed
    { 113, 896, 40, 40, 0, 0 },     // Limited Invisibility
    { 114, 825, 40, 40, 0, 0 },     // Invulnerability
    { 115, 827, 40, 40, 0, 0 },     // Boots of jumping
    { 117, 829, 40, 40, 0, 0 },     // Guns akimbo
    { 118, 830, 80, 64, 0, 0 },     // Diving suit
    { 121, 760, 40, 40, 0, 0 },     // Crystall ball
    { 124, 2428, 40, 40, 0, 0 },    // Reflective shots
    { 125, 839, 40, 40, 0, 0 },     // Beast vision
    { 127, 840, 48, 48, 0, 0 },     // Rage shroom
    { 128, 841, 48, 48, 0, 0 },     // Delirium shroom
    { 129, 842, 48, 48, 0, 0 },     // Grow shroom
    { 130, 843, 48, 48, 0, 0 },     // Shrink shroom
    { 136, 518, 40, 40, 0, 0 },     // Tome
    { 137, 522, 40, 40, 0, 0 },     // Black chest
    { 138, 523, 40, 40, 0, 0 },     // Wooden chest
    { 140, 2628, 64, 64, 0, 0 },    // Basic armor
    { 141, 2586, 64, 64, 0, 0 },    // Body armor
    { 142, 2578, 64, 64, 0, 0 },    // Fire armor
    { 143, 2602, 64, 64, 0, 0 },    // Spirit armor
    { 144, 2594, 64, 64, 0, 0 },    // Super armor
    { 145, -1, 64, 64, 1, 0 },      // Blue team base
    { 146, -1, 64, 64, 1, 0 },      // Red team base
    { 201, 2820, 40, 40, 1, 3 },    // Cultist w/ tommy
    { 202, 2825, 40, 40, 1, 0 },    // Cultist w/ shotgun
    { 247, 2820, 40, 40, 1, 11 },   // Cultist w/ tesla
    { 248, 2820, 40, 40, 1, 13 },   // Cultist w/ dynamite
    { 249, 2825, 48, 48, 1, 12 },   // Beast cultist
    { 250, 3870, 16, 16, 1, 12 },   // Tiny caleb
    { 251, 2960, 48, 48, 1, 0 },    // The beast
    { 203, 1170, 40, 40, 1, 0 },    // Axe zombie
    { 204, 1370, 48, 48, 1, 0 },    // Fat zombie
    { 205, 3054, 40, 40, 1, 0 },    // Axe zombie (earth)
    { 244, 1209, 40, 40, 1, 0 },    // Axe zombie (sleep)
    { 245, 3798, 40, 40, 1, 0 },    // Innocent
    { 206, 1470, 40, 40, 1, 0 },    // Flesh Gargoyle
    { 207, 1470, 40, 40, 1, 5 },    // Stone Gargoyle
    { 208, 1530, 40, 40, 1, 0 },    // Flesh statue
    { 209, 1530, 40, 40, 1, 5 },    // Stone statue
    { 210, 3060, 40, 40, 1, 0 },    // Phantasm
    { 211, 1270, 40, 40, 1, 0 },    // Hound
    { 212, 1980, 32, 32, 1, 0 },    // Hand
    { 213, 1920, 16, 16, 1, 7 },    // Brown spider
    { 214, 1925, 24, 24, 1, 4 },    // Red spider
    { 216, 1930, 40, 40, 1, 0 },    // Mother spider
    { 215, 1935, 32, 32, 1, 4 },    // Black spider
    { 217, 1570, 48, 48, 1, 0 },    // Gillbeast
    { 218, 1870, 32, 32, 1, 0 },    // Eel
    { 219, 1948, 32, 32, 1, 0 },    // Bat
    { 220, 1745, 24, 24, 1, 0 },    // Rat
    { 221, 1792, 32, 32, 1, 0 },    // Green pod
    { 222, 1797, 32, 32, 1, 0 },    // Green tentacle
    { 223, 1792, 48, 48, 1, 2 },    // Fire pod
    { 224, 1797, 48, 48, 1, 2 },    // Fire tentacle
    { 227, 2680, 64, 64, 1, -1 },   // Cerberus
    { 229, 3140, -1, -1, 1, 0 },    // Tchernobog
    { 230, 3385, 40, 40, 1, 3 },    // Cultist w/ Tommy (prone)
    { 246, 3385, 40, 40, 1, 0 },    // Cultist w/ Shotgun (prone)
    { 400, 907, 64, 64, 1, 0 },     // TNT Barrel
    { 401, 3444, 40, 40, 1, 0 },    // Armed Prox Bomb
    { 402, 3457, 40, 40, 1, 0 },    // Armed Remote
    { 405, 462, -1, -1, 1, 0 },     // Crate face
    { 407, 796, -1, -1, 1, -1 },    // Fluorescent Light
    { 408, -1, -1, -1, 0, -1 },     // Wall Crack
    { 409, 1142, -1, -1, 1, -1 },   // Wood beam
    { 410, 1069, -1, -1, 1, -1 },   // Spider's Web
    { 411, 483, -1, -1, 1, -1 },    // MetalGrate1
    { 412, -1, -1, -1, 1, -1 },     // FlammableTree
    { 413, -1, 64, 64, 0, -1 },     // Machine Gun
    { 414, -1, -1, -1, 1, -1 },     // Falling Rock
    { 415, -1, 48, 48, 1, -1 },     // Kickable Pail
    { 416, -1, -1, -1, -1, -1 },    // Gib object
    { 417, -1, -1, -1, -1, -1 },    // Explode object
    { 427, -1, 40, 40, 1, -1 },     // Zombie head
    { 452, 2183, -1, -1, 0, -1 },   // Flame Trap
    { 454, 655, -1, -1, 1, -1 },    // Saw Blade
    { 455, 1156, -1, -1, 0, -1 },   // Electric Zap
    { 456, 1156, -1, -1, 0, -1 },   // Switched Zap
    { 457, 1080, -1, -1, 1, -1 },   // Pendulum
    { 458, 835, -1, -1, 1, -1 },    // Guillotine
    { 459, 908, 4, -1, 0, -1 },     // Hidden Exploder
    { 708, 2519, 40, 40, 0, 6 },    // SFX Gen
    { 709, 2520, 32, 32, 0, 9 },    // Sector SFX
    { 710, 2521, 40, 40, 0, 0 },    // Ambient sound
    { 711, 2519, 40, 40, 0, 5 },    // Player SFX
    
    { 200, 821, 64, 64, 1, 0 },     // Random dude (NOT in text list)
    { 243, 2860, 40, 40, 1, 0 },    // Unused cultist (NOT in text list)
    { 225, 1792, 64, 64, 1, 6 },    // Mother pod (NOT in text list)
    { 226, 1797, 64, 64, 1, 6 },    // Mother tentacle (NOT in text list)
    { 231, 2860, 40, 40, 1, 0 },    // Players (NOT in text list)
    { 232, 2860, 40, 40, 1, 0 },    // ...
    { 233, 2860, 40, 40, 1, 0 },    // ...
    { 234, 2860, 40, 40, 1, 0 },    // ...
    { 235, 2860, 40, 40, 1, 0 },    // ...
    { 236, 2860, 40, 40, 1, 0 },    // ...
    { 237, 2860, 40, 40, 1, 0 },    // ...
    { 238, 2860, 40, 40, 1, 0 },    // ...
    { 144, 2594, 64, 64, 0, 0 },    // (Super Armor duplicate)
    
    { 8, 3193, 255, 255, 0, 9 },    // Teleport Target (ADDED)
    { 111, 517, -1, -1, 0, -1 },    // Red potion (ADDED)
    { 112, 783, 48, 48, 0, -1 },    // Feather fall (ADDED)
    { 119, 831, 56, 56, 0, 0 },     // Gas mask (ADDED)
    { 123, -1, -1, -1, 0, -1 },     // Doppleganger (ADDED, no pic)
    { 139, 837, 76, 76, 0, 0 },     // Asbestos Armor (ADDED)
    { 228, 2749, 64, 64, 1, -1 },   // 1 Head Cerberus (ADDED in text list)
    { 701, 2028, 128, 128, 0, 10 }, // WaterDripGen (ADDED)
    { 702, 2028, 128, 128, 0, 2 },  // BloodDripGen (ADDED)

    { 450, 968, 64, 64, 0, -1 },    // Spike Trap (REMOVED from text list)
    { 451, -1, 64, 64, 0, -1 },     // Rock Trap (REMOVED from text list)
    { 406, 266, -1, -1, 1, 0 },     // Glass window (REMOVED from text list)
    { 403, 739, -1, -1, 1, 0 },     // Blue vase (REMOVED from text list)
    { 404, 642, -1, -1, 1, 0 },     // Brown vase (REMOVED from text list)
    { 80, 832, 40, 40, 0, 0 },      // Random Ammo (REMOVED from text list)
};




// by NoOne: do changes in adjustSprites function
int gCorrectedSprites = 0;
void adjustSprites()
{
    int vbx = gCorrectedSprites;
    for (int i = 0; i < kMaxSprites; i++)
    {
        spritetype* pSprite = &sprite[i];
        if (pSprite->statnum < kMaxStatus)
        {
            if ((pSprite->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) == CSTAT_SPRITE_ALIGNMENT_MASK) pSprite->cstat &= ~CSTAT_SPRITE_ALIGNMENT_MASK;
            if (pSprite->statnum == 1) continue;

            // don't turn unnamed types in Decoration
            //if (pSprite->type > 0 && !pzSpriteType[pSprite->type]) pSprite->type = 0;

            // search by tile and type
            int nType = -1, nTile = -1, adIndex = -1;
            for (int j = 0; j < ARRAY_SIZE(gAdjustData); j++) {

                if (gAdjustData[j].nTile >= 0 && gAdjustData[j].nTile == pSprite->picnum) nTile = j;
                if (gAdjustData[j].nType == pSprite->type) nType = j;
            
            }

            if (nTile >= 0 || (nTile >= 0 && pSprite->type == gAdjustData[nType].nType)) adIndex = nTile;
            if (nType >= 0) adIndex = nType;
            
            // don't ignore sprite if nothing found, but give it proper statnum.
            if (adIndex >= 0) {

                // give sprite proper type and look
                pSprite->type = gAdjustData[adIndex].nType;
                if (gAdjustData[adIndex].nTile >= 0) pSprite->picnum = gAdjustData[adIndex].nTile;
                if (gAdjustData[adIndex].xr >= 0) pSprite->xrepeat = gAdjustData[adIndex].xr;
                if (gAdjustData[adIndex].yr >= 0) pSprite->yrepeat = gAdjustData[adIndex].yr;

                if (gAdjustData[adIndex].hitscan == 0) pSprite->cstat &= ~CSTAT_SPRITE_BLOCK_HITSCAN;
                else if (gAdjustData[adIndex].hitscan > 0) pSprite->cstat |= CSTAT_SPRITE_BLOCK_HITSCAN;
                if (gAdjustData[adIndex].nPlu >= 0) pSprite->pal = gAdjustData[adIndex].nPlu;
            } 

            int type = pSprite->type;
            
            // adjust unique types
            switch (type) {
                case kMarkerSPStart:
                case kMarkerMPStart:
                {
                    pSprite->cstat &= ~CSTAT_SPRITE_BLOCK;
                    pSprite->shade = -128; // Nuke: Needed?
                    int nXSprite = getXSprite(i);
                    xsprite[nXSprite].data1 &= 0x07;
                    pSprite->picnum = 2522 + xsprite[nXSprite].data1;
                    ChangeSpriteStat(i, 0);
                }
                break;
                case kMarkerWarpDest:
                    pSprite->cstat |= CSTAT_SPRITE_ALIGNMENT_WALL; // Nuke: Needed?
                    break;
                case 18: // Dude spawn
                case 19: // Earthquake
                case kMarkerLowLink:
                case kMarkerUpLink:
                case kMarkerUpWater:
                case kMarkerLowWater:
                case kMarkerUpStack:
                case kMarkerLowStack:
                case kMarkerUpGoo:
                case kMarkerLowGoo:
                    pSprite->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_YFLIP); // Nuke: Y-Flip? weird.
                    pSprite->cstat |= CSTAT_SPRITE_INVISIBLE;
                    pSprite->shade = -128; // Nuke: Needed?
                    ChangeSpriteStat(i, 0);
                    getXSprite(i);
                    break;
                case kMarkerPath:
                    pSprite->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
                    pSprite->cstat |= CSTAT_SPRITE_INVISIBLE;
                    pSprite->shade = 127; // Nuke: Needed?
                    ChangeSpriteStat(i, 16);
                    getXSprite(i);
                    break;
                case 30: // Torch // Nuke: New?
                {
                    int nXSprite = getXSprite(i);
                    xsprite[nXSprite].state = 1;
                }
                break;
                case 32: // Candle // Nuke: New?
                {
                    int nXSprite = getXSprite(i);
                    if (pSprite->picnum == 938)
                        xsprite[nXSprite].state = 1;
                }
                break;
                case 228: // 1-Head Cerberus // Nuke: New?
                {
                    int nXSprite = getXSprite(i);
                    if (xsprite[nXSprite].rxID <= 0) xsprite[nXSprite].rxID = 7;
                    ChangeSpriteStat(i, 6);
                }
                break;
                case 700: // Trigger Gen // Nuke: New?
                case 701: // WaterDrip Gen // Nuke: New?
                case 702: // BloodDrip Gen // Nuke: New?
                case 708: // SFX Gen // Nuke: New?
                case 709: // Sector SFX // Nuke: New?
                case 711: // Player SFX
                    pSprite->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
                    pSprite->cstat |= CSTAT_SPRITE_INVISIBLE;
                    pSprite->shade = -128;
                    ChangeSpriteStat(i, 0);
                    getXSprite(i);
                    break;
                case 710: // Ambient SFX
                    pSprite->cstat &= ~(CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN);
                    pSprite->cstat |= CSTAT_SPRITE_INVISIBLE;
                    pSprite->shade = -128;
                    ChangeSpriteStat(i, 12);
                    getXSprite(i);
                    break;
                default:
                    switch (pSprite->statnum) { // count invalid sprites if any
                    case 3:
                        if (pSprite->type < kItemWeaponBase || pSprite->type >= kItemMax) gCorrectedSprites++;
                        break;
                    case 4:
                        if (pSprite->type < kThingBase || pSprite->type >= kThingMax) gCorrectedSprites++;
                        break;
                    case 6:
                        if (pSprite->type < kDudeBase || pSprite->type >= kDudeMax) gCorrectedSprites++;
                        break;
                    }
                    ChangeSpriteStat(i, 0);
                    break;
            }
            
            // adjust groups of types
            int top, bottom;
            if (type >= kSwitchBase && type < kSwitchMax) {
                pSprite->cstat &= ~CSTAT_SPRITE_BLOCK;
                ChangeSpriteStat(i, 0);
                getXSprite(i);
            }
            else if (type >= kItemWeaponBase && type < kItemMax)
            {
                if ((pSprite->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) != CSTAT_SPRITE_ALIGNMENT_FACING) continue; // why?
                pSprite->cstat &= ~CSTAT_SPRITE_BLOCK;
                ChangeSpriteStat(i, 3);
                getXSprite(i);
            }
            else if (type >= kDudeBase && type < kDudeMax)
            {
                if ((pSprite->cstat & CSTAT_SPRITE_ALIGNMENT_MASK) != CSTAT_SPRITE_ALIGNMENT_FACING) continue; // why?
                pSprite->cstat &= ~CSTAT_SPRITE_BLOCK;
                ChangeSpriteStat(i, 6);
                getXSprite(i);

                GetSpriteExtents(pSprite, &top, &bottom);
                if (!(sector[pSprite->sectnum].ceilingstat & kSecCParallax))
                    pSprite->z += ClipLow(sector[pSprite->sectnum].ceilingz - top, 0);
                if (!(sector[pSprite->sectnum].floorstat & kSecCParallax))
                    pSprite->z += ClipHigh(sector[pSprite->sectnum].floorz - bottom, 0);
            }
            else if (type >= kThingBase && type < kThingMax)
            {
                ChangeSpriteStat(i, 4);
                getXSprite(i);

                GetSpriteExtents(pSprite, &top, &bottom);
                if (!(sector[pSprite->sectnum].ceilingstat & kSecCParallax))
                    pSprite->z += ClipLow(sector[pSprite->sectnum].ceilingz - top, 0);
                if (!(sector[pSprite->sectnum].floorstat & kSecCParallax))
                    pSprite->z += ClipHigh(sector[pSprite->sectnum].floorz - bottom, 0);
            }
            else if (type >= 450 && type < 460) // traps
            {
                ChangeSpriteStat(i, 11);
                getXSprite(i);
            }
        }
    }
    if (vbx != gCorrectedSprites)
    {
        char buffer[40];
        sprintf(buffer, "Fixed %d sprites", gCorrectedSprites - vbx);
        if (qsetmode == 200)
        {
            // NUKE-TODO:
            //scrSetMessage(buffer);
        }
        else
            printmessage16(buffer);
    }
}

// NUKE-TODO: Implement pc speaker sound?
void Beep() { }
void ModifyBeep() { asksave = 1; /* Necessary? */ }

char gTempBuf[256];

enum CONTROL_TYPE
{
    CONTROL_TYPE_0 = 0, // LABEL
    CONTROL_TYPE_1, // NUMBER
    CONTROL_TYPE_2, // TOGGLEBUTTON
    RADIOBUTTON, // 3 RADIOBUTTON
    CONTROL_TYPE_4, // LIST
    CONTROL_TYPE_5,
    CONTROL_END = 255
};

struct CONTROL {
    int at0;
    int at4;
    int at8; // tag?
    CONTROL_TYPE type; // atc
    const char* atd;
    int at11; // minvalue
    int at15; // maxvalue
    const char** names; // at19
    char (*at1d)(CONTROL* control, char key);
    int at21; // value
};

char sub_1BE80(CONTROL* control, char a2);
char sub_1BFD8(CONTROL* control, char a2);
char sub_1BFF4(CONTROL* control, char a2);

CONTROL controlXSprite[] = {
    { 0, 0, 1, CONTROL_TYPE_4, "Type %4d: %-18.18s", 0, 1023, pzSpriteType, NULL, 0 },
    { 0, 1, 2, CONTROL_TYPE_1, "RX ID: %4d", 0, 1023, NULL, sub_1BE80, 0 },
    { 0, 2, 3, CONTROL_TYPE_1, "TX ID: %4d", 0, 1023, NULL, sub_1BE80, 0 },
    { 0, 3, 4, CONTROL_TYPE_4, "State %1d: %-3.3s", 0, 1, pzOffOn, NULL, 0 },
    { 0, 4, 5, CONTROL_TYPE_4, "Cmd: %3d: %-12.12s", 0, 255, pzCmd, NULL, 0 },
    { 0, 5, 0, CONTROL_TYPE_0, "Send when:", 0, 0, NULL, NULL, 0 },
    { 0, 6, 6, CONTROL_TYPE_2, "going ON:", 0, 0, NULL, NULL, 0 },
    { 0, 7, 7, CONTROL_TYPE_2, "going OFF:", 0, 0, NULL, NULL, 0 },
    { 0, 8, 8, CONTROL_TYPE_1, "busyTime = %4d", 0, 4095, NULL, NULL, 0 },
    { 0, 9, 9, CONTROL_TYPE_1, "waitTime = %4d", 0, 4095, NULL, NULL, 0 },
    { 0, 10, 10, CONTROL_TYPE_4, "restState %1d: %-3.3s", 0, 1, pzOffOn, NULL, 0 },
    { 30, 0, 0, CONTROL_TYPE_0, "Trigger On:", 0, 0, NULL, NULL, 0 },
    { 30, 1, 11, CONTROL_TYPE_2, "Push", 0, 0, NULL, NULL, 0 },
    { 30, 2, 12, CONTROL_TYPE_2, "Vector", 0, 0, NULL, NULL, 0 },
    { 30, 3, 13, CONTROL_TYPE_2, "Impact", 0, 0, NULL, NULL, 0 },
    { 30, 4, 14, CONTROL_TYPE_2, "Pickup", 0, 0, NULL, NULL, 0 },
    { 30, 5, 15, CONTROL_TYPE_2, "Touch", 0, 0, NULL, NULL, 0 },
    { 30, 6, 16, CONTROL_TYPE_2, "Sight", 0, 0, NULL, NULL, 0 },
    { 30, 7, 17, CONTROL_TYPE_2, "Proximity", 0, 0, NULL, NULL, 0 },
    { 30, 8, 18, CONTROL_TYPE_2, "DudeLockout", 0, 0, NULL, NULL, 0 },
    { 21, 9, 0, CONTROL_TYPE_0, "Launch 1 2 3 4 5 S B C T", 0, 0, NULL, NULL, 0 },
    { 28, 10, 19, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 30, 10, 20, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 32, 10, 21, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 34, 10, 22, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 36, 10, 23, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 38, 10, 24, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 40, 10, 25, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 42, 10, 26, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 44, 10, 27, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 46, 0, 0, CONTROL_TYPE_0, "Trigger Flags:", 0, 0, NULL, NULL, 0 },
    { 46, 1, 28, CONTROL_TYPE_2, "Decoupled", 0, 0, NULL, NULL, 0 },
    { 46, 2, 29, CONTROL_TYPE_2, "1-shot", 0, 0, NULL, NULL, 0 },
    { 46, 3, 30, CONTROL_TYPE_2, "Locked", 0, 0, NULL, NULL, 0 },
    { 46, 4, 31, CONTROL_TYPE_2, "Interruptable", 0, 0, NULL, NULL, 0 },
    { 46, 5, 32, CONTROL_TYPE_1, "Data1: %5d", 0, 65535, NULL, sub_1BFF4, 0 },
    { 46, 6, 33, CONTROL_TYPE_1, "Data2: %5d", 0, 65535, NULL, sub_1BFF4, 0 },
    { 46, 7, 34, CONTROL_TYPE_1, "Data3: %5d", 0, 65535, NULL, sub_1BFF4, 0 },
    { 46, 8, 35, CONTROL_TYPE_1, "Data4: %5d", 0, 65535, NULL, sub_1BFF4, 0 },
    { 46, 9, 36, CONTROL_TYPE_1, "Key: %1d", 0, 7, NULL, NULL, 0 },
    { 46, 10, 37, CONTROL_TYPE_4, "Wave: %1d %-8.8s", 0, 7, pzWaveForm, NULL, 0 },
    { 62, 0, 0, CONTROL_TYPE_0, "Respawn:", 0, 0, NULL, NULL, 0 },
    { 62, 1, 38, CONTROL_TYPE_4, "When %1d: %-6.6s", 0, 3, pzRespawn, NULL, 0 },
    { 62, 3, 0, CONTROL_TYPE_0, "Dude Flags:", 0, 0, NULL, NULL, 0 },
    { 62, 4, 39, CONTROL_TYPE_2, "dudeDeaf", 0, 0, NULL, NULL, 0 },
    { 62, 5, 40, CONTROL_TYPE_2, "dudeAmbush", 0, 0, NULL, NULL, 0 },
    { 62, 6, 41, CONTROL_TYPE_2, "dudeGuard", 0, 0, NULL, NULL, 0 },
    { 62, 7, 42, CONTROL_TYPE_2, "reserved", 0, 0, NULL, NULL, 0 },
    { 62, 9, 43, CONTROL_TYPE_1, "Lock msg: %3d", 0, 255, NULL, NULL, 0 },
    { 62, 10, 44, CONTROL_TYPE_1, "Drop item: %3d", 0, 255, NULL, NULL, 0 },
    { 0, 0, 0, CONTROL_END, NULL, 0, 0, NULL, NULL, 0 },
};

CONTROL controlXWall[] = {
    { 0, 0, 1, CONTROL_TYPE_4, "Type %4d: %-18.18s", 0, 1023, pzWallType, NULL, 0 },
    { 0, 1, 2, CONTROL_TYPE_1, "RX ID: %4d", 0, 1023, NULL, sub_1BE80, 0 },
    { 0, 2, 3, CONTROL_TYPE_1, "TX ID: %4d", 0, 1023, NULL, sub_1BE80, 0 },
    { 0, 3, 4, CONTROL_TYPE_4, "State %1d: %-3.3s", 0, 1, pzOffOn, NULL, 0 },
    { 0, 4, 5, CONTROL_TYPE_4, "Cmd: %3d: %-12.12s", 0, 255, pzCmd, NULL, 0 },
    { 0, 5, 0, CONTROL_TYPE_0, "Send when:", 0, 0, NULL, NULL, 0 },
    { 0, 6, 6, CONTROL_TYPE_2, "going ON:", 0, 0, NULL, NULL, 0 },
    { 0, 7, 7, CONTROL_TYPE_2, "going OFF:", 0, 0, NULL, NULL, 0 },
    { 0, 8, 8, CONTROL_TYPE_1, "busyTime = %4d", 0, 4095, NULL, NULL, 0 },
    { 0, 9, 9, CONTROL_TYPE_1, "waitTime = %4d", 0, 4095, NULL, NULL, 0 },
    { 0, 10, 10, CONTROL_TYPE_4, "restState %1d: %-3.3s", 0, 1, pzOffOn, NULL, 0 },
    { 30, 0, 0, CONTROL_TYPE_0, "Trigger On:", 0, 0, NULL, NULL, 0 },
    { 30, 1, 11, CONTROL_TYPE_2, "Push", 0, 0, NULL, NULL, 0 },
    { 30, 2, 12, CONTROL_TYPE_2, "Vector", 0, 0, NULL, NULL, 0 },
    { 30, 3, 13, CONTROL_TYPE_2, "Reserved", 0, 0, NULL, NULL, 0 },
    { 30, 4, 14, CONTROL_TYPE_2, "DudeLockout", 0, 0, NULL, NULL, 0 },
    { 46, 0, 0, CONTROL_TYPE_0, "Trigger Flags:", 0, 0, NULL, NULL, 0 },
    { 46, 1, 15, CONTROL_TYPE_2, "Decoupled", 0, 0, NULL, NULL, 0 },
    { 46, 2, 16, CONTROL_TYPE_2, "1-shot", 0, 0, NULL, NULL, 0 },
    { 46, 3, 17, CONTROL_TYPE_2, "Locked", 0, 0, NULL, NULL, 0 },
    { 46, 4, 18, CONTROL_TYPE_2, "Interruptable", 0, 0, NULL, NULL, 0 },
    { 46, 6, 19, CONTROL_TYPE_1, "Data: %5d", 0, 65535, NULL, NULL, 0 },
    { 46, 7, 20, CONTROL_TYPE_1, "Key: %1d", 0, 7, NULL, NULL, 0 },
    { 46, 8, 21, CONTROL_TYPE_1, "panX = %4d", -128, 127, NULL, NULL, 0 },
    { 46, 9, 22, CONTROL_TYPE_1, "panY = %4d", -128, 127, NULL, NULL, 0 },
    { 46, 10, 23, CONTROL_TYPE_2, "panAlways", 0, 0, NULL, NULL, 0 },
    { 0, 0, 0, CONTROL_END, NULL, 0, 0, NULL, NULL, 0 },
};

CONTROL controlXSector2[] = {
    { 0, 0, 0, CONTROL_TYPE_0, "Lighting:", 0, 0, NULL, NULL, 0 },
    { 0, 1, 1, CONTROL_TYPE_4, "Wave: %1d %-9.9s", 0, 15, pzLighting, NULL, 0 },
    { 0, 2, 2, CONTROL_TYPE_1, "Amplitude: %+4d", -128, 127, NULL, NULL, 0 },
    { 0, 3, 3, CONTROL_TYPE_1, "Freq:    %3d", 0, 255, NULL, NULL, 0 },
    { 0, 4, 4, CONTROL_TYPE_1, "Phase:     %3d", 0, 255, NULL, NULL, 0 },
    { 0, 5, 5, CONTROL_TYPE_2, "floor", 0, 0, NULL, NULL, 0 },
    { 0, 6, 6, CONTROL_TYPE_2, "ceiling", 0, 0, NULL, NULL, 0 },
    { 0, 7, 7, CONTROL_TYPE_2, "walls", 0, 0, NULL, NULL, 0 },
    { 0, 8, 8, CONTROL_TYPE_2, "shadeAlways", 0, 0, NULL, NULL, 0 },
    { 20, 0, 0, CONTROL_TYPE_0, "More Lighting:", 0, 0, NULL, NULL, 0 },
    { 20, 1, 9, CONTROL_TYPE_2, "Color Lights", 0, 0, NULL, NULL, 0 },
    { 20, 2, 10, CONTROL_TYPE_1, "ceil  pal2 = %3d", 0, 15, NULL, NULL, 0 },
    { 20, 3, 11, CONTROL_TYPE_1, "floor pal2 = %3d", 0, 15, NULL, NULL, 0 },
    { 40, 0, 0, CONTROL_TYPE_0, "Motion FX:", 0, 0, NULL, NULL, 0 },
    { 40, 1, 12, CONTROL_TYPE_1, "Speed = %4d", 0, 255, NULL, NULL, 0 },
    { 40, 2, 13, CONTROL_TYPE_1, "Angle = %4d", 0, 2047, NULL, NULL, 0 },
    { 40, 3, 14, CONTROL_TYPE_2, "pan floor", 0, 0, NULL, NULL, 0 },
    { 40, 4, 15, CONTROL_TYPE_2, "pan ceiling", 0, 0, NULL, NULL, 0 },
    { 40, 5, 16, CONTROL_TYPE_2, "panAlways", 0, 0, NULL, NULL, 0 },
    { 40, 6, 17, CONTROL_TYPE_2, "drag", 0, 0, NULL, NULL, 0 },
    { 40, 8, 18, CONTROL_TYPE_1, "Wind vel: %4d", 0, 1023, NULL, NULL, 0 },
    { 40, 9, 19, CONTROL_TYPE_1, "Wind ang: %4d", 0, 2047, NULL, NULL, 0 },
    { 40, 10, 20, CONTROL_TYPE_2, "Wind always", 0, 0, NULL, NULL, 0 },
    { 60, 0, 0, CONTROL_TYPE_0, "Continuous motion:", 0, 0, NULL, NULL, 0 },
    { 60, 1, 21, CONTROL_TYPE_1, "Z range: %3d", 0, 31, NULL, NULL, 0 },
    { 60, 2, 22, CONTROL_TYPE_1, "Theta: %-4d", 0, 2047, NULL, NULL, 0 },
    { 60, 3, 23, CONTROL_TYPE_1, "Speed: %-4d", -2048, 2047, NULL, NULL, 0 },
    { 60, 4, 24, CONTROL_TYPE_2, "always", 0, 0, NULL, NULL, 0 },
    { 60, 5, 25, CONTROL_TYPE_2, "bob floor", 0, 0, NULL, NULL, 0 },
    { 60, 6, 26, CONTROL_TYPE_2, "bob ceiling", 0, 0, NULL, NULL, 0 },
    { 60, 7, 27, CONTROL_TYPE_2, "rotate", 0, 0, NULL, NULL, 0 },
    { 60, 9, 28, CONTROL_TYPE_1, "DamageType: %1d", 0, 5, NULL, NULL, 0 },
    { 0, 0, 0, CONTROL_END, NULL, 0, 0, NULL, NULL, 0 },
};

CONTROL controlXSector[] = {
    { 0, 0, 1, CONTROL_TYPE_4, "Type %4d: %-16.16s", 0, 1023, pzSectType, NULL, 0 },
    { 0, 1, 2, CONTROL_TYPE_1, "RX ID: %4d", 0, 1023, NULL, sub_1BE80, 0 },
    { 0, 2, 3, CONTROL_TYPE_1, "TX ID: %4d", 0, 1023, NULL, sub_1BE80, 0 },
    { 0, 3, 4, CONTROL_TYPE_4, "State %1d: %-3.3s", 0, 1, pzOffOn, NULL, 0 },
    { 0, 4, 5, CONTROL_TYPE_4, "Cmd: %3d: %-12.12s", 0, 255, pzCmd, NULL, 0 },
    { 0, 5, 0, CONTROL_TYPE_0, "Trigger Flags:", 0, 0, NULL, NULL, 0 },
    { 0, 6, 6, CONTROL_TYPE_2, "Decoupled", 0, 0, NULL, NULL, 0 },
    { 0, 7, 7, CONTROL_TYPE_2, "1-shot", 0, 0, NULL, NULL, 0 },
    { 0, 8, 8, CONTROL_TYPE_2, "Locked", 0, 0, NULL, NULL, 0 },
    { 0, 9, 9, CONTROL_TYPE_2, "Interruptable", 0, 0, NULL, NULL, 0 },
    { 0, 10, 10, CONTROL_TYPE_2, "DudeLockout", 0, 0, NULL, NULL, 0 },
    { 30, 0, 0, CONTROL_TYPE_0, "OFF->ON:", 0, 0, NULL, NULL, 0 },
    { 30, 1, 11, CONTROL_TYPE_2, "send at ON", 0, 0, NULL, NULL, 0 },
    { 30, 2, 12, CONTROL_TYPE_1, "busyTime = %3d", 0, 255, NULL, NULL, 0 },
    { 30, 3, 13, CONTROL_TYPE_4, "wave: %1d %-8.8s", 0, 7, pzWaveForm, NULL, 0 },
    { 30, 4, 14, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 32, 4, 15, CONTROL_TYPE_1, "waitTime = %3d", 0, 255, NULL, NULL, 0 },
    { 30, 6, 0, CONTROL_TYPE_0, "ON->OFF:", 0, 0, NULL, NULL, 0 },
    { 30, 7, 16, CONTROL_TYPE_2, "send at OFF", 0, 0, NULL, NULL, 0 },
    { 30, 8, 17, CONTROL_TYPE_1, "busyTime = %3d", 0, 255, NULL, NULL, 0 },
    { 30, 9, 18, CONTROL_TYPE_4, "wave: %1d %-8.8s", 0, 7, pzWaveForm, NULL, 0 },
    { 30, 10, 19, CONTROL_TYPE_2, "", 0, 0, NULL, NULL, 0 },
    { 32, 10, 20, CONTROL_TYPE_1, "waitTime = %3d", 0, 255, NULL, NULL, 0 },
    { 48, 0, 0, CONTROL_TYPE_0, "Trigger On:", 0, 0, NULL, NULL, 0 },
    { 48, 1, 21, CONTROL_TYPE_2, "Push", 0, 0, NULL, NULL, 0 },
    { 48, 2, 22, CONTROL_TYPE_2, "Vector", 0, 0, NULL, NULL, 0 },
    { 48, 3, 23, CONTROL_TYPE_2, "Reserved", 0, 0, NULL, NULL, 0 },
    { 48, 4, 24, CONTROL_TYPE_2, "Enter", 0, 0, NULL, NULL, 0 },
    { 48, 5, 25, CONTROL_TYPE_2, "Exit", 0, 0, NULL, NULL, 0 },
    { 48, 6, 26, CONTROL_TYPE_2, "WallPush", 0, 0, NULL, NULL, 0 },
    { 60, 0, 27, CONTROL_TYPE_1, "Data: %5d", 0, 65535, NULL, NULL, 0 },
    { 60, 1, 28, CONTROL_TYPE_1, "Key: %1d", 0, 7, NULL, NULL, 0 },
    { 60, 2, 29, CONTROL_TYPE_1, "Depth = %1d", 0, 7, NULL, NULL, 0 },
    { 60, 3, 30, CONTROL_TYPE_2, "Underwater", 0, 0, NULL, NULL, 0 },
    { 60, 4, 31, CONTROL_TYPE_2, "Crush", 0, 0, NULL, NULL, 0 },
    { 60 ,10, 32, CONTROL_TYPE_5, "FX...", 0, 0, NULL, sub_1BFD8, 0 },
    { 0, 0, 0, CONTROL_END, NULL, 0, 0, NULL, NULL, 0 },
};

void PrintText(int x, int y, short c, short bc, const char *pString)
{
    printext16(x*8+4, y*8+28+ydim-STATUS2DSIZ, editorcolors[c], editorcolors[bc], pString, 0);
}

void ClearMidStatBar(void)
{
    clearmidstatbar16();             //Clear middle of status bar

    ydim -= 8;
    drawgradient();
    ydim += 8;
}

void ControlPrint(CONTROL* control, int a2)
{
    int vsi, vdi;
    vsi = 11;
    vdi = 0;
    if (a2)
    {
        vsi = 14;
        vdi = 1;
    }
    // Label?
    if (control->at8 == 0)
    {
        vsi = 15;
        vdi = 2;
    }
    switch (control->type)
    {
    case CONTROL_TYPE_0:
    case CONTROL_TYPE_5:
        PrintText(control->at0, control->at4, vsi, vdi, control->atd);
        break;
    case CONTROL_TYPE_1:
        sprintf(gTempBuf, control->atd, control->at21);
        PrintText(control->at0, control->at4, vsi, vdi, gTempBuf);
        break;
    case CONTROL_TYPE_4:
        dassert(control->names != NULL);
        sprintf(gTempBuf, control->atd, control->at21, control->names[control->at21]);
        PrintText(control->at0, control->at4, vsi, vdi, gTempBuf);
        break;
    case CONTROL_TYPE_2:
        sprintf(gTempBuf, "%c %s", 2 + (control->at21 != 0), control->atd);
        PrintText(control->at0, control->at4, vsi, vdi, gTempBuf);
        break;
    case RADIOBUTTON:
        sprintf(gTempBuf, "%c %s", 4 + (control->at21 != 0), control->atd);
        PrintText(control->at0, control->at4, vsi, vdi, gTempBuf);
        break;
    }
}

char pzControlMsg[256];

void ControlPrintList(CONTROL* control)
{
    ClearMidStatBar();
    _printmessage16(pzControlMsg);
    while (control->type != CONTROL_END)
    {
        ControlPrint(control++, 0);
    }
}

CONTROL *ControlFindTag(CONTROL* control, int tag)
{
    for (; control->type != CONTROL_END && control->at8 != tag; control++) {};

    dassert(control->type != CONTROL_END);

    if (control->type == RADIOBUTTON)
    {
        while (control->at21 == 0)
        {
            control++;
            dassert(control->type != CONTROL_END);
            dassert(control->type == RADIOBUTTON);
        }
    }
    return control;
}

void ControlSet(CONTROL* control, int tag, int value)
{
    for (; control->type != CONTROL_END && control->at8 != tag; control++) {};

    dassert(control->type != CONTROL_END);

    control->at21 = value;
}

int ControlRead(CONTROL* control, int tag)
{
    for (; control->type != CONTROL_END && control->at8 != tag; control++) {};

    dassert(control->type != CONTROL_END);

    return control->at21;
}

void sub_1B988(CONTROL* control, int tag, int value)
{
    for (; control->type != CONTROL_END && control->at8 != tag; control++) {};

    dassert(control->type != CONTROL_END);

    // ???
    while (control->type != CONTROL_END && control->at8 == tag)
    {
        control->at21 = value == 0;
        value--;
    }
    dassert(value < 0);
}

int sub_1B9F4(CONTROL* control, int tag)
{
    int value = 0;
    for (; control->type != CONTROL_END && control->at8 != tag; control++) {};

    dassert(control->type != CONTROL_END);

    // ???
    while (control->type != CONTROL_END)
    {
        dassert(control->type == RADIOBUTTON);
        if (control->at21 == 0)
            break;
        value++;
    }

    dassert(control->type != CONTROL_END);

    return value;
}

int ControlKeys(CONTROL* list)
{
    keyFlushScans();
    ControlPrintList(list);
    int tags = 0;
    int tag = 1;
    for (CONTROL* control = list; control->type != CONTROL_END; control++)
    {
        if (control->at8 > tags)
            tags = control->at8;
    }
    CONTROL* control = ControlFindTag(list, tag);
    while (true)
    {
        ControlPrint(control, 1);
        videoShowFrame(1);
        idle_waitevent();
        if (handleevents())
            quitevent = 0;

        char key;
        while (key = keyGetScan())
        {
            if (control->at1d)
            {
                key = control->at1d(control, key);
                ControlPrintList(list);
            }
            switch (key)
            {
            case sc_Return:
            case sc_kpad_Enter:
                keystatus[key] = 0;
                ModifyBeep();
                return 1;
            case sc_Escape:
                keystatus[key] = 0;
                return 0;
            case sc_LeftArrow:
                tag--;
                if (tag == 0)
                    tag = tags;
                ControlPrint(control, 0);
                control = ControlFindTag(list, tag);
                break;
            case sc_RightArrow:
                tag++;
                if (tag > tags)
                    tag = 1;
                ControlPrint(control, 0);
                control = ControlFindTag(list, tag);
                break;
            case sc_Tab:
                if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
                {
                    tag--;
                    if (tag == 0)
                        tag = tags;
                    ControlPrint(control, 0);
                    control = ControlFindTag(list, tag);
                }
                else
                {
                    tag++;
                    if (tag > tags)
                        tag = 1;
                    ControlPrint(control, 0);
                    control = ControlFindTag(list, tag);
                }
                break;
            default:
                switch (control->type)
                {
                case CONTROL_TYPE_1:
                    switch (key)
                    {
                    case sc_BackSpace:
                        control->at21 /= 10;
                        break;
                    case sc_UpArrow:
                    case sc_kpad_Plus:
                        control->at21++;
                        break;
                    case sc_DownArrow:
                    case sc_kpad_Minus:
                        control->at21--;
                        break;
                    case sc_PgUp:
                        control->at21 = IncBy(control->at21, 10);
                        break;
                    case sc_PgDn:
                        control->at21 = DecBy(control->at21, 10);
                        break;
                    default:
                        key = key < 128 ? g_keyAsciiTable[key] : 0;
                        if (key >= '0' && key <= '9')
                            control->at21 = control->at21 * 10 + key - '0';
                    }
                    if (control->at21 > control->at15)
                        control->at21 = control->at15;

                    if (control->at21 < control->at11)
                        control->at21 = control->at11;

                    break;
                case CONTROL_TYPE_2:
                    switch (key)
                    {
                    case sc_Space:
                        control->at21 = !control->at21;
                        break;
                    }
                    break;
                case RADIOBUTTON:
                    ControlPrint(control, 0);
                    switch (key)
                    {
                    case sc_UpArrow:
                        control->at21 = 0;
                        ControlPrint(control, 0);
                        control--;
                        if (control < list || control->at8 != tag)
                            control--;
                        control->at21 = 1;
                        break;
                    case sc_DownArrow:
                        control->at21 = 0;
                        ControlPrint(control, 0);
                        control++;
                        if (control->at8 != tag)
                            control--;
                        control->at21 = 1;
                        break;
                    }
                    break;
                case CONTROL_TYPE_4:
                {
                    int t = control->at21;
                    switch (key)
                    {
                    case sc_UpArrow:
                    case sc_kpad_Plus:
                        do
                        {
                            t++;
                        } while (t <= control->at15 && !control->names[t]);
                        break;
                    case sc_DownArrow:
                    case sc_kpad_Minus:
                        do
                        {
                            t--;
                        } while (t >= control->at11 && !control->names[t]);
                        break;
                    case sc_PgUp:
                        do
                        {
                            t = IncBy(t, 10);
                        } while (t <= control->at15 && !control->names[t]);
                        break;
                    case sc_PgDn:
                        do
                        {
                            t = DecBy(t, 10);
                        } while (t >= control->at11 && !control->names[t]);
                        break;
                    default:
                        key = key < 128 ? g_keyAsciiTable[key] : 0;
                        if (key >= '0' && key <= '9')
                            control->at21 = control->at21 * 10 + key - '0';
                    }

                    if (t >= control->at11 && t <= control->at15 && control->names[t])
                        control->at21 = t;

                    break;
                }

                }
            }
        }
    }
}

char sub_1BE80(CONTROL* control, char a2) // Next tx/rx id
{
    char t[1024];
    if (a2 == sc_F10)
    {
        memset(t, 0, sizeof(t));
        int i;
        for (i = 0; i < numsectors; i++)
        {
            int nXSector = sector[i].extra;
            if (nXSector <= 0)
                continue;
            t[xsector[nXSector].txID] = 1;
            t[xsector[nXSector].rxID] = 1;
        }
        for (i = 0; i < numwalls; i++)
        {
            int nXWall = wall[i].extra;
            if (nXWall <= 0)
                continue;
            t[xwall[nXWall].txID] = 1;
            t[xwall[nXWall].rxID] = 1;
        }
        for (i = 0; i < kMaxSprites; i++)
        {
            int nXSprite = sprite[i].extra;
            if (nXSprite <= 0)
                continue;
            t[xsprite[nXSprite].txID] = 1;
            t[xsprite[nXSprite].rxID] = 1;
        }
        for (i = 100; i < 1024; i++)
        {
            if (!t[i])
                break;
        }
        control->at21 = 0;
        return 0;
    }
    return a2;
}

char sub_1BFD8(CONTROL* control, char a2)
{
    if (a2 == sc_Return)
    {
        if (ControlKeys(controlXSector2))
            return a2;
        return 0;
    }
    return a2;
}

char sub_1BFF4(CONTROL* control, char a2) // sound
{
    int i;
    DICTNODE* hSnd;
    switch (a2)
    {
    case sc_UpArrow:
        for (i = control->at21+1; i < 65535; i++)
        {
            if (gSoundRes.Lookup(i, "SFX") != NULL)
            {
                control->at21 = i;
                break;
            }
        }
        if (i == 65535)
            Beep();
        return 0;
    case sc_DownArrow:
        for (i = control->at21-1; i > 0; i--)
        {
            if (gSoundRes.Lookup(i, "SFX") != NULL)
            {
                control->at21 = i;
                break;
            }
        }
        return 0;
    case sc_F10:
        hSnd = gSoundRes.Lookup(control->at21, "SFX");
        if (hSnd != NULL)
        {
            SFX* pSfx = (SFX*)gSoundRes.Load(hSnd);
            Bstrcpy(pzControlMsg, pSfx->rawName);
            sndStartSample(control->at21, FXVolume, 0, 0);
        }
        return 0;
    }
    return a2;
}

void XWallControlSet(int nWall)
{
    int nXWall = wall[nWall].extra;
    dassert(nXWall > 0 && nXWall < kMaxXWalls);
    XWALL* pXWall = &xwall[nXWall];
    ControlSet(controlXWall, 1, wall[nWall].lotag); // type
    ControlSet(controlXWall, 2, pXWall->rxID); // rx id
    ControlSet(controlXWall, 3, pXWall->txID); // tx id
    ControlSet(controlXWall, 4, pXWall->state); // state
    ControlSet(controlXWall, 5, pXWall->command); // cmd
    ControlSet(controlXWall, 6, pXWall->triggerOn); // going on
    ControlSet(controlXWall, 7, pXWall->triggerOff); // going off
    ControlSet(controlXWall, 8, pXWall->busyTime); // busyTime
    ControlSet(controlXWall, 9, pXWall->waitTime); // waitTime
    ControlSet(controlXWall, 10, pXWall->restState); // restState
    ControlSet(controlXWall, 11, pXWall->triggerPush); // Push
    ControlSet(controlXWall, 12, pXWall->triggerVector); // Vector
    ControlSet(controlXWall, 13, pXWall->triggerTouch); // Reserved
    ControlSet(controlXWall, 14, pXWall->dudeLockout); // DudeLockout
    ControlSet(controlXWall, 15, pXWall->decoupled); // Decoupled
    ControlSet(controlXWall, 16, pXWall->triggerOnce); // 1-Shot
    ControlSet(controlXWall, 17, pXWall->locked); // Locked
    ControlSet(controlXWall, 18, pXWall->interruptable); // Interruptable
    ControlSet(controlXWall, 19, pXWall->data); // Data
    ControlSet(controlXWall, 20, pXWall->key); // Key
    ControlSet(controlXWall, 21, pXWall->panXVel); // panX
    ControlSet(controlXWall, 22, pXWall->panYVel); // panY
    ControlSet(controlXWall, 23, pXWall->panAlways); // panAlways
}

void XWallControlRead(int nWall)
{
    int nXWall = wall[nWall].extra;
    dassert(nXWall > 0 && nXWall < kMaxXWalls);
    XWALL* pXWall = &xwall[nXWall];
    wall[nWall].lotag = ControlRead(controlXWall, 1); // type
    pXWall->rxID = ControlRead(controlXWall, 2); // rx id
    pXWall->txID = ControlRead(controlXWall, 3); // tx id
    pXWall->state = ControlRead(controlXWall, 4); // state
    pXWall->command = ControlRead(controlXWall, 5); // cmd
    pXWall->triggerOn = ControlRead(controlXWall, 6); // going on
    pXWall->triggerOff = ControlRead(controlXWall, 7); // going off
    pXWall->busyTime = ControlRead(controlXWall, 8); // busyTime
    pXWall->waitTime = ControlRead(controlXWall, 9); // waitTime
    pXWall->restState = ControlRead(controlXWall, 10); // restState
    pXWall->triggerPush = ControlRead(controlXWall, 11); // Push
    pXWall->triggerVector = ControlRead(controlXWall, 12); // Vector
    pXWall->triggerTouch = ControlRead(controlXWall, 13); // Reserved
    pXWall->dudeLockout = ControlRead(controlXWall, 14); // DudeLockout
    pXWall->decoupled = ControlRead(controlXWall, 15); // Decoupled
    pXWall->triggerOnce = ControlRead(controlXWall, 16); // 1-Shot
    pXWall->locked = ControlRead(controlXWall, 17); // Locked
    pXWall->interruptable = ControlRead(controlXWall, 18); // Interruptable
    pXWall->data = ControlRead(controlXWall, 19); // Data
    pXWall->key = ControlRead(controlXWall, 20); // Key
    pXWall->panXVel = ControlRead(controlXWall, 21); // panX
    pXWall->panYVel = ControlRead(controlXWall, 22); // panY
    pXWall->panAlways = ControlRead(controlXWall, 23); // panAlways
}

void XSpriteControlSet(int nSprite)
{
    int nXSprite = sprite[nSprite].extra;
    dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
    XSPRITE* pXSprite = &xsprite[nXSprite];
    ControlSet(controlXSprite, 1, sprite[nSprite].type); // type
    ControlSet(controlXSprite, 2, pXSprite->rxID); // rx id
    ControlSet(controlXSprite, 3, pXSprite->txID); // tx id
    ControlSet(controlXSprite, 4, pXSprite->state); // state
    ControlSet(controlXSprite, 5, pXSprite->command); // cmd
    ControlSet(controlXSprite, 6, pXSprite->triggerOn); // going on
    ControlSet(controlXSprite, 7, pXSprite->triggerOff); // going off
    ControlSet(controlXSprite, 8, pXSprite->busyTime); // busyTime
    ControlSet(controlXSprite, 9, pXSprite->waitTime); // waitTime
    ControlSet(controlXSprite, 10, pXSprite->restState); // restState
    ControlSet(controlXSprite, 11, pXSprite->Push); // Push
    ControlSet(controlXSprite, 12, pXSprite->Vector); // Vector
    ControlSet(controlXSprite, 13, pXSprite->Impact); // Impact
    ControlSet(controlXSprite, 14, pXSprite->Pickup); // Pickup
    ControlSet(controlXSprite, 15, pXSprite->Touch); // Touch
    ControlSet(controlXSprite, 16, pXSprite->Sight); // Sight
    ControlSet(controlXSprite, 17, pXSprite->Proximity); // Proximity
    ControlSet(controlXSprite, 18, pXSprite->DudeLockout); // DudeLockout
    ControlSet(controlXSprite, 19, !((pXSprite->lSkill>>0)&1)); // Launch 1
    ControlSet(controlXSprite, 20, !((pXSprite->lSkill>>1)&1)); // Launch 2
    ControlSet(controlXSprite, 21, !((pXSprite->lSkill>>2)&1)); // Launch 3
    ControlSet(controlXSprite, 22, !((pXSprite->lSkill>>3)&1)); // Launch 4
    ControlSet(controlXSprite, 23, !((pXSprite->lSkill>>4)&1)); // Launch 5
    ControlSet(controlXSprite, 24, !pXSprite->lS); // Launch S
    ControlSet(controlXSprite, 25, !pXSprite->lB); // Launch B
    ControlSet(controlXSprite, 26, !pXSprite->lC); // Launch C
    ControlSet(controlXSprite, 27, !pXSprite->lT); // Launch T
    ControlSet(controlXSprite, 28, pXSprite->Decoupled); // Decoupled
    ControlSet(controlXSprite, 29, pXSprite->triggerOnce); // 1-shot
    ControlSet(controlXSprite, 30, pXSprite->locked); // Locked
    ControlSet(controlXSprite, 31, pXSprite->Interrutable); // Interruptable
    ControlSet(controlXSprite, 32, pXSprite->data1); // Data 1
    ControlSet(controlXSprite, 33, pXSprite->data2); // Data 2
    ControlSet(controlXSprite, 34, pXSprite->data3); // Data 3
    ControlSet(controlXSprite, 35, pXSprite->data4); // Data 4
    ControlSet(controlXSprite, 36, pXSprite->key); // Key
    ControlSet(controlXSprite, 37, pXSprite->wave); // Wave
    ControlSet(controlXSprite, 38, pXSprite->respawn); // Respawn option
    ControlSet(controlXSprite, 39, pXSprite->dudeDeaf); // dudeDeaf
    ControlSet(controlXSprite, 40, pXSprite->dudeAmbush); // dudeAmbush
    ControlSet(controlXSprite, 41, pXSprite->dudeGuard); // dudeGuard
    ControlSet(controlXSprite, 42, pXSprite->dudeFlag4); // reserved
    ControlSet(controlXSprite, 43, pXSprite->lockMsg); // Lock msg
    ControlSet(controlXSprite, 44, pXSprite->dropMsg); // Drop item
}

void XSpriteControlRead(int nSprite)
{
    int nXSprite = sprite[nSprite].extra;
    dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
    XSPRITE* pXSprite = &xsprite[nXSprite];
    sprite[nSprite].type = ControlRead(controlXSprite, 1); // type
    pXSprite->rxID = ControlRead(controlXSprite, 2); // rx id
    pXSprite->txID = ControlRead(controlXSprite, 3); // tx id
    pXSprite->state = ControlRead(controlXSprite, 4); // state
    pXSprite->command = ControlRead(controlXSprite, 5); // cmd
    pXSprite->triggerOn = ControlRead(controlXSprite, 6); // going on
    pXSprite->triggerOff = ControlRead(controlXSprite, 7); // going off
    pXSprite->busyTime = ControlRead(controlXSprite, 8); // busyTime
    pXSprite->waitTime = ControlRead(controlXSprite, 9); // waitTime
    pXSprite->restState = ControlRead(controlXSprite, 10); // restState
    pXSprite->Push = ControlRead(controlXSprite, 11); // Push
    pXSprite->Vector = ControlRead(controlXSprite, 12); // Vector
    pXSprite->Impact = ControlRead(controlXSprite, 13); // Impact
    pXSprite->Pickup = ControlRead(controlXSprite, 14); // Pickup
    pXSprite->Touch = ControlRead(controlXSprite, 15); // Touch
    pXSprite->Sight = ControlRead(controlXSprite, 16); // Sight
    pXSprite->Proximity = ControlRead(controlXSprite, 17); // Proximity
    pXSprite->DudeLockout = ControlRead(controlXSprite, 18); // DudeLockout
    pXSprite->lSkill = (!ControlRead(controlXSprite, 19) << 0) // Launch 12345
                     | (!ControlRead(controlXSprite, 20) << 1)
                     | (!ControlRead(controlXSprite, 21) << 2)
                     | (!ControlRead(controlXSprite, 22) << 3)
                     | (!ControlRead(controlXSprite, 23) << 4);
    pXSprite->lS = !ControlRead(controlXSprite, 24); // Launch S
    pXSprite->lB = !ControlRead(controlXSprite, 25); // Launch B
    pXSprite->lC = !ControlRead(controlXSprite, 26); // Launch C
    pXSprite->lT = !ControlRead(controlXSprite, 27); // Launch T
    pXSprite->Decoupled = ControlRead(controlXSprite, 28); // Decoupled
    pXSprite->triggerOnce = ControlRead(controlXSprite, 29); // 1-shot
    pXSprite->locked = ControlRead(controlXSprite, 30); // Locked
    pXSprite->Interrutable = ControlRead(controlXSprite, 31); // Interruptable
    pXSprite->data1 = ControlRead(controlXSprite, 32); // Data 1
    pXSprite->data2 = ControlRead(controlXSprite, 33); // Data 2
    pXSprite->data3 = ControlRead(controlXSprite, 34); // Data 3
    pXSprite->data4 = ControlRead(controlXSprite, 35); // Data 4
    pXSprite->key = ControlRead(controlXSprite, 36); // Key
    pXSprite->wave = ControlRead(controlXSprite, 37); // Wave
    pXSprite->respawn = ControlRead(controlXSprite, 38); // Respawn option
    pXSprite->dudeDeaf = ControlRead(controlXSprite, 39); // dudeDeaf
    pXSprite->dudeAmbush = ControlRead(controlXSprite, 40); // dudeAmbush
    pXSprite->dudeGuard = ControlRead(controlXSprite, 41); // dudeGuard
    pXSprite->dudeFlag4 = ControlRead(controlXSprite, 42); // reserved
    pXSprite->lockMsg = ControlRead(controlXSprite, 43); // Lock msg
    pXSprite->dropMsg = ControlRead(controlXSprite, 44); // Drop item
}

void XSectorControlSet(int nSector)
{
    int nXSector = sector[nSector].extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR* pXSector = &xsector[nXSector];
    ControlSet(controlXSector, 1, sector[nSector].lotag); // type
    ControlSet(controlXSector, 2, pXSector->rxID); // rx id
    ControlSet(controlXSector, 3, pXSector->txID); // tx id
    ControlSet(controlXSector, 4, pXSector->state); // state
    ControlSet(controlXSector, 5, pXSector->command); // cmd
    ControlSet(controlXSector, 6, pXSector->decoupled); // Decoupled
    ControlSet(controlXSector, 7, pXSector->triggerOnce); // 1-shot
    ControlSet(controlXSector, 8, pXSector->locked); // Locked
    ControlSet(controlXSector, 9, pXSector->interruptable); // Interruptable
    ControlSet(controlXSector, 10, pXSector->dudeLockout); // DudeLockout
    ControlSet(controlXSector, 11, pXSector->triggerOn); // Send at ON
    ControlSet(controlXSector, 12, pXSector->busyTimeA); // OFF->ON busyTime
    ControlSet(controlXSector, 13, pXSector->busyWaveA); // OFF->ON wave
    ControlSet(controlXSector, 14, pXSector->reTriggerA); // OFF->ON wait
    ControlSet(controlXSector, 15, pXSector->waitTimeA); // OFF->ON waitTime
    ControlSet(controlXSector, 16, pXSector->triggerOff); // Send at OFF
    ControlSet(controlXSector, 17, pXSector->busyTimeB); // ON->OFF busyTime
    ControlSet(controlXSector, 18, pXSector->busyWaveB); // ON->OFF wave
    ControlSet(controlXSector, 19, pXSector->reTriggerB); // ON->OFF wait
    ControlSet(controlXSector, 20, pXSector->waitTimeB); // ON->OFF waitTime
    ControlSet(controlXSector, 21, pXSector->Push); // Push
    ControlSet(controlXSector, 22, pXSector->Vector); // Vector
    ControlSet(controlXSector, 23, pXSector->Reserved); // Reserved
    ControlSet(controlXSector, 24, pXSector->Enter); // Enter
    ControlSet(controlXSector, 25, pXSector->Exit); // Exit
    ControlSet(controlXSector, 26, pXSector->Wallpush); // WallPush
    ControlSet(controlXSector, 27, pXSector->data); // Data
    ControlSet(controlXSector, 28, pXSector->Key); // Key
    ControlSet(controlXSector, 29, pXSector->Depth); // Depth
    ControlSet(controlXSector, 30, pXSector->Underwater); // Underwater
    ControlSet(controlXSector, 31, pXSector->Crush); // Crush
    ControlSet(controlXSector2, 1, pXSector->wave); // Lighting wave
    ControlSet(controlXSector2, 2, pXSector->amplitude); // Lighting amplitude
    ControlSet(controlXSector2, 3, pXSector->freq); // Lighting freq
    ControlSet(controlXSector2, 4, pXSector->phase); // Lighting phase
    ControlSet(controlXSector2, 5, pXSector->shadeFloor); // Lighting floor
    ControlSet(controlXSector2, 6, pXSector->shadeCeiling); // Lighting ceiling
    ControlSet(controlXSector2, 7, pXSector->shadeWalls); // Lighting walls
    ControlSet(controlXSector2, 8, pXSector->shadeAlways); // Lighting shadeAlways
    ControlSet(controlXSector2, 9, pXSector->color); // Color Lights
    ControlSet(controlXSector2, 10, pXSector->ceilpal); // Ceil pal2
    ControlSet(controlXSector2, 11, pXSector->floorpal); // floor pal2
    ControlSet(controlXSector2, 12, pXSector->panVel); // Motion speed
    ControlSet(controlXSector2, 13, pXSector->panAngle); // Motion angle
    ControlSet(controlXSector2, 14, pXSector->panFloor); // Pan floor
    ControlSet(controlXSector2, 15, pXSector->panCeiling); // Pan ceiling
    ControlSet(controlXSector2, 16, pXSector->panAlways); // Pan always
    ControlSet(controlXSector2, 17, pXSector->Drag); // Pan drag
    ControlSet(controlXSector2, 18, pXSector->windVel); // Wind vel
    ControlSet(controlXSector2, 19, pXSector->windAng); // Wind ang
    ControlSet(controlXSector2, 20, pXSector->windAlways); // Wind always
    ControlSet(controlXSector2, 21, pXSector->bobZRange); // Motion Z range
    ControlSet(controlXSector2, 22, pXSector->bobTheta); // Motion Theta
    ControlSet(controlXSector2, 23, pXSector->bobSpeed); // Motion speed
    ControlSet(controlXSector2, 24, pXSector->bobAlways); // Motion always
    ControlSet(controlXSector2, 25, pXSector->bobFloor); // Motion bob floor
    ControlSet(controlXSector2, 26, pXSector->bobCeiling); // Motion bob ceiling
    ControlSet(controlXSector2, 27, pXSector->bobRotate); // Motion rotate
    ControlSet(controlXSector2, 28, pXSector->damageType); // DamageType
}

void XSectorControlRead(int nSector)
{
    int nXSector = sector[nSector].extra;
    dassert(nXSector > 0 && nXSector < kMaxXSectors);
    XSECTOR* pXSector = &xsector[nXSector];
    sector[nSector].lotag = ControlRead(controlXSector, 1); // type
    pXSector->txID = ControlRead(controlXSector, 2); // rx id
    pXSector->rxID = ControlRead(controlXSector, 3); // tx id
    pXSector->state = ControlRead(controlXSector, 4); // state
    pXSector->command = ControlRead(controlXSector, 5); // cmd
    pXSector->decoupled = ControlRead(controlXSector, 6); // Decoupled
    pXSector->triggerOnce = ControlRead(controlXSector, 7); // 1-shot
    pXSector->locked = ControlRead(controlXSector, 8); // Locked
    pXSector->interruptable = ControlRead(controlXSector, 9); // Interruptable
    pXSector->dudeLockout = ControlRead(controlXSector, 10); // DudeLockout
    pXSector->triggerOn = ControlRead(controlXSector, 11); // Send at ON
    pXSector->busyTimeA = ControlRead(controlXSector, 12); // OFF->ON busyTime
    pXSector->busyWaveA = ControlRead(controlXSector, 13); // OFF->ON wave
    pXSector->reTriggerA = ControlRead(controlXSector, 14); // OFF->ON wait
    pXSector->waitTimeA = ControlRead(controlXSector, 15); // OFF->ON waitTime
    pXSector->triggerOff = ControlRead(controlXSector, 16); // Send at OFF
    pXSector->busyTimeB = ControlRead(controlXSector, 17); // ON->OFF busyTime
    pXSector->busyWaveB = ControlRead(controlXSector, 18); // ON->OFF wave
    pXSector->reTriggerB = ControlRead(controlXSector, 19); // ON->OFF wait
    pXSector->waitTimeB = ControlRead(controlXSector, 20); // ON->OFF waitTime
    pXSector->Push = ControlRead(controlXSector, 21); // Push
    pXSector->Vector = ControlRead(controlXSector, 22); // Vector
    pXSector->Reserved = ControlRead(controlXSector, 23); // Reserved
    pXSector->Enter = ControlRead(controlXSector, 24); // Enter
    pXSector->Exit = ControlRead(controlXSector, 25); // Exit
    pXSector->Wallpush = ControlRead(controlXSector, 26); // WallPush
    pXSector->data = ControlRead(controlXSector, 27); // Data
    pXSector->Key = ControlRead(controlXSector, 28); // Key
    pXSector->Depth = ControlRead(controlXSector, 29); // Depth
    pXSector->Underwater = ControlRead(controlXSector, 30); // Underwater
    pXSector->Crush = ControlRead(controlXSector, 31); // Crush
    pXSector->wave = ControlRead(controlXSector2, 1); // Lighting wave
    pXSector->amplitude = ControlRead(controlXSector2, 2); // Lighting amplitude
    pXSector->freq = ControlRead(controlXSector2, 3); // Lighting freq
    pXSector->phase = ControlRead(controlXSector2, 4); // Lighting phase
    pXSector->shadeFloor = ControlRead(controlXSector2, 5); // Lighting floor
    pXSector->shadeCeiling = ControlRead(controlXSector2, 6); // Lighting ceiling
    pXSector->shadeWalls = ControlRead(controlXSector2, 7); // Lighting walls
    pXSector->shadeAlways = ControlRead(controlXSector2, 8); // Lighting shadeAlways
    pXSector->color = ControlRead(controlXSector2, 9); // Color Lights
    pXSector->ceilpal = ControlRead(controlXSector2, 10); // Ceil pal2
    pXSector->floorpal = ControlRead(controlXSector2, 11); // floor pal2
    pXSector->bobSpeed = ControlRead(controlXSector2, 12); // Motion speed
    pXSector->panAngle = ControlRead(controlXSector2, 13); // Motion angle
    pXSector->panFloor = ControlRead(controlXSector2, 14); // Pan floor
    pXSector->panCeiling = ControlRead(controlXSector2, 15); // Pan ceiling
    pXSector->panAlways = ControlRead(controlXSector2, 16); // Pan always
    pXSector->Drag = ControlRead(controlXSector2, 17); // Pan drag
    pXSector->windVel = ControlRead(controlXSector2, 18); // Wind vel
    pXSector->windAng = ControlRead(controlXSector2, 19); // Wind ang
    pXSector->windAlways = ControlRead(controlXSector2, 20); // Wind always
    pXSector->bobZRange = ControlRead(controlXSector2, 21); // Motion Z range
    pXSector->bobTheta = ControlRead(controlXSector2, 22); // Motion Theta
    pXSector->bobSpeed = ControlRead(controlXSector2, 23); // Motion speed
    pXSector->bobAlways = ControlRead(controlXSector2, 24); // Motion always
    pXSector->bobFloor = ControlRead(controlXSector2, 25); // Motion bob floor
    pXSector->bobCeiling = ControlRead(controlXSector2, 26); // Motion bob ceiling
    pXSector->bobRotate = ControlRead(controlXSector2, 27); // Motion rotate
    pXSector->damageType = ControlRead(controlXSector2, 28); // DamageType
}

int ShowSectorData(int nSector)
{
    dassert(nSector >= 0 && nSector < kMaxSectors);
    sprintf(pzControlMsg, "^10Sector %d", nSector);
    int nXSector = sector[nSector].extra;
    if (nXSector > 0)
    {
        dassert(nXSector < kMaxXSectors);
        XSectorControlSet(nSector);
        ControlPrintList(controlXSector);
        return 1;
    }
    else
        ClearMidStatBar();
    return 0;
}

void XEditSectorData(int nSector)
{
    dassert(nSector >= 0 && nSector < kMaxSectors);
    cleanUp();
    sprintf(pzControlMsg, "^10Sector %d", nSector);
    sub_10DBC(nSector);
    XSectorControlSet(nSector);
    if (ControlKeys(controlXSector))
        XSectorControlRead(nSector);
    ShowSectorData(nSector);
    //angvel = 0;
    //svel = 0;
    //vel = 0;
    cleanUp();
}

int ShowWallData(int nWall)
{
    dassert(nWall >= 0 && nWall < kMaxWalls);
    int nLen = wallength(nWall);
    sprintf(pzControlMsg, "^10Wall %d:  Length = %d", nWall, nLen);
#ifdef YAX_ENABLE
    if (yax_hasnextwall(nWall))
        return 0;
#endif
    int nXWall = wall[nWall].extra;
    if (nXWall > 0)
    {
        XWallControlSet(nWall);
        ControlPrintList(controlXWall);
        return 1;
    }
    else
        ClearMidStatBar();
    return 0;
}

void XEditWallData(int nWall)
{
    dassert(nWall >= 0 && nWall < kMaxWalls);
    cleanUp();
    sprintf(pzControlMsg, "^10Wall %d", nWall);
#ifdef YAX_ENABLE
    if (yax_hasnextwall(nWall))
        return;
#endif
    sub_10E08(nWall);
    XWallControlSet(nWall);
    if (ControlKeys(controlXWall))
        XWallControlRead(nWall);
    ShowWallData(nWall);
    //angvel = 0;
    //svel = 0;
    //vel = 0;
    cleanUp();
}

int ShowSpriteData(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    if (sprite[nSprite].extra > 0)
    {
        int nXSprite = sprite[nSprite].extra;
        sprintf(gTempBuf, "Sprite %d  Extra %d XRef %d", nSprite, nXSprite, xsprite[nXSprite].reference);
    }
    else
        sprintf(gTempBuf, "Sprite %d", nSprite);
    int nXSprite = sprite[nSprite].extra;
    if (nXSprite > 0)
    {
        dassert(nXSprite < kMaxXSprites);
        XSpriteControlSet(nSprite);
        ControlPrintList(controlXSprite);
        return 1;
    }
    else
        ClearMidStatBar();
    return 0;
}

void XEditSpriteData(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    cleanUp();
    sprintf(pzControlMsg, "Sprite %d", nSprite);
    getXSprite(nSprite);
    XSpriteControlSet(nSprite);
    if (ControlKeys(controlXSprite))
        XSpriteControlRead(nSprite);
    ShowSpriteData(nSprite);
    //angvel = 0;
    //svel = 0;
    //vel = 0;
    cleanUp();
    adjustSprites();
}

static int32_t getTileGroup(const char *groupName)
{
    int32_t temp;
    for (temp = 0; temp < MAX_TILE_GROUPS; temp++)
    {
        if (s_TileGroups[temp].szText == NULL)
            return -1;

        if (!Bstrcmp(s_TileGroups[temp].szText, groupName))
            return temp;
    }
    return -1;
}

static int32_t tileInGroup(int32_t group, int32_t tilenum)
{
    // @todo Make a bitmap instead of doing this slow search..
    int32_t temp;
    if (group < 0 || group >= MAX_TILE_GROUPS || s_TileGroups[group].szText == NULL)
    {
        // group isn't valid.
        return 0;
    }
    for (temp=0; temp<s_TileGroups[group].nIds; temp++)
    {
        if (tilenum == s_TileGroups[group].pIds[temp])
            return 1;
    }
    return 0;
}

const char *ExtGetSectorType(int32_t lotag)
{
    switch (lotag)
    {
    case 1: return "WATER";
    case 2: return "UNDERWATER";
    case 9: return "STAR TREK DOORS";
    case 15: return "ELEVATOR TRANSPORT (SE 17)";
    case 16: return "ELEVATOR PLATFORM DOWN";
    case 17: return "ELEVATOR PLATFORM UP";
    case 18: return "ELEVATOR DOWN";
    case 19: return "ELEVATOR UP";
    case 20: return "CEILING DOOR";
    case 21: return "FLOOR DOOR";
    case 22: return "SPLIT DOOR";
    case 23: return "SWING DOOR (SE 11)";
    case 25: return "SLIDE DOOR (SE 15)";
    case 26: return "SPLIT STAR TREK DOOR";
    case 27: return "BRIDGE (SE 20)";
    case 28: return "DROP FLOOR (SE 21)";
    case 29: return "TEETH DOOR (SE 22)";
    case 30: return "ROTATE RISE BRIDGE";
    case 31: return "2 WAY TRAIN (SE=30)";
    case 32767: return "SECRET AREA";
    case -1: return "END OF LEVEL";
    default:
        if (lotag > 10000 && lotag < 32767)
            return "1 TIME SOUND";
//        else Bsprintf(tempbuf,"%hu",lotag);
    }

    return "";
}

const char *ExtGetSectorCaption(int16_t nSector)
{
    char v100[256];
    static char tempbuf[256];

    dassert(nSector >= 0 && nSector < kMaxSectors);

    Bmemset(tempbuf, 0, sizeof(tempbuf));

    if (!in3dmode() && ((onnames!=1 && onnames!=4 && onnames!=7) || onnames==8))
        return tempbuf;

    int nXSector = sector[nSector].extra;
    if (nXSector > 0)
    {
        int32_t lt = taglab_linkxtags(1, nSector);
        char rxstr[TAGLAB_MAX+16], txstr[TAGLAB_MAX+16];

        lt &= ~(int)(xsector[nXSector].rxID<=0);
        lt &= ~(int)((xsector[nXSector].txID<=0)<<1);

        taglab_handle1(lt&1, xsector[nXSector].rxID, rxstr);
        taglab_handle1(lt&2, xsector[nXSector].txID, txstr);
        if (lt&1)
            Bsprintf(tempbuf, "%s:", rxstr);

        Bstrcat(tempbuf, pzSectType[sector[nSector].lotag]);

        if (lt&2)
        {
            Bsprintf(v100, ":%s", txstr);
            Bstrcat(tempbuf, v100);
        }

        if (xsector[nXSector].panVel)
        {
            Bsprintf(v100, " PAN(%i,%i)", xsector[nXSector].panAngle, xsector[nXSector].panVel);
            Bstrcat(tempbuf, v100);
        }

        Bstrcat(tempbuf, " ");
        Bstrcat(tempbuf, pzOffOn[xsector[nXSector].state]);
    }
    else if (sector[nSector].lotag || sector[nSector].hitag)
    {
        Bsprintf(tempbuf, "{%i:%i}", TrackerCast(sector[nSector].hitag), TrackerCast(sector[nSector].lotag));
    }
    return tempbuf;
}

const char *ExtGetWallCaption(int16_t nWall)
{
    char v100[256];
    static char tempbuf[256];

    dassert(nWall >= 0 && nWall < kMaxWalls);

    Bmemset(tempbuf,0,sizeof(tempbuf));

    if (editwall[nWall>>3]&(1<<(nWall&7)))
    {
        Bsprintf(tempbuf,"%d", wallength(nWall));
        editwall[nWall>>3] &= ~(1<<(nWall&7));
        return tempbuf;
    }

    if (!(onnames==2 || onnames==4))
    {
        tempbuf[0] = 0;
        return tempbuf;
    }

#ifdef YAX_ENABLE__COMPAT
    if (yax_hasnextwall(nWall))
    {
        tempbuf[0] = 0;
        return tempbuf;
    }
#endif

    // HERE

    int nXWall = wall[nWall].extra;
    if (nXWall > 0)
    {
        int32_t lt = taglab_linkxtags(0, nWall);
        char rxstr[TAGLAB_MAX+16], txstr[TAGLAB_MAX+16];

        lt &= ~(int)(xwall[nXWall].rxID<=0);
        lt &= ~(int)((xwall[nXWall].txID<=0)<<1);

        taglab_handle1(lt&1, xwall[nXWall].rxID, rxstr);
        taglab_handle1(lt&2, xwall[nXWall].txID, txstr);
        if (lt&1)
        {
            Bsprintf(v100, "%s:", rxstr);
            Bstrcat(tempbuf, v100);
        }

        Bstrcat(tempbuf, pzWallType[wall[nXWall].lotag]);

        if (lt&2)
        {
            Bsprintf(v100, ":%s", txstr);
            Bstrcat(tempbuf, v100);
        }

        if (xwall[nXWall].panXVel || xwall[nXWall].panYVel)
        {
            Bsprintf(v100, " PAN(%i,%i)", xwall[nXWall].panXVel, xwall[nXWall].panYVel);
            Bstrcat(tempbuf, v100);
        }

        Bstrcat(tempbuf, " ");
        Bstrcat(tempbuf, pzOffOn[xwall[nXWall].state]);
    }
    else if (wall[nWall].lotag || wall[nWall].hitag)
    {
        Bsprintf(tempbuf, "{%i:%i}", TrackerCast(wall[nWall].hitag), TrackerCast(wall[nWall].lotag));
    }

    return tempbuf;
} //end

//const char *SectorEffectorTagText(int32_t lotag)
//{
//    static char tempbuf[64];
//
//    static const char *tags[] =
//    {
//        "ROTATED SECTOR",                // 0
//        "ROTATION PIVOT",
//        "EARTHQUAKE",
//        "RANDOM LIGHTS AFTER SHOT OUT",
//        "RANDOM LIGHTS",
//        "(UNKNOWN)",                     // 5
//        "SUBWAY",
//        "TRANSPORT",
//        "RISING DOOR LIGHTS",
//        "LOWERING DOOR LIGHTS",
//        "DOOR CLOSE DELAY",              // 10
//        "SWING DOOR PIVOT (ST 23)",
//        "LIGHT SWITCH",
//        "EXPLOSIVE",
//        "SUBWAY CAR",
//        "SLIDE DOOR (ST 25)",            // 15
//        "ROTATE REACTOR SECTOR",
//        "ELEVATOR TRANSPORT (ST 15)",
//        "INCREMENTAL SECTOR RISE/FALL",
//        "CEILING FALL ON EXPLOSION",
//        "BRIDGE (ST 27)",                // 20
//        "DROP FLOOR (ST 28)",
//        "TEETH DOOR (ST 29)",
//        "1-WAY TRANSPORT DESTINATION",
//        "CONVEYOR BELT",
//        "ENGINE",                        // 25
//        "(UNKNOWN)",
//        "DEMO CAMERA",
//        "LIGHTNING (4890) CONTROLLER",
//        "FLOAT",
//        "2 WAY TRAIN (ST 31)",           // 30
//        "FLOOR Z",
//        "CEILING Z",
//        "EARTHQUAKE DEBRIS",
//    };
//
//    Bmemset(tempbuf,0,sizeof(tempbuf));
//
//    if (lotag>=0 && lotag<(int32_t)ARRAY_SIZE(tags))
//        Bsprintf(tempbuf, "%s", tags[lotag]);
//    else
//        switch (lotag)
//        {
//        case 36:
//            Bsprintf(tempbuf,"SHOOTER");
//            break;
//        case 49:
//            Bsprintf(tempbuf,"POINT LIGHT");
//            break;
//        case 50:
//            Bsprintf(tempbuf,"SPOTLIGHT");
//            break;
//        default:
//            Bsprintf(tempbuf,"%d: (UNKNOWN)",lotag);
//            break;
//        }
//
//    return tempbuf;
//}

//const char *MusicAndSFXTagText(int32_t lotag)
//{
//    static char tempbuf[16];
//
//    Bmemset(tempbuf, 0, sizeof(tempbuf));
//
//    if (g_numsounds <= 0)
//        return tempbuf;
//
//    if (lotag>0 && lotag<999 && g_sounds[lotag].definedname)
//        return g_sounds[lotag].definedname;
//
//    if (lotag>=1000 && lotag<2000)
//        Bsprintf(tempbuf, "REVERB");
//    return tempbuf;
//}

//const char *SectorEffectorText(int32_t spritenum)
//{
//    static char tempbuf[64];
//
//    Bmemset(tempbuf, 0, sizeof(tempbuf));
//    Bmemset(lo, 0, sizeof(lo));
//
//    Bstrcpy(lo, SectorEffectorTagText(sprite[spritenum].lotag));
//    if (!lo[5]) // tags are 5 chars or less
//        SpriteName(spritenum, tempbuf);
//    else
//    {
//        if (cursprite == spritenum)
//            Bsnprintf(tempbuf, sizeof(tempbuf), "SE %d %s", TrackerCast(sprite[spritenum].lotag), lo);
//        else Bstrcpy(tempbuf, lo);
//    }
//
//    return tempbuf;
//}

const char* ExtGetSpriteCaption(int16_t nSprite)
{
    char caption[256];
    static char tempbuf[1024];

    dassert(nSprite >= 0 && nSprite < kMaxSprites);

    int32_t retfast = 0, lt;

    if (!(onnames >= 3 && onnames <= 6))
        retfast = 1;
    if (onnames == 5 && !(sprite[nSprite].type >= 40 && sprite[nSprite].type < 200))
        retfast = 1;
    if (onnames == 6 && sprite[nSprite].type != sprite[cursprite].type)
        retfast = 1;

    tempbuf[0] = 0;

    if (retfast)
        return tempbuf;

    spritetype * pSprite = &sprite[nSprite];
    const char* pzType = pzSpriteType[pSprite->type];
    
    if (pzType == NULL) {
        Bsprintf(tempbuf, ">>> %s_%d <<<", "UNNAMED_SPRITE_TYPE", pSprite->type);
        return tempbuf;
    }
    else if (pSprite->statnum != 10)
    {
        int nXSprite = pSprite->extra;
        if (nXSprite <= 0 && pSprite->type <= 0) return tempbuf;
        else if (nXSprite > 0)
        {
            XSPRITE* pXSprite = &xsprite[nXSprite];
            switch (pSprite->type)
            {
            case 3:
            case 4:
            case 5:
            case 8:
                return tempbuf;
            case 1:
            case 2:
            case 6:
            case 7:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 18:
            case 19:
                Bsprintf(tempbuf, "%s [%d]", pzType, pXSprite->data1);
                return tempbuf;
            case 15: // Path Marker
                Bsprintf(tempbuf, "%s [%d -> %d]", pzType, pXSprite->data1, pXSprite->data2);
                return tempbuf;
            }
            int32_t lt = taglab_linkxtags(3, nSprite);
            char rxstr[TAGLAB_MAX+16], txstr[TAGLAB_MAX+16];

            lt &= ~(int)(xsprite[nXSprite].rxID<=0);
            lt &= ~(int)((xsprite[nXSprite].txID<=0)<<1);

            taglab_handle1(lt&1, xsprite[nXSprite].rxID, rxstr);
            taglab_handle1(lt&2, xsprite[nXSprite].txID, txstr);
            if (lt&1)
            {
                Bsprintf(caption, "%s:", rxstr);
                Bstrcat(tempbuf, caption);
            }

            Bstrcat(tempbuf, pzType);

            if (lt&2)
            {
                Bsprintf(caption, ":%s", txstr);
                Bstrcat(tempbuf, caption);
            }

            if (pSprite->type >= 20 && pSprite->type < 33)
            {
                Bstrcat(tempbuf, " ");
                Bstrcat(tempbuf, pzOffOn[xsprite[nXSprite].state]);
            }
            return tempbuf;
        }
        return pzType;
    }

    //else if (sprite[nSprite].lotag || sprite[nSprite].hitag)
    //{
    //    Bsprintf(tempbuf, "{%i:%i}", TrackerCast(sprite[nSprite].hitag), TrackerCast(sprite[nSprite].lotag));
    //}
    return tempbuf;

} //end

//printext16 parameters:
//printext16(int32_t xpos, int32_t ypos, int16_t col, int16_t backcol,
//           char name[82], char fontsize)
//  xpos 0-639   (top left)
//  ypos 0-479   (top left)
//  col 0-15
//  backcol 0-15, -1 is transparent background
//  name
//  fontsize 0=8*8, 1=3*5

//drawline16 parameters:
// drawline16(int32_t x1, int32_t y1, int32_t x2, int32_t y2, char col)
//  x1, x2  0-639
//  y1, y2  0-143  (status bar is 144 high, origin is top-left of STATUS BAR)
//  col     0-15

static void PrintStatus(const char *string, int32_t num, int32_t x, int32_t y, int32_t color)
{
    Bsprintf(tempbuf, "%s %d", string, num);
    printext16(x*8, ydim-STATUS2DSIZ+y*8, editorcolors[color], -1, tempbuf, 0);
}

static void PrintNextTag(void)
{
    int32_t obj;
    int32_t nexttag = taglab_getnextfreetag(&obj);

    if (nexttag >= 1)
    {
        if (obj == -1)
            printmessage16("Level %s next tag %d (no tagged objects)", levelname, nexttag);
        else
            printmessage16("Level %s next tag %d (%s %d has greatest)", levelname, nexttag,
                           (obj&32768) ? "sprite" : "wall", obj&32767);
    }
}

void ExtShowSectorData(int16_t sectnum)   //F5
{
    if (in3dmode())
        return;

    if (sectnum >= 0 && !eitherSHIFT)
    {
        if (eitherALT)
        {
            XEditSectorData(sectnum);
            return;
        }
        else
        {
            if (ShowSectorData(sectnum))
                return;
        }
    }
    // NUKE-TODO: rewrite to show Blood stuff
#if 0
    int32_t x,x2,y;
    int32_t i,yi;
    int32_t secrets=0;
    int32_t totalactors1=0,totalactors2=0,totalactors3=0,totalactors4=0;
    int32_t totalrespawn=0;

    UNREFERENCED_PARAMETER(sectnum);
    if (in3dmode())
        return;

    for (i=0; i<numsectors; i++)
        secrets += (sector[i].lotag==32767);

    for (i=headspritestat[0]; i != -1; i=nextspritestat[i])
    {
        // Count all non-player actors.
        if (tileInGroup(tilegroupActors, sprite[i].picnum))
        {
            if (sprite[i].lotag<=1) totalactors1++;
            if (sprite[i].lotag<=2) totalactors2++;
            if (sprite[i].lotag<=3) totalactors3++;
            if (sprite[i].lotag<=4) totalactors4++;
        }

        if (sprite[i].picnum == RESPAWN)
            totalrespawn++;
    }

    Bmemset(numsprite, 0, sizeof(numsprite));
    Bmemset(multisprite, 0, sizeof(numsprite));

    for (i=0; i<MAXSPRITES; i++)
    {
        if (sprite[i].statnum==0 && sprite[i].picnum>=0 && sprite[i].picnum<MAXTILES)
        {
            if (sprite[i].pal!=0)
                multisprite[sprite[i].picnum]++;
            else
                numsprite[sprite[i].picnum]++;
        }
    }

    clearmidstatbar16();             //Clear middle of status bar

    ydim -= 8;
    drawgradient();
    ydim += 8;

    PrintNextTag();

#define PRSTAT(Str, Tiledef) \
    PrintStatus(Str, numsprite[Tiledef], x, y+yi, numsprite[Tiledef]?11:7); \
    PrintStatus("",multisprite[Tiledef], x2,y+yi, multisprite[Tiledef]?9:7); \
    yi++;

    ydim -= 8; // vvvvvv reset at end!!

    videoBeginDrawing();  //{{{

    x=1; x2=14;
    y=4; yi=2;
    printext16(x*8, ydim-STATUS2DSIZ+y*8, editorcolors[11], -1, "Item Count", 0);

    PRSTAT("10%health=", COLA);
    PRSTAT("30%health=", SIXPAK);
    PRSTAT("Med-Kit  =", FIRSTAID);
    PRSTAT("Atom     =", ATOMICHEALTH);
    PRSTAT("Shields  =", SHIELD);

    x=17; x2=30;
    y=4; yi=2;
    printext16(x*8, ydim-STATUS2DSIZ+y*8, editorcolors[11], -1, "Inventory", 0);

    PRSTAT("Steroids =", STEROIDS);
    PRSTAT("Airtank  =", AIRTANK);
    PRSTAT("Jetpack  =", JETPACK);
    PRSTAT("Goggles  =", HEATSENSOR);
    PRSTAT("Boots    =", BOOTS);
    PRSTAT("HoloDuke =", HOLODUKE);
    PRSTAT("Multi D  =", APLAYER);

    x=33; x2=46;
    y=4; yi=2;
    printext16(x*8, ydim-STATUS2DSIZ+y*8, editorcolors[11], -1, "Weapon Count", 0);

    PRSTAT("Pistol   =", FIRSTGUNSPRITE);
    PRSTAT("Shotgun  =", SHOTGUNSPRITE);
    PRSTAT("Chaingun =", CHAINGUNSPRITE);
    PRSTAT("RPG      =", RPGSPRITE);
    PRSTAT("Pipe Bomb=", HEAVYHBOMB);
    PRSTAT("Shrinker =", SHRINKERSPRITE);
    PRSTAT("Devastatr=", DEVISTATORSPRITE);
    PRSTAT("Trip mine=", TRIPBOMBSPRITE);
    PRSTAT("Freezeray=", FREEZESPRITE);

    x=49; x2=62;
    y=4; yi=2;
    printext16(x*8,ydim-STATUS2DSIZ+y*8,editorcolors[11],-1,"Ammo Count",0);

    PRSTAT("Pistol   =", AMMO);
    PRSTAT("Shot     =", SHOTGUNAMMO);
    PRSTAT("Chain    =", BATTERYAMMO);
    PRSTAT("RPG Box  =", RPGAMMO);
    PRSTAT("Pipe Bomb=", HBOMBAMMO);
    PRSTAT("Shrinker =", CRYSTALAMMO);
    PRSTAT("Devastatr=", DEVISTATORAMMO);
    PRSTAT("Expander =", GROWAMMO);
    PRSTAT("Freezeray=", FREEZEAMMO);

    printext16(65*8, ydim-STATUS2DSIZ+4*8, editorcolors[11], -1, "MISC", 0);
    printext16(65*8, ydim-STATUS2DSIZ+8*8, editorcolors[11], -1, "ACTORS", 0);

#undef PRSTAT

    PrintStatus("Secrets =", secrets, 65, 6, 11);
    PrintStatus("Skill 1 =", totalactors1, 65, 10, 11);
    PrintStatus("Skill 2 =", totalactors2, 65, 11, 11);
    PrintStatus("Skill 3 =", totalactors3, 65, 12, 11);
    PrintStatus("Skill 4 =", totalactors4, 65, 13, 11);
    PrintStatus("Respawn =", totalrespawn, 65, 14, 11);

    videoEndDrawing();  //}}}

    ydim += 8; // ^^^^^^ see above!
#endif
}

void ExtShowWallData(int16_t wallnum)       //F6
{
    if (wallnum >= 0 && !eitherSHIFT)
    {
        if (eitherALT)
        {
            XEditWallData(wallnum);
            return;
        }
        else
        {
            if (ShowWallData(wallnum))
                return;
        }
    }
    // NUKE-TODO: Rewrite to show Blood stuff
#if 0
    int32_t i, runi, total=0, x, y, yi;

    UNREFERENCED_PARAMETER(wallnum);

    if (in3dmode())
        return;

    clearmidstatbar16();
    drawgradient();

    PrintNextTag();

#define CASES_LIZTROOP \
    LIZTROOP: case LIZTROOPRUNNING : case LIZTROOPSTAYPUT: case LIZTROOPSHOOT: \
              case LIZTROOPJETPACK: case LIZTROOPONTOILET: case LIZTROOPDUCKING
#define CASES_BOSS1  BOSS1: case BOSS1STAYPUT: case BOSS1SHOOT: case BOSS1LOB: case BOSSTOP

    Bmemset(numsprite, 0, sizeof(numsprite));
    Bmemset(multisprite, 0, sizeof(multisprite));

    for (i=0; i<MAXSPRITES; i++)
    {
        if (sprite[i].statnum==0 && sprite[i].pal)
            switch (sprite[i].picnum)
            {
            case CASES_LIZTROOP:
                numsprite[LIZTROOP]++;
                break;
            case CASES_BOSS1:
                multisprite[BOSS1]++;
                break;
            case BOSS2:
                multisprite[BOSS2]++;
                break;
            case BOSS3:
                multisprite[BOSS3]++;
            default:
                break;
            }
    }

    // runi==0: Count Normal Actors
    // runi==1: Count Respawn Actors
    for (runi=0; runi<2; runi++)
    {
        if (runi==1)
        {
            Bmemset(numsprite, 0, sizeof(numsprite));
            Bmemset(multisprite, 0, sizeof(multisprite));
        }

        for (i=0; i<MAXSPRITES; i++)
        {
            int32_t pic;

            if (sprite[i].statnum!=0)
                continue;

            if (runi==0 && sprite[i].pal!=0)
                continue;

            if (runi==1 && sprite[i].picnum!=RESPAWN)
                continue;

            pic = (runi==0) ? (int)sprite[i].picnum : (int)sprite[i].hitag;
            if (pic<0 || pic>=MAXTILES)
                continue;

            switch (pic)
            {
            case CASES_LIZTROOP:
                numsprite[LIZTROOP]++;
                break;
            case PIGCOP: case PIGCOPSTAYPUT: case PIGCOPDIVE:
                numsprite[PIGCOP]++;
                break;
            case LIZMAN: case LIZMANSTAYPUT: case LIZMANSPITTING: case LIZMANFEEDING: case LIZMANJUMP:
                numsprite[LIZMAN]++;
                break;
            case CASES_BOSS1:
                if (runi==0 || sprite[i].pal==0)
                    numsprite[BOSS1]++;
                else
                    multisprite[BOSS1]++;
                break;
            case COMMANDER:
            case COMMANDERSTAYPUT:
                numsprite[COMMANDER]++;
                break;
            case OCTABRAIN:
            case OCTABRAINSTAYPUT:
                numsprite[OCTABRAIN]++;
                break;
            case RECON: case DRONE: case ROTATEGUN: case EGG: case ORGANTIC: case GREENSLIME:
            case BOSS2: case BOSS3: case TANK: case NEWBEAST: case NEWBEASTSTAYPUT: case BOSS4:
                numsprite[pic]++;
                break;
            default:
                break;
            }
        }

#undef CASES_LIZTROOP
#undef CASES_BOSS1

        total=0;
        for (i=0; i<MAXTILES; i++)
            total += numsprite[i];
        for (i=0; i<MAXTILES; i++)
            total += multisprite[i];

        videoBeginDrawing();  //{{{

        x=2+runi*34;
        y=4;
        PrintStatus(runi==0?"Normal actors:":"Respawn actors:", total, x, y, 11);

#define PRSTAT(Str, Tiledef)  PrintStatus(Str, numsprite[Tiledef], x, y+(yi++), numsprite[Tiledef]?11:7);
        yi=1;

        PRSTAT(" Liztroop  =", LIZTROOP);
        PRSTAT(" Lizman    =", LIZMAN);
        PRSTAT(" Commander =", COMMANDER);
        PRSTAT(" Octabrain =", OCTABRAIN);
        PRSTAT(" PigCop    =", PIGCOP);
        PRSTAT(" Recon Car =", RECON);
        PRSTAT(" Drone     =", DRONE);
        PRSTAT(" Turret    =", ROTATEGUN);
        PRSTAT(" Egg       =", EGG);

        x+=17;
        yi=1;
        PRSTAT("Slimer    =", GREENSLIME);
        PRSTAT("Boss1     =", BOSS1);
        PrintStatus("MiniBoss1 =", multisprite[BOSS1], x, y+(yi++), multisprite[BOSS1]?11:7);
        PRSTAT("Boss2     =", BOSS2);
        PRSTAT("Boss3     =", BOSS3);
        PRSTAT("Riot Tank =", TANK);
        PRSTAT("Newbeast  =", NEWBEAST);
        PRSTAT("Boss4     =", BOSS4);
#undef PRSTAT

        videoEndDrawing();  //}}}
    }
#endif
}

// formerly Show2dText and Show3dText
static void ShowFileText(const char *name)
{
    int32_t fp,t;
    uint8_t x=0,y=4,xmax=0,xx=0,col=0;

    if (!in3dmode())
    {
        clearmidstatbar16();
        drawgradient();
    }

    if ((fp=kopen4load(name,0)) == -1)
    {
        Bsprintf(tempbuf, "ERROR: file \"%s\" not found.", name);
        if (in3dmode())
            printext256(1*4,4*8,whitecol,-1,tempbuf,0);
        else
            printext16(1*4,ydim-STATUS2DSIZ+4*8,editorcolors[11],-1,tempbuf,0);
        return;
    }

    t=65;
    videoBeginDrawing();
    while (t!=EOF && col<5)
    {
        t = 0;
        if (kread(fp,&t,1)<=0)
            t = EOF;
        while (t!=EOF && t!='\n' && x<250)
        {
            tempbuf[x]=t;
            t = 0;
            if (kread(fp,&t,1)<=0) t = EOF;
            x++;
            if (x>xmax) xmax=x;
        }
        tempbuf[x]=0;

        if (in3dmode())
            printext256(xx*4,(y*6)+2,whitecol,-1,tempbuf,1);
        else
            printext16(xx*4,ydim-STATUS2DSIZ+(y*6)+2,editorcolors[11],-1,tempbuf,1);

        x=0;
        y++;
        if (y>18)
        {
            col++;
            y=6;
            xx+=xmax;
            xmax=0;
        }
    }
    videoEndDrawing();

    kclose(fp);

}

// PK_ vvvv
typedef struct helppage_
{
    int32_t numlines;
    char line[1][80];  // C99 flexible array member
} helppage_t;

static helppage_t **helppage=NULL;
static int32_t numhelppages=0;

static int32_t emptyline(const char *start)
{
    int32_t i;
    for (i=0; i<80; i++)
    {
        if (start[i]=='\n' || !start[i]) break;
        if (start[i]!=' ' && start[i]!='\t' && start[i]!='\r')
            return 0;
    }
    return 1;
}

static int32_t newpage(const char *start)
{
    int32_t i;
    for (i=80-1; i>=0; i--)
    {
        if (start[i] == '^' && start[i+1] == 'P')
            return 1;
    }
    return 0;
}

#define IHELP_INITPAGES 32
#define IHELP_INITLINES 16

static void ReadHelpFile(const char *name)
{
    BFILE *fp;
    int32_t i, j, k, numallocpages;
    int32_t pos, charsread=0;
    helppage_t *hp;
    char skip=0;

    LOG_F(INFO, "Loading \"%s\"",name);

    if ((fp=fopenfrompath(name,"rb")) == NULL)
    {
        LOG_F(ERROR, "Error initializing integrated help: file \"%s\" not found.", name);
        return;
    }

    helppage=(helppage_t **)Xmalloc(IHELP_INITPAGES * sizeof(helppage_t *));
    numallocpages=IHELP_INITPAGES;

    i=0;
    while (!Bfeof(fp) && !ferror(fp))
    {
        while (!Bfeof(fp))    // skip empty lines
        {
            pos = ftell(fp);
            if (Bfgets(tempbuf, 80, fp) == NULL) break;
            charsread = ftell(fp)-pos;
            if (!newpage(tempbuf))
            {
                break;
            }
        }

        if (Bfeof(fp) || charsread<=0) break;

        hp=(helppage_t *)Xcalloc(1,sizeof(helppage_t) + IHELP_INITLINES*80);

        hp->numlines = IHELP_INITLINES;

        if (charsread == 79 && tempbuf[78]!='\n') skip=1;
        j=0;

        do
        {
            if (j >= hp->numlines)
            {
                hp=(helppage_t *)Xrealloc(hp, sizeof(helppage_t) + 2*hp->numlines*80);
                hp->numlines *= 2;
            }

            // limit the line length to 78 chars and probably get rid of the CR
            if (charsread>0)
            {
                tempbuf[charsread-1]=0;
                if (charsread>=2 && tempbuf[charsread-2]==0x0d)
                    tempbuf[charsread-2]=0;
            }

            Bmemcpy(hp->line[j], tempbuf, 80);

            for (k=charsread; k<80; k++) hp->line[j][k]=0;

            if (skip)
            {
                while (fgetc(fp)!='\n' && !Bfeof(fp)) /*skip rest of line*/;
                skip=0;
            }

            pos = ftell(fp);
            if (Bfgets(tempbuf, 80, fp) == NULL) break;
            charsread = ftell(fp)-pos;
            if (charsread == 79 && tempbuf[78]!='\n') skip=1;

            j++;

        }
        while (!newpage(tempbuf) && !Bfeof(fp) && charsread>0);

        hp=(helppage_t *)Xrealloc(hp, sizeof(helppage_t) + j*80);
        hp->numlines=j;

        if (i >= numallocpages)
        {
            helppage = (helppage_t **)Xrealloc(helppage, 2*numallocpages*sizeof(helppage_t *));
            numallocpages *= 2;
        }
        helppage[i] = hp;
        i++;
    }

    helppage =(helppage_t **)Xrealloc(helppage, i*sizeof(helppage_t *));
    numhelppages = i;

    Bfclose(fp);
}

// why can't MSVC allocate an array of variable size?!
#define IHELP_NUMDISPLINES 110 // ((overridepm16y>>4)+(overridepm16y>>5)+(overridepm16y>>7)-2)
#define IHELP_PATLEN 45
extern int32_t overridepm16y;  // influences clearmidstatbar16()

static void IntegratedHelp(void)
{
    if (!helppage) return;

    overridepm16y = ydim;//3*STATUS2DSIZ;

    {
        int32_t i, j;
        static int32_t curhp=0, curline=0;
        int32_t highlighthp=-1, highlightline=-1, lasthighlighttime=0;
        char disptext[IHELP_NUMDISPLINES][80];
        char oldpattern[IHELP_PATLEN+1];

        Bmemset(oldpattern, 0, sizeof(char));
        //    clearmidstatbar16();

        videoBeginDrawing();
        CLEARLINES2D(0, ydim, 0);
        videoEndDrawing();

        while (keystatus[KEYSC_ESC]==0 && keystatus[KEYSC_Q]==0 && keystatus[KEYSC_F1]==0)
        {
            idle_waitevent();
            if (handleevents())
                quitevent = 0;

            //        printmessage16("Help mode, press <Esc> to exit");

            if (keystatus[KEYSC_S])
            {
                fade_editor_screen(-1);
            }
            else
            {
                videoBeginDrawing();
                CLEARLINES2D(0, ydim, 0);
                videoEndDrawing();
            }

            // based on 'save as' dialog in overheadeditor()
            if (keystatus[KEYSC_S])    // text search
            {
                char ch, bad=0, pattern[IHELP_PATLEN+1];

                for (i=0; i<IHELP_PATLEN+1; i++) pattern[i]=0;

                i=0;
                keyFlushChars();
                while (bad == 0)
                {
                    _printmessage16("Search: %s_", pattern);
                    videoShowFrame(1);

                    idle_waitevent();

                    if (handleevents())
                        quitevent = 0;

                    ch = keyGetChar();

                    if (keystatus[sc_Escape]) bad = 1;
                    else if (ch == 13) bad = 2;
                    else if (ch > 0)
                    {
                        if (i > 0 && (ch == 8 || ch == 127))
                        {
                            i--;
                            pattern[i] = 0;
                        }
                        else if (i < IHELP_PATLEN && ch >= 32 && ch < 128)
                        {
                            pattern[i++] = ch;
                            pattern[i] = 0;
                        }
                    }
                }

                if (bad==1)
                {
                    keystatus[KEYSC_ESC] = keystatus[KEYSC_Q] = keystatus[KEYSC_F1] = 0;
                }

                if (bad==2)
                {
                    keystatus[KEYSC_ENTER] = 0;

                    for (i=curhp; i<numhelppages; i++)
                    {
                        for (j = (i==curhp)?(curline+1):0; j<helppage[i]->numlines; j++)
                        {
                            // entering an empty pattern will search with the last used pattern
                            if (Bstrstr(helppage[i]->line[j], pattern[0]?pattern:oldpattern))
                            {
                                curhp = i;

                                if ((curline=j) <= helppage[i]->numlines - 32 /*-IHELP_NUMDISPLINES*/) /**/;
                                else if ((curline=helppage[i]->numlines- 32 /*-IHELP_NUMDISPLINES*/) >= 0) /**/;
                                else curline=0;

                                highlighthp = i;
                                highlightline = j;
                                lasthighlighttime = (int)totalclock;
                                goto ENDFOR1;
                            }
                        }
                    }
ENDFOR1:
                    if (pattern[0])
                        Bmemcpy(oldpattern, pattern, IHELP_PATLEN+1);
                }
            }
            else if (PRESSED_KEYSC(T))    // goto table of contents
            {
                curhp=0;
                curline=0;
            }
            else if (PRESSED_KEYSC(G))    // goto arbitrary page
            {
                curhp=getnumber16("Goto page: ", 0, numhelppages-1, 0);
                curline=0;
            }
            else if (PRESSED_KEYSC(UP))    // scroll up
            {
                if (curline>0) curline--;
            }
            else if (PRESSED_KEYSC(DOWN))    // scroll down
            {
                if (curline + 32/*+IHELP_NUMDISPLINES*/ < helppage[curhp]->numlines) curline++;
            }
            else if (PRESSED_KEYSC(PGUP))    // scroll one page up
            {
                i=IHELP_NUMDISPLINES;
                while (i>0 && curline>0) i--, curline--;
            }
            else if (PRESSED_KEYSC(PGDN))    // scroll one page down
            {
                i=IHELP_NUMDISPLINES;
                while (i>0 && curline + 32/*+IHELP_NUMDISPLINES*/ < helppage[curhp]->numlines) i--, curline++;
            }
            else if (PRESSED_KEYSC(SPACE))   // goto next paragraph
            {
                for (i=curline, j=0; i < helppage[curhp]->numlines; i++)
                {
                    if (emptyline(helppage[curhp]->line[i])) { j=1; continue; }
                    if (j==1 && !emptyline(helppage[curhp]->line[i])) { j=2; break; }
                }
                if (j==2)
                {
                    if (i + 32 /*+IHELP_NUMDISPLINES*/ < helppage[curhp]->numlines)
                        curline=i;
                    else if (helppage[curhp]->numlines - 32/*-IHELP_NUMDISPLINES*/ > curline)
                        curline = helppage[curhp]->numlines - 32/*-IHELP_NUMDISPLINES*/;
                }
            }
            else if (PRESSED_KEYSC(BS))   // goto prev paragraph
            {
                for (i=curline-1, j=0; i>=0; i--)
                {
                    if (!emptyline(helppage[curhp]->line[i])) { j=1; continue; }
                    if (j==1 && emptyline(helppage[curhp]->line[i])) { j=2; break; }
                }
                if (j==2 || i==-1) curline=i+1;
            }
            else if (PRESSED_KEYSC(HOME))    // goto beginning of page
            {
                curline=0;
            }
            else if (PRESSED_KEYSC(END))    // goto end of page
            {
                if ((curline=helppage[curhp]->numlines - 32/*-IHELP_NUMDISPLINES*/) >= 0) /**/;
                else curline=0;
            }
            else if (PRESSED_KEYSC(LEFT) || PRESSED_KEYSC(LBRACK))    // prev page
            {
                if (curhp>0)
                {
                    curhp--;
                    curline=0;
                }
            }
            else if (PRESSED_KEYSC(RIGHT) || PRESSED_KEYSC(RBRACK))    // next page
            {
                if (curhp<numhelppages-1)
                {
                    curhp++;
                    curline=0;
                }
            }
            else    // '1'-'0' on the upper row
            {
                for (i=2; i<=11; i++)
                    if (keystatus[i]) break;
                if (i--<12 && i<numhelppages)
                {
                    curhp=i;
                    curline=0;
                }
            }

//            drawgradient();

            videoBeginDrawing();
            printext16(9, ydim2d-overridepm16y+9, editorcolors[4], -1, "Help Mode", 0);
            printext16(8, ydim2d-overridepm16y+8, editorcolors[12], -1, "Help Mode", 0);
            printext16(8 + 9*8 + 2*8, ydim2d-overridepm16y+8, editorcolors[15], -1, "(S:search)", 0);
            videoEndDrawing();

            if (curhp < helppage[0]->numlines)
                _printmessage16("%s", helppage[0]->line[curhp]);
            else
                _printmessage16("%d. (Untitled page)", curhp);

            for (i=0; j=(curhp==0)?(i+curline+1):(i+curline),
                    i<IHELP_NUMDISPLINES && j<helppage[curhp]->numlines; i++)
            {
                if (ydim-overridepm16y+28+i*9+32 >= ydim)
                    break;
                Bmemcpy(disptext[i], helppage[curhp]->line[j], 80);
                printext16(8, ydim-overridepm16y+28+i*9, editorcolors[10],
                           (j==highlightline && curhp==highlighthp
                            && totalclock-lasthighlighttime<120*5) ?
                           editorcolors[1] : -1,
                           disptext[i], 0);
            }

            videoShowFrame(1);
        }

        clearkeys();
    }

    overridepm16y = -1;
}

int32_t AmbienceToggle = 1;
int32_t ParentalLock = 0;
#if 0

#define SOUND_NUMDISPLINES IHELP_NUMDISPLINES

static int32_t compare_sounds_s(int16_t k1, int16_t k2)
{
    return (int32_t)k1 - (int32_t)k2;
}
static int32_t compare_sounds_d(int16_t k1, int16_t k2)
{
    sound_t *s1 = &g_sounds[k1], *s2 = &g_sounds[k2];
    char *n1 = s1->definedname, *n2 = s2->definedname;

    if (!n1 && !n2) return 0;
    if (!n1) return -1;
    if (!n2) return 1;
    return Bstrcasecmp(n1, n2);
}
static int32_t compare_sounds_f(int16_t k1, int16_t k2)
{
    sound_t *s1 = &g_sounds[k1], *s2 = &g_sounds[k2];
    char *n1 = s1->filename, *n2 = s2->filename;

    if (!n1 && !n2) return 0;
    if (!n1) return -1;
    if (!n2) return 1;
    return Bstrcasecmp(n1, n2);
}
static int32_t compare_sounds_1(int16_t k1, int16_t k2)
{
    return (g_sounds[k2].m&1) - (g_sounds[k1].m&1);
}
static int32_t compare_sounds_2(int16_t k1, int16_t k2)
{
    return (g_sounds[k2].m&2) - (g_sounds[k1].m&2);
}
static int32_t compare_sounds_3(int16_t k1, int16_t k2)
{
    return (g_sounds[k2].m&4) - (g_sounds[k1].m&4);
}
static int32_t compare_sounds_4(int16_t k1, int16_t k2)
{
    return (g_sounds[k2].m&8) - (g_sounds[k1].m&8);
}
static int32_t compare_sounds_5(int16_t k1, int16_t k2)
{
    return (g_sounds[k2].m&16) - (g_sounds[k1].m&16);
}


static int32_t sort_sounds(int32_t how)
{
    int32_t (*compare_sounds)(int16_t, int16_t) = NULL;

    int32_t ms, ofs, l, lb, r, rb, d, n, k1, k2;
    int16_t *src, *dst, *source, *dest, *tmp;

    n = g_numsounds;
    src = source = g_sndnum;
    dest = (int16_t *)Xmalloc(sizeof(int16_t) * n);
    dst = dest;

    switch (how)
    {
    case 'g':  // restore original order
        Bmemcpy(g_sndnum, g_definedsndnum, sizeof(int16_t)*n);
        Xfree(dst);
        return 0;
    case 's':
        compare_sounds = compare_sounds_s;
        break;
    case 'd':
        compare_sounds = compare_sounds_d;
        break;
    case 'f':
        compare_sounds = compare_sounds_f;
        break;
    case '1':
        compare_sounds = compare_sounds_1;
        break;
    case '2':
        compare_sounds = compare_sounds_2;
        break;
    case '3':
        compare_sounds = compare_sounds_3;
        break;
    case '4':
        compare_sounds = compare_sounds_4;
        break;
    case '5':
        compare_sounds = compare_sounds_5;
        break;
    default:
        Xfree(dst);
        return -2;
    }

    for (ms=1; ms<n; ms*=2)
    {
        for (ofs=0; ofs<n; ofs+=2*ms)
        {
            l = ofs;
            r = ofs+ms;
            d = ofs;
            lb = min((l+ms), n);
            rb = min((r+ms), n);
            while (l < lb || r < rb)
            {
                if (l >= lb)
                {
                    dst[d++] = src[r++];
                    continue;
                }
                if (r >= rb)
                {
                    dst[d++] = src[l++];
                    continue;
                }
                k1 = src[l];
                k2 = src[r];
                if (compare_sounds(k1, k2) <= 0)
                {
                    dst[d++] = src[l++];
                    continue;
                }
                dst[d++] = src[r++];
            }
        }
        tmp = src;
        src = dst;
        dst = tmp;
    }
    if (src != source)
        Bmemcpy(source, src, sizeof(int16_t) * n);

    Xfree(dest);
    return 0;
}

static void SoundDisplay(void)
{
    if (g_numsounds <= 0) return;

    overridepm16y = ydim;//3*STATUS2DSIZ;

    {
        // cursnd is the first displayed line, cursnd+curofs is where the cursor is
        static int32_t cursnd=0, curofs=0;
        char disptext[80];

        int32_t i, j;
        const int32_t halfpage = (ydim-64)/(2*9);

        while (keystatus[KEYSC_ESC]==0 && keystatus[KEYSC_Q]==0 && keystatus[KEYSC_F2]==0
                && keystatus[buildkeys[BK_MODE2D_3D]]==0)  // quickjump to 3d mode
        {
            videoBeginDrawing();
            CLEARLINES2D(0, ydim16, 0);
            videoEndDrawing();

            idle_waitevent();
            if (handleevents())
                quitevent = 0;

//            drawgradient();

            videoBeginDrawing();
            printext16(9, ydim2d-overridepm16y+9, editorcolors[4], -1, "Sound Index", 0);
            printext16(8, ydim2d-overridepm16y+8, editorcolors[12], -1, "Sound Index", 0);
            printext16(8 + 11*8 + 2*8, ydim2d-overridepm16y+8, editorcolors[15], -1, "(SPACE:play, S:sort)", 0);
            videoEndDrawing();

            if (PRESSED_KEYSC(G))    // goto specified sound#
            {
                _printmessage16("                                                    ");
                j = getnumber16("Goto sound#: ", 0, g_numsounds-1, 0);
                for (i=0; i<g_numsounds; i++)
                    if (g_sndnum[i]==j)
                        break;
                if (i != g_numsounds)
                {
                    if (i<SOUND_NUMDISPLINES)
                        cursnd = 0, curofs = i;
                    else if (i>=g_numsounds-halfpage)
                        cursnd = g_numsounds-halfpage, curofs = i-cursnd;
                    else
                        curofs = halfpage/2, cursnd = i-curofs;
                }
            }
            else if (PRESSED_KEYSC(UP))    // scroll up
            {
                if (curofs>0) curofs--;
                else if (cursnd>0) cursnd--;
            }
            else if (PRESSED_KEYSC(DOWN))    // scroll down
            {
                if (curofs < halfpage-1 && cursnd+curofs<g_numsounds-1)
                    curofs++;
                else if (cursnd + halfpage < g_numsounds)
                    cursnd++;
            }
            else if (PRESSED_KEYSC(PGUP))    // scroll one page up
            {
                i = halfpage/2;

                while (i>0 && curofs>0)
                    i--, curofs--;
                while (i>0 && cursnd>0)
                    i--, cursnd--;
            }
            else if (PRESSED_KEYSC(PGDN))    // scroll one page down
            {
                i = halfpage/2;

                while (i>0 && curofs < halfpage-1 && cursnd+curofs<g_numsounds-1)
                    i--, curofs++;
                while (i>0 && cursnd + halfpage < g_numsounds)
                    i--, cursnd++;
            }
            else if (PRESSED_KEYSC(SPACE) || PRESSED_KEYSC(ENTER))   // play/stop sound
            {
                int32_t j = cursnd+curofs;
                int32_t k = g_sndnum[j];

                if (S_CheckSoundPlaying(0, k) > 0)
                    S_StopSound(k);
                else
                    S_PlaySound(k);
            }
            else if (PRESSED_KEYSC(HOME))    // goto first sound#
            {
                cursnd = curofs = 0;
            }
            else if (PRESSED_KEYSC(END))    // goto last sound#
            {
                if ((cursnd = g_numsounds - halfpage) >= 0)
                    curofs = halfpage-1;
                else
                {
                    cursnd = 0;
                    curofs = g_numsounds-1;
                }
            }

            _printmessage16("                          FILE NAME         PITCH RANGE  PRI FLAGS VOLUME");
            for (i=0; j=cursnd+i, i<SOUND_NUMDISPLINES && j<g_numsounds; i++)
            {
                int32_t l, m, k=g_sndnum[j];
                sound_t *snd=&g_sounds[k];
                char *cp;

                if (ydim-overridepm16y+28+i*9+32 >= ydim) break;

                Bsprintf(disptext,
                         "%4d .................... ................ %6d:%-6d %3d %c%c%c%c%c %6d",
                         //   5678901234567890X23456789012345678901234567
                         k, snd->ps, snd->pe, snd->pr,
                         snd->m&1 ? 'R':'-', snd->m&2 ? 'M':'-', snd->m&4 ? 'D':'-',
                         snd->m&8 ? 'P':'-', snd->m&16 ? 'G':'-', snd->vo);
                for (l = Bsnprintf(disptext+5, 20, "%s", snd->definedname); l<20; l++)
                    disptext[5+l] = ' ';
                if (snd->filename)
                {
                    l = Bstrlen(snd->filename);
                    if (l<=16)
                        cp = snd->filename;
                    else
                        cp = snd->filename + l-15;
                    for (m = Bsnprintf(disptext+26, 16, "%s", cp); m<16; m++)
                        disptext[26+m] = ' ';
                    if (l>16)
                        disptext[26] = disptext[27] = disptext[28] = '.';
                }

                printext16(8, ydim-overridepm16y+28+i*9,
                           keystatus[KEYSC_S]?editorcolors[8] : (S_CheckSoundPlaying(-1, k) ? editorcolors[2] : editorcolors[10]),
                           j==cursnd+curofs ? editorcolors[1] : -1,
                           disptext, 0);
            }

            if (keystatus[KEYSC_S])    // sorting
            {

                char ch, bad=0;

                _printmessage16("Sort by: (S)oundnum (D)ef (F)ile ori(g) or flags (12345)");
                videoShowFrame(1);

                i=0;
                keyFlushChars();
                while (bad == 0)
                {
                    idle_waitevent();
                    if (handleevents())
                        quitevent = 0;

                    ch = keyGetChar();

                    if (keystatus[sc_Escape]) bad = 1;

                    else if (ch == 's' || ch == 'd' || ch == 'f' || ch == 'g' ||
                             ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5')
                    {
                        bad = 2;
                        sort_sounds(ch);
                    }
                }

                clearkeys();
            }
            else
                videoShowFrame(1);
        }

        overridepm16y = -1;

        FX_StopAllSounds();
        S_ClearSoundLocks();

        clearkeys();
    }
}
// PK_ ^^^^

int32_t AmbienceToggle = 1;
int32_t ParentalLock = 0;

uint8_t g_ambiencePlaying[MAXSPRITES>>3];

#define testbit(bitarray, i) (bitarray[(i)>>3] & (1<<((i)&7)))
#define setbit(bitarray, i) bitarray[(i)>>3] |= (1<<((i)&7))
#define clearbit(bitarray, i) bitarray[(i)>>3] &= ~(1<<((i)&7))

// adapted from actors.c
static void M32_MoveFX(void)
{
    int32_t i, j;
    int32_t x, ht;
    spritetype *s;

    for (i=headspritestat[0]; i>=0; i=nextspritestat[i])
    {
        s = &sprite[i];

        if (s->picnum != MUSICANDSFX)
        {
            if (testbit(g_ambiencePlaying, i))
            {
                clearbit(g_ambiencePlaying, i);
                S_StopEnvSound(s->lotag, i);
            }
        }
        else if (s->sectnum>=0)
        {
            ht = s->hitag;

            if (s->lotag < 999 && (unsigned)sector[s->sectnum].lotag < 9 &&
                    AmbienceToggle && sector[s->sectnum].floorz != sector[s->sectnum].ceilingz)
            {
                if ((g_sounds[s->lotag].m & SF_MSFX))
                {
                    x = dist((spritetype *)&pos,s);
                    if (x < ht && !testbit(g_ambiencePlaying, i) && FX_VoiceAvailable(g_sounds[s->lotag].pr-1))
                    {
                        char om = g_sounds[s->lotag].m;
                        if (g_numEnvSoundsPlaying == NumVoices)
                        {
                            for (j = headspritestat[0]; j >= 0; j = nextspritestat[j])
                            {
                                if (s->picnum == MUSICANDSFX && j != i && sprite[j].lotag < 999 &&
                                        testbit(g_ambiencePlaying, j) && dist(&sprite[j],(spritetype *)&pos) > x)
                                {
                                    S_StopEnvSound(sprite[j].lotag,j);
                                    break;
                                }

                            }
                            if (j == -1) continue;
                        }
                        g_sounds[s->lotag].m |= SF_LOOP;
                        A_PlaySound(s->lotag,i);
                        g_sounds[s->lotag].m = om;
                        setbit(g_ambiencePlaying, i);
                    }
                    if (x >= ht && testbit(g_ambiencePlaying, i))
                    {
                        clearbit(g_ambiencePlaying, i);
                        S_StopEnvSound(s->lotag,i);
                    }
                }
            }
        }
    }
}
#endif

///__ShowHelpText__

void ExtShowSpriteData(int16_t spritenum)   //F6
{
    if (spritenum >= 0)
    {
        if (eitherALT)
            XEditSpriteData(spritenum);
        else
            ShowSpriteData(spritenum);
    }
}

void ExtEditSectorData(int16_t sectnum)    //F7
{
    if (in3dmode())
        return;

    if (eitherCTRL)
    {
        gXTracker.TrackSector(sectnum, !eitherALT);
        return;
    }

    if (eitherALT)  //ALT
    {
        keystatus[KEYSC_F7] = 0;
        wallsprite=0;
        curwall = 0;
        curwallnum = 0;
        cursearchspritenum = 0;
        cursectornum=0;
        cursector_lotag = sector[sectnum].lotag;
        cursector_lotag = getnumber16("Enter search sector lotag : ", cursector_lotag, BTAG_MAX,0);
        _printmessage16("Search sector lotag %d",cursector_lotag);
    }
    else EditSectorData(sectnum);
}// end ExtEditSectorData

void ExtEditWallData(int16_t wallnum)       //F8
{
    if (in3dmode())
        return;

    if (eitherCTRL)
    {
        gXTracker.TrackWall(wallnum, !eitherALT);
        return;
    }

    if (eitherALT)  //ALT
    {
        wallsprite=1;
        curwall = wallnum;
        curwallnum = 0;
        cursearchspritenum = 0;
        cursectornum = 0;
        search_lotag = wall[curwall].lotag;
        search_hitag = wall[curwall].hitag;
        search_lotag = getnumber16("Enter wall search lotag : ", search_lotag, BTAG_MAX,0);
        search_hitag = getnumber16("Enter wall search hitag : ", search_hitag, BTAG_MAX,0);
        //    Bsprintf(tempbuf,"Current wall %d lo=%d hi=%d",
        //             curwall,wall[curwall].lotag,wall[curwall].hitag);
        _printmessage16("Search wall lo=%d hi=%d",search_lotag,search_hitag);
    }
    else EditWallData(wallnum);
}

static void GenericSpriteSearch(void);

void ExtEditSpriteData(int16_t spritenum)   //F8
{
    if (in3dmode())
        return;

    if (eitherCTRL)
    {
        gXTracker.TrackSprite(spritenum, !eitherALT);
        return;
    }

    if (eitherALT)  //ALT
        GenericSpriteSearch();
    else EditSpriteData(spritenum);
}

static inline void SpriteName(int16_t spritenum, char *lo2)
{
    Bstrcpy(lo2, names[sprite[spritenum].picnum]);
}// end SpriteName

int32_t AskIfSure(const char *text)
{
    int32_t retval=1;

    if (in3dmode())
    {
        videoBeginDrawing(); //{{{
        printext256(0,0,whitecol,0,text?text:"Are you sure you want to proceed?",0);
        videoEndDrawing();   //}}}
    }
    else
    {
        _printmessage16("%s", text?text:"Are you sure you want to proceed?");
    }

    videoShowFrame(1);

    while ((keystatus[KEYSC_ESC]|keystatus[KEYSC_ENTER]|keystatus[KEYSC_SPACE]|keystatus[KEYSC_N]) == 0)
    {
        idle_waitevent();

        if (handleevents())
        {
            if (quitevent)
            {
                retval = 1;
                break;
            }
        }

        if (PRESSED_KEYSC(Y) || PRESSED_KEYSC(ENTER))
        {
            retval = 0;
            break;
        }
    }

    if (PRESSED_KEYSC(ESC))
        retval = 1;

    return retval;
}

static int32_t IsValidTile(int32_t idTile)
{
    // this is MAXTILES-1 because TROR uses that tile and we don't need it showing up in the tile selector, etc
    return (idTile>=0 && idTile<MAXTILES-1) && (tilesiz[idTile].x && tilesiz[idTile].y) && rottile[idTile].owner == -1;
}

static int32_t SelectAllTiles(int32_t iCurrentTile)
{
    int32_t i;

    if (iCurrentTile < localartlookupnum)
        iCurrentTile = localartlookup[iCurrentTile];
    else
        iCurrentTile = 0;

    localartlookupnum = MAXTILES;

    for (i = 0; i < MAXTILES; i++)
    {
        localartlookup[i] = i;
        localartfreq[i] = 0;
    }

    return iCurrentTile;
}

static int32_t OnGotoTile(int32_t tileNum);
static int32_t OnSelectTile(int32_t tileNum);
static int32_t OnSaveTileGroup();
static int32_t loadtilegroups(const char *fn);
static int32_t s_Zoom = INITIAL_ZOOM;
static int32_t s_TileZoom = 1;
static char tilesel_errmsg[128], tilesel_showerr=0;

static int32_t DrawTiles(int32_t iTopLeft, int32_t iSelected, int32_t nXTiles, int32_t nYTiles,
                         int32_t TileDim, int32_t offset, int32_t showmsg);

#define TMPERRMSG_SHOW(alsoOSD) do { \
    printext256(0, 0, whitecol, 0, tilesel_errmsg, 0); \
    if (alsoOSD) OSD_Printf("%s\n", tilesel_errmsg); \
} while (0)

#define TMPERRMSG_PRINT(Msg, ...) do {  \
    Bsprintf(tilesel_errmsg, Msg, ## __VA_ARGS__); \
    TMPERRMSG_SHOW(1); \
    videoShowFrame(1); \
    tilesel_showerr = 1; \
} while (0)

#define TMPERRMSG_RETURN(Msg, ...) do   \
{ \
    TMPERRMSG_PRINT(Msg, ## __VA_ARGS__);  \
    return 1; \
} while (0)


static inline void pushDisableFog(void)
{
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
    {
        glPushAttrib(GL_ENABLE_BIT);
        polymost_setFogEnabled(false);
    }
#endif
}

static inline void popDisableFog(void)
{
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
    {
        glPopAttrib();
    }
#endif
}

static int32_t m32gettile(int32_t idInitialTile)
{
    int32_t gap, temp, zoomsz;
    int32_t nXTiles, nYTiles, nDisplayedTiles;
    int32_t i;
    int32_t tileNum, iTopLeftTile, iLastTile;
    int32_t idSelectedTile, idInitialPal;
    int32_t scrollmode;
    int32_t mousedx, mousedy, mtile, omousex=searchx, omousey=searchy, moffset=0;

    int32_t noTilesMarked=1;
    int32_t mark_lastk = -1;

    FX_StopAllSounds();
    // NUKE-TODO:
    //S_ClearSoundLocks();

    pushDisableFog();

    // Enable following line for testing. I couldn't work out how to change vidmode on the fly
    // s_Zoom = NUM_ZOOMS - 1;

    idInitialTile = clamp(idInitialTile, 0, MAXTILES-1);

    // Ensure zoom not to big (which can happen if display size
    //   changes whilst Mapster is running)
    do
    {
        nXTiles = xdim / ZoomToThumbSize[s_Zoom];
        nYTiles = ydim / ZoomToThumbSize[s_Zoom];
        // Refuse to draw less than half of a row.
        if (ZoomToThumbSize[s_Zoom]/2 < 12) nYTiles--;
        nDisplayedTiles  = nXTiles * nYTiles;

        if (!nDisplayedTiles)
        {
            // Eh-up, resolution changed since we were last displaying tiles.
            s_Zoom--;
        }
    }
    while (!nDisplayedTiles);

    keystatus[KEYSC_V] = 0;

    for (i = 0; i < MAXTILES; i++)
    {
        localartfreq[i] = 0;
        localartlookup[i] = i;
    }

    iLastTile = tileNum = idSelectedTile = idInitialTile;
    idInitialPal = globalpal;

    switch (searchstat)
    {
    case SEARCH_WALL:
        for (i = 0; i < numwalls; i++)
            localartfreq[ wall[i].picnum ]++;
        break;

    case SEARCH_CEILING:
    case SEARCH_FLOOR:
        for (i = 0; i < numsectors; i++)
        {
            localartfreq[ sector[i].ceilingpicnum ]++;
            localartfreq[ sector[i].floorpicnum ]++;
        }
        break;

    case SEARCH_SPRITE:
        for (i=0; i<MAXSPRITES; i++)
            localartfreq[ sprite[i].picnum ] += (sprite[i].statnum < MAXSTATUS);
        break;

    case SEARCH_MASKWALL:
        for (i = 0; i < numwalls; i++)
            localartfreq[ wall[i].overpicnum ]++;
        break;

    default:
        break;
    }


    //
    //	Sort tiles into frequency order
    //

    gap = MAXTILES / 2;

    do
    {
        for (i = 0; i < MAXTILES-gap; i++)
        {
            temp = i;

            while (temp >= 0 && localartfreq[temp]<localartfreq[temp+gap])
            {
                int32_t tempint;

                tempint = localartfreq[temp];
                localartfreq[temp] = localartfreq[temp+gap];
                localartfreq[temp+gap] = tempint;

                tempint = localartlookup[temp];
                localartlookup[temp] = localartlookup[temp+gap];
                localartlookup[temp+gap] = tempint;

                if (tileNum == temp)
                    tileNum = temp + gap;
                else if (tileNum == temp + gap)
                    tileNum = temp;

                temp -= gap;
            }
        }
        gap >>= 1;
    }
    while (gap > 0);

    //
    // Set up count of number of used tiles
    //

    localartlookupnum = 0;
    while (localartfreq[localartlookupnum] > 0)
        localartlookupnum++;

    //
    // Check : If no tiles used at all then switch to displaying all tiles
    //

    if (!localartfreq[0])
    {
        localartlookupnum = MAXTILES;

        for (i = 0; i < MAXTILES; i++)
        {
            localartlookup[i] = i;
            localartfreq[i] = 0; // Terrible bodge : zero tilefreq's not displayed in tile view. Still, when in Rome ... :-)
        }

        tileNum = idInitialTile;
    }

    //
    //
    //

    iTopLeftTile = tileNum - (tileNum % nXTiles);
    iTopLeftTile = clamp(iTopLeftTile, 0, MAXTILES-nDisplayedTiles);

    zoomsz = ZoomToThumbSize[s_Zoom];

    searchx = ((tileNum-iTopLeftTile)%nXTiles)*zoomsz + zoomsz/2;
    searchy = ((tileNum-iTopLeftTile)/nXTiles)*zoomsz + zoomsz/2;

    ////////////////////////////////
    // Start of key handling code //
    ////////////////////////////////

    while ((keystatus[KEYSC_ENTER]|keystatus[KEYSC_ESC]|(bstatus&1)) == 0)
    {
        int32_t ret;
        zoomsz = ZoomToThumbSize[s_Zoom];

        ret = DrawTiles(iTopLeftTile, (tileNum >= localartlookupnum) ? localartlookupnum-1 : tileNum,
                        nXTiles, nYTiles, zoomsz, moffset,
                        (tilesel_showerr && (tileNum==iLastTile || (tilesel_showerr=0))));

        if (ret==0)
        {
            idle_waitevent_timeout(500);
            // SDL seems to miss mousewheel events when rotated slowly.

            if (handleevents())
                quitevent = 0;
        }
        mouseGetValues(&mousedx,&mousedy,&bstatus);

        iLastTile = tileNum;

        searchx += mousedx;
        searchy += mousedy;

        if (bstatus&2)
        {
            moffset += mousedy*2;
            searchy += mousedy;
            searchx -= mousedx;

            if ((moffset < 0 && iTopLeftTile > localartlookupnum-nDisplayedTiles-1)
                    || (moffset > 0 && iTopLeftTile==0))
            {
                moffset=0;
                searchy -= mousedy*2;
            }

            while (moffset > zoomsz)
            {
                iTopLeftTile -= nXTiles;
                moffset -= zoomsz;
            }
            while (moffset < -zoomsz)
            {
                iTopLeftTile += nXTiles;
                moffset += zoomsz;
            }
        }

        // Keep the pointer visible at all times.
        temp = min(zoomsz/2, 12);
        inpclamp(&searchx, temp, xdim-temp);
        inpclamp(&searchy, temp, ydim-temp);

        scrollmode = !(eitherCTRL^revertCTRL);
        if (bstatus&16 && scrollmode && iTopLeftTile > 0)
        {
            g_mouseBits &= ~16;
            iTopLeftTile -= (nXTiles*scrollamount);
        }
        else if (bstatus&32 && scrollmode && iTopLeftTile < localartlookupnum-nDisplayedTiles-1)
        {
            g_mouseBits &= ~32;
            iTopLeftTile += (nXTiles*scrollamount);
        }

        mtile = tileNum = searchx/zoomsz + ((searchy-moffset)/zoomsz)*nXTiles + iTopLeftTile;
        while (tileNum >= iTopLeftTile + nDisplayedTiles)
        {
            tileNum -= nXTiles;
            mtile = tileNum;
        }

        // These two lines are so obvious I don't need to comment them ...;-)
        synctics = (int)totalclock-lockclock;
        lockclock += synctics;

        // Zoom in / out using numeric key pad's / and * keys
        if (((keystatus[KEYSC_gSLASH] || (!scrollmode && bstatus&16)) && s_Zoom<(signed)(NUM_ZOOMS-1))
                || ((keystatus[KEYSC_gSTAR]  || (!scrollmode && bstatus&32)) && s_Zoom>0))
        {
            if (PRESSED_KEYSC(gSLASH) || (!scrollmode && bstatus&16))
            {
                g_mouseBits &= ~16;
                bstatus &= ~16;

                // Watch out : If editor window is small, then the next zoom level
                //  might get so large that even one tile might not fit !
                if (ZoomToThumbSize[s_Zoom+1]<=xdim && ZoomToThumbSize[s_Zoom+1]<=ydim)
                {
                    // Phew, plenty of room.
                    s_Zoom++;
                }
            }
            else
            {
                keystatus[KEYSC_gSTAR] = 0;
                g_mouseBits &= ~32;
                bstatus &= ~32;
                s_Zoom--;
            }

            zoomsz = ZoomToThumbSize[s_Zoom];

            if (tileNum >= localartlookupnum)
                tileNum = localartlookupnum-1;

            // Calculate new num of tiles to display
            nXTiles = xdim / zoomsz;
            nYTiles = ydim / zoomsz;
            // Refuse to draw less than half of a row.
            if (zoomsz/2 < 12)
                nYTiles--;
            nDisplayedTiles  = nXTiles * nYTiles;

            // Determine if the top-left displayed tile needs to
            //   alter in order to display selected tile
            iTopLeftTile = tileNum - (tileNum % nXTiles);
            iTopLeftTile = clamp(iTopLeftTile, 0, MAXTILES - nDisplayedTiles);

            // scroll window so mouse points the same tile as it was before zooming
            iTopLeftTile -= searchx/zoomsz + ((searchy-moffset)/zoomsz)*nXTiles + iTopLeftTile-tileNum;
        }

        if (PRESSED_KEYSC(LEFT))
        {
            if (eitherCTRL)  // same as HOME, for consistency with CTRL-UP/DOWN
                tileNum = (tileNum/nXTiles)*nXTiles;
            else
                tileNum--;
        }

        if (PRESSED_KEYSC(RIGHT))
        {
            if (eitherCTRL)  // same as END, for consistency with CTRL-UP/DOWN
                tileNum = ((tileNum+nXTiles)/nXTiles)*nXTiles - 1;
            else
                tileNum++;
        }

        if (PRESSED_KEYSC(UP))
        {
            if (eitherCTRL)
                while (tileNum-nXTiles >= iTopLeftTile)
                    tileNum -= nXTiles;
            else
                tileNum -= nXTiles;
        }

        if (PRESSED_KEYSC(DOWN))
        {
            if (eitherCTRL)
                while (tileNum+nXTiles < iTopLeftTile + nDisplayedTiles)
                    tileNum += nXTiles;
            else
                tileNum += nXTiles;
        }

        if (PRESSED_KEYSC(PGUP))
        {
            if (eitherCTRL)
                tileNum = 0;
            else
                tileNum -= nDisplayedTiles;
        }

        if (PRESSED_KEYSC(PGDN))
        {
            if (eitherCTRL)
                tileNum = localartlookupnum-1;
            else
                tileNum += nDisplayedTiles;
        }

        if (PRESSED_KEYSC(HOME))
        {
            if (eitherCTRL)
                tileNum = iTopLeftTile;
            else
                tileNum = (tileNum/nXTiles)*nXTiles;
        }

        if (PRESSED_KEYSC(END))
        {
            if (eitherCTRL)
                tileNum = iTopLeftTile + nDisplayedTiles - 1;
            else
                tileNum = ((tileNum+nXTiles)/nXTiles)*nXTiles - 1;
        }

        if (PRESSED_KEYSC(DASH))
        {
            if (globalpal > 0)
                globalpal--;
            else globalpal = M32_MAXPALOOKUPS;

            while (globalpal > 0 && (!palookup[globalpal] || palookup[globalpal] == palookup[0]))
                globalpal--;
        }

        if (PRESSED_KEYSC(EQUAL))
        {
            if (globalpal < M32_MAXPALOOKUPS)
            {
                globalpal++;

                while (globalpal < M32_MAXPALOOKUPS && (!palookup[globalpal] || palookup[globalpal] == palookup[0]))
                    globalpal++;

                if (globalpal == M32_MAXPALOOKUPS)
                    globalpal = 0;
            }
            else globalpal = 0;
        }

        if (PRESSED_KEYSC(BS))
            globalpal = 0;

        // 'V'  KEYPRESS
        if (PRESSED_KEYSC(V))
            tileNum = SelectAllTiles(tileNum);

        // 'G'  KEYPRESS - Goto frame
        if (PRESSED_KEYSC(G))
        {
            if (eitherCTRL)
            {
                if (OnSaveTileGroup() == 0)
                {
//                    tileNum = SelectAllTiles(tileNum);
                    Bmemset(tilemarked, 0, sizeof(tilemarked));
                    mark_lastk = -1;
                    noTilesMarked = 1;
                }
            }
            else
                tileNum = OnGotoTile(tileNum);
        }

        // change palette
        if (PRESSED_KEYSC(P))
        {
            Bsprintf(tempbuf, "%s pal: ", Typestr[searchstat]);

            int pal;

            do
            {
                pal = getnumber256(tempbuf, globalpal, M32_MAXPALOOKUPS, 0+2+16);
            } while (pal != 0 && (palookup[pal] == palookup[0] || palookup[pal] == NULL));

            globalpal = pal;
        }

        // 'U'  KEYPRESS : go straight to user defined art
        if (PRESSED_KEYSC(U))
        {
            SelectAllTiles(tileNum);
            tileNum = FIRST_USER_ART_TILE;
        }

        // 'A'  KEYPRESS : Go straight to start of Atomic edition's art
        if (PRESSED_KEYSC(A))
        {
            SelectAllTiles(tileNum);
            tileNum = FIRST_ATOMIC_TILE;
        }

        // 'E'  KEYPRESS : Go straight to start of extended art
        if (PRESSED_KEYSC(E))
        {
            SelectAllTiles(tileNum);

            if (tileNum == FIRST_EXTENDED_TILE)
                tileNum = SECOND_EXTENDED_TILE;
            else tileNum = FIRST_EXTENDED_TILE;
        }

        // 'T' KEYPRESS = Select from pre-defined tileset
        if (PRESSED_KEYSC(T))
            tileNum = OnSelectTile(tileNum);

        if (PRESSED_KEYSC(Z))
            s_TileZoom = !s_TileZoom;

        //
        // Ensure tilenum is within valid range
        //
        tileNum = clamp(tileNum, 0, min(MAXTILES-1, localartlookupnum+nDisplayedTiles-1));

        // 'S' KEYPRESS: search for named tile
        if (PRESSED_KEYSC(S))
        {
            static char laststr[25] = "";
            const char *searchstr = getstring_simple("Search for tile name: ", laststr, sizeof(names[0])-1, 1);
            static char buf[2][25];

            if (searchstr && searchstr[0])
            {
                int32_t i, i0, slen=Bstrlen(searchstr)-1;

                Bstrncpyz(laststr, searchstr, 25);
                i0 = localartlookup[tileNum];

                Bmemcpy(buf[0], laststr, 25);
                Bstrupr(buf[0]);

                for (i=(i0+1)%MAXTILES; i!=i0; i=(i+1)%MAXTILES)
                {
                    Bmemcpy(buf[1], names[i], 25);
                    buf[1][24]=0;
                    Bstrupr(buf[1]);

                    if ((searchstr[0]=='^' && !Bstrncmp(buf[1], buf[0]+1, slen)) ||
                        (searchstr[0]!='^' && Bstrstr(buf[1], buf[0])))
                    {
                        SelectAllTiles(i);
                        tileNum = i;
                        break;
                    }
                }
            }

            g_mousePos.x = g_mousePos.y = g_mouseBits = 0;
        }

        //
        //	Adjust top-left to ensure tilenum is within displayed range of tiles
        //

        while (tileNum < iTopLeftTile - (moffset<0)?nXTiles:0)
            iTopLeftTile -= nXTiles;

        while (tileNum >= iTopLeftTile + nDisplayedTiles)
            iTopLeftTile += nXTiles;

        iTopLeftTile = clamp(iTopLeftTile, 0, MAXTILES - nDisplayedTiles);


        // SPACE keypress: mark/unmark selected tile
        if (PRESSED_KEYSC(SPACE))
        {
            if (tileNum < localartlookupnum && IsValidTile(localartlookup[tileNum]))
            {
                if (keystatus[KEYSC_LCTRL] && keystatus[KEYSC_RSHIFT])
                {
                    Bmemset(tilemarked, 0, sizeof(tilemarked));
                    mark_lastk = -1;
                    noTilesMarked = 1;
                }
                else
                {
                    int32_t k=tileNum, kend, dir;

                    if (noTilesMarked)
                    {
                        noTilesMarked = 0;
                        TMPERRMSG_PRINT("Beginning marking tiles. To group, press Ctrl-G."
                                        " To reset, press LCtrl-RShift-SPACE.");
                    }

                    if (mark_lastk>=0 && eitherCTRL)
                    {
                        kend = mark_lastk;
                        dir = ksgn(mark_lastk-k);
                    }
                    else
                    {
                        kend = k;
                        dir = 0;
                    }

                    mark_lastk = k;

                    for (; dir==0 || dir*(kend-k)>=1; k+=dir)
                    {
                        tilemarked[localartlookup[k]>>3] ^= (1<<(localartlookup[k]&7));
                        if (dir==0)
                            break;
                    }
                }
            }
        }

        if (mtile!=tileNum) // if changed by keyboard, update mouse cursor
        {
            searchx = ((tileNum-iTopLeftTile)%nXTiles) * zoomsz + zoomsz/2;
            searchy = ((tileNum-iTopLeftTile)/nXTiles) * zoomsz + zoomsz/2 + moffset;
        }
    }

    if ((keystatus[KEYSC_ENTER] || (bstatus&1)) == 0)
    {
        idSelectedTile = idInitialTile;
        globalpal      = idInitialPal;
    }
    else
    {
        if (tileNum < localartlookupnum)
        {
            // Convert tile num from index to actual tile num
            idSelectedTile = localartlookup[tileNum];

            // Check : if invalid tile selected, return original tile num
            if (!IsValidTile(idSelectedTile))
            {
                idSelectedTile = idInitialTile;
                globalpal      = idInitialPal;
            }
        }
        else
        {
            idSelectedTile = idInitialTile;
            globalpal      = idInitialPal;
        }
    }

    searchx=omousex;
    searchy=omousey;

    keystatus[KEYSC_ESC] = 0;
    keystatus[KEYSC_ENTER] = 0;

    popDisableFog();

    return idSelectedTile;
}

// Dir = 0 (zoom out) or 1 (zoom in)
//void OnZoomInOut( int32_t *pZoom, int32_t Dir /*0*/ )
//{
//}

static int32_t OnSaveTileGroup(void)
{
    int32_t i, n=0;
    char hotkey;
    const char *cp, *name;

    if (tile_groups==MAX_TILE_GROUPS)
        TMPERRMSG_RETURN("Cannot save tile group: maximum number of groups (%d) exceeded.", MAX_TILE_GROUPS);

    for (i=0; i<MAXTILES; i++)
        n += !!(tilemarked[i>>3]&(1<<(i&7)));

    if (n==0)
        TMPERRMSG_RETURN("Cannot save tile group: no tiles marked.");
    else if (n > MAX_TILE_GROUP_ENTRIES)
        TMPERRMSG_RETURN("Cannot save tile group: too many tiles in group. Have %d, max is %d.",
                  n, MAX_TILE_GROUP_ENTRIES);

    cp = getstring_simple("Hotkey for new group: ", "", 1, 0);
    if (!cp || !*cp)
        return 1;

    hotkey = Btoupper(cp[0]);
    if (!isalpha(hotkey))
        TMPERRMSG_RETURN("Hotkey must be alphabetic.");

    for (i=0; i<tile_groups; i++)
        if (s_TileGroups[i].key1==hotkey || s_TileGroups[i].key2==Btolower(hotkey))
            TMPERRMSG_RETURN("Hotkey '%c' already in use by tile group `%s'.", hotkey, s_TileGroups[i].szText);

    name = getstring_simple("Name for new tile group: ", "", 63, 0);
    if (!name || !*name)
        return 1;

    for (i=0; name[i]; i++)
        if (!(isalnum(name[i]) || name[i]==' ' || name[i]==','))
            TMPERRMSG_RETURN("Name may only consist of alphabetic, numeric and space characters.");

    {
        int32_t lasti=-1, col=0, j, k, opathsearchmode=pathsearchmode;
        BFILE *fp;

        pathsearchmode = pathsearchmode_oninit;  // use the same pathsearchmode as on init
        fp = fopenfrompath(default_tiles_cfg, "a");
        pathsearchmode = opathsearchmode;
        if (!fp)
            TMPERRMSG_RETURN("Could not open `%s' for writing: %s.", default_tiles_cfg, strerror(errno));
        if (fseek(fp, 0, BSEEK_END))  // seems to be necessary even though we fopen with "a"
            TMPERRMSG_RETURN("Could not seek to end of file `%s'.", default_tiles_cfg);

#define TTAB "\t"
#define TBITCHK(i) ((i)<MAXTILES && (tilemarked[(i)>>3]&(1<<((i)&7))))
        Bfprintf(fp, OURNEWL);
        Bfprintf(fp, "tilegroup \"%s\"" OURNEWL"{" OURNEWL, name);
        Bfprintf(fp, TTAB "hotkey \"%c\"" OURNEWL OURNEWL, hotkey);

        s_TileGroups[tile_groups].pIds = (int32_t *)Xmalloc(n * sizeof(s_TileGroups[tile_groups].pIds[0]));

        j = 0;
        // tileranges for consecutive runs of 3 or more tiles
        for (i=0; i<MAXTILES; i++)
        {
            if (lasti>=0 && !TBITCHK(i))
            {
                if (names[lasti][0] && names[i-1][0])
                    Bfprintf(fp, TTAB "tilerange %s %s" OURNEWL, names[lasti], names[i-1]);
                else
                    Bfprintf(fp, TTAB "tilerange %d %d" OURNEWL, lasti, i-1);

                for (k=lasti; k<i; k++)
                {
                    s_TileGroups[tile_groups].pIds[j++] = k;
                    tilemarked[k>>3] &= ~(1<<(k&7));
                }

                lasti = -1;
            }
            else if (lasti==-1 && TBITCHK(i))
            {
                if (TBITCHK(i+1) && TBITCHK(i+2))
                {
                    lasti = i;
                    i += 2;
                }
            }
        }
        if (lasti>=0 && lasti<=MAXTILES-3)
        {
            for (k=lasti; k<MAXTILES; k++)
            {
                s_TileGroups[tile_groups].pIds[j++] = k;
                tilemarked[k>>3] &= ~(1<<(k&7));
            }
            Bfprintf(fp, TTAB "tilerange %d %d" OURNEWL, lasti, MAXTILES-1);
        }
        Bfprintf(fp, OURNEWL);

        k = 0;
        for (i=0; i<MAXTILES; i++)
            if (tilemarked[i>>3]&(1<<(i&7)))
            {
                k = 1;
                break;
            }

        if (k)
        {
            // throw them all in a tiles{...} group else
            Bfprintf(fp, TTAB "tiles\n" TTAB "{" OURNEWL);
            for (i=0; i<MAXTILES; i++)
            {
                if (TBITCHK(i))
                {
                    s_TileGroups[tile_groups].pIds[j++] = i;

                    if (col==0)
                        Bfprintf(fp, TTAB TTAB), col+=8;

                    if (names[i][0])
                        col+=Bfprintf(fp, "%s ", names[i]);
                    else
                        col+=Bfprintf(fp, "%d ", i);

                    if (col>80)
                    {
                        Bfprintf(fp, OURNEWL);
                        col = 0;
                    }
                }
            }
            if (col>0)
                Bfprintf(fp, OURNEWL);
            Bfprintf(fp, TTAB "}" OURNEWL);
        }
#undef TBITCHK
#undef TTAB
        Bfprintf(fp, "}" OURNEWL);

        Bfclose(fp);

        s_TileGroups[tile_groups].szText = Xstrdup(name);

        s_TileGroups[tile_groups].nIds = n;
        s_TileGroups[tile_groups].key1 = Btoupper(hotkey);
        s_TileGroups[tile_groups].key2 = Btolower(hotkey);
        s_TileGroups[tile_groups].color1 = s_TileGroups[tile_groups].color2 = 0;
        tile_groups++;

        TMPERRMSG_PRINT("Wrote and installed new tile group.");
    }

    return 0;
}


static int32_t OnGotoTile(int32_t tileNum)
{
    //Automatically press 'V'
    tileNum = SelectAllTiles(tileNum);

    return getnumber256("Goto tile: ", 0, MAXTILES-1, 0+2+16);
}


static int32_t LoadTileSet(const int32_t idCurrentTile, const int32_t *pIds, const int32_t nIds)
{
    int32_t iNewTile = 0;
    int32_t i;

    localartlookupnum = nIds;

    for (i = 0; i < localartlookupnum; i++)
    {
        localartlookup[i] = pIds[i];
        // REM : Could we still utilise localartfreq[] to mark
        //  which tiles are currently used in the map ? Set to 0xFFFF perhaps ?
        localartfreq[i] = 0;

        if (idCurrentTile == pIds[i])
            iNewTile = i;
    }

    return iNewTile;
}

static int32_t OnSelectTile(int32_t tileNum)
{
    int32_t bDone = 0;
    int32_t i;
    char ch;

    if (tile_groups <= 0)
    {
        TMPERRMSG_PRINT("No tile groups loaded. Check for existence of `%s'.", default_tiles_cfg);
        return tileNum;
    }

    SelectAllTiles(tileNum);

    keyFlushChars();

    polymostSet2dView();

    videoClearViewableArea(-1);

    //
    //	Await appropriate selection keypress.
    //

    bDone = 0;

    while (keystatus[KEYSC_ESC] == 0 && !bDone)
    {
        if (handleevents())
            quitevent = 0;

        idle_waitevent();

        //
        // Display the description strings for each available tile group
        //
        int32_t j = 0;

        for (i = 0; i < tile_groups; i++)
        {
            if (s_TileGroups[i].szText != NULL)
            {
                if ((j+2)*16 > ydimgame) break;

                if (s_TileGroups[i].key1)
                {
                    Bsprintf(tempbuf, "(%c) %s", s_TileGroups[i].key1, s_TileGroups[i].szText);
                    printext256(10L, (j+1)*16, whitecol, -1, tempbuf, 0);
                    j++;
                }
            }
        }
        videoShowFrame(1);

        ch = keyGetChar();

        for (i = 0; i < tile_groups; i++)
        {
            if (s_TileGroups[i].pIds != NULL && s_TileGroups[i].key1)
                if (ch == s_TileGroups[i].key1 || ch == s_TileGroups[i].key2)
                {
                    tileNum = LoadTileSet(tileNum, s_TileGroups[i].pIds, s_TileGroups[i].nIds);
                    bDone = 1;
                }
        }
    }

    videoShowFrame(1);

    clearkeys();

    return tileNum;
}

static const char *GetTilePixels(int32_t idTile)
{
    char *pPixelData = 0;

    if (idTile >= 0 && idTile < MAXTILES)
    {
        if (!waloff[idTile])
            tileLoad(idTile);

        if (IsValidTile(idTile))
            pPixelData = (char *)waloff[idTile];
    }

    return pPixelData;
}

static void classic_drawtilescreen(int32_t x, int32_t y, int32_t idTile, int32_t TileDim,
                                   const char *pRawPixels)
{
    int32_t dispxsz = tilesiz[idTile].x, dispysz = tilesiz[idTile].y;
    int32_t divinc = 1, mulinc = 1;

    int32_t xofs, yofs;
    char *pScreen;

    while ((dispxsz/divinc > TileDim) || (dispysz/divinc) > TileDim)
    {
        divinc++;
    }

    if (divinc == 1 && s_TileZoom)
    {
        while ((dispxsz*(mulinc+1)) <= TileDim && (dispysz*(mulinc+1)) <= TileDim)
        {
            mulinc++;
        }
    }

    dispxsz = (dispxsz / divinc) * mulinc;
    dispysz = (dispysz / divinc) * mulinc;

    for (yofs = 0; yofs < dispysz; yofs++)
    {
        y += yofs;
        if (y>=0 && y<ydim)
        {
            pScreen = (char *)ylookup[y]+x+frameplace;
            for (xofs = 0; xofs < dispxsz; xofs++)
            {
                pScreen[xofs] = palookup[globalpal][pRawPixels[((yofs*divinc)/mulinc) + (((xofs*divinc)/mulinc)*tilesiz[idTile].y)]];
            }
        }
        y -= yofs;
    }
}

static void tilescreen_drawbox(int32_t iTopLeft, int32_t iSelected, int32_t nXTiles,
                               int32_t TileDim, int32_t offset,
                               int32_t tileNum, int32_t idTile)
{
    int32_t marked = (IsValidTile(idTile) && tilemarked[idTile>>3]&(1<<(idTile&7)));

    //
    // Draw white box around currently selected tile or marked tile
    // p1=(x1, y1), p2=(x1+TileDim-1, y1+TileDim-1)
    //
    if (tileNum == iSelected || marked)
    {
        int32_t x1 = ((tileNum-iTopLeft) % nXTiles)*TileDim;
        int32_t y1 = ((tileNum - ((tileNum-iTopLeft) % nXTiles) - iTopLeft)/nXTiles)*TileDim + offset;
        int32_t x2 = x1+TileDim-1;
        int32_t y2 = y1+TileDim-1, oydim16=ydim16;

        char markedcol = editorcolors[14];

        polymostSet2dView();

        y1=max(y1, 0);
        y2=min(y2, ydim-1);

        // plotlines2d uses drawline16, which clips against ydim16...
        ydim16 = ydim;

        {
            // box
            int32_t xx[] = {x1, x1, x2, x2, x1};
            int32_t yy[] = {y1, y2, y2, y1, y1};
            plotlines2d(xx, yy, 5, tileNum==iSelected ? whitecol : markedcol);
        }

        // cross
        if (marked)
        {
            int32_t xx[] = {x1, x2};
            int32_t yy[] = {y1, y2};

            plotlines2d(xx, yy, 2, markedcol);
            swaplong(&yy[0], &yy[1]);
            plotlines2d(xx, yy, 2, markedcol);
        }

        ydim16 = oydim16;
    }
}

static void tilescreen_drawrest(int32_t iSelected, int32_t showmsg)
{
    if (iSelected>=0 && iSelected<MAXTILES)
    {
        int32_t idTile = localartlookup[iSelected];
        int32_t i;
        char szT[128];

        // Draw info bar at bottom.

        // Clear out behind the text for improved visibility.
        //drawline256(0, (ydim-12)<<12, xdim<<12, (ydim-12)<<12, whitecol);
        for (i=ydim-12; i<ydim; i++)
            renderDrawLine(0, i<<12, xdim<<12, i<<12, (ydim-i));

        // Tile number on left.
        Bsprintf(szT, "Tile: %d (p)al: %d" , idTile, globalpal);
        printext256(1, ydim-10, whitecol, -1, szT, 0);

        // Tile name on right.
        printext256(xdim-(Bstrlen(names[idTile])<<3)-1,ydim-10,whitecol,-1,names[idTile],0);

        // Tile dimensions.
        Bsprintf(szT,"dim: %dx%d",tilesiz[idTile].x,tilesiz[idTile].y);
        printext256((xdim>>2)+(4<<3),ydim-10,whitecol,-1,szT,0);

        // EditArt offset flags.
        Bsprintf(szT,"offs: %d, %d", picanm[idTile].xofs, picanm[idTile].yofs);
        printext256((xdim>>2)+(4<<3)+100,ydim-10,whitecol,-1,szT,0);

        // EditArt animation flags.
        if (picanm[idTile].sf&PICANM_ANIMTYPE_MASK)
        {
            static const char *anmtype[] = {"", "Osc", "Fwd", "Bck"};
            int32_t ii = (picanm[idTile].sf&PICANM_ANIMTYPE_MASK)>>PICANM_ANIMTYPE_SHIFT;

            Bsprintf(szT,"%s %d", anmtype[ii], picanm[idTile].num);
            printext256((xdim>>2)+(4<<3)+100+14*8,ydim-10,whitecol,-1,szT,0);
        }
    }

    if (showmsg)
        TMPERRMSG_SHOW(0);

    m32_showmouse();
}

////////// main tile drawing function //////////
static int32_t DrawTiles(int32_t iTopLeft, int32_t iSelected, int32_t nXTiles, int32_t nYTiles,
                         int32_t TileDim, int32_t offset, int32_t showmsg)
{
    int32_t XTile, YTile;
    int32_t tileNum, idTile;
    int32_t i, x, y;
    const char *pRawPixels;
    char szT[128];
#ifdef USE_OPENGL
    int32_t lazyselector = g_lazy_tileselector && usehightile;
    int32_t usehitile;
#else
    int32_t lazyselector = g_lazy_tileselector;
#endif
    int32_t runi=0;
    static uint8_t loadedhitile[bitmap_size(MAXTILES)];

#ifdef USE_OPENGL
    polymostSet2dView();

    if (videoGetRenderMode() >= REND_POLYMOST)
    {
        if (lazyselector)
            glDrawBuffer(GL_FRONT_AND_BACK);
    }
#endif
    videoClearViewableArea(-1);

    videoBeginDrawing();

restart:
    for (YTile = 0-(offset>0); YTile < nYTiles+(offset<0)+1; YTile++)
    {
        for (XTile = 0; XTile < nXTiles; XTile++)
        {
            tileNum = iTopLeft + XTile + (YTile * nXTiles);

            if (tileNum < 0 || tileNum >= localartlookupnum)
                continue;
#ifdef USE_OPENGL
            usehitile = (runi || !lazyselector);
#endif

            idTile = localartlookup[ tileNum ];
            if (loadedhitile[idTile>>3]&(1<<(idTile&7)))
            {
                if (runi==1)
                    continue;

#ifdef USE_OPENGL
                usehitile = 1;
#endif
            }

            // Get pointer to tile's raw pixel data
            pRawPixels = GetTilePixels(idTile);

            // don't draw rotated tiles generated near MAXTILES
            if (EDUKE32_PREDICT_FALSE(rottile[idTile].owner != -1))
                pRawPixels = NULL;

            if (pRawPixels != NULL)
            {
                x = XTile * TileDim;
                y = YTile * TileDim+offset;

#ifdef USE_OPENGL
                if (polymost_drawtilescreen(x, y, idTile, TileDim, s_TileZoom,
                                            usehitile, loadedhitile))
#endif
                    classic_drawtilescreen(x, y, idTile, TileDim, pRawPixels);

                if (localartfreq[tileNum] != 0 && y >= 0 && y <= ydim-20)
                {
                    Bsprintf(szT, "%d", localartfreq[tileNum]);
                    printext256(x, y, whitecol, -1, szT, 1);
                }
            }

            tilescreen_drawbox(iTopLeft, iSelected, nXTiles, TileDim, offset, tileNum, idTile);

            if (runi==1 && lazyselector)
            {
                int32_t k;

                if (handleevents())
                    quitevent=0;

                tilescreen_drawrest(iSelected, showmsg);

                k = (g_mousePos.x || g_mousePos.y || g_mouseBits);
                if (!k)
                    for (i=0; i<(signed)ARRAY_SIZE(keystatus); i++)
                        if (keystatus[i])
                        {
                            k = 1;
                            break;
                        }
                if (k)
                {
                    videoEndDrawing();
                    videoShowFrame(1);
#ifdef USE_OPENGL
                    if (videoGetRenderMode() >= REND_POLYMOST && lazyselector)
                        glDrawBuffer(GL_BACK);
#endif
                    return 1;
                }

                videoEndDrawing();
                videoShowFrame(1);
                videoBeginDrawing();
            }
        }
    }

    tilescreen_drawrest(iSelected, showmsg);

    if (videoGetRenderMode() >= REND_POLYMOST && in3dmode() && lazyselector)
    {
        if (runi==0)
        {
            videoEndDrawing();
            videoShowFrame(1);
            videoBeginDrawing();

            runi = 1;
            goto restart;
        }
    }

    videoEndDrawing();
    videoShowFrame(1);

#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST && lazyselector)
        glDrawBuffer(GL_BACK);
#endif

    return 0;

}

#undef TMPERRMSG_SHOW
#undef TMPERRMSG_PRINT
#undef TMPERRMSG_RETURN

#define WIND1X   3
#define WIND1Y 150

static char const *tileinfo_colorstr = "";

static void tileinfo_doprint(int32_t x, int32_t y, char *buf, const char *label, int32_t value, int32_t value2, int32_t pos)
{
    int32_t small = (xdimgame<=640), i = ydimgame>>6;

    if (value2)
        Bsprintf(buf, "%s:%s%4d, %d", label, tileinfo_colorstr, value, value2);
    else Bsprintf(buf,"%s:%s%4d", label, tileinfo_colorstr, value);

    printext256(x+2, y+2+i*pos, 0, -1, buf, small);
    printext256(x, y+i*pos, whitecol, -1, buf, small);
}

// flags: 1:draw asterisk for lotag
//        2:draw asterisk for extra
//        4:print bottom-swapped wall members colored
static void drawtileinfo(const char *title,int32_t x,int32_t y,int32_t picnum,int32_t shade,int32_t pal,int32_t cstat,
                         int32_t lotag,int32_t hitag,int32_t extra, int32_t blend, int32_t statnum, uint32_t flags)
{
    char buf[64];
    int32_t small = (xdimgame<=640);

    if (tilesiz[picnum].x>0 && tilesiz[picnum].y>0)
    {
        int32_t x1 = x+80;

        if (small)
            x1 /= 2;

        x1 = (int32_t)(x1 * 320.0/xdimgame);
        const double scalediv = max(tilesiz[picnum].x,tilesiz[picnum].y)/24.0;
        const int32_t scale = (int32_t)(65536.0/scalediv);

        const int32_t oviewingrange=viewingrange, oyxaspect=yxaspect;
        renderSetAspect(65536, divscale16(ydim*320, xdim*200));
        // +1024: prevents rotatesprite from setting aspect itself
        rotatesprite_fs((x1+13)<<16,(y+11)<<16,scale,0, picnum,shade,pal, 2+1024);
        renderSetAspect(oviewingrange, oyxaspect);
    }

    x = (int32_t)(x * xdimgame/320.0);
    y = (int32_t)(y * ydimgame/200.0);

    videoBeginDrawing();
    printext256(x+2,y+2,0,-1,title,small);
    printext256(x,y,editorcolors[14],-1,title,small);

    if (flags&4)
    {
        Bsprintf(tempbuf, "^%d", editorcolors[14]);
        tileinfo_colorstr = tempbuf;
    }

    tileinfo_doprint(x, y, buf, "Pic", picnum, 0, 1);

    if (blend)
    {
        Bsprintf(tempbuf, "^%d", editorcolors[14]);
        tileinfo_colorstr = tempbuf;
    }

    tileinfo_doprint(x, y, buf, "Shd", shade, blend, 2);

    if (!(flags & 4))
        tileinfo_colorstr = "";

    tileinfo_doprint(x, y, buf, "Pal", pal, 0, 3);
    tileinfo_doprint(x, y, buf, "Cst", cstat, 0, 4);
    tileinfo_colorstr = "";
    tileinfo_doprint(x, y, buf, (flags&1)?"Lo*":"Lot", lotag, 0, 5);
    tileinfo_doprint(x, y, buf, "Hit", hitag, 0, 6);
    tileinfo_doprint(x, y, buf, (flags&2)?"Ex*":"Ext", extra, statnum, 7);

    videoEndDrawing();
}
//int32_t snap=0;
//int32_t saveval1,saveval2,saveval3;

static inline void getnumber_dochar(char *ptr, int32_t num)
{
    *ptr = (char) num;
}

static inline void getnumber_doint16_t(int16_t *ptr, int32_t num)
{
    *ptr = (int16_t) num;
}

static inline void getnumber_doint32(int32_t *ptr, int32_t num)
{
    *ptr = (int32_t) num;
}

static inline void getnumber_doint64(int64_t *ptr, int32_t num)
{
    *ptr = (int64_t) num;
}

static void getnumberptr256(const char *namestart, void *num, int32_t bytes, int32_t maxnumber, char sign, void *(func)(int32_t))
{
    char buffer[80], ch;
    int32_t danum = 0, oldnum;
    uint8_t flags = (sign>>1)&3;
    sign &= 1;

    switch (bytes)
    {
    case 1:
        danum = sign ? *(int8_t *)num : *(uint8_t *)num;
        break;
    case 2:
        danum = sign ? *(int16_t *)num : *(uint16_t *)num;
        break;
    case 4:
        danum = *(int32_t *)num;
        break;
    case 8:
        danum = *(int64_t *)num;
        break;
    }

    oldnum = danum;
    keyFlushChars();
    while (keystatus[sc_Escape] == 0)
    {
        if (handleevents())
            quitevent = 0;

        M32_DrawRoomsAndMasks();

        ch = keyGetChar();

        if (keystatus[sc_Escape]) break;

        clearkeys();

        g_mouseBits = 0;
        searchx = osearchx;
        searchy = osearchy;

        inputchecked = 1;

        ExtCheckKeys();

        Bsprintf(buffer,"%s%d",namestart,danum);
        if ((int)totalclock & 32) Bstrcat(buffer,"_ ");
        printmessage256(0, 0, buffer);
        if (func != NULL)
        {
            Bsprintf(buffer,"%s",(char *)func((int32_t)danum));
            printmessage256(0, 9, buffer);
        }
        videoShowFrame(1);

        if (getnumber_internal1(ch, &danum, maxnumber, sign) ||
            getnumber_autocomplete(namestart, ch, &danum, flags))
        {
            if (danum != oldnum)
                asksave = 1;
            oldnum = danum;
            break;
        }

        switch (bytes)
        {
        case 1:
            getnumber_dochar((char *)num, danum);
            break;
        case 2:
            getnumber_doint16_t((int16_t *)num, danum);
            break;
        case 4:
            getnumber_doint32((int32_t *)num, danum);
            break;
        case 8:
            getnumber_doint64((int64_t *)num, danum);
            break;
        }
    }

    clearkeys();

    lockclock = (int)totalclock;  //Reset timing

    switch (bytes)
    {
    case 1:
        getnumber_dochar((char *)num, oldnum);
        break;
    case 2:
        getnumber_doint16_t((int16_t *)num, oldnum);
        break;
    case 4:
        getnumber_doint32((int32_t *)num, oldnum);
        break;
    case 8:
        getnumber_doint64((int64_t *)num, oldnum);
        break;
    }
}

#if 0
int64_t ldistsqr(spritetype *s1,spritetype *s2)
{
    return (((int64_t)(s2->x - s1->x))*((int64_t)(s2->x - s1->x)) +
            ((int64_t)(s2->y - s1->y))*((int64_t)(s2->y - s1->y)));
}
#endif

static void TextEntryMode(int16_t startspr)
{
    char ch, buffer[80], doingspace=0;
    int16_t daang = 0, t, alphidx, linebegspr, curspr, cursor;
    int32_t i, j, k, dax = 0, day = 0;
    static uint8_t hgap=0, vgap=4;
    static uint8_t spcgap[MAX_ALPHABETS], firstrun=1;
    spritetype *sp;

    int16_t *spritenums;
    int32_t stackallocsize=32, numletters=0;

    if (firstrun)
    {
        firstrun=0;
        for (i=0; i<numalphabets; i++)
            spcgap[i] = 0;
    }

    if (startspr<0 || startspr>=MAXSPRITES ||
            sprite[startspr].statnum == MAXSTATUS)
        return;

    if (numalphabets == 0)
    {
        message("Alphabet configuration not read.");
        return;
    }

    if ((sprite[startspr].cstat&16) == 0)
    {
        message("Must point at a wall-aligned text sprite.");
        return;
    }

    int16_t basetile = t = sprite[startspr].picnum;
    alphidx = -1;
    for (i=0; i<numalphabets; i++)
    {
        for (j=0; j<NUMPRINTABLES; j++)
            if (alphabets[i].pic[j] == t)
            {
                alphidx = i;
                basetile = t;
                if (spcgap[i] == 0)
                    spcgap[i] = 3*tilesiz[t].x/2;
                goto ENDFOR1;
            }
    }
ENDFOR1:
    if (alphidx==-1)
    {
        message("Must point at a text sprite.");
        return;
    }

    curspr = linebegspr = startspr;

    t = sprite[startspr].picnum;
    sprite[startspr].xoffset = -picanm[t].xofs;
    sprite[startspr].yoffset = -picanm[t].yofs;

    spritenums = (int16_t *)Xmalloc(stackallocsize * sizeof(int16_t));

    cursor = insertsprite(sprite[startspr].sectnum,0);
    if (cursor < 0) goto ERROR_TOOMANYSPRITES;

    sp = &sprite[cursor];
    Bmemcpy(sp, &sprite[startspr], sizeof(spritetype));
    sp->yoffset = 0;
    // NUKE-TODO:
    //sp->picnum = SMALLFNTCURSOR;
    sp->picnum = 0;
    sp->xrepeat = clamp(sp->xrepeat/tilesiz[sp->picnum].x, 2, 255);
    sp->yrepeat = clamp((sp->yrepeat*tilesiz[sprite[startspr].picnum].y)/tilesiz[sp->picnum].y, 4, 255);
    sp->pal = 0;
    sp->cstat = 18;

    keyFlushChars();
    while (keystatus[sc_Escape] == 0)
    {
        if (handleevents())
            quitevent = 0;


        if (PRESSED_KEYSC(UP))  // vertical gap in pixels (32 x-units)
            vgap += (vgap<255);

        if (PRESSED_KEYSC(DOWN))
            vgap -= (vgap>0);

        if (PRESSED_KEYSC(RIGHT))  // horizontal gap in half pixels
            hgap += (hgap<255);

        if (PRESSED_KEYSC(LEFT))
            hgap -= (hgap>0);

        if (PRESSED_KEYSC(INSERT))  // space gap in half pixels
            spcgap[alphidx] += (spcgap[alphidx]<255);

        if (PRESSED_KEYSC(DELETE))
            spcgap[alphidx] -= (spcgap[alphidx]>1);

        if (PRESSED_KEYSC(HOME))  // shade
            sprite[linebegspr].shade += (sprite[linebegspr].shade<127);

        if (PRESSED_KEYSC(END))
            sprite[linebegspr].shade -= (sprite[linebegspr].shade>-128);

        if (PRESSED_KEYSC(PGUP))  // pal
            sprite[linebegspr].pal += (sprite[linebegspr].pal<255);

        if (PRESSED_KEYSC(PGDN))
            sprite[linebegspr].pal -= (sprite[linebegspr].pal>0);

        M32_DrawRoomsAndMasks();

        ch = keyGetChar();

        if (keystatus[sc_Escape]) break;

        clearkeys();

        g_mouseBits = 0;
        searchx = osearchx;
        searchy = osearchy;

        inputchecked = 1;

        ExtCheckKeys();

        Bsprintf(tempbuf, "^%dText entry mode.^%d Navigation keys change vars.", editorcolors[10], whitecol);
        printmessage256(0,0,tempbuf);
        Bsprintf(buffer, "Hgap=%d, Vgap=%d, SPCgap=%d, Shd=%d, Pal=%d",
                 hgap, vgap, spcgap[alphidx], TrackerCast(sprite[linebegspr].shade), TrackerCast(sprite[linebegspr].pal));
        printmessage256(0, 9, buffer);
        videoShowFrame(1);

        // ---
        sp = &sprite[curspr];
        if (!doingspace)
        {
            dax = sp->x; day = sp->y;
            daang = sp->ang;
        }

        j = sp->xrepeat*(hgap+tilesiz[sp->picnum].x+2);
        {
            vec3_t vect;
            vect.x = dax + ((j*sintable[daang])>>17);
            vect.y = day - ((j*sintable[(daang+512)&2047])>>17);
            vect.z = sp->z;
            setsprite(cursor,&vect);
        }

        if (ch>=33 && ch<=126 && alphabets[alphidx].pic[ch-33] >= 0)
        {
            int16_t sect;

            // mapping char->tilenum
            t = alphabets[alphidx].pic[ch-33];
            j = sp->xrepeat*(hgap+tilesiz[sp->picnum].x+tilesiz[t].x);

            dax += (j*sintable[daang])>>17;
            day -= (j*sintable[(daang+512)&2047])>>17;
            dax += (j*sintable[(sprite[curspr].ang+2560)&2047])>>17;
            day += (j*sintable[(sprite[curspr].ang+2048)&2047])>>17;

            sect = sprite[curspr].sectnum;
            updatesector(dax,day,&sect);
            if (Numsprites < MAXSPRITES && sect >= 0)
            {
                i = insertsprite(sect,0);
                Bmemcpy(&sprite[i], &sprite[linebegspr], sizeof(spritetype));
                sprite[i].sectnum = sect;

                sprite[i].x = dax, sprite[i].y = day;
                sprite[i].picnum = t;
                sprite[i].ang = daang;

                sprite[i].xoffset = -picanm[t].xofs;
                sprite[i].yoffset = -picanm[t].yofs;
                sprite[i].xoffset += alphabets[alphidx].xofs[(int32_t)ch-33];
                sprite[i].yoffset += alphabets[alphidx].yofs[(int32_t)ch-33];

                DoSpriteOrnament(i);

                for (k=0; k<MAXTILES; k++)
                    localartfreq[k] = 0;
                for (k=0; k<MAXSPRITES; k++)
                    if (sprite[k].statnum < MAXSTATUS)
                        localartfreq[sprite[k].picnum]++;

                curspr = i;
                doingspace = 0;

                asksave = 1;

                if (numletters >= stackallocsize)
                {
                    stackallocsize *= 2;
                    spritenums = (int16_t *)Xrealloc(spritenums, stackallocsize*sizeof(int16_t));
                }
                spritenums[numletters++] = i;
            }
        }
        else if (ch == 32)
        {
            dax += ((sp->xrepeat*spcgap[alphidx]*sintable[daang])>>17);
            day -= ((sp->xrepeat*spcgap[alphidx]*sintable[(daang+512)&2047])>>17);
            doingspace = 1;
        }
        else if (ch == 8)  // backspace
        {
            if (doingspace)
                doingspace = 0;
            else if (numletters > 0)
            {
                int16_t last = spritenums[numletters-1];

                if (sprite[last].z != sprite[linebegspr].z)  // only "delete" line break
                {
                    sprite[linebegspr].z = sprite[last].z;
                    curspr = last;
                }
                else if (numletters > 1)
                {
                    int16_t sectolast = spritenums[numletters-2];

                    if (sprite[last].z == sprite[sectolast].z)
                        curspr = sectolast;
                    else  // if we delete the first letter on the line
                        curspr = linebegspr;

                    numletters--;
                    deletesprite(last);

                    asksave = 1;
                }
                else
                {
                    numletters--;
                    deletesprite(last);

                    curspr = linebegspr;
                    asksave = 1;
                }
            }
            else
            {
                sprite[linebegspr].z -= ((sprite[linebegspr].yrepeat*(vgap+tilesiz[basetile].y))<<2);
                asksave = 1;
            }
        }
        else if (ch == 13)  // enter
        {
            sprite[linebegspr].z += ((sprite[linebegspr].yrepeat*(vgap+tilesiz[basetile].y))<<2);
            curspr = linebegspr;
            doingspace = 0;
            asksave = 1;
        }
    }

ERROR_TOOMANYSPRITES:
    if (cursor < 0) message("Too many sprites in map!");
    else deletesprite(cursor);

    Xfree(spritenums);

    clearkeys();

    lockclock = (int)totalclock;  //Reset timing
}

static void mouseaction_movesprites(int32_t *sumxvect, int32_t *sumyvect, int32_t yangofs, int32_t mousexory)
{
    int32_t xvect,yvect, daxvect,dayvect, ii, spi;
    int32_t units, gridlock = (eitherCTRL && grid > 0 && grid < 9);
    auto const sp = &sprite[searchwall];
    int16_t tsect = sp->sectnum;
    vec3_t tvec = { sp->x, sp->y, sp->z };

    xvect = -((mousexory*(int32_t)sintable[(ang+yangofs+512)&2047])<<3);
    yvect = -((mousexory*(int32_t)sintable[(ang+yangofs)&2047])<<3);

    if (gridlock)
    {
        units = 1<<(11-grid);

        if ((tvec.x & (units-1)) || (tvec.y & (units-1)))
        {
            daxvect = ((tvec.x & ~(units-1)) - tvec.x)<<14;
            dayvect = ((tvec.y & ~(units-1)) - tvec.y)<<14;
        }
        else
        {
            units <<= 14;

            *sumxvect += xvect;
            *sumyvect += yvect;

            if (klabs(*sumxvect) >= units)
            {
                daxvect = ((*sumxvect)/units)*units;
                *sumxvect %= units;
            }
            else
                daxvect = 0;

            if (klabs(*sumyvect) >= units)
            {
                dayvect = ((*sumyvect)/units)*units;
                *sumyvect %= units;
            }
            else
                dayvect = 0;
        }
    }
    else
    {
        daxvect = xvect;
        dayvect = yvect;
    }

    if (highlightcnt<=0 || (show2dsprite[searchwall>>3] & (1<<(searchwall&7)))==0)
    {
        clipmove(&tvec, &tsect, daxvect,dayvect, sp->clipdist,64<<4,64<<4, spnoclip?1:CLIPMASK0);
        setsprite(searchwall, &tvec);
    }
    else
    {
        xvect = daxvect;
        yvect = dayvect;

        // test run
        for (ii=0; ii<highlightcnt; ii++)
            if (highlight[ii]&16384)
            {
                spi = highlight[ii]&16383;
                Bmemcpy(&tvec, &sprite[spi], sizeof(vec3_t));
                tsect = sprite[spi].sectnum;
                clipmove(&tvec, &tsect, xvect,yvect, 128,64<<4,64<<4, spnoclip?1:CLIPMASK0);

                xvect = (tvec.x - sprite[spi].x)<<14;
                yvect = (tvec.y - sprite[spi].y)<<14;
            }
        // the real thing
        for (ii=0; ii<highlightcnt; ii++)
            if (highlight[ii]&16384)
            {
                spi = highlight[ii]&16383;
                Bmemcpy(&tvec, &sprite[spi], sizeof(vec3_t));
                tsect = sprite[spi].sectnum;
                clipmove(&tvec, &tsect, xvect,yvect, 128,64<<4,64<<4, spnoclip?1:CLIPMASK0);
                setsprite(spi, &tvec);
            }
    }
}

static int32_t addtobyte(int8_t *byte, int32_t num)
{
    int32_t onum = *byte, clamped=0;
    if (onum + num != (int8_t)(onum + num))
        clamped = 1;
    if (!clamped)
        *byte = (onum + num);
    return clamped;
}

static void toggle_sprite_alignment(int32_t spritenum)
{
    static const char *aligntype[4] = { "view", "wall", "floor", "???" };

    int32_t i = sprite[spritenum].cstat;

    if ((i&48) < 32)
        i += 16;
    else
        i &= ~48;
    sprite[spritenum].cstat = i;

    message("Sprite %d now %s aligned", spritenum, aligntype[(i&48)/16]);
    asksave = 1;
}

static void toggle_cf_flipping(int32_t sectnum, int32_t floorp)
{
    static const int8_t next3[8] = { 6, 7, 4, 5, 0, 1, 3, 2 };  // 0->6->3->5->1->7->2->4->0
    static const int8_t prev3[8] = { 4, 5, 7, 6, 2, 3, 0, 1 };  // 0<-6<-3<-5<-1<-7<-2<-4<-0

    static const int16_t orient[8] = { 360, -360, -180, 180, -270, 270, 90, -90 };

    const int32_t search = floorp ? SEARCH_FLOOR : SEARCH_CEILING;

    uint16_t *const stat = &SECTORFLD(sectnum,stat, floorp);
    int32_t i = *stat;

    i = (i&0x4)+((i>>4)&3);
    i = eitherSHIFT ? prev3[i] : next3[i];
    message("Sector %d %s flip %d deg%s", sectnum, typestr[search],
            klabs(orient[i])%360, orient[i] < 0 ? " mirrored":"");
    i = (i&0x4)+((i&3)<<4);
    *stat &= ~0x34;
    *stat |= i;
    asksave = 1;
}

// vec2_t initializer from a 1st point of wall
#define WALLVEC_INITIALIZER(w) { POINT2(w).x-wall[w].x, POINT2(w).y-wall[w].y }

static int64_t lldotv2(const vec2_t *v1, const vec2_t *v2)
{
    return (int64_t)v1->x * v2->x + (int64_t)v1->y * v2->y;
}

#ifdef NEW_MAP_FORMAT
# define YAX_BIT_PROTECT 0
#else
# define YAX_BIT_PROTECT YAX_BIT
#endif

////////// KEY PRESS HANDLER IN 3D MODE //////////
static void Keys3d(void)
{
    int32_t i = 0, changedir,tsign; // ,count,nexti
    int32_t j, k, tempint = 0, hiz, loz;
    int32_t hihit, lohit;
    char smooshyalign=0, repeatpanalign=0; //, buffer[80];
    int16_t startwall, endwall, dasector; //, statnum=0;
    char tempbuf[128];

    /* start Mapster32 */

    // NUKE-TODO: Add some sound stuff here(asound.cpp?)
    //if (g_numsounds > 0 && AmbienceToggle)
    //{
    //    M32_MoveFX();
    //    S_Update();
    //}

    if (usedcount && !helpon)
    {
#if 0
        if (!AIMING_AT_SPRITE)
        {
            count=0;
            for (i=0; i<numwalls; i++)
            {
                if (wall[i].picnum == temppicnum) count++;
                if (wall[i].overpicnum == temppicnum) count++;
            }
            for (i=0; i<numsectors; i++)
            {
                if (sector[i].ceilingpicnum == temppicnum) count++;
                if (sector[i].floorpicnum == temppicnum) count++;
            }
        }
        else
        {
            count=0;
            statnum=0;

            i = headspritestat[statnum];
            while (i != -1)
            {
                nexti = nextspritestat[i];
                if (sprite[i].picnum == temppicnum) count++;
                i = nexti;
            }
        }
#endif
        drawtileinfo("Clipboard",3,124,temppicnum,tempshade,temppal,tempcstat,templotag,temphitag,tempextra,tempblend,tempstatnum,0);
    }// end if usedcount

    if (searchsector > -1 && searchsector < numsectors)
    {
        char lines[8][128];
        int32_t num=0;
        int32_t x,y,flags=0;

        if (infobox&1)
        {
            int32_t height2 = sector[searchsector].floorz - sector[searchsector].ceilingz;

            switch (searchstat)
            {
            case SEARCH_WALL:
            case SEARCH_MASKWALL:
            {
                int32_t dist, height1=0, height3=0;
                const int32_t w = SELECT_WALL();
                const int32_t swappedbot = (int32_t)(w != searchwall);
                flags |= (swappedbot<<2);
#ifdef YAX_ENABLE__COMPAT
                flags |= (yax_getnextwall(searchwall, YAX_CEILING)>=0) + 2*(yax_getnextwall(searchwall, YAX_FLOOR)>=0);
#endif
                drawtileinfo("Selected", WIND1X,WIND1Y,
                             AIMING_AT_WALL ? wall[w].picnum : wall[w].overpicnum,
                             wall[w].shade, wall[w].pal, wall[w].cstat,
                             wall[searchwall].lotag, wall[searchwall].hitag, wall[searchwall].extra, 0, 0, flags);

                dist = wallength(searchwall);

                if (wall[searchwall].nextsector >= 0 && wall[searchwall].nextsector < numsectors)
                {
                    int32_t nextsect = wall[searchwall].nextsector;
                    height1 = sector[searchsector].floorz - sector[nextsect].floorz;
                    height2 = sector[nextsect].floorz - sector[nextsect].ceilingz;
                    height3 = sector[nextsect].ceilingz - sector[searchsector].ceilingz;
                }

                Bsprintf(lines[num++],"Panning:^%d %d, %d", swappedbot?editorcolors[14]:whitecol,
                                TrackerCast(wall[w].xpanning), TrackerCast(wall[w].ypanning));
                Bsprintf(lines[num++],"Repeat:  %d, %d", TrackerCast(wall[searchwall].xrepeat), TrackerCast(wall[searchwall].yrepeat));
                Bsprintf(lines[num++],"Overpic: %d", TrackerCast(wall[searchwall].overpicnum));
                lines[num++][0]=0;

                if (getmessageleng)
                    break;

                if (searchwall != searchbottomwall)
                    Bsprintf(lines[num++],"^%dWall%d ->^%d %d", editorcolors[10], searchwall, editorcolors[14], searchbottomwall);
                else
                    Bsprintf(lines[num++],"^%dWall^%d %d", editorcolors[10], (wall[searchwall].cstat & CSTAT_WALL_ROTATE_90) ? editorcolors[11] : editorcolors[10], searchwall);

                if (wall[searchwall].nextsector!=-1)
                    Bsprintf(lines[num++],"LoHeight:%d, HiHeight:%d, Length:%d",height1,height3,dist);
                else
                    Bsprintf(lines[num++],"Height:%d, Length:%d",height2,dist);
                break;
            }

            case SEARCH_CEILING:
            case SEARCH_FLOOR:
                drawtileinfo("Selected", WIND1X, WIND1Y, AIMED_CEILINGFLOOR(picnum), AIMED_CEILINGFLOOR(shade),
                             AIMED_CEILINGFLOOR(pal), AIMED_CEILINGFLOOR(stat),
                             sector[searchsector].lotag, sector[searchsector].hitag, sector[searchsector].extra,0,0,0);

                {
                    int32_t xp=AIMED_CEILINGFLOOR(xpanning), yp=AIMED_CEILINGFLOOR(ypanning);
#ifdef YAX_ENABLE__COMPAT
                    int32_t notextended = 1;
                    if (yax_getbunch(searchsector, AIMING_AT_FLOOR) >= 0)
                        notextended = 0;
                    Bsprintf(lines[num++],"Panning:  %d%s, %d", xp, notextended?"":"*", yp);
#else
                    Bsprintf(lines[num++],"Panning:  %d, %d", xp, yp);
#endif
                }
                Bsprintf(lines[num++],"%sZ: %d", Typestr[searchstat], AIMED_CEILINGFLOOR(z));
                Bsprintf(lines[num++],"Slope:    %d", AIMED_CEILINGFLOOR(heinum));
                lines[num++][0]=0;

                if (getmessageleng)
                    break;

                Bsprintf(lines[num++],"^%dSector %d^%d %s, %s", editorcolors[10], searchsector, whitecol, typestr[searchstat], ExtGetSectorCaption(searchsector));
                Bsprintf(lines[num++],"Height: %d, Visibility:%d", height2, TrackerCast(sector[searchsector].visibility));
                break;

            case SEARCH_SPRITE:
                drawtileinfo("Selected", WIND1X, WIND1Y, sprite[searchwall].picnum, sprite[searchwall].shade,
                             sprite[searchwall].pal, sprite[searchwall].cstat, sprite[searchwall].lotag,
                             sprite[searchwall].hitag, sprite[searchwall].extra,sprite[searchwall].blend, sprite[searchwall].statnum, 0);

                Bsprintf(lines[num++], "Repeat:  %d,%d",
                    TrackerCast(sprite[searchwall].xrepeat), TrackerCast(sprite[searchwall].yrepeat));
                Bsprintf(lines[num++], "PosXY:   %d,%d%s",
                    TrackerCast(sprite[searchwall].x), TrackerCast(sprite[searchwall].y),
                         sprite[searchwall].xoffset|sprite[searchwall].yoffset ? " ^251*":"");
                Bsprintf(lines[num++], "PosZ: ""   %d", TrackerCast(sprite[searchwall].z));// prevents tab character
                lines[num++][0]=0;

                if (getmessageleng)
                    break;

                if (sprite[searchwall].picnum<0 || sprite[searchwall].picnum>=MAXTILES)
                    break;

                if (names[sprite[searchwall].picnum][0])
                {
                    //if (sprite[searchwall].picnum==SECTOREFFECTOR)
                    //    Bsprintf(lines[num++],"^%dSprite %d^%d %s", editorcolors[10], searchwall, whitecol, SectorEffectorText(searchwall));
                    //else
                        Bsprintf(lines[num++],"^%dSprite %d^%d %s", editorcolors[10], searchwall, whitecol, names[sprite[searchwall].picnum]);
                }
                else Bsprintf(lines[num++],"^%dSprite %d^%d", editorcolors[10], searchwall, whitecol);

                Bsprintf(lines[num++], "Elevation:%d",
                         getflorzofslope(searchsector, sprite[searchwall].x, sprite[searchwall].y) - sprite[searchwall].z);
                break;
            }
        }

        x = (int32_t)(WIND1X*(xdimgame/320.));
        y = (int32_t)(WIND1Y*(ydimgame/200.));
        y += (ydimgame>>6)*8;

        if (getmessageleng && !m32_is2d3dmode())
        {
            while (num < 4)
                lines[num++][0] = 0;
            Bsprintf(lines[num++], "^%d%s", editorcolors[10], getmessage);
        }

        videoBeginDrawing();
        for (i=0; i<num; i++)
        {
            printext256(x+2, y+2, 0, -1, lines[i], xdimgame<=640);
            printext256(x, y, whitecol, -1, lines[i], xdimgame<=640);
            y += ydimgame>>6;
        }
        videoEndDrawing();
    }

    VM_OnEvent(EVENT_PREKEYS3D, -1);

    if (keystatus[KEYSC_RALT] && keystatus[KEYSC_RCTRL] && AIMING_AT_CEILING_OR_FLOOR && searchsector >= 0)
    {
        keystatus[KEYSC_RALT] = 0;

        if (highlightcnt >= 0)
        {
            message("WARNING: Cannot select sectors while sprites/walls selected");
        }
        else
        {
            const int32_t sub = keystatus[KEYSC_SEMI];
            const int32_t add = keystatus[KEYSC_QUOTE];

            if (!add && !sub)
                reset_highlightsector();

            handlesecthighlight1(searchsector, sub, 1);
            update_highlightsector();

            message("%s sector %d", sub ? "De-selected" : "Selected", searchsector);
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(V)) // ' V
    {
        if (AIMING_AT_CEILING_OR_FLOOR)
            getnumberptr256("Sector visibility: ", &sector[searchsector].visibility, sizeof(sector[0].visibility), 255, 0, NULL);
    }

    if (keystatus[KEYSC_SEMI] && PRESSED_KEYSC(V))   // ; V
    {
        int16_t currsector;
        uint8_t visval;

        if (highlightsectorcnt == -1)
        {
            message("You didn't select any sectors!");
            return;
        }

        visval = getnumber256("Visibility of selected sectors: ", sector[searchsector].visibility, 255, 0);

        if (AskIfSure("Are you sure to change the visibility of all selected sectors?"))
            return;

        for (i=0; i<highlightsectorcnt; i++)
        {
            currsector = highlightsector[i];
            sector[currsector].visibility = visval;
        }

        message("Visibility changed on all selected sectors");
    }

    if (!keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(L))  // L (grid lock)
    {
        gridlock = !gridlock;
        message("Grid locking %s", gridlock ? "on" : "off");
    }

    if (PRESSED_KEYSC(V))  //V
    {
        if (ASSERT_AIMING)
        {
            int32_t oldtile = AIMED_SELOVR_PICNUM;

            globalpal = AIMED_CF_SEL(pal);
            tempint = m32gettile(oldtile);
            AIMED_SELOVR_PICNUM = tempint;
            AIMED_CF_SEL(pal) = globalpal;

            if (AIMING_AT_MASKWALL && wall[searchwall].nextwall >= 0)
                NEXTWALL(searchwall).overpicnum = tempint;

            if (AIMING_AT_SPRITE)
                correct_sprite_yoffset(searchwall);

            if (oldtile != tempint)
                asksave = 1;
        }
    }

    if (PRESSED_KEYSC(3))  /* 3 (toggle floor-over-floor (cduke3d only) */
    {
        floor_over_floor = !floor_over_floor;
        //        if (!floor_over_floor) ResetFOFSize();
        message("Floor-over-floor display %s",floor_over_floor?"enabled":"disabled");
    }

    if (PRESSED_KEYSC(F3) && !m32_is2d3dmode())
    {
        mlook = !mlook;
        message("Mouselook: %s",mlook?"enabled":"disabled");
    }

    // NUKE-TODO: Add sound stuff
    //if (PRESSED_KEYSC(F4))
    //{
    //    AmbienceToggle = !AmbienceToggle;
    //    message("Ambience sounds: %s",AmbienceToggle?"enabled":"disabled");
    //    if (!AmbienceToggle)
    //    {
    //        FX_StopAllSounds();
    //        S_ClearSoundLocks();
    //    }
    //}

    // PK
    if (PRESSED_KEYSC(F5))
    {
        unrealedlook = !unrealedlook;
        message("UnrealEd mouse navigation: %s",unrealedlook?"enabled":"disabled");
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(DELETE)) // ' del
    {
        if (AIMING_AT_WALL_OR_MASK)
        {
#ifdef NEW_MAP_FORMAT
            wall[searchwall].cstat = 0;
#else
            wall[searchwall].cstat &= YAX_NEXTWALLBITS;
#endif
            message("Wall %d cstat = %d", searchwall, TrackerCast(wall[searchwall].cstat));
        }
        else if (AIMING_AT_SPRITE)
        {
            sprite[searchwall].cstat = 0;
            message("Sprite %d cstat = 0", searchwall);
        }
    }

    // 'P and ;P - Will copy palette to all sectors/walls/sprites highlighted with R-Alt key
    if ((keystatus[KEYSC_QUOTE] || keystatus[KEYSC_SEMI]) && PRESSED_KEYSC(P))   // ' P  ; P
    {
        int16_t w, start_wall, end_wall, currsector;
        int8_t pal[4];

        if (highlightsectorcnt == -1)
        {
            message("You didn't select any sectors!");
            return;
        }

        if (keystatus[KEYSC_QUOTE])
        {
            pal[0] = getnumber256("Ceiling palette: ", -1, M32_MAXPALOOKUPS, 1);
            pal[1] = getnumber256("Floor palette: ", -1, M32_MAXPALOOKUPS, 1);
            pal[2] = getnumber256("Wall palette: ", -1, M32_MAXPALOOKUPS, 1);
            pal[3] = getnumber256("Sprite palette: ", -1, M32_MAXPALOOKUPS, 1);
        }
        else
        {
            pal[0] = getnumber256("Global palette: ", 0, M32_MAXPALOOKUPS, 0);
            pal[1] = pal[2] = pal[3] = pal[0];
        }

        if (AskIfSure(0)) return;

        for (i = 0; i < highlightsectorcnt; i++)
        {
            currsector = highlightsector[i];

            if (pal[0] > -1)
                sector[currsector].ceilingpal = pal[0];
            if (pal[1] > -1)
                sector[currsector].floorpal = pal[1];

            // Do all the walls in the sector
            start_wall = sector[currsector].wallptr;
            end_wall = start_wall + sector[currsector].wallnum;

            if (pal[2] > -1)
                for (w = start_wall; w < end_wall; w++)
                    wall[w].pal = pal[2];

            if (pal[3] > -1)
            {
                for (k=0; k<highlightsectorcnt; k++)
                    for (w=headspritesect[highlightsector[k]]; w >= 0; w=nextspritesect[w])
                        sprite[w].pal = pal[3];
            }
        }

        message("Palettes changed");
    }

    if (PRESSED_KEYSC(DELETE))
    {
        if (AIMING_AT_SPRITE)
        {
            deletesprite(searchwall);

            message("Sprite %d deleted",searchwall);
            
            // NUKE-TODO: Sound stuff
            //if (AmbienceToggle)
            //{
            //    clearbit(g_ambiencePlaying, searchwall);
            //    S_StopEnvSound(sprite[searchwall].lotag, searchwall);
            //}
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(BS))  // backspace
    {
        if (AIMING_AT_WALL_OR_MASK)
        {
#ifdef YAX_ENABLE
            int16_t ynw, cf=-1;

            if (m32_script_expertmode)
            {
                if (eitherSHIFT && !eitherCTRL)
                    cf = 0;
                else if (!eitherSHIFT && eitherCTRL)
                    cf = 1;
            }

            if (cf >= 0)
            {
                // clear TROR uplink/downlink
                ynw = yax_getnextwall(searchwall, cf);
                if (ynw >= 0)
                {
                    yax_setnextwall(ynw, !cf, -1);
                    yax_setnextwall(searchwall, cf, -1);
                }

                message("Cleared wall %d's %s link to wall %d", searchwall, yupdownwall[cf], ynw);
            }
            else
#endif
            if (!eitherSHIFT && !eitherCTRL)
            {
                int32_t nw = wall[searchwall].nextwall;

                if ((unsigned)nw < (unsigned)numwalls)
                {
                    wall[nw].nextsector = wall[nw].nextwall = -1;
                    wall[searchwall].nextsector = wall[searchwall].nextwall = -1;

                    message("Cleared connection between wall %d and %d", searchwall, nw);
                }
            }
        }
    }

    if (PRESSED_KEYSC(F6))  //F6
    {
        autospritehelp = !autospritehelp;
        message("Automatic SECTOREFFECTOR help %s", autospritehelp?"enabled":"disabled");
    }
    if (PRESSED_KEYSC(F7))  //F7
    {
        autosecthelp = !autosecthelp;
        message("Automatic sector tag help %s", autosecthelp?"enabled":"disabled");
    }

    if (autospritehelp && helpon==0)
    {
        // NUKE-TODO: Make hlp files
        //if (AIMING_AT_SPRITE && sprite[searchwall].picnum==SECTOREFFECTOR)
        //    ShowFileText("sehelp.hlp");
        //else if (AIMING_AT_CEILING_OR_FLOOR)
        //    ShowFileText("sthelp.hlp");
    }

    // [.] or [,]: Search & fix panning to the right or left (3D)
    if (AIMING_AT_WALL_OR_MASK && ((tsign=PRESSED_KEYSC(PERIOD)) || PRESSED_KEYSC(COMMA)))
    {
        uint32_t flags = (!eitherSHIFT) | (tsign?0:16) |
            ((!eitherALT)<<2) | ((!!keystatus[KEYSC_QUOTE])<<3) |
            32*(searchwall != searchbottomwall);

        int32_t naligned=AutoAlignWalls(searchwall, flags, 0);
        // Do it a second time because the first one is wrong. FIXME!!!
        AutoAlignWalls(searchwall, flags, 0);

        message("Aligned %d wall%s based on wall %d%s%s%s%s", naligned,
                naligned==1 ? "" : "s", searchwall,
                !eitherSHIFT ? " iteratively" : "",
                !eitherALT ? ", aligning xrepeats" : "",
                keystatus[KEYSC_QUOTE] ? ", aligning TROR-nextwalls" : "",
                (wall[searchbottomwall].cstat&4) ? "" : ". WARNING: top-aligned");
    }

    tsign = 0;
    tsign -= PRESSED_KEYSC(COMMA);
    tsign += PRESSED_KEYSC(PERIOD);

    if (tsign)
    {
        if (AIMING_AT_SPRITE)
        {
            sprite[searchwall].ang += tsign<<(!eitherSHIFT*7);
            sprite[searchwall].ang &= 2047;
            message("Sprite %d angle: %d", searchwall, TrackerCast(sprite[searchwall].ang));
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(L)) // ' L
#ifdef YAX_ENABLE
    if (YAXCHK(!AIMING_AT_CEILING_OR_FLOOR || yax_getbunch(searchsector, AIMING_AT_FLOOR) < 0))
#endif
    {
        i = m32_clipping;
        m32_clipping = 0;

        switch (searchstat)
        {
        case SEARCH_CEILING:
            getnumberptr256("Sector ceilingz: ", &sector[searchsector].ceilingz,
                            sizeof(sector[0].ceilingz), BZ_MAX, 1, NULL);
            if (!(sector[searchsector].ceilingstat&2))
            {
                sector[searchsector].ceilingstat |= 2;
                sector[searchsector].ceilingheinum = 0;
            }
            getnumberptr256("Sector ceiling slope: ", &sector[searchsector].ceilingheinum,
                            sizeof(sector[0].ceilingheinum), BHEINUM_MAX, 1, NULL);
            break;
        case SEARCH_FLOOR:
            getnumberptr256("Sector floorz: ", &sector[searchsector].floorz,
                            sizeof(sector[0].floorz), BZ_MAX, 1, NULL);
            if (!(sector[searchsector].floorstat&2))
            {
                sector[searchsector].floorheinum = 0;
                sector[searchsector].floorstat |= 2;
            }
            getnumberptr256("Sector floor slope: ", &sector[searchsector].floorheinum,
                            sizeof(sector[0].floorheinum), BHEINUM_MAX, 1, NULL);
            break;

        case SEARCH_SPRITE:
            getnumberptr256("Sprite x: ", &sprite[searchwall].x, sizeof(sprite[0].x), editorgridextent-1, 1, NULL);
            getnumberptr256("Sprite y: ", &sprite[searchwall].y, sizeof(sprite[0].y), editorgridextent-1, 1, NULL);
            getnumberptr256("Sprite z: ", &sprite[searchwall].z, sizeof(sprite[0].z), BZ_MAX, 1, NULL);
            getnumberptr256("Sprite angle: ", &sprite[searchwall].ang, sizeof(sprite[0].ang), 2047, 1, NULL);
            sprite[searchwall].ang &= 2047;
            break;
        }

        setslope(searchsector, YAX_CEILING, sector[searchsector].ceilingheinum);
        setslope(searchsector, YAX_FLOOR, sector[searchsector].floorheinum);

        asksave = 1;
        m32_clipping = i;
    }


    getzrange(&pos, cursectnum, &hiz, &hihit, &loz, &lohit, 128, CLIPMASK0);

    if (PRESSED_KEYSC(CAPS) || (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(Z)))  // CAPS LOCK
    {
        zmode = (zmode+1)%3;
        if (zmode == 1)
            zlock = (loz-pos.z)&0xfffffc00;
        switch (zmode)
        {
        case 0: message("Zmode = Gravity"); break;
        case 1: message("Zmode = Locked/Sector"); break;
        case 2: message("Zmode = Locked/Free"); break;
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(M))  // 'M
    {
        if (ASSERT_AIMING)
        {
            Bsprintf(tempbuf, "%s extra: ", Typestr_wss[searchstat]);
            getnumberptr256(tempbuf, &AIMED(extra), sizeof(int16_t), BTAG_MAX, 1, NULL);
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(1) && ASSERT_AIMING)  // 1 (make 1-way wall)
    {
        if (!AIMING_AT_SPRITE)
        {
            wall[searchwall].cstat ^= 32;
            message("Wall %d one side masking bit %s", searchwall, ONOFF(wall[searchwall].cstat&32));
        }
        else
        {
            i = sprite[searchwall].cstat;
            i ^= 64;
            if ((i&48) == 32)
            {
                i &= ~8;
                if ((i&64) && pos.z>sprite[searchwall].z)
                    i |= 8;
            }
            message("Sprite %d one sided bit %s", searchwall, ONOFF(i&64));
            sprite[searchwall].cstat = i;
        }
        asksave = 1;
    }

    if (PRESSED_KEYSC(2))  // 2 (bottom wall swapping)
    {
        if (!AIMING_AT_SPRITE && searchwall>=0)
        {
            wall[searchwall].cstat ^= 2;
            message("Wall %d bottom texture swap bit %s", searchwall, ONOFF(wall[searchwall].cstat&2));
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(O))  // O (top/bottom orientation - for doors)
    {
        if (AIMING_AT_WALL_OR_MASK)
        {
            int16_t w = SELECT_WALL();
            wall[w].cstat ^= 4;
            message("Wall %d %s orientation", w, wall[w].cstat&4?"bottom":"top");
            asksave = 1;
        }
        else if (AIMING_AT_SPRITE)   // O (ornament onto wall) (2D)
        {
            DoSpriteOrnament(searchwall);
            message("Sprite %d ornament onto wall", searchwall);
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(M))  // M (masking walls)
    {
        if (!AIMING_AT_SPRITE && ASSERT_AIMING)
        {
            int16_t next = wall[searchwall].nextwall;

            if (next >= 0)
            {
                wall[searchwall].cstat ^= 16;
                message("Wall %d masking bit %s", searchwall, ONOFF(wall[searchwall].cstat&16));

                wall[searchwall].cstat &= ~8;

                if (wall[searchwall].cstat&16)
                {
                    if (!eitherSHIFT)
                    {
                        wall[next].cstat |= 8;           //auto other-side flip
                        wall[next].cstat |= 16;
                        wall[next].overpicnum = wall[searchwall].overpicnum;
                    }
                }
                else
                {
                    if (!eitherSHIFT)
                    {
                        wall[next].cstat &= ~8;         //auto other-side unflip
                        wall[next].cstat &= ~16;
                    }
                }

                wall[searchwall].cstat &= ~32;
                if (!eitherSHIFT)
                    wall[next].cstat &= ~32;

                asksave = 1;
            }
        }
    }

    if (PRESSED_KEYSC(H))  // H (hitscan sensitivity)
    {
        if (keystatus[KEYSC_QUOTE])
        {
            if (ASSERT_AIMING)
            {
                j = 0;
                if (AIMING_AT_WALL || AIMING_AT_SPRITE)
                {
                    j = taglab_linktags(AIMING_AT_SPRITE, searchwall);
                    j = 2*(j&2);
                }
#ifdef YAX_ENABLE__COMPAT
                if (AIMING_AT_WALL_OR_MASK && yax_getnextwall(searchwall, YAX_FLOOR)>=0)
                    message("Can't change hitag in protected wall");
                else
#endif
                {
                    Bsprintf(tempbuf, "%s hitag: ", Typestr_wss[searchstat]);
                    getnumberptr256(tempbuf, &AIMED(hitag), sizeof(int16_t), BTAG_MAX, 0+j, NULL);
                }
            }
        }
        else
        {
            if (AIMING_AT_SPRITE)
            {
                sprite[searchwall].cstat ^= 256;
                message("Sprite %d hitscan sensitivity bit %s", searchwall, ONOFF(sprite[searchwall].cstat&256));
                asksave = 1;
            }
            else if (AIMING_AT_WALL_OR_MASK || AIMING_AT_CEILING_OR_FLOOR)
            {
#ifdef YAX_ENABLE
                if (AIMING_AT_CEILING_OR_FLOOR && yax_getbunch(searchsector, AIMING_AT_FLOOR)>=0)
                {
                    SECTORFLD(searchsector,stat, AIMING_AT_FLOOR) ^= 2048;
                    message("Sector %d's %s hitscan sensitivity bit %s", searchsector, typestr[searchstat],
                            ONOFF(SECTORFLD(searchsector,stat, AIMING_AT_FLOOR)&2048));
                    asksave = 1;
                }
                else
#endif
                {
                    wall[searchwall].cstat ^= 64;

                    if (wall[searchwall].nextwall >= 0 && !eitherSHIFT)
                    {
                        NEXTWALL(searchwall).cstat &= ~64;
                        NEXTWALL(searchwall).cstat |= (wall[searchwall].cstat&64);
                    }

                    message("Wall %d hitscan sensitivity bit %s%s", searchwall,
                            ONOFF(wall[searchwall].cstat&64), eitherSHIFT?" (one-sided)":"");
                    asksave = 1;
                }
            }
        }
    }

    smooshyalign = keystatus[KEYSC_gKP5];
    repeatpanalign = eitherSHIFT || eitherALT || (bstatus&2);

    {
        static int32_t omlook;

        if (mlook == 2)
        {
            mlook = omlook;
        }
        else if (!unrealedlook && (bstatus&4))
        {
            omlook = mlook;
            mlook = 2;
        }
    }

    // PK: no btn: wheel changes shade
    if ((bstatus&(16|32) && !(bstatus&(1|2|4))) || keystatus[KEYSC_gMINUS] || keystatus[KEYSC_gPLUS])
    {
        //        if (bstatus&1)
        //            mlook = 2;
        tsign = 0;
        if (bstatus&32 || PRESSED_KEYSC(gMINUS))  // -
            tsign = 1;
        if (bstatus&16 || PRESSED_KEYSC(gPLUS))  // +
            tsign = -1;

        if (tsign)
        {
            g_mouseBits &= ~(16|32);
            bstatus &= ~(16|32);

            if (eitherALT)  //ALT
            {
                if (eitherCTRL)  //CTRL
                {
                    if (tsign==1)
                        g_visibility <<= (int)(g_visibility < 16384);
                    else
                        g_visibility >>= (int)(g_visibility > 32);
                    silentmessage("Global visibility %d", g_visibility);
                }
                else
                {
                    k=eitherSHIFT?1:16;

                    if (highlightsectorcnt > 0 && (hlsectorbitmap[searchsector>>3]&(1<<(searchsector&7))))
                    {
                        while (k-- > 0)
                        {
                            for (i=0; i<highlightsectorcnt; i++)
                            {
                                j = highlightsector[i];
                                sector[j].visibility += tsign;

                                if (tsign==1 && sector[j].visibility == 240)
                                    sector[j].visibility = 239;
                                else if (tsign==-1 && sector[j].visibility == 239)
                                    sector[j].visibility = 240;
                            }
                        }
                    }
                    else
                    {
                        while (k-- > 0)
                        {
                            sector[searchsector].visibility += tsign;

                            if (tsign==1 && sector[searchsector].visibility == 240)
                                sector[searchsector].visibility = 239;
                            else if (tsign==-1 && sector[searchsector].visibility == 239)
                                sector[searchsector].visibility = 240;
                        }
                    }

                    silentmessage("Sector %d visibility %d", searchsector,
                                  TrackerCast(sector[searchsector].visibility));
                    asksave = 1;
                }
            }
            else  // if !eitherALT
            {
                int32_t clamped=0;

                k = (highlightsectorcnt>0 && (hlsectorbitmap[searchsector>>3]&(1<<(searchsector&7))));
                tsign *= (1+3*eitherCTRL);

                if (k == 0)
                {
                    if (ASSERT_AIMING)
                    {
                        if (!eitherSHIFT && AIMING_AT_SPRITE && (show2dsprite[searchwall>>3]&(1<<(searchwall&7))))
                        {
                            for (i=0; i<highlightcnt; i++)
                                if (highlight[i]&16384)
                                    clamped = addtobyte(&sprite[highlight[i]&16383].shade, tsign);
                            (clamped ? message : silentmessage)
                                ("Highlighted sprite shade changed by %d%s",
                                 tsign, clamped?" (some sprites' shade clamped)":"");
                        }
                        else
                        {
                            clamped = addtobyte(&AIMED_CF_SEL(shade), tsign);
                            // TODO: factor formatting stuff out and use elsewhere?
                            (clamped ? message : silentmessage)
                                ("%s %s%d shade %d%s", Typestr[searchstat],
                                 AIMING_AT_CEILING_OR_FLOOR ? "of sector " : "",
                                 AIMING_AT_WALL_OR_MASK ? SELECT_WALL() :
                                     (AIMING_AT_CEILING_OR_FLOOR ? searchsector : searchwall),
                                 AIMED_CF_SEL(shade), clamped ? " (clamped)":"");
                        }
                    }
                }
                else
                {
                    for (i=0; i<highlightsectorcnt; i++)
                    {
                        dasector = highlightsector[i];

                        // sector shade
                        clamped |= addtobyte(&sector[dasector].ceilingshade, tsign);
                        clamped |= addtobyte(&sector[dasector].floorshade, tsign);

                        // wall shade
                        for (WALLS_OF_SECTOR(dasector, j))
                            clamped |= addtobyte(&wall[j].shade, tsign);

                        // sprite shade
                        for (j=headspritesect[dasector]; j!=-1; j=nextspritesect[j])
                            clamped |= addtobyte(&sprite[j].shade, tsign);
                    }
                    (clamped ? message : silentmessage)("Highlighted sector shade changed by %d%s", tsign,
                            clamped?" (some objects' shade clamped)":"");
                }
                asksave = 1;
            }
        }
    }

    // PK: lmb only & mousewheel, -, and +, cycle picnum
    if (keystatus[KEYSC_DASH] || keystatus[KEYSC_EQUAL] || (bstatus&(16|32) && (bstatus&1) && !(bstatus&2)))
    {
        if (ASSERT_AIMING)
        {
            int32_t pic = AIMED_SELOVR_PICNUM;
            int32_t dir = (keystatus[KEYSC_EQUAL] || (bstatus&16)) ? 1 : -1;

            do
            {
                pic += dir + MAXTILES;
                pic %= MAXTILES;
            }
            while (!IsValidTile(pic));
            AIMED_SELOVR_PICNUM = pic;

            if (AIMING_AT_SPRITE)
                correct_sprite_yoffset(searchwall);

            asksave = 1;
        }
        keystatus[KEYSC_DASH] = keystatus[KEYSC_EQUAL] = 0;
        g_mouseBits &= ~(16|32);
    }

    if (PRESSED_KEYSC(E))  // E (expand)
    {
        if (AIMING_AT_CEILING_OR_FLOOR)
        {
            AIMED_CEILINGFLOOR(stat) ^= 8;
            message("Sector %d %s texture expansion bit %s", searchsector, typestr[searchstat],
                    ONOFF(AIMED_CEILINGFLOOR(stat)&8));
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(R))  // R (relative alignment, rotation)
    {
        if (keystatus[KEYSC_QUOTE]) // FRAMERATE TOGGLE
        {
            framerateon = !framerateon;
            message("Show framerate %s", ONOFF(framerateon));
        }
        else
        {
            if (AIMING_AT_CEILING_OR_FLOOR)
            {
                AIMED_CEILINGFLOOR(stat) ^= 64;
                message("Sector %d %s texture relativity bit %s", searchsector, typestr[searchstat],
                        ONOFF(AIMED_CEILINGFLOOR(stat)&64));
                asksave = 1;
            }
            if (AIMING_AT_WALL_OR_MASK)
            {
                AIMED_SEL_WALL(cstat) ^= CSTAT_WALL_ROTATE_90;

                message("Wall %d texture rotation bit %s", SELECT_WALL(),
                        ONOFF(AIMED_SEL_WALL(cstat)&CSTAT_WALL_ROTATE_90));
                asksave = 1;
            }
            else if (AIMING_AT_SPRITE)
                toggle_sprite_alignment(searchwall);
        }
    }

    if (PRESSED_KEYSC(F))  //F (Flip)
    {
        if (eitherALT)  //ALT-F (relative alignmment flip)
        {
            if (!AIMING_AT_SPRITE && ASSERT_AIMING)
                SetFirstWall(searchsector, searchwall, eitherSHIFT);
        }
        else
        {
            static const int8_t next2[4] = { 1, 3, 0, 2 };  // 0->1->3->2->0
            static const int8_t prev2[4] = { 2, 0, 3, 1 };  // 0<-1<-3<-2<-0

            static const char *flip2label[4] = { "none", "X", "Y", "X and Y" };

            if (AIMING_AT_WALL_OR_MASK)
            {
                i = wall[searchbottomwall].cstat;
                i = ((i>>3)&1)+((i>>7)&2);    //3-x,8-y

                i = eitherSHIFT ? prev2[i] : next2[i];
                message("Wall %d flip %s", searchwall, flip2label[i]);

                i = ((i&1)<<3)+((i&2)<<7);
                wall[searchbottomwall].cstat &= ~0x0108;
                wall[searchbottomwall].cstat |= i;
                asksave = 1;
            }
            else if (AIMING_AT_CEILING_OR_FLOOR)  //8-way ceiling/floor flipping (bits 2,4,5)
                toggle_cf_flipping(searchsector, AIMING_AT_FLOOR);
            else if (AIMING_AT_SPRITE)
            {
                i = sprite[searchwall].cstat;
                if (((i&48) == 32) && ((i&64) == 0))
                {
                    sprite[searchwall].cstat &= ~0xc;
                    sprite[searchwall].cstat |= ((i&4)^4);
                    message("Sprite %d flip bit %s",searchwall, ONOFF(sprite[searchwall].cstat&4));
                }
                else
                {
                    i = ((i>>2)&3);

                    i = eitherSHIFT ? prev2[i] : next2[i];
                    message("Sprite %d flip %s", searchwall, flip2label[i]);

                    sprite[searchwall].cstat &= ~0xc;
                    sprite[searchwall].cstat |= (i<<2);
                }
                asksave = 1;
            }
        }
    }


    if (keystatus[KEYSC_HOME] && keystatus[KEYSC_END])
        updownunits = 128;
    else if (keystatus[KEYSC_HOME])
        updownunits = 256;
    else if (keystatus[KEYSC_END])
        updownunits = 512;
    else
        updownunits = 1024;

    mouseaction=0;
    if (eitherALT && (bstatus&1))
    {
        g_mousePos.x=0; mskip=1;
        if (g_mousePos.y!=0)
        {
            updownunits=klabs(g_mousePos.y*128);
            mouseaction=1;
        }
    }


    tsign = 0;
    if (ASSERT_AIMING)
    {
        // PK: PGUP/PGDN, rmb only & mwheel
        tsign -= (PRESSED_KEYSC(PGUP) || (mouseaction && g_mousePos.y<0) || ((bstatus&(16|2|1))==(16|2)));
        tsign += (PRESSED_KEYSC(PGDN) || (mouseaction && g_mousePos.y>0) || ((bstatus&(32|2|1))==(32|2)));
    }

    if (tsign)
    {
        int16_t sect0, sect, havebtm=0, havetop=0, moveCeilings, moveFloors;
        int32_t cz, fz;

        k = 0;
        if (highlightsectorcnt > 0 && searchsector>=0 && searchsector<numsectors)
        {
            if (hlsectorbitmap[searchsector>>3]&(1<<(searchsector&7)))
                k = highlightsectorcnt;
        }

        if (k)
        {
            sect = highlightsector[0];
            havetop = AIMING_AT_WALL_OR_MASK;
        }
        else
        {
            if (AIMING_AT_WALL_OR_MASK && wall[searchwall].nextsector>=0
                    && eitherALT && !(bstatus&1))
            {
                sect = wall[searchwall].nextsector;
                havebtm = !AIMING_AT_MASKWALL && searchisbottom;
                havetop = !havebtm;
            }
            else
            {
                sect = searchsector;
                havetop = AIMING_AT_WALL_OR_MASK;
            }
        }

        sect0 = sect;
        moveCeilings = (AIMING_AT_CEILING || havetop);
        moveFloors = (AIMING_AT_FLOOR || havebtm);

        if (moveCeilings || moveFloors)
        {
            int32_t dz = tsign * (updownunits << (eitherCTRL<<1));   // JBF 20031128
            static const char *cfs[2] = { "ceiling", "floor" };
#ifdef YAX_ENABLE
            int16_t bunchnum=-1, maxbunchnum=-1, cb, fb;
            Bmemset(havebunch, 0, sizeof(havebunch));
#endif
            for (j=0; j<(k?k:1); j++, sect=highlightsector[j])
            {
                // stage one: see if we don't move beyond the other side
                // (ceiling if floor and vice versa)

                if (moveCeilings && (dz > 0) && sector[sect].ceilingz+dz > sector[sect].floorz)
                    dz = (k > 1) ? 0 : min(sector[sect].floorz - sector[sect].ceilingz, dz);
                else if (moveFloors && (dz < 0) && sector[sect].floorz+dz < sector[sect].ceilingz)
                    dz = (k > 1) ? 0 : max(sector[sect].ceilingz - sector[sect].floorz, dz);

                if (dz == 0)
                    break;
            }

            if (dz)
            {
                // now truly move things if we're clear to go!
                sect = sect0;
                for (j=0; j<(k?k:1); j++, sect=highlightsector[j])
                {
                    for (i=headspritesect[sect]; i!=-1; i=nextspritesect[i])
                    {
                        spriteoncfz(i, &cz, &fz);
                        if ((moveCeilings && sprite[i].z == cz) || (moveFloors && sprite[i].z == fz))
                            sprite[i].z += dz;
                    }

                    SECTORFLD(sect,z, moveFloors) += dz;
#ifdef YAX_ENABLE
                    bunchnum = yax_getbunch(sect, moveFloors);
                    if (bunchnum >= 0 && !(havebunch[bunchnum>>3]&(1<<(bunchnum&7))))
                    {
                        maxbunchnum = max(maxbunchnum, bunchnum);
                        havebunch[bunchnum>>3] |= (1<<(bunchnum&7));
                        tempzar[bunchnum] = &SECTORFLD(sect,z, moveFloors);
                    }
#endif
                }
            }

#ifdef YAX_ENABLE
            if (dz)
            {
                // sync z values of extended sectors' ceilings/floors
                for (i=0; i<numsectors; i++)
                {
                    yax_getbunches(i, &cb, &fb);
                    if (cb >= 0 && (havebunch[cb>>3]&(1<<(cb&7))))
                        sector[i].ceilingz = *tempzar[cb];
                    if (fb >= 0 && (havebunch[fb>>3]&(1<<(fb&7))))
                        sector[i].floorz = *tempzar[fb];
                }
            }

            if (!dz)
                silentmessage("Didn't move sector %ss", cfs[moveFloors]);
            else if (k<=1 && bunchnum>=0)
                silentmessage("Bunch %d's ceilings and floors = %d", bunchnum, SECTORFLD(sect0,z, moveFloors));
            else
#endif
            if (k<=1)
                silentmessage("Sector %d %sz = %d", sect0, cfs[moveFloors], SECTORFLD(sect0,z, moveFloors));
            else
                silentmessage("%s %d sector %ss by %d units", tsign<0 ? "Raised" : "Lowered",
                              k, cfs[moveFloors], dz*tsign);
        }

        if (AIMING_AT_SPRITE)
        {
            int32_t cz, fz;

            if (eitherCTRL && !eitherALT)  //CTRL - put sprite on ceiling/floor
            {
                spriteoncfz(searchwall, &cz, &fz);
                sprite[searchwall].z = (tsign==1) ? fz : cz;
            }
            else
            {
                k = !!(show2dsprite[searchwall>>3]&(1<<(searchwall&7)));

                tsign *= (updownunits << ((eitherCTRL && mouseaction)*3));

                for (i=0; i<highlightcnt || k==0; i++)
                {
                    if (k==0 || (highlight[i]&0xc000) == 16384)
                    {
                        int16_t sp = k==0 ? searchwall : highlight[i]&16383;

                        sprite[sp].z += tsign;

                        if (!spnoclip)
                        {
                            spriteoncfz(sp, &cz, &fz);
                            inpclamp(&sprite[sp].z, cz, fz);
                        }
#ifdef YAX_ENABLE
                        else if (sprite[sp].sectnum >= 0)
                        {
                            int16_t cb, fb;
                            yax_getbunches(sprite[sp].sectnum, &cb, &fb);
                            if (cb >= 0 || fb >= 0)
                                setspritez(sp, (vec3_t *)&sprite[sp]);
                        }
#endif
                        if (k==0)
                        {
                            silentmessage("Sprite %d z = %d", searchwall, TrackerCast(sprite[searchwall].z));
                            break;
                        }
                    }

                    if (k==1)
                        silentmessage("Sprites %s by %d units", tsign<0 ? "raised" : "lowered", tsign);
                }
            }
        }

        asksave = 1;
        g_mouseBits &= ~(16|32);
    }

    /* end Mapster32 */

    //  DoWater(horiz);

    if (framerateon)
    {
        static int32_t FrameCount = 0;
        static int32_t LastCount = 0;
        static int32_t LastSec = 0;
        static int32_t LastMS = 0;

        int32_t ms = timerGetTicks();
        int32_t howlong = ms - LastMS;

        if (howlong >= 0)
        {
            int32_t thisSec = ms/1000;
            int32_t x = (xdim <= 640);
            int32_t chars = Bsprintf(tempbuf, "%2u ms (%3u fps)", howlong, LastCount);

            int32_t const dax = m32_is2d3dmode() ? m32_2d3d.x + XSIZE_2D3D - 3: windowxy2.x;
            int32_t const day = m32_is2d3dmode() ? m32_2d3d.y + 4 : windowxy1.y;

            if (!x)
            {
                printext256(dax-(chars<<3)+1, day+2,0,-1,tempbuf,x);
                printext256(dax-(chars<<3), day+1,whitecol,-1,tempbuf,x);
            }
            else
            {
                printext256(dax-(chars<<2)+1, day+2,0,-1,tempbuf,x);
                printext256(dax-(chars<<2), day+1,whitecol,-1,tempbuf,x);
            }

            if (LastSec < thisSec)
            {
                LastCount = FrameCount / (thisSec - LastSec);
                LastSec = thisSec;
                FrameCount = 0;
            }
            FrameCount++;
        }
        LastMS = ms;
    }

    tempbuf[0] = 0;

    if ((bstatus&(4|2|1))==4 && !unrealedlook)  //PK
        Bsprintf(tempbuf,"VIEW");
    else if ((bstatus&(2|1))==2)
        Bsprintf(tempbuf, "Z%s", keystatus[KEYSC_HOME] && keystatus[KEYSC_END]
                                 ? " 128"
                                 : keystatus[KEYSC_HOME]
                                 ? " 256"
                                 : keystatus[KEYSC_END]
                                 ? " 512"
                                 : "");

    if ((bstatus&(2|1))==1 || (keystatus[KEYSC_SPACE]))
        Bsprintf(tempbuf,"LOCK");

    if (bstatus&1)
    {
        switch (searchstat)
        {
        case SEARCH_WALL:
        case SEARCH_MASKWALL:
            if (eitherALT)
                Bsprintf(tempbuf,"CEILING Z %s", eitherCTRL?"512":"");
            else if (eitherSHIFT)
                Bsprintf(tempbuf,"PAN %s", eitherCTRL?"8":"");
            else if (eitherCTRL)
                Bsprintf(tempbuf,"SCALE");
            break;
        case SEARCH_CEILING:
        case SEARCH_FLOOR:
            if (eitherALT)
                Bsprintf(tempbuf,"%s Z %s", AIMING_AT_CEILING?"CEILING":"FLOOR", eitherCTRL?"512":"");
            else if (eitherSHIFT)
                Bsprintf(tempbuf,"PAN");
            else if (eitherCTRL)
                Bsprintf(tempbuf,"SLOPE");
            break;
        case SEARCH_SPRITE:
            if (eitherALT)
                Bsprintf(tempbuf,"MOVE Z %s", eitherCTRL?"1024":"");
            else if (eitherSHIFT)
                Bsprintf(tempbuf,"MOVE XY %s", eitherCTRL?"GRID":"");
            else if (eitherCTRL)
                Bsprintf(tempbuf,"SIZE");
            break;
        }
    }

    if (tempbuf[0] != 0)
    {
        i = (Bstrlen(tempbuf)<<3)+6;
        i = max((searchx+i)-(xdim-1), 0);

        j = max((searchy+16)-(ydim-1), 0);

        printext256(searchx+4+2-i, searchy+4+2-j, 0,-1,tempbuf,!(xdimgame > 640));
        printext256(searchx+4-i,   searchy+4-j,   whitecol,-1,tempbuf,!(xdimgame > 640));
    }

    if (helpon==1)
    {
        int32_t small = !(xdimgame > 640);
        for (i=0; i<MAXHELP3D; i++)
        {
            videoBeginDrawing();
            printext256(2, 8+(i*(8+!small))+2, 0, -1, Help3d[i], small);
            printext256(0, 8+(i*(8+!small)), whitecol, -1, Help3d[i], small);
            videoEndDrawing();

            switch (i)
            {
            case 8:
                Bsprintf(tempbuf,"%d",autosave);
                break;
            case 9:
                Bsprintf(tempbuf,"%s",SKILLMODE[skill]);
                break;
            case 10:
                Bsprintf(tempbuf,"%d",framerateon);
                break;
            case 11:
                Bsprintf(tempbuf,"%s",SPRDSPMODE[nosprites]);
                break;
            case 12:
                Bsprintf(tempbuf,"%d",shadepreview);
                break;
            case 13:
                Bsprintf(tempbuf,"%d",showinvisibility);
                break;
            default :
                Bsprintf(tempbuf," ");
                break;
            }

            videoBeginDrawing();
            if (!Bstrcmp(tempbuf,"0"))
                Bsprintf(tempbuf,"OFF");
            else if (!Bstrcmp(tempbuf,"1"))
                Bsprintf(tempbuf,"ON");
            else if (!Bstrcmp(tempbuf,"2"))
                Bsprintf(tempbuf,"ON (2)");

            printext256((20+(!small * 20))*8+2, 8+(i*(8+!small))+2, 0, -1, tempbuf, small);
            printext256((20+(!small * 20))*8, 8+(i*(8+!small)), whitecol, -1, tempbuf, small);
            videoEndDrawing();
        }
    }

    if (keystatus[buildkeys[BK_MODE2D_3D]])  // Enter
    {
        //SetGamePalette(BASEPAL);
        scrSetPalette(0);
        if (videoGetRenderMode() >= REND_POLYMOST)
            videoSetPalette(0, 0, 2);
        FX_StopAllSounds();
        // NUKE-TODO:
        //S_ClearSoundLocks();

#ifdef POLYMER
        DeletePolymerLights();
#endif
    }

    //Stick this in 3D part of ExtCheckKeys
    //Also choose your own key scan codes

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(D)) // ' d
    {
        skill = (skill+1)%MAXSKILL;
        message("%s", SKILLMODE[skill]);
    }

    if (keystatus[KEYSC_I])
    {
        keystatus[KEYSC_I] = 0;

        if (keystatus[KEYSC_QUOTE])  // ' i
        {
            if (AIMING_AT_SPRITE)
            {
                sprite[searchwall].cstat ^= 32768;
                message("Sprite %d made %svisible", searchwall, (sprite[searchwall].cstat&32768) ? "in":"");
            }
        }
        else
        {
            showinvisibility = !showinvisibility;
#if !defined YAX_ENABLE
            message("Show invisible sprites %s", showinvisibility?"enabled":"disabled");
#else
            message("Show invisible objects %s", showinvisibility?"enabled":"disabled");
#endif
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(X)) // ' x
    {
        shadepreview = !shadepreview;
        message("Map shade preview %s", shadepreview?"enabled":"disabled");

#ifdef POLYMER
        DeletePolymerLights();
#endif
    }
///___unused_keys___
    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(W)) // ' w
    {
        nosprites = (nosprites+1)%MAXNOSPRITES;
        message("%s", SPRDSPMODE[nosprites]);
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(Y)) // ' y
    {
        purpleon = !purpleon;
        if (nosprites>3) nosprites=0;
        message("Purple %s", ONOFF(purpleon));
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(C)) // ' C
    {
        if (AIMING_AT_WALL_OR_MASK)
        {
            for (i=0; i<MAXWALLS; i++)
                if (wall[i].picnum==temppicnum)
                    wall[i].shade=tempshade;
        }
        else if (AIMING_AT_CEILING_OR_FLOOR)
        {
            for (i=0; i<MAXSECTORS; i++)
                if (CEILINGFLOOR(i, picnum)==temppicnum)
                    CEILINGFLOOR(i, shade)=tempshade;
        }
        else if (AIMING_AT_SPRITE)
        {
            for (i=0; i<MAXSPRITES; i++)
                if (sprite[i].picnum==temppicnum)
                    sprite[i].shade=tempshade;
        }

        if (ASSERT_AIMING)
        {
            message("%ss with picnum %d now have shade of %d", Typestr[searchstat], temppicnum, tempshade);
            asksave = 1;
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(T)) // ' T
    {
        j = 0;
        if (AIMING_AT_WALL || AIMING_AT_SPRITE)
        {
            j = taglab_linktags(AIMING_AT_SPRITE, searchwall);
            j = 4*(j&1);
        }

        if (AIMING_AT_WALL_OR_MASK)
        {
#ifdef YAX_ENABLE__COMPAT
            if (yax_getnextwall(searchwall, YAX_CEILING)>=0)
                message("Can't change lotag in protected wall");
            else
#endif
            wall[searchwall].lotag = getnumber256("Wall lotag: ", wall[searchwall].lotag, BTAG_MAX, 0+j);
        }
        else if (AIMING_AT_CEILING_OR_FLOOR)
        {
            sector[searchsector].lotag =
                _getnumber256("Sector lotag: ", sector[searchsector].lotag, BTAG_MAX, 0, &ExtGetSectorType);
        }
        else if (AIMING_AT_SPRITE)
        {
            // NUKE-TODO:
            //if (sprite[searchwall].picnum == SECTOREFFECTOR)
            //{
            //    sprite[searchwall].lotag =
            //        _getnumber256("Sprite lotag: ", sprite[searchwall].lotag, BTAG_MAX, 0+j, &SectorEffectorTagText);
            //}
            //else if (sprite[searchwall].picnum == MUSICANDSFX)
            //{
            //    int16_t oldtag = sprite[searchwall].lotag;
            //
            //    sprite[searchwall].lotag =
            //        _getnumber256("Sprite lotag: ", sprite[searchwall].lotag, BTAG_MAX, 0+j, &MusicAndSFXTagText);
            //
            //    if (testbit(g_ambiencePlaying, searchwall) && sprite[searchwall].lotag != oldtag)
            //    {
            //        clearbit(g_ambiencePlaying, searchwall);
            //        S_StopEnvSound(oldtag, searchwall);
            //    }
            //}
            //else
                sprite[searchwall].lotag = getnumber256("Sprite lotag: ", sprite[searchwall].lotag, BTAG_MAX, 0+j);
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(S)) // ' S
    {
        if (ASSERT_AIMING)
        {
            int8_t oshade = AIMED_CF_SEL(shade);
            Bsprintf(tempbuf, "%s shade: ", Typestr[searchstat]);
            getnumberptr256(tempbuf, &AIMED_CF_SEL(shade), sizeof(int8_t), 128, 1, NULL);
            if (AIMED_CF_SEL(shade) != oshade)
                asksave = 1;
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(B)) // ' B
    {
        if (AIMING_AT_SPRITE)
        {
            int8_t oblend = sprite[searchwall].blend;
            Bsprintf(tempbuf, "%s blend: ", Typestr[searchstat]);
            getnumberptr256(tempbuf, &sprite[searchwall].blend, sizeof(int8_t), 255, 0, NULL);
            if (sprite[searchwall].blend != oblend)
                asksave = 1;
        }
    }

    if (PRESSED_KEYSC(F2))  // F2
    {
        if (eitherCTRL || eitherSHIFT)
            infobox ^= (eitherSHIFT | ((eitherCTRL)<<1));
        else
            usedcount = !usedcount;
    }

    if (PRESSED_KEYSC(F1)) // F1
    {
        helpon = !helpon;
//        keystatus[KEYSC_H]=0;  // delete this line?
    }

    if (PRESSED_KEYSC(G)) // G
    {
        if (ASSERT_AIMING)
        {
            int16_t *const picnumptr = AIMING_AT_WALL_OR_MASK ? &AIMED_SELOVR_PICNUM : &AIMED_CF_SEL(picnum);
            const int32_t aiming_at_sprite = AIMING_AT_SPRITE;
            const int32_t opicnum = *picnumptr, osearchwall = searchwall;

            static const char *Typestr_tmp[5] = { "Wall", "Sector ceiling", "Sector floor", "Sprite", "Masked wall" };

            Bsprintf(tempbuf, "%s picnum: ", Typestr_tmp[searchstat]);
            getnumberptr256(tempbuf, picnumptr, sizeof(int16_t), MAXTILES-1, 0+2, NULL);

            Bassert((unsigned)*picnumptr < MAXTILES);
            if (!tileLoad(*picnumptr))
                *picnumptr = opicnum;

            if (*picnumptr != opicnum)
                asksave = 1;

            // need to use the old value because aiming might have changed in getnumberptr256
            if (aiming_at_sprite)
                correct_sprite_yoffset(osearchwall);
        }
    }

    if (PRESSED_KEYSC(B))  // B (clip Blocking xor) (3D)
    {
        if (AIMING_AT_SPRITE)
        {
            sprite[searchwall].cstat ^= 1;
            //                                sprite[searchwall].cstat &= ~256;
            //                                sprite[searchwall].cstat |= ((sprite[searchwall].cstat&1)<<8);
            message("Sprite %d blocking bit %s", searchwall, ONOFF(sprite[searchwall].cstat&1));
            asksave = 1;
        }
        else if (AIMING_AT_WALL_OR_MASK || AIMING_AT_CEILING_OR_FLOOR)
        {
#ifdef YAX_ENABLE
            if (AIMING_AT_CEILING_OR_FLOOR && yax_getbunch(searchsector, AIMING_AT_FLOOR)>=0)
            {
                SECTORFLD(searchsector,stat, AIMING_AT_FLOOR) ^= 512;
                message("Sector %d's %s blocking bit %s", searchsector, typestr[searchstat],
                        ONOFF(SECTORFLD(searchsector,stat, AIMING_AT_FLOOR)&512));
                asksave = 1;
            }
            else
#endif
            {
                wall[searchwall].cstat ^= 1;
                //                                wall[searchwall].cstat &= ~64;
                if ((wall[searchwall].nextwall >= 0) && !eitherSHIFT)
                {
                    NEXTWALL(searchwall).cstat &= ~(1+64);
                    NEXTWALL(searchwall).cstat |= (wall[searchwall].cstat&1);
                }

                message("Wall %d blocking bit %s%s", searchwall, ONOFF(wall[searchwall].cstat&1),
                        eitherSHIFT ? " (one-sided)":"");
                asksave = 1;
            }
        }
    }

    // N (set "spritenoshade" bit)
    if (PRESSED_KEYSC(N) && !eitherCTRL && !keystatus[KEYSC_QUOTE])
    {
        if (AIMING_AT_SPRITE)
        {
            sprite[searchwall].cstat ^= CSTAT_SPRITE_NOSHADE;
            message("Sprite %d spritenoshade bit: %s", searchwall,
                    ONOFF(sprite[searchwall].cstat&CSTAT_SPRITE_NOSHADE));
        }
    }

    if (PRESSED_KEYSC(T))  // T (transluscence for sprites/masked walls)
    {
        if (AIMING_AT_CEILING_OR_FLOOR)   //Set masked/transluscent ceilings/floors
        {
            int32_t nexti[4] = { 128, 256, 384, 0 };
            uint16_t *stat = &AIMED_CEILINGFLOOR(stat);
            const char *statmsg[4] = {"normal", "masked", "translucent", "translucent (2)"};

            i = (*stat&(128+256))>>7;
            i = nexti[i];
            *stat &= ~(128+256);
            *stat |= i;

            message("Sector %d's %s made %s.", searchsector, typestr[searchstat], statmsg[i>>7]);

            asksave = 1;
        }

        if (keystatus[KEYSC_QUOTE])
        {
            if (ASSERT_AIMING)
            {
                int32_t olotag = AIMED(lotag);
                Bsprintf(tempbuf, "%s lotag: ", Typestr_wss[searchstat]);
                AIMED(lotag) = getnumber256(tempbuf, AIMED(lotag), BTAG_MAX, 0);
                if (olotag != AIMED(lotag))
                    asksave = 1;
            }
        }
        else if (eitherCTRL)
        {
            if (AIMING_AT_SPRITE)
                TextEntryMode(searchwall);
        }
        else
        {
            if (AIMING_AT_SPRITE)
            {
                if ((sprite[searchwall].cstat&2) == 0)
                    sprite[searchwall].cstat |= 2;
                else if ((sprite[searchwall].cstat&512) == 0)
                    sprite[searchwall].cstat |= 512;
                else
                    sprite[searchwall].cstat &= ~(2+512);
                asksave = 1;
            }
            else if (AIMING_AT_MASKWALL)
            {
                if ((wall[searchwall].cstat&128) == 0)
                    wall[searchwall].cstat |= 128;
                else if ((wall[searchwall].cstat&512) == 0)
                    wall[searchwall].cstat |= 512;
                else
                    wall[searchwall].cstat &= ~(128+512);

                if (wall[searchwall].nextwall >= 0)
                {
                    NEXTWALL(searchwall).cstat &= ~(128+512);
                    NEXTWALL(searchwall).cstat |= (wall[searchwall].cstat&(128+512));
                }
                asksave = 1;
            }
        }
    }

    // ----------
    i = 512;

    if (keystatus[KEYSC_RSHIFT])
        i = 8;
    else if (keystatus[KEYSC_LSHIFT])
        i = 1;
    else if (keystatus[KEYSC_HOME] && keystatus[KEYSC_END])
        i = 64;
    else if (keystatus[KEYSC_HOME])
        i = 128;
    else if (keystatus[KEYSC_END])
        i = 256;

    mouseaction=0;

    if (eitherCTRL && !eitherSHIFT && (bstatus&1) && AIMING_AT_CEILING_OR_FLOOR)
    {
        g_mousePos.x=0; mskip=1;
        if (g_mousePos.y)
        {
            i=klabs(g_mousePos.y*2);
            mouseaction=1;
        }
    }

    tsign = 0;
    if (ASSERT_AIMING)
    {
        tsign -= (PRESSED_KEYSC(LBRACK) || (mouseaction && g_mousePos.y<0));   // [
        tsign += (PRESSED_KEYSC(RBRACK) || (mouseaction && g_mousePos.y>0));   // ]
    }

    if (tsign)
    {
#ifdef YAX_ENABLE
        int16_t bunchnum, othersidesect=0;
#endif
        if (eitherALT)
        {
            int32_t ns=wall[searchwall].nextsector, sx=wall[searchwall].x, sy=wall[searchwall].y;

            if (ns >= 0 && !mouseaction)
            {
                if (AIMING_AT_CEILING || (tsign < 0 && AIMING_AT_WALL_OR_MASK))
#ifdef YAX_ENABLE
                if (YAXCHK((bunchnum=yax_getbunch(searchsector, YAX_CEILING)) < 0 ||
                           (othersidesect=yax_is121(bunchnum, 1))>=0) &&
                        (bunchnum < 0 || YAXSLOPECHK(searchsector, othersidesect)))
#endif
                {
                    alignceilslope(searchsector, sx, sy, getceilzofslope(ns, sx, sy));
#ifdef YAX_ENABLE
                    if (bunchnum>=0)
                        setslope(othersidesect, 1, sector[searchsector].ceilingheinum);
#endif
                    message("Sector %d align ceiling to wall %d", searchsector, searchwall);
                }

                if (AIMING_AT_FLOOR || (tsign > 0 && AIMING_AT_WALL_OR_MASK))
#ifdef YAX_ENABLE
                if (YAXCHK((bunchnum=yax_getbunch(searchsector, YAX_FLOOR)) < 0 ||
                           (othersidesect=yax_is121(bunchnum, 0))>=0) &&
                        (bunchnum < 0 || YAXSLOPECHK(searchsector, othersidesect)))
#endif
                {
                    alignflorslope(searchsector, sx, sy, getflorzofslope(ns, sx, sy));
#ifdef YAX_ENABLE
                    if (bunchnum>=0)
                        setslope(othersidesect, 0, sector[searchsector].floorheinum);
#endif
                    message("Sector %d align floor to wall %d", searchsector, searchwall);
                }
            }
        }
        else
        {
            if (AIMING_AT_CEILING_OR_FLOOR)
#ifdef YAX_ENABLE
            if (YAXCHK((bunchnum=yax_getbunch(searchsector, AIMING_AT_FLOOR)) < 0 ||
                       (othersidesect=yax_is121(bunchnum, AIMING_AT_CEILING))>=0) &&
                    (bunchnum < 0 || YAXSLOPECHK(searchsector, othersidesect)))
#endif
            {
                int32_t oldslope = (AIMED_CEILINGFLOOR(stat)&2) ? AIMED_CEILINGFLOOR(heinum) : 0;
                int32_t newslope = clamp(oldslope + tsign*i, -BHEINUM_MAX, BHEINUM_MAX);

                setslope(searchsector, AIMING_AT_FLOOR, newslope);
#ifdef YAX_ENABLE
                if (bunchnum >= 0)
                    setslope(othersidesect, !AIMING_AT_FLOOR, newslope);
#endif
                silentmessage("Sector %d %s slope = %d", searchsector,
                              typestr[searchstat], AIMED_CEILINGFLOOR(heinum));
            }
        }

        asksave = 1;
    }


    if ((bstatus&1) && eitherSHIFT)
        mskip=1;

    if ((bstatus&1) && eitherSHIFT && AIMING_AT_CEILING_OR_FLOOR && (g_mousePos.x|g_mousePos.y))
    {
        int32_t fw,x1,y1,x2,y2,stat,ma,a=0;

        stat = SECTORFLD(searchsector,stat, AIMING_AT_FLOOR);
        if (stat&64) // align to first wall
        {
            fw=sector[searchsector].wallptr;
            x1=wall[fw].x,y1=wall[fw].y;
            x2=POINT2(fw).x,y2=POINT2(fw).y;
            a=getangle(x1-x2,y1-y2);
        }
        mouseax+=g_mousePos.x; mouseay+=g_mousePos.y;
        ma = getangle(mouseax,mouseay);
        ma += ang-a;

        i = stat;
        i = (i&0x4)+((i>>4)&3);
        if (stat&64) // align to first wall
            switch (i)
            {
            case 0:break;
            case 1:ma=-ma; break;
            case 2:ma=1024-ma; break;
            case 3:ma+=1024; break;
            case 4:ma=-512-ma; break;
            case 5:ma+=512; break;
            case 6:ma-=512; break;
            case 7:ma=512-ma; break;
            }
        else
            switch (i)
            {
            case 0:ma=-ma; break;
            case 1:break;
            case 2:ma+=1024; break;
            case 3:ma=1024-ma; break;
            case 4:ma-=512; break;
            case 5:ma=512-ma; break;
            case 6:ma=-512-ma; break;
            case 7:ma+=512; break;
            }

        a = ksqrt(uhypsq(mouseax,mouseay));
        if (a)
        {
            int32_t mult = (stat&8) ? 8192 : 8192*2;
            x1 = -a*sintable[(ma+2048)&2047]/mult;
            y1 = -a*sintable[(ma+1536)&2047]/mult;

            if (x1||y1)
            {
                mouseax=0;
                mouseay=0;

                if (AIMING_AT_CEILING_OR_FLOOR)
                {
                    changedir = 1-2*(x1<0);
                    x1 = klabs(x1);
#ifdef YAX_ENABLE__COMPAT
                    if (yax_getbunch(searchsector, AIMING_AT_FLOOR) < 0)
#endif
                    while (x1--)
                        AIMED_CEILINGFLOOR(xpanning) = changechar(AIMED_CEILINGFLOOR(xpanning),changedir,0,0);

                    changedir = 1-2*(y1<0);
                    y1 = klabs(y1);

                    while (y1--)
                        AIMED_CEILINGFLOOR(ypanning) = changechar(AIMED_CEILINGFLOOR(ypanning),changedir,0,0);

                    silentmessage("Sector %d %s panning: %d, %d", searchsector, typestr[searchstat],
                                  AIMED_CEILINGFLOOR(xpanning), AIMED_CEILINGFLOOR(ypanning));
                    asksave=1;
                }
            }
        }
        g_mousePos.x=0;
        g_mousePos.y=0;
    }


    smooshyalign = keystatus[KEYSC_gKP5];
    repeatpanalign = eitherSHIFT || eitherALT;

    ////////////////////
    updownunits=1;
    mouseaction=0;

    if (!g_mouseBits)
    {
        mouseax=0;
        mouseay=0;
    }

    if ((bstatus&1) && !AIMING_AT_CEILING_OR_FLOOR)
    {
        if (eitherSHIFT)
        {
            mskip=1;
            if (g_mousePos.x)
            {
                mouseaction = 1;
                mouseax += g_mousePos.x;
                updownunits = klabs(mouseax/2);
                if (updownunits)
                    mouseax=0;
            }
        }
        else if (eitherCTRL && !eitherALT)
        {
            mskip=1;
            if (g_mousePos.x)
            {
                mouseaction = 2;
                repeatpanalign = 0;
                updownunits = klabs(mouseax+=g_mousePos.x)/(16 - 12*AIMING_AT_SPRITE);
                if (updownunits)
                    mouseax=0;
            }
        }
    }

    if (keystatus[KEYSC_gLEFT] || keystatus[KEYSC_gRIGHT] || mouseaction) // 4 & 6 (keypad)
    {
        if (repeatcountx == 0 || repeatcountx > 32 || mouseaction)
        {
            changedir = 0;
            if (keystatus[KEYSC_gLEFT]  || g_mousePos.x>0)
                changedir = -1;
            if (keystatus[KEYSC_gRIGHT] || g_mousePos.x<0)
                changedir = 1;

            if (AIMING_AT_WALL_OR_MASK)
            {
                if (repeatpanalign == 0)
                {
                    while (updownunits--)
                        wall[searchwall].xrepeat = changechar(wall[searchwall].xrepeat, changedir, smooshyalign, 1);
                    silentmessage("Wall %d repeat: %d, %d", searchwall,
                                  TrackerCast(wall[searchwall].xrepeat),
                                  TrackerCast(wall[searchwall].yrepeat));
                }
                else
                {
                    int16_t w = SELECT_WALL();

                    if (mouseaction)
                    {
                        i = wall[w].cstat;
                        i &= (8|256);

                        if (i==8 || i==256)
                            changedir*=-1;

                        if (eitherCTRL)
                            updownunits *= 8;
                    }

                    while (updownunits--)
                        wall[w].xpanning = changechar(wall[w].xpanning, changedir, smooshyalign, 0);
                    silentmessage("Wall %d panning: %d, %d", w,
                                  TrackerCast(wall[w].xpanning), TrackerCast(wall[w].ypanning));
                }
                asksave = 1;
            }
            else if (AIMING_AT_CEILING_OR_FLOOR)
            {
#ifdef YAX_ENABLE__COMPAT
                if (YAXCHK(yax_getbunch(searchsector, AIMING_AT_FLOOR) < 0))
#endif
                {
                    while (updownunits--)
                        AIMED_CEILINGFLOOR(xpanning) = changechar(AIMED_CEILINGFLOOR(xpanning), changedir, smooshyalign, 0);
                    silentmessage("Sector %d %s panning: %d, %d", searchsector, typestr[searchstat],
                                  AIMED_CEILINGFLOOR(xpanning), AIMED_CEILINGFLOOR(ypanning));
                    asksave = 1;
                }
            }
            else if (AIMING_AT_SPRITE)
            {
                static int32_t sumxvect=0, sumyvect=0;

                if (mouseaction==1)
                    mouseaction_movesprites(&sumxvect, &sumyvect, 1536, g_mousePos.x);
                else
                {
                    sumxvect = sumyvect = 0;
                    if (mouseaction==2)
                        changedir *= -1;
                    while (updownunits--)
                        sprite[searchwall].xrepeat = changechar(sprite[searchwall].xrepeat, changedir, smooshyalign, 1);
                    if (sprite[searchwall].xrepeat < 4)
                        sprite[searchwall].xrepeat = 4;
                    silentmessage("Sprite %d repeat: %d, %d", searchwall,
                                  TrackerCast(sprite[searchwall].xrepeat),
                                  TrackerCast(sprite[searchwall].yrepeat));
                }
            }
            asksave = 1;
            repeatcountx = max(1,repeatcountx-2);
        }
        repeatcountx += synctics;
    }
    else
        repeatcountx = 0;


    ////////////////////
    updownunits=1;
    mouseaction=0;

    if ((bstatus&1) && !AIMING_AT_CEILING_OR_FLOOR)
    {
        if (eitherSHIFT)
        {
            mskip=1;
            if (g_mousePos.y)
            {
                mouseaction = 1;
                updownunits = klabs(g_mousePos.y);

                if (!AIMING_AT_SPRITE)
                    updownunits = klabs((int32_t)(g_mousePos.y*128./tilesiz[wall[searchwall].picnum].y));
            }
        }
        else if (eitherCTRL && !eitherALT)
        {
            mskip=1;
            if (g_mousePos.y)
            {
                mouseaction = 2;
                repeatpanalign = 0;
                mouseay += g_mousePos.y;
                updownunits = klabs(mouseay)/(32 - 28*AIMING_AT_SPRITE);
                if (updownunits)
                    mouseay=0;
            }
        }
    }

    if (!g_mouseBits)
    {
        mouseax=0;
        mouseay=0;
    }

    if (keystatus[KEYSC_gUP] || keystatus[KEYSC_gDOWN] || mouseaction)  // 2 & 8 (keypad)
    {
        if (repeatcounty == 0 || repeatcounty > 32 || mouseaction)
        {
            changedir = 0;
            if (keystatus[KEYSC_gUP]   || g_mousePos.y>0)
                changedir = -1;
            if (keystatus[KEYSC_gDOWN] || g_mousePos.y<0)
                changedir = 1;

            if (AIMING_AT_WALL_OR_MASK)
            {
                if (repeatpanalign == 0)
                {
                    while (updownunits--)
                        wall[searchwall].yrepeat = changechar(wall[searchwall].yrepeat, changedir, smooshyalign, 1);
                    silentmessage("Wall %d repeat: %d, %d", searchwall,
                                  TrackerCast(wall[searchwall].xrepeat),
                                  TrackerCast(wall[searchwall].yrepeat));
                }
                else
                {
                    int16_t w = SELECT_WALL();
                    if (mouseaction && eitherCTRL)
                        updownunits *= 8;
                    while (updownunits--)
                        wall[w].ypanning = changechar(wall[w].ypanning, changedir, smooshyalign, 0);
                    silentmessage("Wall %d panning: %d, %d", w,
                                  TrackerCast(wall[w].xpanning), TrackerCast(wall[w].ypanning));
                }
            }
            else if (AIMING_AT_CEILING_OR_FLOOR)
            {
                {
                    while (updownunits--)
                        AIMED_CEILINGFLOOR(ypanning) = changechar(AIMED_CEILINGFLOOR(ypanning), changedir, smooshyalign, 0);
                    silentmessage("Sector %d %s panning: %d, %d", searchsector, typestr[searchstat],
                                  AIMED_CEILINGFLOOR(xpanning), AIMED_CEILINGFLOOR(ypanning));
                    asksave = 1;
                }
            }
            else if (AIMING_AT_SPRITE)
            {
                static int32_t sumxvect=0, sumyvect=0;

                if (mouseaction==1)
                    mouseaction_movesprites(&sumxvect, &sumyvect, 2048, g_mousePos.y);
                else
                {
                    sumxvect = sumyvect = 0;
                    while (updownunits--)
                        sprite[searchwall].yrepeat = changechar(sprite[searchwall].yrepeat, changedir, smooshyalign, 1);
                    if (sprite[searchwall].yrepeat < 4)
                        sprite[searchwall].yrepeat = 4;
                    silentmessage("Sprite %d repeat: %d, %d", searchwall,
                                  TrackerCast(sprite[searchwall].xrepeat),
                                  TrackerCast(sprite[searchwall].yrepeat));
                }
            }
            asksave = 1;
            repeatcounty = max(1,repeatcounty-2);
        }
        repeatcounty += synctics;
    }
    else
        repeatcounty = 0;

    ////////////////////

    if (PRESSED_KEYSC(F11))  //F11 - brightness
    {
        static int16_t brightness = -1;

        if (brightness==-1)
            brightness = ((int16_t)((g_videoGamma-1.0)*10.0))&15;

        brightness = brightness + (1-2*eitherSHIFT);
        brightness &= 15;

        g_videoGamma = 1.0 + ((float)brightness / 10.0);
        videoSetPalette(brightness, 0, 0);
        message("Brightness: %d/16", brightness+1);
    }

    if (PRESSED_KEYSC(TAB))  //TAB
    {
        if (ASSERT_AIMING)
        {
            tempshade = AIMED_CF_SEL(shade);
            temppal = AIMED_CF_SEL(pal);
            templotag = AIMED_SEL(lotag);
            temphitag = AIMED_SEL(hitag);
            tempextra = AIMED_SEL(extra);

            if (AIMING_AT_WALL_OR_MASK)
            {
#ifdef YAX_ENABLE__COMPAT
                if (yax_getnextwall(searchwall, YAX_CEILING) >= 0)
                    templotag = 0;
                if (yax_getnextwall(searchwall, YAX_FLOOR) >= 0)
                    tempextra = -1;
#endif
                temppicnum = AIMED_SELOVR_WALL(picnum);
                tempxrepeat = AIMED_SEL_WALL(xrepeat);
                tempxrepeat = max<int>(1, tempxrepeat);
                tempyrepeat = AIMED_SEL_WALL(yrepeat);
                tempxpanning = AIMED_SEL_WALL(xpanning);
                tempypanning = AIMED_SEL_WALL(ypanning);
#ifdef NEW_MAP_FORMAT
                tempcstat = AIMED_SEL_WALL(cstat);
#else
                tempcstat = AIMED_SEL_WALL(cstat) & ~YAX_NEXTWALLBITS;
#endif
                templenrepquot = getlenbyrep(wallength(searchwall), tempxrepeat);

                tempsectornum = sectorofwall(searchwall);

                tempstatnum = 0;
                tempblend = 0;
            }
            else if (AIMING_AT_CEILING_OR_FLOOR)
            {
                temppicnum = AIMED_CEILINGFLOOR(picnum);
                tempvis = sector[searchsector].visibility;
                tempxrepeat = AIMED_CEILINGFLOOR(xpanning);
                tempyrepeat = AIMED_CEILINGFLOOR(ypanning);
#ifdef YAX_ENABLE__COMPAT
                if (yax_getbunch(searchsector, AIMING_AT_FLOOR) >= 0)
                    tempxrepeat = 0;
#endif

#ifdef NEW_MAP_FORMAT
                tempcstat = AIMED_CEILINGFLOOR(stat);
#else
                tempcstat = AIMED_CEILINGFLOOR(stat) & ~YAX_BIT;
#endif
                tempsectornum = searchsector;

                tempstatnum = 0;
                tempblend = 0;
            }
            else if (AIMING_AT_SPRITE)
            {
                temppicnum = sprite[searchwall].picnum;
                tempxrepeat = sprite[searchwall].xrepeat;
                tempyrepeat = sprite[searchwall].yrepeat;
                tempcstat = sprite[searchwall].cstat;
                tempxvel = sprite[searchwall].xvel;
                tempyvel = sprite[searchwall].yvel;
                tempzvel = sprite[searchwall].zvel;

                tempstatnum = sprite[searchwall].statnum;
                tempblend = sprite[searchwall].blend;

                tempsectornum = -1;
            }

            somethingintab = searchstat;
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(ENTER)) // ' ENTER
    {
        if (AIMED_SELOVR_PICNUM != temppicnum)
        {
            AIMED_SELOVR_PICNUM = temppicnum;
            asksave = 1;
        }

        if (AIMING_AT_SPRITE)
            correct_sprite_yoffset(searchwall);

        message("Pasted picnum only");
    }
    else if (keystatus[KEYSC_SEMI] && PRESSED_KEYSC(ENTER) && AIMING_AT_CEILING_OR_FLOOR)  // ; ENTER
    {
        if ((unsigned)tempsectornum >= (unsigned)numsectors)
        {
            message("Can't align sector %d's %s, have no reference sector",
                    searchsector, typestr[searchstat]);
        }
#ifdef YAX_ENABLE__COMPAT
        else if (yax_getbunch(searchsector, AIMING_AT_FLOOR) >= 0)
        {
            yax_invalidop();
        }
#endif
        else if (tempsectornum == searchsector)
        {
            message("Didn't align sector %d with itself as reference", tempsectornum);
        }
        else
        {
            // auto-align ceiling/floor
            const int32_t refwall = sector[tempsectornum].wallptr;
            const int32_t ourwall = sector[searchsector].wallptr;

            const vec2_t vecw1 = WALLVEC_INITIALIZER(refwall);
            const vec2_t vecw2 = WALLVEC_INITIALIZER(ourwall);
            const vec2_t vecw2r90 = { -vecw2.y, vecw2.x };  // v, rotated 90 deg CW

            const int32_t bits = CEILINGFLOOR(tempsectornum, stat)&(64+32+16+8+4);
            const int32_t tile = CEILINGFLOOR(tempsectornum, picnum);

            if ((CEILINGFLOOR(tempsectornum, stat)&64) == 0)
            {
                // world-aligned texture

                if (tilesiz[tile].x<=0 || tilesiz[tile].y<=0)
                {
                    message("Can't align sector %d's %s, reference sector %d's has void tile",
                            searchsector, typestr[searchstat], tempsectornum);
                }
                else
                {
                    // non-smooshed: 1 texel corresponds to 16 BUILD units
                    int32_t dx = (wall[ourwall].x-wall[refwall].x)<<4;
                    int32_t dy = (wall[ourwall].y-wall[refwall].y)<<4;

                    dx >>= ((picsiz[tile]&15) - !!(bits&8));
                    dy >>= ((picsiz[tile]>>4) - !!(bits&8));

                    CEILINGFLOOR(searchsector, xpanning) = CEILINGFLOOR(tempsectornum, xpanning) + dy;
                    CEILINGFLOOR(searchsector, ypanning) = CEILINGFLOOR(tempsectornum, ypanning) + dx;

                    CEILINGFLOOR(searchsector, stat) &= ~(64+32+16+8+4);
                    CEILINGFLOOR(searchsector, stat) |= bits;

                    message("Aligned sector %d's %s with sector %d's",
                            searchsector, typestr[searchstat], tempsectornum);
                }
            }
            else
            {
                // firstwall-aligned texture

                const int64_t a = lldotv2(&vecw1, &vecw2);
                const int64_t b = lldotv2(&vecw1, &vecw2r90);

                if (a!=0 && b!=0)
                {
                    message("Walls %d and %d are nether parallel nor perpendicular",
                            refwall, ourwall);
                }
                else
                {
                    // 0<-6<-3<-5<-0, 1<-7<-2<-4<-1 (mirrored/not mirrored separately)
                    static const int8_t prev3each[8] = { 5, 4, 7, 6, 2, 3, 0, 1 };

                    const int32_t degang = 90*(b<0) + 180*(a<0) + 270*(b>0);
                    int32_t i, tempbits = (bits&4) + ((bits>>4)&3);

//message("Wall %d is rotated %d degrees CW wrt wall %d", ourwall, degang, refwall);
                    CEILINGFLOOR(searchsector, stat) &= ~(64+32+16+8+4);
                    for (i=0; i<degang/90; i++)
                        tempbits = prev3each[tempbits];
                    tempbits = (tempbits&4) + ((tempbits&3)<<4);
                    CEILINGFLOOR(searchsector, stat) |= 64 + (bits&8) + tempbits;

                    {
                        const walltype *rw = &wall[refwall], *rw2 = &POINT2(refwall);
                        const walltype *ow = &wall[ourwall], *ow2 = &POINT2(ourwall);
                        int32_t intx, inty, sign12, sign34;

                        if (b != 0)  // perpendicular
                            inflineintersect(rw->x,rw->y, rw2->x,rw2->y, ow->x,ow->y, ow2->x,ow2->y,
                                             &intx,&inty, &sign12,&sign34);
                        else  // parallel
                            inflineintersect(rw->x,rw->y, rw2->x,rw2->y,
                                             ow->x,ow->y, ow->x+vecw2r90.x,ow->y+vecw2r90.y,
                                             &intx,&inty, &sign12,&sign34);
                        if (sign12 == 0)
                        {
                            message("INTERNAL ERROR: Couldn't get intersection"
                                    " of reference and alignee walls");
                        }
                        else
                        {
                            double dx = (double)(rw->x-intx)*(rw->x-intx) + (double)(rw->y-inty)*(rw->y-inty);
                            double dy = (double)(ow->x-intx)*(ow->x-intx) + (double)(ow->y-inty)*(ow->y-inty);

                            dx = -sign12 * sqrt(dx) * 16;
                            dy = sign34 * sqrt(dy) * 16;
                            if (a < 0 || b > 0)
                                dx = -dx;
                            dx /= 1<<((picsiz[tile]&15) - !!(bits&8));
                            dy /= 1<<((picsiz[tile]>>4) - !!(bits&8));
//LOG_F(INFO, "int=(%d,%d), dx=%.03f dy=%.03f", intx,inty, dx, dy);
                            CEILINGFLOOR(searchsector, xpanning) =
                                CEILINGFLOOR(tempsectornum, xpanning) + (int32_t)dx;
                            CEILINGFLOOR(searchsector, ypanning) =
                                CEILINGFLOOR(tempsectornum, ypanning) + (int32_t)dy;

                            message("%sAligned sector %d %s (firstwall-relative) with sector %d's",
                                    (CEILINGFLOOR(tempsectornum, stat)&(32+16+4)) ? "(TODO) ":"",
                                    searchsector, typestr[searchstat], tempsectornum);
                        }
                    }
                }
            }
        }
    }

    if (PRESSED_KEYSC(ENTER))  // ENTER -- paste clipboard contents
    {
        if (eitherSHIFT)
        {
            if (AIMING_AT_WALL_OR_MASK && eitherCTRL)  //Ctrl-shift Enter (auto-shade)
            {
                int16_t daang;
                int8_t dashade[2] = { 127, -128 };

                i = searchwall;
                do
                {
                    dashade[0] = min(dashade[0], TrackerCast(wall[i].shade));
                    dashade[1] = max(dashade[1], TrackerCast(wall[i].shade));

                    i = wall[i].point2;
                }
                while (i != searchwall);

                daang = getangle(POINT2(searchwall).x-wall[searchwall].x, POINT2(searchwall).y-wall[searchwall].y);

                i = searchwall;
                do
                {
                    j = getangle(POINT2(i).x-wall[i].x,POINT2(i).y-wall[i].y);
                    k = (j+2048-daang)&2047;
                    if (k > 1024)
                        k = 2048-k;

                    wall[i].shade = dashade[0]+mulscale10(k, dashade[1]-dashade[0]);

                    i = wall[i].point2;
                }
                while (i != searchwall);

                message("Auto-shaded wall %d's loop", searchwall);
                asksave = 1;
            }
            else if (somethingintab < 255)
            {
                if (ASSERT_AIMING)
                {
                    AIMED_CF_SEL(shade) = tempshade;
                    AIMED_CF_SEL(pal) = temppal;

                    k = 0;
                    if (AIMING_AT_CEILING_OR_FLOOR)
                    {
                        if (somethingintab == SEARCH_CEILING || somethingintab == SEARCH_FLOOR)
                            k=1, sector[searchsector].visibility = tempvis;
                    }
                    else if (AIMING_AT_SPRITE)
                        sprite[searchwall].blend = tempblend;

                    message("Pasted shade+pal%s", k?"+visibility":"");
                    asksave = 1;
                }
            }
        }
        else if (AIMING_AT_WALL_OR_MASK && eitherCTRL && somethingintab < 255)  //Either ctrl key
        {
            int32_t clipboard_has_wall = (somethingintab == SEARCH_WALL || somethingintab == SEARCH_MASKWALL);
            i = searchwall;
            do
            {
                wall[i].picnum = temppicnum;
                wall[i].shade = tempshade;
                wall[i].pal = temppal;

                if (clipboard_has_wall)
                {
                    wall[i].xrepeat = tempxrepeat;
                    wall[i].yrepeat = tempyrepeat;

                    wall[i].xpanning = tempxpanning;
                    wall[i].ypanning = tempypanning;

                    wall[i].cstat &= ~(4 + 1+64 + 8+256 + 4096);
                    wall[i].cstat |= (tempcstat & (4 + 1+64 + 8+256 + 4096));

                    fixxrepeat(i, templenrepquot);
                }

                i = wall[i].point2;
            }
            while (i != searchwall);

            message("Pasted picnum+shade+pal%s to wall %d's loop",
                    clipboard_has_wall?"+pixelwidth":"", searchwall);
            asksave = 1;
        }
        else if (AIMING_AT_CEILING_OR_FLOOR && eitherCTRL && somethingintab < 255)  //Either ctrl key
        {
            static const char *addnstr[4] = {"", "+stat+panning", "+stat", "+stat + panning (some)"};

            static int16_t sectlist[MAXSECTORS];
            static uint8_t sectbitmap[bitmap_size(MAXSECTORS)];
            int16_t sectcnt, sectnum;

            i = searchsector;
            if (CEILINGFLOOR(i, stat)&1)
            {
                // collect neighboring parallaxed sectors
                bfirst_search_init(sectlist, sectbitmap, &sectnum, MAXSECTORS, i);

                for (sectcnt=0; sectcnt<sectnum; sectcnt++)
                    for (WALLS_OF_SECTOR(sectlist[sectcnt], j))
                    {
                        k = wall[j].nextsector;
                        if (k>=0 && (CEILINGFLOOR(k, stat)&1))
                            bfirst_search_try(sectlist, sectbitmap, &sectnum, wall[j].nextsector);
                    }

                k = 0;
                for (sectcnt=0; sectcnt<sectnum; sectcnt++)
                {
                    i = sectlist[sectcnt];

                    CEILINGFLOOR(i, picnum) = temppicnum;
                    CEILINGFLOOR(i, shade) = tempshade;
                    CEILINGFLOOR(i, pal) = temppal;

                    if (somethingintab == SEARCH_CEILING || somethingintab == SEARCH_FLOOR)
                    {
#ifdef YAX_ENABLE__COMPAT
                        if (yax_getbunch(i, AIMING_AT_FLOOR) >= 0)
                            k |= 2;
                        else
#endif
                        {
                            CEILINGFLOOR(i, xpanning) = tempxrepeat;
                            CEILINGFLOOR(i, ypanning) = tempyrepeat;
                            k |= 1;
                        }

                        SET_PROTECT_BITS(CEILINGFLOOR(i, stat), tempcstat, YAX_BIT_PROTECT);
                    }
                }

                message("Pasted picnum+shade+pal%s to parallaxed sector %ss",
                        addnstr[k], typestr[searchstat]);
                asksave = 1;
            }
#ifdef YAX_ENABLE
            else
            {
                k = yax_getbunch(searchsector, AIMING_AT_FLOOR);
                if (k < 0)
                    goto paste_ceiling_or_floor;

                j = (somethingintab==SEARCH_CEILING || somethingintab==SEARCH_FLOOR);

                for (SECTORS_OF_BUNCH(k,AIMING_AT_FLOOR, i))
                {
                    SECTORFLD(i,picnum, AIMING_AT_FLOOR) = temppicnum;
                    SECTORFLD(i,shade, AIMING_AT_FLOOR) = tempshade;
                    SECTORFLD(i,pal, AIMING_AT_FLOOR) = temppal;
                    if (j)
                        SET_PROTECT_BITS(SECTORFLD(i,stat, AIMING_AT_FLOOR), tempcstat, YAX_BIT_PROTECT);
                }

                message("Pasted picnum+shade+pal%s to sector %ss with bunchnum %d",
                        j?"+stat":"", typestr[searchstat], k);
            }
#endif
        }
        else if (somethingintab < 255)
        {
            // wall/overwall common:
            if (AIMING_AT_WALL_OR_MASK && searchstat==somethingintab &&
                    (!AIMING_AT_WALL || searchwall==searchbottomwall))
            {
                wall[searchwall].xrepeat = tempxrepeat;
                wall[searchwall].yrepeat = tempyrepeat;
                wall[searchwall].xpanning = tempxpanning;
                wall[searchwall].ypanning = tempypanning;

                SET_PROTECT_BITS(wall[searchwall].cstat, tempcstat, ~(4 + 1+64 + 8+256 + 4096));

                wall[searchwall].hitag = temphitag;
#ifdef YAX_ENABLE
                if (yax_getnextwall(searchwall, YAX_CEILING) == -1)
#endif
                    wall[searchwall].lotag = templotag;
#ifdef YAX_ENABLE
                if (yax_getnextwall(searchwall, YAX_FLOOR) == -1)
#endif
                    wall[searchwall].extra = tempextra;

                fixxrepeat(searchwall, templenrepquot);
            }


            if (AIMING_AT_WALL)
            {
                if (searchisbottom)
                    SET_PROTECT_BITS(wall[searchbottomwall].cstat, tempcstat, ~256);

                wall[searchbottomwall].picnum = temppicnum;
                wall[searchbottomwall].shade = tempshade;
                wall[searchbottomwall].pal = temppal;
            }
            else if (AIMING_AT_MASKWALL)
            {
                wall[searchwall].overpicnum = temppicnum;
                if (wall[searchwall].nextwall >= 0)
                    NEXTWALL(searchwall).overpicnum = temppicnum;

                wall[searchwall].shade = tempshade;
                wall[searchwall].pal = temppal;
            }
            else if (AIMING_AT_CEILING_OR_FLOOR)
            {
#ifdef YAX_ENABLE
paste_ceiling_or_floor:
#endif
                AIMED_CEILINGFLOOR(picnum) = temppicnum;
                AIMED_CEILINGFLOOR(shade) = tempshade;
                AIMED_CEILINGFLOOR(pal) = temppal;

                if (somethingintab == SEARCH_CEILING || somethingintab == SEARCH_FLOOR)
                {
#ifdef YAX_ENABLE
                    if (yax_getbunch(searchsector, AIMING_AT_FLOOR) < 0)
#endif
                        AIMED_CEILINGFLOOR(xpanning) = tempxrepeat;
                    AIMED_CEILINGFLOOR(ypanning) = tempyrepeat;

                    SET_PROTECT_BITS(AIMED_CEILINGFLOOR(stat), tempcstat, YAX_BIT_PROTECT|2);

                    sector[searchsector].visibility = tempvis;
                    sector[searchsector].lotag = templotag;
                    sector[searchsector].hitag = temphitag;
                    sector[searchsector].extra = tempextra;
                }
            }
            else if (AIMING_AT_SPRITE)
            {
                sprite[searchwall].picnum = temppicnum;

                if (tilesiz[temppicnum].x <= 0 || tilesiz[temppicnum].y <= 0)
                {
                    j = 0;
                    for (k=0; k<MAXTILES; k++)
                        if (tilesiz[k].x > 0 && tilesiz[k].y > 0)
                        {
                            j = k;
                            break;
                        }
                    sprite[searchwall].picnum = j;
                }

                sprite[searchwall].shade = tempshade;
                sprite[searchwall].pal = temppal;

                if (somethingintab == SEARCH_SPRITE)
                {
                    sprite[searchwall].xrepeat = max(tempxrepeat, 1u);
                    sprite[searchwall].yrepeat = max(tempyrepeat, 1u);
                    sprite[searchwall].cstat = tempcstat;
                    sprite[searchwall].lotag = templotag;
                    sprite[searchwall].hitag = temphitag;
                    sprite[searchwall].extra = tempextra;
                    sprite[searchwall].xvel = tempxvel;
                    sprite[searchwall].yvel = tempyvel;
                    sprite[searchwall].zvel = tempzvel;
                    changespritestat(searchwall, tempstatnum);
                    sprite[searchwall].blend = tempblend;
                }
                else
                    correct_sprite_yoffset(searchwall);
            }

            message("Pasted clipboard");
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(C))
    {
        if (eitherALT)  // Alt-C  picnum replacer
        {
            if (ASSERT_AIMING && somethingintab < 255)
            {
                switch (searchstat)
                {
                case SEARCH_WALL:
                    j = wall[searchbottomwall].picnum;
                    for (i=0; i<numwalls; i++)
                        if (wall[i].picnum == j)
                            wall[i].picnum = temppicnum;
                    break;
                case SEARCH_CEILING:
                case SEARCH_FLOOR:
                    j = AIMED_CEILINGFLOOR(picnum);
                    for (i=0; i<numsectors; i++)
                        if (CEILINGFLOOR(i, picnum) == j)
                            CEILINGFLOOR(i, picnum) = temppicnum;
                    break;
                case SEARCH_SPRITE:
                    j = sprite[searchwall].picnum;
                    for (i=0; i<MAXSPRITES; i++)
                        if (sprite[i].statnum < MAXSTATUS)
                            if (sprite[i].picnum == j)
                            {
                                sprite[i].picnum = temppicnum;
                                correct_sprite_yoffset(i);
                            }
                    break;
                case SEARCH_MASKWALL:
                    j = wall[searchwall].overpicnum;
                    for (i=0; i<numwalls; i++)
                        if (wall[i].overpicnum == j)
                            wall[i].overpicnum = temppicnum;
                    break;
                default:
                    j = -1;
                    break;
                }

                if (j>=0)
                    message("Replaced %ss with picnum %d to picnum %d",
                            typestr[searchstat], j, temppicnum);
                asksave = 1;
            }
        }
        else  //C
        {
            if (AIMING_AT_SPRITE)
            {
                sprite[searchwall].cstat ^= 128;
                message("Sprite %d center bit %s",searchwall, ONOFF((sprite[searchwall].cstat&128)));
                asksave = 1;
            }
        }
    }

    if (PRESSED_KEYSC(SLASH))  // /?     Reset panning&repeat to 0
    {
        if (AIMING_AT_WALL_OR_MASK)
        {
            int16_t w = SELECT_WALL();
            wall[w].xpanning = 0;
            wall[w].ypanning = 0;
            wall[w].xrepeat = 8;
            wall[w].yrepeat = 8;
#ifdef NEW_MAP_FORMAT
            wall[w].cstat = 0;
#else
            wall[w].cstat &= YAX_NEXTWALLBITS;
#endif
            fixrepeats(searchwall);
        }
        else if (AIMING_AT_CEILING_OR_FLOOR)
        {
#ifdef YAX_ENABLE
            int16_t bunchnum = yax_getbunch(searchsector, AIMING_AT_FLOOR);
# if !defined NEW_MAP_FORMAY
            if (bunchnum < 0)
# endif
#endif
                AIMED_CEILINGFLOOR(xpanning) = 0;
            AIMED_CEILINGFLOOR(ypanning) = 0;
            AIMED_CEILINGFLOOR(stat) &= ~2;
            AIMED_CEILINGFLOOR(heinum) = 0;
#ifdef YAX_ENABLE
            if (bunchnum >= 0)
                for (SECTORS_OF_BUNCH(bunchnum,!AIMING_AT_FLOOR, i))
                {
                    SECTORFLD(i,stat, !AIMING_AT_FLOOR) &= ~2;
                    SECTORFLD(i,heinum, !AIMING_AT_FLOOR) = 0;
                }
#endif
        }
        else if (AIMING_AT_SPRITE)
        {
            if (eitherSHIFT)
                sprite[searchwall].xrepeat = sprite[searchwall].yrepeat;
            else
            {
                sprite[searchwall].xrepeat = 64;
                sprite[searchwall].yrepeat = 64;
            }

            correct_sprite_yoffset(searchwall);
        }

        if (ASSERT_AIMING)
        {
            message("%s's size and panning reset", Typestr[searchstat]);
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(P))  // P (parallaxing sky)
    {
        if (eitherCTRL)
        {
            parallaxtype = (parallaxtype+1)%3;
            message("Parallax type %d", parallaxtype);
        }
        else if (eitherALT)
        {
            if (ASSERT_AIMING)
            {
                Bsprintf(tempbuf, "%s pal: ", Typestr[searchstat]);
                getnumberptr256(tempbuf, &AIMED_CF_SEL(pal), sizeof(uint8_t), M32_MAXPALOOKUPS, 0, NULL);
                asksave = 1;
            }
        }
        else
        {
            if (AIMING_AT_WALL_OR_MASK || AIMING_AT_CEILING)
            {
                sector[searchsector].ceilingstat ^= 1;
                message("Sector %d ceiling parallax bit %s",searchsector, ONOFF(sector[searchsector].ceilingstat&1));
                asksave = 1;
            }
            else if (AIMING_AT_FLOOR)
            {
                sector[searchsector].floorstat ^= 1;
                message("Sector %d floor parallax bit %s",searchsector, ONOFF(sector[searchsector].floorstat&1));
                asksave = 1;
            }
        }
    }

    if (PRESSED_KEYSC(D))   //Alt-D  (adjust sprite[].clipdist)
    {
        if (eitherALT && AIMING_AT_SPRITE)
            sprite[searchwall].clipdist = getnumber256("Sprite clipdist: ", sprite[searchwall].clipdist, 255, 0);
    }

    VM_OnEvent(EVENT_KEYS3D, -1);
}// end 3d

// returns: whether sprite is out of grid
static int32_t jump_to_sprite(int32_t spritenum)
{
    auto const spr = &sprite[spritenum];

    if (pos.x >= -editorgridextent && pos.x <= editorgridextent &&
            pos.y >= -editorgridextent && pos.y <= editorgridextent)
    {
        pos.x = spr->x;
        pos.y = spr->y;

        // BZ_MAX?
        if (pos.z >= -(editorgridextent<<4) && pos.z <= editorgridextent<<4)
            pos.z = spr->z;

        ang = spr->ang;

        if ((unsigned)spr->sectnum < (unsigned)numsectors)
            cursectnum = spr->sectnum;

        return 0;
    }

    return 1;
}

static void DoSpriteSearch(int32_t dir)  // <0: backwards, >=0: forwards
{
    char did_wrap = 0, outofgrid;
    int32_t i, j, k = 0;

    dir = 1-2*(dir<0);

    for (gs_cursprite += dir;; gs_cursprite += dir)
    {
        if (gs_cursprite < 0 || gs_cursprite >= MAXSPRITES)
        {
            if (did_wrap)
                break;

            did_wrap = 1;
            gs_cursprite &= (MAXSPRITES-1);
        }

        if (sprite[gs_cursprite].statnum == MAXSTATUS)
            continue;

        for (i=0; i<3; i++)
            for (j=0; j<7; j++)
            {
                if (!gs_spriteTagInterested[i][j])
                    continue;

                if (i==0)
                {
                    switch (j)
                    {
                    case 0: k = sprite[gs_cursprite].x; break;
                    case 1: k = sprite[gs_cursprite].y; break;
                    case 2: k = sprite[gs_cursprite].z; break;
                    case 3: k = sprite[gs_cursprite].sectnum; break;
                    case 4: k = sprite[gs_cursprite].statnum; break;
                    case 5: k = (uint16_t)sprite[gs_cursprite].hitag; break;
                    case 6: k = (uint16_t)sprite[gs_cursprite].lotag; break;
                    }
                }
                else if (i==1)
                {
                    switch (j)
                    {
                    case 0:
                        k = sprite[gs_cursprite].cstat;
                        k &= gs_spriteTagValue[1][j];
                        break;
                    case 1: k = sprite[gs_cursprite].shade; break;
                    case 2: k = sprite[gs_cursprite].pal; break;
                    case 3: k = sprite[gs_cursprite].blend; break;
                    case 4:
                        k = gs_spriteTagValue[1][j];
                        if (k != sprite[gs_cursprite].xrepeat &&
                                k != sprite[gs_cursprite].yrepeat)
                            goto NEXTSPRITE;
                        break;
                    case 5:
                        k = gs_spriteTagValue[1][j];
                        if (k != sprite[gs_cursprite].xoffset &&
                                k != sprite[gs_cursprite].yoffset)
                            goto NEXTSPRITE;
                        break;
                    case 6: k = sprite[gs_cursprite].picnum; break;
                    }
                }
                else if (i==2)
                {
                    switch (j)
                    {
                    case 0: k = sprite[gs_cursprite].ang; break;
                    case 1: k = (uint16_t)sprite[gs_cursprite].xvel; break;
                    case 2: k = (uint16_t)sprite[gs_cursprite].yvel; break;
                    case 3: k = (uint16_t)sprite[gs_cursprite].zvel; break;
                    case 4: k = (uint16_t)sprite[gs_cursprite].owner; break;
                    case 5: k = sprite[gs_cursprite].clipdist; break;
                    case 6: k = sprite[gs_cursprite].extra; break;
                    }
                }

                if (k != gs_spriteTagValue[i][j])
                    goto NEXTSPRITE;
            }

        // found matching sprite
        outofgrid = jump_to_sprite(gs_cursprite);

        printmessage16("%s Sprite seach%s: found sprite %d%s", dir<0 ? "<" : ">",
                       did_wrap ? " (wrap)" : "", gs_cursprite, outofgrid?"(outside grid)":"");
        return;

NEXTSPRITE:
        ;
    }
    printmessage16("%s Sprite search: none found", dir<0 ? "<" : ">");
}

////////// KEY PRESS HANDLER IN 2D MODE //////////
static void Keys2d(void)
{
    int32_t i=0, j, k;
    int32_t smooshy, changedir;
    static int32_t repeatcnt[2] = {0,0};  // was repeatcountx, repeatcounty
    int32_t tcursectornum;

//    for(i=0;i<0x50;i++) if(keystatus[i]==1) Bsprintf(tempbuf,"key %d",i); printmessage16(tempbuf);

    tcursectornum = -1;
    for (i=0; i<numsectors; i++)
    {
        YAX_SKIPSECTOR(i);
        if (inside_editor_curpos(i) == 1)
        {
            tcursectornum = i;
            break;
        }
    }
    searchsector = tcursectornum;

    if (keystatus[KEYSC_TAB])  //TAB
    {
        if (eitherCTRL)
        {
            g_fillCurSector = !g_fillCurSector;
            silentmessage("Fill currently pointed-at sector: %s", ONOFF(g_fillCurSector));
            keystatus[KEYSC_TAB] = 0;
        }
        else if (eitherSHIFT)
        {
            if (pointhighlight >= 16384)
            {
                drawgradient();
                showspritedata(pointhighlight&16383, 0);
            }
            else if (linehighlight >= 0 /* && ((bstatus&1) || sectorofwall(linehighlight)==tcursectornum)*/)
            {
                drawgradient();
                showwalldata(linehighlight, 0);
            }
        }
        else if (tcursectornum >= 0)
        {
            drawgradient();
            showsectordata(tcursectornum, 0);
        }
    }
    else if (!(keystatus[KEYSC_F5]|keystatus[KEYSC_F6]|keystatus[KEYSC_F7]|keystatus[KEYSC_F8]) && !eitherSHIFT)
    {
        static int32_t counter = 0;
        static int32_t omx = 0, omy = 0;
        /*
          static int32_t opointhighlight, olinehighlight, ocursectornum;
          if (pointhighlight == opointhighlight && linehighlight == olinehighlight && tcursectornum == ocursectornum)
        */
        if (omx == mousxplc && omy == mousyplc)
        {
            if (counter < 6)
                counter++;
        }
        else if (counter > 0)
            counter--;

        omx = mousxplc;
        omy = mousyplc;

        /*
          opointhighlight = pointhighlight;
          olinehighlight = linehighlight;
          ocursectornum = tcursectornum;
        */

        if (totalclock < lastpm16time + 120*2)
            _printmessage16("%s", lastpm16buf);
        else if (counter >= 2 && totalclock >= 120*6)
        {
            if (pointhighlight >= 16384)
            {
                i = pointhighlight-16384;
                showspritedata(i, 1);

                // NUKE-TODO:
                //if (sprite[i].picnum==SECTOREFFECTOR)
                //    _printmessage16("^10%s", SectorEffectorText(i));
            }
            else if (linehighlight >= 0 && ((bstatus&1) || sectorofwall(linehighlight)==tcursectornum))
                showwalldata(linehighlight, 1);
            else if (tcursectornum >= 0)
                showsectordata(tcursectornum, 1);
        }
    }

///__bigcomment__

    if ((i=tcursectornum)>=0 && g_fillCurSector && (hlsectorbitmap[i>>3]&(1<<(i&7)))==0)
    {
        int32_t col = editorcolors[4];
#ifdef YAX_ENABLE
        if (yax_getbunch(tcursectornum, YAX_FLOOR)>=0)
            col = editorcolors[12];
#endif
        fillsector(tcursectornum, col);
    }

#ifdef YAX_ENABLE
    if (eitherCTRL && PRESSED_KEYSC(U) && tcursectornum>=0)  // Ctrl-U: unlink bunch sectors
    {
        int16_t cf, fb = yax_getbunch(tcursectornum, YAX_FLOOR);
        if (fb >= 0)
        {
            for (SECTORS_OF_BUNCH(fb,YAX_FLOOR, i))
                fillsector_notrans(i, editorcolors[11]);
            fade_editor_screen(editorcolors[11]);

            if (ask_if_sure("Clear all TROR extensions from marked sectors?", 0))
            {
                for (cf=0; cf<2; cf++)
                    for (SECTORS_OF_BUNCH(fb,cf, i))
                        yax_setbunch(i, cf, -1);

                yax_update(0);
                yax_updategrays(pos.z);

                message("Cleared TROR bunch %d", fb);
                asksave = 1;
            }
        }
    }

    if (/*!m32_sideview &&*/ numyaxbunches>0)
    {
        int32_t zsign=0;

        if (PRESSED_KEYSC(PGDN) || (eitherCTRL && PRESSED_KEYSC(DOWN)))
            zsign = 1;
        else if (PRESSED_KEYSC(PGUP) || (eitherCTRL && PRESSED_KEYSC(UP)))
            zsign = -1;

        if (zsign)
        {
            int32_t bestzdiff=INT32_MAX, hiz=0, loz=0, bottomp=0;

            for (i=0; i<numsectors; i++)
            {
                if (yax_getbunch(i, YAX_FLOOR) < 0 /*&& yax_getbunch(i, YAX_CEILING) < 0*/)
                    continue;

                loz = min(loz, TrackerCast(sector[i].floorz));
                hiz = max(hiz, TrackerCast(sector[i].floorz));

                // TODO: see if at least one sector point inside sceeen
                j = (sector[i].floorz-pos.z)*zsign;
                if (j>0 && j < bestzdiff)
                    bestzdiff = j;
            }

            if (bestzdiff==INT32_MAX)
            {
                if (zsign == 1)
                    bottomp=1, bestzdiff = (hiz+(1024<<4) - pos.z);
                else
                    bestzdiff = pos.z - loz;
            }

            pos.z += zsign*bestzdiff;
            yax_updategrays(pos.z);

            printmessage16("Z position: %d%s", pos.z,
                           bottomp ? " (bottom)":(pos.z==loz ? " (top)":""));
            updatesectorz(pos.x, pos.y, pos.z, &cursectnum);
        }
    }

    if (eitherCTRL && PRESSED_KEYSC(A))
    {
        if (eitherALT)
        {
            showinnergray = !showinnergray;
            printmessage16("Display grayed out walls: %s", ONOFF(showinnergray));
        }
        else
        {
            autogray = !autogray;
            printmessage16("Automatic grayout of plain sectors %s", ONOFF(autogray));
            yax_updategrays(pos.z);
        }
    }
#endif

    // Ctrl-R set editor z range to hightlightsectors' c/f bounds
    if (eitherCTRL && PRESSED_KEYSC(R))
    {
        if (highlightsectorcnt <= 0)
        {
            editorzrange[0] = INT32_MIN;
            editorzrange[1] = INT32_MAX;
            printmessage16("Reset Z range");
        }
        else
        {
            int32_t damin=INT32_MAX, damax=INT32_MIN;

            for (i=0; i<highlightsectorcnt; i++)
            {
                damin = min(damin, TrackerCast(sector[highlightsector[i]].ceilingz));
                damax = max(damax, TrackerCast(sector[highlightsector[i]].floorz));
            }

            if (damin < damax)
            {
                editorzrange[0] = damin;
                editorzrange[1] = damax;
                printmessage16("Set Z range to highlighted sector bounds (%d..%d)",
                               editorzrange[0], editorzrange[1]);
            }
        }
        yax_updategrays(pos.z);
    }

    if (PRESSED_KEYSC(T))  // T (tag)
    {
        char buffer[80];

        if (eitherCTRL)  //Ctrl-T
        {
            if (eitherSHIFT)
                showtags--;
            else
                showtags++;
            showtags += 3;
            showtags %= 3;
            printmessage16("Show tags %s", showtags<2?ONOFF(showtags):"LABELED");
        }
        else if (eitherALT)  //ALT
        {
            if (pointhighlight >= 16384)
            {
                i = pointhighlight-16384;
                Bsprintf(buffer,"Sprite (%d) Lo-tag: ", i);
                // NUKE-TODO:
                sprite[i].lotag = _getnumber16(buffer, sprite[i].lotag, BTAG_MAX, 0, /*sprite[i].picnum==SECTOREFFECTOR ?
                                               &SectorEffectorTagText : */NULL);
            }
            else if (linehighlight >= 0)
            {
#ifdef YAX_ENABLE__COMPAT
                if (yax_getnextwall(linehighlight, YAX_CEILING)>=0)
                    message("Can't change lotag in protected wall");
                else
#endif
                {
                    i = linehighlight;
                    j = taglab_linktags(1, i);
                    j = 4*(j&1);
                    Bsprintf(buffer,"Wall (%d) Lo-tag: ", i);
                    wall[i].lotag = getnumber16(buffer, wall[i].lotag, BTAG_MAX, 0+j);
                }
            }
        }
        else
        {
            if (tcursectornum >= 0)
            {
                Bsprintf(buffer,"Sector (%d) Lo-tag: ", tcursectornum);
                sector[tcursectornum].lotag =
                    _getnumber16(buffer, sector[tcursectornum].lotag, BTAG_MAX, 0, &ExtGetSectorType);
            }
        }
    }

    if (PRESSED_KEYSC(F1) || (keystatus[KEYSC_QUOTE] && keystatus[KEYSC_TILDE])) //F1 or ' ~
    {
        // PK_
        if (numhelppages>0)
            IntegratedHelp();
        else
            printmessage16("m32help.hlp invalid or not found!");
    }

    // NUKE-TODO:
    //if (PRESSED_KEYSC(F2))
    //    if (g_numsounds > 0)
    //    {
    //        SoundDisplay();
    //    }

    // F3: side view toggle (handled in build.c)

    getpoint(searchx,searchy, &mousxplc,&mousyplc);
    ppointhighlight = getpointhighlight(mousxplc,mousyplc, ppointhighlight);

    if ((ppointhighlight&0xc000) == 16384)
    {
        //   sprite[ppointhighlight&16383].cstat ^= 1;
        cursprite = ppointhighlight&16383;
    }

    if (keystatus[KEYSC_F9]) // F9 f1=3b
        ShowFileText("sthelp.hlp");

    /* start Mapster32 */

    if (PRESSED_KEYSC(F4))
    {
        showfirstwall = !showfirstwall;
        message("Sector firstwall highlight %s", showfirstwall?"enabled":"disabled");
    }

    if (PRESSED_KEYSC(M))  // M (tag)
    {
        if (eitherALT)  //ALT
        {
            if (pointhighlight >= 16384)
            {
                i = pointhighlight-16384;
                Bsprintf(tempbuf, "Sprite %d Extra: ", i);
                sprite[i].extra = getnumber16(tempbuf, sprite[i].extra, BTAG_MAX, 1);
            }
            else if (linehighlight >= 0)
            {
#ifdef YAX_ENABLE__COMPAT
                if (yax_getnextwall(linehighlight, YAX_FLOOR)>=0)
                    message("Can't change extra in protected wall");
                else
#endif
                {
                    i = linehighlight;
                    Bsprintf(tempbuf,"Wall %d Extra: ",i);
                    wall[i].extra = getnumber16(tempbuf,wall[i].extra,BTAG_MAX,1);
                }
            }
        }
        else
        {
            if (tcursectornum >= 0)
            {
                Bsprintf(tempbuf,"Sector %d Extra: ",tcursectornum);
                sector[tcursectornum].extra = getnumber16(tempbuf,sector[tcursectornum].extra,BTAG_MAX,1);
            }
        }
    }

    if (!eitherCTRL && PRESSED_KEYSC(E))  // E (expand)
    {
        if (tcursectornum >= 0)
        {
            sector[tcursectornum].floorstat ^= 8;
            message("Sector %d floor texture expansion bit %s", tcursectornum,
                    ONOFF(sector[tcursectornum].floorstat&8));
            asksave = 1;
        }
    }

    if (PRESSED_KEYSC(SLASH))  // /     Reset panning&repeat to 0
    {
        if ((ppointhighlight&0xc000) == 16384)
        {
            if (eitherSHIFT)
                sprite[cursprite].xrepeat = sprite[cursprite].yrepeat;
            else
                sprite[cursprite].xrepeat = sprite[cursprite].yrepeat = 64;
            asksave = 1;
        }
        else if (graphicsmode != 0)
        {
            i = tcursectornum;

            if (i >= 0)
            {
#ifdef YAX_ENABLE
                if (yax_getbunch(i, YAX_FLOOR) < 0)
#endif
                    sector[i].floorxpanning = 0;
                sector[i].floorypanning = 0;
                message("Sector %d floor panning reset", i);
                asksave = 1;
            }
        }
    }

    for (k=0; k<2; k++)    // panning/repeat
    {
        if (k==0)
            j = (keystatus[KEYSC_gLEFT]<<1)|keystatus[KEYSC_gRIGHT];  // 4 & 6 (keypad 2D)
        else
            j = (keystatus[KEYSC_gUP]<<1)|keystatus[KEYSC_gDOWN];  // 2 & 8 (keypad 2D)

        if (j)
        {
            smooshy = keystatus[KEYSC_gKP5];

            if (repeatcnt[k] == 0 || repeatcnt[k] > 32)
            {
                changedir = 1-(j&2);

                if ((ppointhighlight&0xc000) == 16384 && (sprite[cursprite].cstat & 48))
                {
                    uint8_t *repeat = (k==0) ? &sprite[cursprite].xrepeat : &sprite[cursprite].yrepeat;
                    *repeat = max<uint8_t>(4, changechar(*repeat, changedir, smooshy, 1));
                    silentmessage("Sprite %d repeat: %d, %d", cursprite,
                        TrackerCast(sprite[cursprite].xrepeat),
                        TrackerCast(sprite[cursprite].yrepeat));
                }
                else if (graphicsmode)
                {
                    i = tcursectornum;

                    if (i >= 0)
#ifdef YAX_ENABLE
                        if (k==1 || yax_getbunch(i, YAX_FLOOR) < 0)
#endif
                        {
                            uint8_t *panning = (k==0) ? &sector[i].floorxpanning : &sector[i].floorypanning;
                            *panning = changechar(*panning, changedir, smooshy, 0);
                            silentmessage("Sector %d floor panning: %d, %d", searchsector,
                                TrackerCast(sector[i].floorxpanning),
                                TrackerCast(sector[i].floorypanning));
                        }
                }

                asksave = 1;
                repeatcnt[k] = max(1, repeatcnt[k]-2);
            }
            repeatcnt[k] += synctics;
        }
        else
            repeatcnt[k] = 0;
    }

    if (PRESSED_KEYSC(R))  // R (relative alignment, rotation)
    {
        if (pointhighlight >= 16384)
            toggle_sprite_alignment(cursprite);
        else if (tcursectornum >= 0 && graphicsmode)
        {
            sector[tcursectornum].floorstat ^= 64;
            message("Sector %d floor texture relativity bit %s", searchsector,
                    ONOFF(sector[tcursectornum].floorstat&64));
            asksave = 1;
        }
    }

    if (keystatus[KEYSC_QUOTE] && keystatus[KEYSC_S]) // ' S
    {
        if (pointhighlight >= 16384)
        {
            keystatus[KEYSC_S] = 0;
            Bsprintf(tempbuf, "Sprite %d xrepeat: ", cursprite);
            sprite[cursprite].xrepeat = getnumber16(tempbuf, sprite[cursprite].xrepeat, 255, 0);
            Bsprintf(tempbuf, "Sprite %d yrepeat: ", cursprite);
            sprite[cursprite].yrepeat = getnumber16(tempbuf, sprite[cursprite].yrepeat, 255, 0);
            printmessage16("Sprite %d updated", i);
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(F)) // ' F
    {
        FuncMenu();
    }
#ifdef LUNATIC
    else if (keystatus[KEYSC_SEMI] && PRESSED_KEYSC(F))  // ; F
    {
        LuaFuncMenu();
    }
#endif
    else if (!eitherALT && PRESSED_KEYSC(F))
    {
        if (pointhighlight < 16384 && tcursectornum>=0 && graphicsmode)
            toggle_cf_flipping(tcursectornum, 1);
    }

    tsign = 0;
    if (PRESSED_KEYSC(LBRACK)) // [     search backward
        tsign = -1;
    if (PRESSED_KEYSC(RBRACK)) // ]     search forward
        tsign = +1;

    if (tsign)
    {
        if (eitherALT)
        {
            if (numcorruptthings > 0)
            {
                int32_t wrap=0, x, y, z;

                if (curcorruptthing<0 || curcorruptthing>=numcorruptthings)
                    curcorruptthing = 0;
                else
                {
                    curcorruptthing += tsign;
                    wrap = (curcorruptthing<0 || curcorruptthing>=numcorruptthings);
                    curcorruptthing += numcorruptthings;
                    curcorruptthing %= numcorruptthings;
                }

                k = corruptthings[curcorruptthing];
                j = -1;
                switch (k&CORRUPT_MASK)
                {
                case 0:
                    printmessage16("MAP LIMITS EXCEEDED!");
                    fallthrough__;
                default:
                    k = 0;
                    break;
                case CORRUPT_SECTOR:
                    i = k&(MAXSECTORS-1);
                    j = 0;
                    x = wall[sector[i].wallptr].x;
                    y = wall[sector[i].wallptr].y;
                    z = getflorzofslope(i, x, y);
                    break;
                case CORRUPT_WALL:
                    i = k&(MAXWALLS-1);
                    j = 1;
                    x = wall[i].x + (wall[wall[i].point2].x-wall[i].x)/2;
                    y = wall[i].y + (wall[wall[i].point2].y-wall[i].y)/2;
                    z = getflorzofslope(sectorofwall(i), x, y);
                    break;
                case CORRUPT_SPRITE:
                    i = k&(MAXSPRITES-1);
                    j = 2;
                    x = sprite[i].x;
                    y = sprite[i].y;
                    z = sprite[i].z;
                    break;
                }

                if (k)
                {
                    static const char *secwalspr[3] = {"sector", "wall", "sprite"};
                    if (x>=-editorgridextent && x<=editorgridextent &&
                        y>=-editorgridextent && y<=editorgridextent)
                    {
                        pos.x = x;
                        pos.y = y;
                        pos.z = z;
#ifdef YAX_ENABLE
                        yax_updategrays(pos.z);
#endif
                    }
                    else x=editorgridextent+1;

                    printmessage16("#%d: %s Corrupt %s %d%s", curcorruptthing+1, tsign<0?"<":">", secwalspr[j],
                                   i, (x==editorgridextent+1) ? " (outside grid)" : (wrap ? " (wrap)" : ""));
                }
            }
            else
            {
                printmessage16("Map has no corruptions, cannot cycle them.");
            }
        }
        /*else if (keystatus[KEYSC_LSHIFT])
        {
            if (pointhighlight&16384)
            {
                const int32_t refspritenum = pointhighlight&16383;
                const int32_t reftag = select_sprite_tag(refspritenum);
                int32_t tmpspritenum = refspritenum;

                while (reftag != INT32_MIN)  // if (reftag != INT32_MIN) while (1)
                {
                    tmpspritenum += tsign;
                    if ((unsigned)tmpspritenum >= MAXSPRITES)
                        tmpspritenum = (tmpspritenum < 0) ? MAXSPRITES-1 : 0;

                    if (tmpspritenum==refspritenum)
                    {
                        silentmessage("No other sprites with tag %d", reftag);
                        break;
                    }

                    if (reftag==select_sprite_tag(tmpspritenum))
                    {
                        int32_t oog = jump_to_sprite(tmpspritenum);

                        if (!oog)
                        {
                            // center cursor so that we can repeat this
                            searchx = halfxdim16;
                            searchy = midydim16;
                        }

                        silentmessage("%s sprite with tag %d: %d%s", tsign>0 ? "Next" : "Previous",
                                      reftag, tmpspritenum, oog ? "(out of grid)" : "");
                        break;
                    }
                }
            }
            else
            {
                printmessage16("No sprite higlighted, cannot cycle linking sprites.");
            }
        }*/
        else if (wallsprite==0)
        {
            SearchSectors(tsign);
        }
        else if (wallsprite==1)
        {
            if ((tsign<0 && curwallnum>0) || (tsign>0 && curwallnum<numwalls))
                curwallnum += tsign;

            for (i=curwallnum; i>=0 && i<numwalls; i+=tsign)
            {
                if ((wall[i].picnum==wall[curwall].picnum)
                        && (search_lotag==0 || search_lotag==wall[i].lotag)
                        && (search_hitag==0 || search_hitag==wall[i].hitag))
                {
                    pos.x = wall[i].x - (wall[i].x-POINT2(i).x)/2;
                    pos.y = wall[i].y - (wall[i].y-POINT2(i).y)/2;
                    pos.z = getflorzofslope(sectorofwall(i), pos.x, pos.y);
                    printmessage16("%s Wall search: found", tsign<0?"<":">");
                    return;
                }
                curwallnum--;
            }
            printmessage16("%s Wall search: none found", tsign<0?"<":">");
        }
        else if (wallsprite==2)
            DoSpriteSearch(tsign);
    }

    if (PRESSED_KEYSC(G))  // G (grid on/off)
    {
        if (autogrid)
        {
            grid = 8*eitherSHIFT;

            autogrid = 0;
        }
        else
        {
            grid += (1-2*eitherSHIFT);
            if (grid == -1 || grid == 9)
            {
                autogrid = 1;
                grid = 0;
            }
        }

        if (autogrid)
            printmessage16("Grid size: 9 (autosize)");
        else if (!grid)
            printmessage16("Grid off");
        else
            printmessage16("Grid size: %d (%d units)", grid, 2048>>grid);
    }

    if (autogrid)
    {
        grid = -1;

        while (grid++ < 7)
        {
            if (mulscale14((2048>>grid), zoom) <= 16)
                break;
        }
    }


    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(L)) // ' L  (set sprite/wall coordinates)
    {
        if (pointhighlight >= 16384)
        {
            i = pointhighlight - 16384;
            Bsprintf(tempbuf, "Sprite %d x: ", i);
            sprite[i].x = getnumber16(tempbuf, sprite[i].x, editorgridextent-1, 1);
            Bsprintf(tempbuf, "Sprite %d y: ", i);
            sprite[i].y = getnumber16(tempbuf, sprite[i].y, editorgridextent-1, 1);
            Bsprintf(tempbuf, "Sprite %d z: ", i);
            sprite[i].z = getnumber16(tempbuf, sprite[i].z, BZ_MAX, 1);
            Bsprintf(tempbuf, "Sprite %d angle: ", i);
            sprite[i].ang = getnumber16(tempbuf, sprite[i].ang, 2048, 1);
            sprite[i].ang &= 2047;
            printmessage16("Sprite %d updated", i);
        }
        else if (pointhighlight >= 0)
        {
            i = linehighlight;
            j = wall[i].x;
            k = wall[i].y;

            Bsprintf(tempbuf, "Wall %d x: ", i);
            j = getnumber16(tempbuf, j, editorgridextent, 1);
            Bsprintf(tempbuf, "Wall %d y: ", i);
            k = getnumber16(tempbuf, k, editorgridextent, 1);
            dragpoint(i, j, k, 0);
            printmessage16("Wall %d updated", i);
        }
    }


    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(3)) // ' 3
    {
        onnames = (onnames+1)%M32_NUM_SPRITE_MODES;
        printmessage16("Mode %d %s", onnames, SpriteMode[onnames]);
    }
    //   Ver();

///__motorcycle___

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(7)) // ' 7 : swap hilo
    {

        if (pointhighlight >= 16384)
        {
            swapshort(&sprite[cursprite].lotag, &sprite[cursprite].hitag);
            printmessage16("Sprite %d tags swapped", cursprite);
        }
        else if (linehighlight >= 0)
        {
#ifdef YAX_ENABLE
            if (yax_getnextwall(linehighlight, YAX_CEILING)>=0 || yax_getnextwall(searchwall, YAX_FLOOR)>=0)
                message("Can't swap tags in protected wall");
            else
#endif
            {
                swapshort(&wall[linehighlight].lotag, &wall[linehighlight].hitag);
                printmessage16("Wall %d tags swapped", linehighlight);
            }
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(J)) // ' J
    {
        char dachars[4] = {'s', 'w', 'i', 'c'};

        fade_editor_screen(-1);
        i = editor_ask_function("Jump to (s)ector, (w)all, spr(i)te, or (c)oordinates?", dachars, 4);

        switch (i)
        {
        case 0:
            if (numsectors > 0)
            {
                j = getnumber16("Sector: ", 0, numsectors-1, 0+8);
                if (j < 0)
                    break;
                pos.x = wall[sector[j].wallptr].x;
                pos.y = wall[sector[j].wallptr].y;
                pos.z = sector[j].floorz;
                printmessage16("Current pos now on sector %d's first wall-point", j);
            }
            break;
        case 1:
            if (numwalls > 0)
            {
                j = getnumber16("Wall: ", 0, numwalls-1, 0+8);
                if (j < 0)
                    break;
                pos.x = wall[j].x + (wall[wall[j].point2].x-wall[j].x)/4;
                pos.y = wall[j].y + (wall[wall[j].point2].y-wall[j].y)/4;
                pos.z = getflorzofslope(sectorofwall(j), pos.x, pos.y);
                printmessage16("Current pos now on wall %d's midpoint", j);
            }
            break;
        case 2:
            j = getnumber16("Sprite: ", 0, MAXSPRITES-1, 0+8);
            if (j < 0 || sprite[j].statnum==MAXSTATUS)
                break;
            pos.x = sprite[j].x;
            pos.y = sprite[j].y;
            pos.z = sprite[j].z;
            printmessage16("Current pos now on sprite %d", j);
            break;

        case 3:
            pos.x = getnumber16("X-coordinate:    ", pos.x, editorgridextent, 1);
            pos.y = getnumber16("Y-coordinate:    ", pos.y, editorgridextent, 1);
            printmessage16("Current pos now (%d, %d)", pos.x, pos.y);
            break;
        }
#ifdef YAX_ENABLE
        yax_updategrays(pos.z);
#endif
    }
}// end key2d

#if 0
static void InitCustomColors(void)
{
    int32_t i;
    palette_t *edcol;

    vgapal16[2*4+0] = 19;
    vgapal16[2*4+1] = 75;
    vgapal16[2*4+2] = 19;

    vgapal16[3*4+0] = 215;
    vgapal16[3*4+1] = 115;
    vgapal16[3*4+2] = 43;

    vgapal16[4*4+0] = 31;
    vgapal16[4*4+1] = 31;
    vgapal16[4*4+2] = 143;

    vgapal16[5*4+0] = 111;
    vgapal16[5*4+1] = 67;
    vgapal16[5*4+2] = 59;

    vgapal16[9*4+0] = 227;
    vgapal16[9*4+1] = 135;
    vgapal16[9*4+2] = 51;

    vgapal16[10*4+0] = 83;
    vgapal16[10*4+1] = 139;
    vgapal16[10*4+2] = 83;

    vgapal16[12*4+0] = 95;
    vgapal16[12*4+1] = 103;
    vgapal16[12*4+2] = 187;

    vgapal16[13*4+0] = 143;
    vgapal16[13*4+1] = 91;
    vgapal16[13*4+2] = 75;

    //vgapal16[13*4+0] = 51;
    //vgapal16[13*4+1] = 107;
    //vgapal16[13*4+2] = 227;

    /* orange */
    vgapal16[31*4+0] = 80; // blue
    vgapal16[31*4+1] = 180; // green
    vgapal16[31*4+2] = 240; // red

    // UNUSED?
    vgapal16[39*4+0] = 144;
    vgapal16[39*4+1] = 212;
    vgapal16[39*4+2] = 252;


    /* light yellow */
    vgapal16[22*4+0] = 204;
    vgapal16[22*4+1] = 252;
    vgapal16[22*4+2] = 252;

    /* grey */
    vgapal16[23*4+0] = 180;
    vgapal16[23*4+1] = 180;
    vgapal16[23*4+2] = 180;

    /* blue */
    vgapal16[24*4+0] = 204;
    vgapal16[24*4+1] = 164;
    vgapal16[24*4+2] = 48;

    vgapal16[32*4+0] = 240;
    vgapal16[32*4+1] = 200;
    vgapal16[32*4+2] = 84;

    // grid color
    vgapal16[25*4+0] = 59;
    vgapal16[25*4+1] = 59;
    vgapal16[25*4+2] = 59;

    vgapal16[26*4+0] = 96;
    vgapal16[26*4+1] = 96;
    vgapal16[26*4+2] = 96;

    // UNUSED?
    vgapal16[33*4+0] = 0; //60; // blue
    vgapal16[33*4+1] = 0; //120; // green
    vgapal16[33*4+2] = 192; //180; // red

    // UNUSED?
    vgapal16[41*4+0] = 0; //96;
    vgapal16[41*4+1] = 0; //160;
    vgapal16[41*4+2] = 252; //192;

    for (i = 0; i<256; i++)
    {
        //if (editorcolors[i] == 0)
        {
            edcol = (palette_t *)&vgapal16[4*i];
            editorcolors[i] = getclosestcol_lim(edcol->b,edcol->g,edcol->r, 254);
        }
    }
}
#endif

int32_t ExtPreSaveMap(void)
{
    int32_t numfixedsprites;

    numfixedsprites = fixspritesectors();   //Do this before saving!
    updatesectorz(startpos.x,startpos.y,startpos.z,&startsectnum);
    if (startsectnum < 0)
        updatesector(startpos.x,startpos.y,&startsectnum);

    if (fixmaponsave_walls)
    {
        int32_t i, startwall, j, endwall;

        for (i=0; i<numsectors; i++)
        {
            startwall = sector[i].wallptr;
            for (j=startwall; j<numwalls; j++)
                if (wall[j].point2 < startwall)
                    startwall = wall[j].point2;
            if (sector[i].wallptr != startwall)
                LOG_F(WARNING, "set sector %d's wallptr to %d (was %d)", i,
                           TrackerCast(sector[i].wallptr), startwall);
            sector[i].wallptr = startwall;
        }

        for (i=numsectors-2; i>=0; i--)
            sector[i].wallnum = sector[i+1].wallptr-sector[i].wallptr;
        sector[numsectors-1].wallnum = numwalls - sector[numsectors-1].wallptr;

#ifdef YAX_ENABLE
        // setting redwalls from scratch would very likely wreak havoc with TROR maps
        if (numyaxbunches > 0)
            return numfixedsprites;
#endif
        for (i=0; i<numwalls; i++)
        {
            wall[i].nextsector = -1;
            wall[i].nextwall = -1;
        }
        for (i=0; i<numsectors; i++)
        {
            startwall = sector[i].wallptr;
            endwall = startwall + sector[i].wallnum;
            for (j=startwall; j<endwall; j++)
                checksectorpointer(j, i);
        }
    }

    return numfixedsprites;
}

static void G_ShowParameterHelp(void)
{
    const char *s = "Usage: nmapedit [files] [options]\n\n"
              "-g [file.grp], -grp [file.grp]\tLoad extra group file\n"
              "-h [file.def]\t\tLoad an alternate definitions file\n"
              "-x [game.con]\t\tLoad a custom CON script for getting sound definitions\n"
              "-mh [file.def]\t\tInclude additional definitions module\n"
              "-mx [file.con]\t\tInclude additional CON module for getting sound definitions\n"
              "-j [dir], -game_dir [dir]\n"
              "\t\t\tAdds a directory to the file path stack\n"
              "-cachesize #\t\tSets cache size, in Kb\n"
              "-check\t\t\tEnables map pointer checking when saving\n"
#ifdef HAVE_CLIPSHAPE_FEATURE
              "-clipmap [file.map]\t\tLoad an additional clipping map for use with clipshape\n"
#endif
              "-namesfile [file.h]\t\tLoad a custom NAMES.H for tile names\n"
              "-nocheck\t\t\tDisables map pointer checking when saving (default)\n"  // kept for script compat
#ifdef STARTUP_SETUP_WINDOW
              "-setup\t\t\tDisplays the configuration dialog\n"
#endif
              "-usecwd\t\t\tRead game data and configuration file from working directory\n"
              "\n-?, -help, --help\t\tDisplay this help message and exit"
              ;
    Bsprintf(tempbuf, "NMapedit %s", s_buildRev);
    wm_msgbox(tempbuf, "%s", s);
}


#define COPYARG(i) \
    Bmemcpy(&testplay_addparam[j], argv[i], lengths[i]); \
    j += lengths[i]; \
    testplay_addparam[j++] = ' ';

#if defined(RENDERTYPEWIN) && defined(USE_OPENGL)
extern char forcegl;
#endif

#ifdef LUNATIC
char const * const * g_argv;
#endif

static void G_CheckCommandLine(int32_t argc, char const * const * argv)
{
    int32_t i = 1, j, maxlen=0, *lengths;
    const char *c, *k;

    mapster32_fullpath = argv[0];
#ifdef LUNATIC
    g_argv = argv;
#endif

#ifdef HAVE_CLIPSHAPE_FEATURE
    // pre-form the default 10 clipmaps
    for (j = '0'; j<='9'; ++j)
    {
        char clipshape[16] = "_clipshape0.map";

        clipshape[10] = j;
        g_clipMapFiles.append(Xstrdup(clipshape));
    }
#endif

    if (argc <= 1)
        return;

    lengths = (int32_t *)Xmalloc(argc*sizeof(int32_t));
    for (j=1; j<argc; j++)
    {
        lengths[j] = Bstrlen(argv[j]);
        maxlen += lengths[j];
    }

    testplay_addparam = (char *)Xmalloc(maxlen+argc);
    testplay_addparam[0] = 0;

    j = 0;

    // NUKE-TODO: Use getopt here?

    while (i < argc)
    {
        c = argv[i];

        if ((*c == '-')
#ifdef _WIN32
            || (*c == '/')
#endif
)
        {
            if (!Bstrcasecmp(c+1,"?") || !Bstrcasecmp(c+1,"help") || !Bstrcasecmp(c+1,"-help"))
            {
                G_ShowParameterHelp();
                Bexit(0);
            }

            //if (!Bstrcasecmp(c+1,"addon"))
            //{
            //    if (argc > i+1)
            //    {
            //        g_addonNum = Batoi(argv[i+1]);
            //
            //        if (!(g_addonNum > ADDON_NONE && g_addonNum < NUMADDONS))
            //            g_addonNum = ADDON_NONE;
            //
            //        COPYARG(i);
            //        COPYARG(i+1);
            //        i++;
            //    }
            //    i++;
            //    continue;
            //}

            if (!Bstrcasecmp(c+1, "g") || !Bstrcasecmp(c+1, "grp"))
            {
                if (argc > i+1)
                {
                    G_AddGroup(argv[i+1]);
                    COPYARG(i);
                    COPYARG(i+1);
                    i++;
                }
                i++;
                continue;
            }

            if (!Bstrcasecmp(c+1,"game_dir"))
            {
                if (argc > i+1)
                {
                    Bstrncpyz(g_modDir, argv[i+1], sizeof(g_modDir));

                    G_AddPath(argv[i+1]);

                    COPYARG(i);
                    COPYARG(i+1);
                    i++;
                }
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"cachesize"))
            {
                if (argc > i+1)
                {
                    int32_t sz = atoi_safe(argv[i+1]);
                    if (sz >= 16<<10 && sz <= 1024<<10)
                    {
                        MAXCACHE1DSIZE = sz<<10;
                        LOG_F(INFO, "Cache size: %dkB",sz);

                        COPYARG(i);
                        COPYARG(i+1);
                    }
                    i++;
                }
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"cfg"))
            {
                if (argc > i+1)
                {
                    Bstrncpyz(setupfilename, argv[i+1], sizeof(setupfilename));
                    i++;
                }
                i++;
                continue;
            }
            //if (!Bstrcasecmp(c+1,"gamegrp"))
            //{
            //    if (argc > i+1)
            //    {
            //        clearGrpNamePtr();
            //        g_grpNamePtr = dup_filename(argv[i+1]);
            //        COPYARG(i);
            //        COPYARG(i+1);
            //        i++;
            //    }
            //    i++;
            //    continue;
            //}
            if (!Bstrcasecmp(c+1,"namesfile"))
            {
                g_namesFileName = argv[i+1];
                i++;
                continue;
            }
            //if (!Bstrcasecmp(c+1,"x"))
            //{
            //    if (argc > i+1)
            //    {
            //        G_AddCon(argv[i+1]);
            //        COPYARG(i);
            //        COPYARG(i+1);
            //        i++;
            //    }
            //    i++;
            //    continue;
            //}
            //if (!Bstrcasecmp(c+1,"mx"))
            //{
            //    if (argc > i+1)
            //    {
            //        G_AddConModule(argv[i+1]);
            //        COPYARG(i);
            //        COPYARG(i+1);
            //        i++;
            //    }
            //    i++;
            //    continue;
            //}
            if (!Bstrcasecmp(c+1,"h"))
            {
                if (argc > i+1)
                {
                    G_AddDef(argv[i+1]);
                    COPYARG(i);
                    COPYARG(i+1);
                    i++;
                }
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"mh"))
            {
                if (argc > i+1)
                {
                    G_AddDefModule(argv[i+1]);
                    COPYARG(i);
                    COPYARG(i+1);
                    i++;
                }
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"j"))
            {
                if (argc > i+1)
                {
                    G_AddPath(argv[i+1]);
                    COPYARG(i);
                    COPYARG(i+1);
                    i++;
                }
                i++;
                continue;
            }
#ifdef HAVE_CLIPSHAPE_FEATURE
            if (!Bstrcasecmp(c+1,"clipmap"))
            {
                if (argc > i+1)
                {
                    G_AddClipMap(argv[i+1]);
                    COPYARG(i);
                    COPYARG(i+1);
                    i++;
                }
                i++;
                continue;
            }
#endif
            if (!Bstrcasecmp(c+1,"nm") || !Bstrcasecmp(c+1,"ns"))
            {
                COPYARG(i);
                i++;
                continue;
            }
            //if (!Bstrcasecmp(c+1,"nam"))
            //{
            //    g_gameType = GAMEFLAG_NAM;
            //    COPYARG(i);
            //    i++;
            //    continue;
            //}
            //if (!Bstrcasecmp(c+1,"napalm"))
            //{
            //    g_gameType = GAMEFLAG_NAM|GAMEFLAG_NAPALM;
            //    COPYARG(i);
            //    i++;
            //    continue;
            //}
            //if (!Bstrcasecmp(c+1,"ww2gi"))
            //{
            //    g_gameType = GAMEFLAG_WW2GI;
            //    COPYARG(i);
            //    i++;
            //    continue;
            //}
            if (!Bstrcasecmp(c+1,"check"))
            {
                LOG_F(INFO, "Map wall checking on save enabled");
                fixmaponsave_walls = 1;
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"nocheck"))
            {
                LOG_F(INFO, "Map wall checking on save disabled");
                fixmaponsave_walls = 0;
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"noautoload"))
            {
                LOG_F(INFO, "Autoload disabled");
                NoAutoLoad = 1;
                COPYARG(i);
                i++;
                continue;
            }
            if (!Bstrcasecmp(c+1,"usecwd"))
            {
                g_useCwd = 1;
                COPYARG(i);
                i++;
                continue;
            }
#if defined(RENDERTYPEWIN) && defined(USE_OPENGL)
            if (!Bstrcasecmp(c+1,"forcegl"))
            {
                forcegl = 1;
                i++;
                continue;
            }
#endif
        }


        if ((*c == '-')
#ifdef _WIN32
            || (*c == '/')
#endif
)
        {
            c++;
            switch (*c)
            {
            case 'h':
            case 'H':
                c++;
                if (*c)
                {
                    G_AddDef(c);
                    COPYARG(i);
                }
                break;
            case 'j':
            case 'J':
                c++;
                if (!*c) break;
                G_AddPath(c);
                COPYARG(i);
                break;
            case 'g':
            case 'G':
                c++;
                if (!*c) break;
                G_AddGroup(c);
                COPYARG(i);
                break;
            //case 'x':
            //case 'X':
            //    c++;
            //    if (*c)
            //    {
            //        G_AddCon(c);
            //        COPYARG(i);
            //    }
            //    break;
            }
        }
        else
        {
            k = Bstrrchr(c,'.');
            if (k)
            {
                if (!Bstrcasecmp(k,".map"))
                {
                    B_SetBoardFileName(argv[i++]);
                    continue;
                }
                else if (!Bstrcasecmp(k,".grp") || !Bstrcasecmp(k,".zip"))
                {
                    COPYARG(i);
                    G_AddGroup(argv[i++]);
                    continue;
                }
                else if (!Bstrcasecmp(k,".def"))
                {
                    COPYARG(i);
                    G_AddDef(argv[i++]);
                    continue;
                }
                //else if (!Bstrcasecmp(k,".con"))
                //{
                //    COPYARG(i);
                //    G_AddCon(argv[i++]);
                //    continue;
                //}
            }
        }
        i++;
    }

    Xfree(lengths);

    if (j > 0)
    {
        testplay_addparam[j-1] = 0;
        testplay_addparam = (char *)Xrealloc(testplay_addparam, j*sizeof(char));
    }
    else
    {
        DO_FREE_AND_NULL(testplay_addparam);
    }

}
#undef COPYARG

#if defined(_WIN32) && defined(DEBUGGINGAIDS)
// See FILENAME_CASE_CHECK in cache1d.c
static int32_t check_filename_casing(void)
{
    return 1;
}
#endif

int32_t ExtPreInit(int32_t argc,char const * const * argv)
{
#if defined(_WIN32) && defined(DEBUGGINGAIDS)
    {
        extern int32_t (*check_filename_casing_fn)(void);
        check_filename_casing_fn = check_filename_casing;
    }
#endif

    G_ExtPreInit(argc, argv);

    OSD_SetLogFile("nmapedit.log");
    OSD_SetVersion("NMapedit",0,2);
    LOG_F(INFO, "NMapedit %s", s_buildRev);
    PrintBuildInfo();

    G_CheckCommandLine(argc,argv);

    return 0;
}

static int osdcmd_quit(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    OSD_ShowDisplay(0);

    ExtUnInit();
    scrUnInit();

    Bfflush(NULL);

    Bexit(0);
}

static int osdcmd_editorgridextent(osdcmdptr_t parm)
{
    int32_t i;

    if (parm->numparms == 0)
    {
        OSD_Printf("\"editorgridextent\" is \"%d\"\n", editorgridextent);
        return OSDCMD_SHOWHELP;
    }
    else if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    i = Batol(parm->parms[0]);

    if (i >= 65536 && i <= BXY_MAX)
    {
        editorgridextent = i;
        OSD_Printf("editorgridextent %d\n", editorgridextent);
    }
    else
        OSD_Printf("editorgridextent: value out of range (65536 to %d)\n", BXY_MAX);

    return OSDCMD_OK;
}

static int osdcmd_addpath(osdcmdptr_t parm)
{
    char pathname[BMAX_PATH];

    if (parm->numparms != 1) return OSDCMD_SHOWHELP;

    Bstrcpy(pathname,parm->parms[0]);
    addsearchpath(pathname);
    return OSDCMD_OK;
}

static int osdcmd_initgroupfile(osdcmdptr_t parm)
{
    char file[BMAX_PATH];

    if (parm->numparms != 1) return OSDCMD_SHOWHELP;

    Bstrcpy(file,parm->parms[0]);
    initgroupfile(file);
    return OSDCMD_OK;
}

static int osdcmd_sensitivity(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
    {
        OSD_Printf("\"sensitivity\" is \"%.2f\"\n",msens);
        return OSDCMD_SHOWHELP;
    }
    msens = atof(parm->parms[0]);
    OSD_Printf("sensitivity %.2f\n",msens);
    return OSDCMD_OK;
}

static int osdcmd_noclip(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    m32_clipping--;
    if (m32_clipping < 0)
        m32_clipping = 2;
    OSD_Printf("Clipping %s\n", m32_clipping==0 ? "disabled" :
               (m32_clipping==1 ? "non-masks only" : "enabled"));

    return OSDCMD_OK;
}

static int osdcmd_testplay_addparam(osdcmdptr_t parm)
{
    int32_t slen;

    if (parm->numparms != 1)
    {
        OSD_Printf("additional parameters for test playing: %s%s%s\n",
                   testplay_addparam ? "\"" : "",
                   testplay_addparam ? testplay_addparam : "<empty>",
                   testplay_addparam ? "\"" : "");
        return OSDCMD_OK;
    }

    slen = Bstrlen(parm->parms[0]);

    if (slen > 0)
    {
        if (!testplay_addparam)
            testplay_addparam = (char *)Xmalloc(slen+1);
        else
            testplay_addparam = (char *)Xrealloc(testplay_addparam, slen+1);

        Bmemcpy(testplay_addparam, parm->parms[0], slen);
        testplay_addparam[slen] = 0;
    }
    else
    {
        DO_FREE_AND_NULL(testplay_addparam);
    }

    return OSDCMD_OK;
}


//PK vvv ------------
// FIXME: The way the different options are handled is horribly inconsistent.
static int osdcmd_vars_pk(osdcmdptr_t parm)
{
    const int32_t setval = (parm->numparms >= 1);

    // this is something of a misnomer, since it's actually accel+decel
    if (!Bstrcasecmp(parm->name, "pk_turnaccel"))
    {
        if (setval)
        {
            pk_turnaccel = atoi_safe(parm->parms[0]);
            pk_turnaccel = pk_turnaccel<=pk_turndecel ? (pk_turndecel+1):pk_turnaccel;
            pk_turnaccel = pk_turnaccel>256 ? 256:pk_turnaccel;
        }

        OSD_Printf("Turning acceleration+declaration is %d\n", pk_turnaccel);
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "pk_turndecel"))
    {
        if (setval)
        {
            pk_turndecel = atoi_safe(parm->parms[0]);
            pk_turndecel = pk_turndecel<=0 ? 1:pk_turndecel;
            pk_turndecel = pk_turndecel>=pk_turnaccel ? (pk_turnaccel-1):pk_turndecel;
            pk_turndecel = pk_turndecel>128 ? 128:pk_turndecel;
        }

        OSD_Printf("Turning deceleration is %d\n", pk_turndecel);
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "pk_quickmapcycling"))
    {
        OSD_Printf("Quick map cycling ((LShift-)Ctrl-X): %s\n",
                   (quickmapcycling = !quickmapcycling) ? "enabled":"disabled");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "pk_uedaccel"))
    {
        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        if (setval)
            pk_uedaccel = clamp(atoi_safe(parm->parms[0]), 0, 5);

        OSD_Printf("UnrealEd mouse navigation acceleration is %d\n", pk_uedaccel);
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "osd_tryscript"))
    {
        m32_osd_tryscript = !m32_osd_tryscript;
        OSD_Printf("Try M32 script execution on invalid OSD command: %s\n", m32_osd_tryscript?"on":"off");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "sideview_reversehorizrot"))
    {
        sideview_reversehrot = !sideview_reversehrot;
        OSD_Printf("Side view reverse horizontal rotation: %s\n", sideview_reversehrot?"on":"off");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "script_expertmode"))
    {
        if (setval)
            m32_script_expertmode = !!atoi_safe(parm->parms[0]);

        if (m32_script_expertmode)
            OSD_Printf("M32 Script expert mode %sENABLED.  Be sure to know what you are doing!\n",
                       setval ? "" : "is ");
        else
            OSD_Printf("M32 Script expert mode %sDISABLED.\n", setval ? "" : "is ");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "fixmaponsave_sprites"))
    {
        OSD_Printf("Fix sprite sectnums on map saving: %s\n",
                   (fixmaponsave_sprites = !fixmaponsave_sprites) ? "enabled":"disabled");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "show_heightindicators"))
    {
        static const char *how[3] = {"none", "two-sided walls only", "all"};

        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        if (setval)
            showheightindicators = clamp(atoi_safe(parm->parms[0]), 0, 2);
        OSD_Printf("height indicators: %s\n", how[showheightindicators]);

        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "show_ambiencesounds"))
    {
        static const char *how[3] = {"none", "current sector only", "all"};

        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        if (setval)
            showambiencesounds = clamp(atoi_safe(parm->parms[0]), 0, 2);
        OSD_Printf("ambience sound circles: %s\n", how[showambiencesounds]);

        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "corruptcheck_noalreadyrefd"))
    {
        corruptcheck_noalreadyrefd = !corruptcheck_noalreadyrefd;
        OSD_Printf("%s 'already referenced' corruption (i.e. one-to-many nextwalls)\n",
                   corruptcheck_noalreadyrefd?"Ignore":"Regard");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "corruptcheck_game_duke3d"))
    {
        corruptcheck_game_duke3d = !corruptcheck_game_duke3d;
        OSD_Printf("%s Duke3D issues\n",
                   !corruptcheck_game_duke3d?"Ignore":"Regard");
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "corruptcheck_heinum"))
    {
        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        static const char *mode[3] = {"disabled", "auto-correct only", "auto-correct and warn"};

        if (setval)
            corruptcheck_heinum = clamp(atoi_safe(parm->parms[0]), 0, 2);

        OSD_Printf("Check inconsistent ceilingstat/floorstat bit 2 and .heinum: %s\n",
                   mode[corruptcheck_heinum]);

        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "keeptexturestretch"))
    {
        keeptexturestretch = !keeptexturestretch;
        OSD_Printf("Keep texture stretching when dragging wall vertices: %s\n",
                   ONOFF(keeptexturestretch));
        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "m32_2d3dmode"))
    {
        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        if (setval)
            m32_2d3dmode = atoi_safe(parm->parms[0]);

        OSD_Printf("Experimental 2d/3d hybrid mode: %s\n",
            ONOFF(m32_2d3dmode));

        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "pointhighlightdist"))
    {
        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        if (setval)
            pointhighlightdist = atoi_safe(parm->parms[0]);

        OSD_Printf("Point highlight distance: %d\n", pointhighlightdist);

        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "linehighlightdist"))
    {
        if (parm->numparms > 1)
            return OSDCMD_SHOWHELP;

        if (setval)
            linehighlightdist = atoi_safe(parm->parms[0]);

        OSD_Printf("Line highlight distance: %d\n", linehighlightdist);

        return OSDCMD_OK;
    }

    if (!Bstrcasecmp(parm->name, "corruptcheck"))
    {
        if (parm->numparms >= 1)
        {
            if (!Bstrcasecmp(parm->parms[0], "now"))
            {
                int32_t printfromlevel = 1;
                if (parm->numparms > 1)
                    printfromlevel = clamp(atoi_safe(parm->parms[1]), 1, 5);
                if (CheckMapCorruption(printfromlevel, 0)==0)
                    OSD_Printf("All OK.\n");
                return OSDCMD_OK;
            }

            if (!Bstrcasecmp(parm->parms[0], "tryfix"))
            {
                uint64_t whicherrs = parm->numparms==1 ? 0xffffffffffffffffull : 0;
                corrupt_tryfix_alt = 0;

                if (whicherrs==0)
                {
                    int32_t i, n, m;
                    char *endptr;
                    for (i=1; i<parm->numparms; i++)
                    {
                        if (i==parm->numparms-1)
                        {
                            if (!Bstrcmp(parm->parms[i], "??"))
                            {
                                corrupt_tryfix_alt = 2;
                                if (parm->numparms==2)
                                    whicherrs = 0xffffffffffffffffull;
                                break;
                            }
                            else if (!Bstrcmp(parm->parms[i], "?"))
                            {
                                corrupt_tryfix_alt = 1;
                                if (parm->numparms==2)
                                    whicherrs = 0xffffffffffffffffull;
                                break;
                            }
                        }

                        n = (int32_t)Bstrtol(parm->parms[i], &endptr, 10);
                        if (endptr != parm->parms[i])
                        {
                            if (*endptr=='-')
                            {
                                m = (int32_t)Bstrtol(endptr+1, NULL, 10);
                                if (n>=1 && n<=m && m<=MAXCORRUPTTHINGS)
                                {
                                    uint64_t mask = 0xffffffffffffffffull;
                                    m = m-n+1;
                                    mask >>= (MAXCORRUPTTHINGS-m);
                                    mask <<= (n-1);

                                    whicherrs |= mask;
                                }
                            }
                            else
                            {
                                if (n>=1 && n<=MAXCORRUPTTHINGS)
                                    whicherrs |= (1ull<<(n-1));
                            }
                        }
                    }
                }

                CheckMapCorruption(whicherrs?5:3, whicherrs);
                return OSDCMD_OK;
            }

            if (isdigit(parm->parms[0][0]))
            {
                autocorruptcheck = clamp(atoi_safe(parm->parms[0]), 0, 3600);
                corruptchecktimer = (int)totalclock + 120*autocorruptcheck;
            }

            return OSDCMD_OK;
        }

        if (parm->numparms <= 1)
        {
            if (autocorruptcheck)
                OSD_Printf("auto corruption check: %d seconds\n", autocorruptcheck);
            else
                OSD_Printf("auto corruption check: off\n");
            return OSDCMD_OK;
        }

        return OSDCMD_SHOWHELP;
    }

    return OSDCMD_OK;
}

#ifdef USE_OPENGL
static int osdcmd_tint(osdcmdptr_t parm)
{
    int32_t i;
    polytint_t *p;

    if (parm->numparms==1)
    {
        i = atoi_safe(parm->parms[0]);
        if (i>=0 && i<=M32_MAXPALOOKUPS)
        {
            p = &hictinting[i];
            OSD_Printf("pal %d: r=%d g=%d b=%d f=%d\n", i, p->r, p->g, p->b, p->f);
        }
    }
    else if (parm->numparms==0)
    {
        OSD_Printf("Hightile tintings:\n");
        for (i=0,p=&hictinting[0]; i<=M32_MAXPALOOKUPS; i++,p++)
            if (p->r != 255 || p->g != 255 || p->b != 255 || p->f != 0)
                OSD_Printf("pal %d: rgb %3d %3d %3d  f %d\n", i, p->r, p->g, p->b, p->f);
    }
    else if (parm->numparms>=2)
    {
        i = atoi_safe(parm->parms[0]);
        if (i<0 || i>M32_MAXPALOOKUPS)
            return OSDCMD_SHOWHELP;

        p = &hictinting[i];
        p->r = atoi_safe(parm->parms[1]);
        p->g = (parm->numparms>=3) ? atoi_safe(parm->parms[2]) : 255;
        p->b = (parm->numparms>=4) ? atoi_safe(parm->parms[3]) : 255;
        p->f = (parm->numparms>=5) ? atoi_safe(parm->parms[4]) : 0;
    }
    return OSDCMD_OK;
}
#endif

static void SaveInHistory(const char *commandstr)
{
    int32_t i, idx, dosave=1;

    // If running with osdtryscript=1, we could receive e.g.
    //  "// this file is automatically generated by Mapster32"
    // from m32_config.cfg here.
    if (!Bstrncmp(commandstr, "//", 2))
        return;

    for (i=1; i<=4; i++)
    {
        idx = (scripthistend-i)&(SCRIPTHISTSIZ-1);
        if (!scripthist[idx])
            break;
        else if (!Bstrcmp(scripthist[idx], commandstr))
        {
            dosave = 0;
            break;
        }
    }

    if (dosave)
    {
        Xfree(scripthist[scripthistend]);
        scripthist[scripthistend] = Xstrdup(commandstr);
        scripthistend++;
        scripthistend %= SCRIPTHISTSIZ;
    }
}

#ifdef LUNATIC
static int osdcmd_lua(osdcmdptr_t parm)
{
    // Should be used like
    // lua "lua code..."
    // (the quotes making the whole string passed as one argument)

    int32_t ret;

    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    if (!L_IsInitialized(&g_EmState))
    {
        OSD_Printf("Lua state is not initialized.\n");
        return OSDCMD_OK;
    }

    ret = L_RunString(&g_EmState, parm->parms[0], -1, "console");
    if (ret != 0)
        OSD_Printf("Error running the Lua code (error code %d)\n", ret);
    else
        SaveInHistory(parm->raw);

    return OSDCMD_OK;
}
#endif

// M32 script vvv
static int osdcmd_include(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;
    C_Compile(parm->parms[0], 1);
    return OSDCMD_OK;
}

static int osdcmd_scriptinfo(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    C_CompilationInfo();
    return OSDCMD_OK;
}

#ifdef DEBUGGINGAIDS
extern void X_Disasm(ofstype beg, int32_t size);

static int osdcmd_disasm(osdcmdptr_t parm)
{
    int32_t i;

    if (parm->numparms != 2)
        return OSDCMD_SHOWHELP;

    if (!isdigit(parm->parms[1][0]))
        return OSDCMD_SHOWHELP;

    i=atoi_safe(parm->parms[1]);

    if (parm->parms[0][0]=='s')
    {
        if (i>=0 && i<g_stateCount)
            X_Disasm(statesinfo[i].ofs, statesinfo[i].codesize);
    }
    else
    {
        if (i>=0 && i<MAXEVENTS && aEventOffsets[i]>=0)
            X_Disasm(aEventOffsets[i], aEventSizes[i]);
    }
    return OSDCMD_OK;
}
#endif

static int osdcmd_do(osdcmdptr_t parm)
{
    if (parm->numparms==0)
        return OSDCMD_SHOWHELP;

    int32_t onumconstants = g_numSavedConstants;

    intptr_t oscrofs = (g_scriptPtr-apScript);
    int32_t  ofs     = 2 * (parm->numparms > 0);  // true if "do" command
    int32_t  slen    = Bstrlen(parm->raw + ofs);
    char *   tp      = (char *)Xmalloc(slen+2);

    Bmemcpy(tp, parm->raw+ofs, slen);

    // M32script call from 'special functions' menu
    int32_t dontsavehist = (slen == 0 || tp[0] == ' ');

    // needed so that subsequent commands won't execute old stuff.
    tp[slen]   = '\n';
    tp[slen+1] = '\0';

    g_didDefineSomething = 0;

    C_Compile(tp, 0);

    if (parm->numparms >= 0)
        DO_FREE_AND_NULL(tp);

    if (g_numCompilerErrors)
    {
//        g_scriptPtr = script + oscrofs;  // handled in C_Compile()
        Xfree(tp);
        return OSDCMD_OK;
    }

    if (g_didDefineSomething == 0)
    {
        g_numSavedConstants = onumconstants;

        *g_scriptPtr = CON_RETURN + (g_lineNumber<<12);
        g_scriptPtr  = apScript + oscrofs;

        insptr = apScript + oscrofs;
        Bmemcpy(&vm, &vm_default, sizeof(vmstate_t));

        if (in3dmode() && AIMING_AT_SPRITE)
        {
            vm.spriteNum = searchwall;
            vm.pSprite   = &sprite[vm.spriteNum];
        }

        // If OSD is down, that would interfere with user input, so don't consider
        // m32script executed from the console as 'interactive'. Which leaves only
        // that from the 'special functions' menu
        if (OSD_GetRowsCur() < 0)
            vm.miscflags |= VMFLAG_MISC_INTERACTIVE;

        VM_Execute(0);

        M32_PostScriptExec();

        if (!(vm.flags&VMFLAG_ERROR) && !dontsavehist)
            SaveInHistory(parm->raw);

        OSD_Printf(OSDTEXT_GREEN "%s\n", parm->raw);
//        asksave = 1; // handled in Access(Sprite|Sector|Wall)
    }

    return OSDCMD_OK;
}

void M32RunScript(const char *s)
{
    osdfuncparm_t parm;

    parm.numparms = -1;
    parm.raw = s;

    osdcmd_do(&parm);
}

static int osdcmd_endisableevent(osdcmdptr_t parm)
{
    int32_t i, j, enable;

    if (!label) return OSDCMD_OK;

    if (parm->numparms < 1)
    {
        OSD_Printf("--- Defined events:\n");
        for (i=0; i<MAXEVENTS; i++)
            if (aEventOffsets[i] >= 0)
                OSD_Printf("%s (%d): %s\n", label+(i*MAXLABELLEN), i, aEventEnabled[i]?"on":"off");
        return OSDCMD_OK;
    }

    enable = !Bstrcasecmp(parm->name, "enableevent");

    if (parm->numparms == 1)
    {
        if (!Bstrcasecmp(parm->parms[0], "all") || !Bstrcasecmp(parm->parms[0], "a"))
        {
            for (i=0; i<MAXEVENTS; i++)
                aEventEnabled[i] = enable?1:0;
            OSD_Printf("%sabled all events.\n", enable?"En":"Dis");
            return OSDCMD_OK;
        }
    }

    for (i=0; i<parm->numparms; i++)
    {
        char buf[64] = "EVENT_", buf2[64];

        if (isdigit(parm->parms[i][0]))
        {
            j = atoi_safe(parm->parms[i]);
            Bsprintf(buf2, "event %d", j);
        }
        else if (!Bstrncmp(parm->parms[i], "EVENT_", 6))
        {
            j = hash_find(&h_labels, parm->parms[i]);
            Bstrncpyz(buf2, parm->parms[i], sizeof(buf2));
        }
        else
        {
            Bstrncat(buf, parm->parms[i], sizeof(buf)-6-1);
            j = hash_find(&h_labels, buf);
            Bmemcpy(buf2, buf, sizeof(buf2));
        }

        if (j>=0 && j<MAXEVENTS)
        {
            aEventEnabled[j] = enable?1:0;
            OSD_Printf("%sabled %s.\n", enable?"En":"Dis", buf2);
        }
        else
            OSD_Printf("Invalid event %s.\n", buf2);
    }
    return OSDCMD_OK;
}

static int32_t registerosdcommands(void)
{
    OSD_RegisterFunction("addpath","addpath <path>: adds path to game filesystem", osdcmd_addpath);

    OSD_RegisterFunction("editorgridextent","editorgridextent: sets the size of the 2D mode editing grid",osdcmd_editorgridextent);

    OSD_RegisterFunction("initgroupfile","initgroupfile <path>: adds a grp file into the game filesystem", osdcmd_initgroupfile);

    OSD_RegisterFunction("m32_clipping","m32_clipping: toggles clipping mode", osdcmd_noclip);

    OSD_RegisterFunction("quit","quit: exits the editor immediately", osdcmd_quit);
    OSD_RegisterFunction("exit","exit: exits the editor immediately", osdcmd_quit);

    OSD_RegisterFunction("sensitivity","sensitivity <value>: changes the mouse sensitivity", osdcmd_sensitivity);

    //PK
    OSD_RegisterFunction("m32_2d3dmode", "2d3dmode: experimental 2d/3d hybrid mode", osdcmd_vars_pk);
    OSD_RegisterFunction("pointhighlightdist", "pointhighlightdist: distance at which points are selected", osdcmd_vars_pk);
    OSD_RegisterFunction("linehighlightdist", "linehighlightdist: distance at which lines are selected", osdcmd_vars_pk);
    OSD_RegisterFunction("pk_turnaccel", "pk_turnaccel <value>: sets turning acceleration+deceleration", osdcmd_vars_pk);
    OSD_RegisterFunction("pk_turndecel", "pk_turndecel <value>: sets turning deceleration", osdcmd_vars_pk);
    OSD_RegisterFunction("pk_uedaccel", "pk_uedaccel <value>: sets UnrealEd movement speed factor (0-5, exponentially)", osdcmd_vars_pk);
    OSD_RegisterFunction("pk_quickmapcycling", "pk_quickmapcycling: toggles quick cycling of maps with (Shift-)Ctrl-X", osdcmd_vars_pk);
    OSD_RegisterFunction("testplay_addparam", "testplay_addparam \"string\": sets additional parameters for test playing", osdcmd_testplay_addparam);
    OSD_RegisterFunction("show_heightindicators", "show_heightindicators {0, 1 or 2}: sets display of height indicators in 2D mode", osdcmd_vars_pk);
    OSD_RegisterFunction("show_ambiencesounds", "show_ambiencesounds {0, 1 or 2}: sets display of MUSICANDSFX circles in 2D mode", osdcmd_vars_pk);
    OSD_RegisterFunction("corruptcheck_noalreadyrefd", "corruptcheck_noalreadyrefd: toggles ignoring of one-to-many red wall connections", osdcmd_vars_pk);
    OSD_RegisterFunction("corruptcheck_game_duke3d", "corruptcheck_game_duke3d: toggles ignoring of Duke3D issues", osdcmd_vars_pk);
    OSD_RegisterFunction("corruptcheck_heinum", "corruptcheck_heinum: toggles auto-correcting inconsistent c/fstat bit 2 and heinum (2: also warn)", osdcmd_vars_pk);
    OSD_RegisterFunction("keeptexturestretch", "keeptexturestretch: toggles keeping texture stretching when dragging wall vertices", osdcmd_vars_pk);

    OSD_RegisterFunction("corruptcheck", "corruptcheck {<seconds>|now|tryfix}: sets auto corruption check interval if <seconds> given, otherwise as indicated", osdcmd_vars_pk);
#ifdef USE_OPENGL
    OSD_RegisterFunction("tint", "tint <pal> <r> <g> <b> <flags>: queries or sets hightile tinting", osdcmd_tint);
#endif

#ifdef LUNATIC
    OSD_RegisterFunction("lua", "lua \"Lua code...\": runs Lua code", osdcmd_lua);
#endif
    // M32 script
    OSD_RegisterFunction("include", "include <filenames...>: compiles one or more M32 script files", osdcmd_include);
    OSD_RegisterFunction("do", "do (m32 script ...): executes M32 script statements", osdcmd_do);
    OSD_RegisterFunction("script_info", "script_info: shows information about compiled M32 script", osdcmd_scriptinfo);
    OSD_RegisterFunction("script_expertmode", "script_expertmode: toggles M32 script expert mode", osdcmd_vars_pk);
    OSD_RegisterFunction("enableevent", "enableevent {all|EVENT_...|(event number)}", osdcmd_endisableevent);
    OSD_RegisterFunction("disableevent", "disableevent {all|EVENT_...|(event number)}", osdcmd_endisableevent);
    OSD_RegisterFunction("osd_tryscript", "osd_tryscript: toggles execution of M32 script on invalid OSD command", osdcmd_vars_pk);
    OSD_RegisterFunction("sideview_reversehorizrot", "sideview_reversehorizrot: toggles reversion of Q and W keys in side view mode", osdcmd_vars_pk);
#ifdef DEBUGGINGAIDS
    OSD_RegisterFunction("disasm", "disasm [s|e] <state or event number>", osdcmd_disasm);
#endif
    return 0;
}


enum
{
    T_INCLUDE = 0,
    T_DEFINE = 1,
    T_LOADGRP,
    T_TILEGROUP,
    T_TILE,
    T_TILERANGE,
    T_HOTKEY,
    T_TILES,
    T_NOAUTOLOAD,
    T_COLORS,

    T_ALPHABET,
    T_MAP,
    T_MAPA,
    T_MAPRANGE,
    T_MAPRANGEA,
    T_OFFSET,
    T_OFFSETA,

    T_DEFINESOUND,
    T_INCLUDEDEFAULT,

    T_GLOBALGAMEFLAGS,

    T_GAMESTARTUP,

    T_DUMMY,
};

static int32_t parsegroupfiles(scriptfile *script);

static void parsegroupfiles_include(const char *fn, scriptfile *script, const char *cmdtokptr)
{
    scriptfile *included = scriptfile_fromfile(fn);

    if (!included)
    {
        if (!Bstrcasecmp(cmdtokptr,"null"))
            LOG_F(WARNING, "warning: failed including %s as module", fn);
        else
            LOG_F(WARNING, "%s:%d: warning: failed including %s",
                           script->filename, scriptfile_getlinum(script, cmdtokptr)
                           fn);
    }
    else
    {
        parsegroupfiles(included);
        scriptfile_close(included);
    }
}

static int32_t parsegroupfiles(scriptfile *script)
{
    int32_t tokn;
    char *cmdtokptr;

    tokenlist grptokens[] =
    {
        { "include",         T_INCLUDE },
        { "#include",        T_INCLUDE },
        { "includedefault",  T_INCLUDEDEFAULT },
        { "#includedefault", T_INCLUDEDEFAULT },
        { "loadgrp",         T_LOADGRP },
        { "noautoload",      T_NOAUTOLOAD },
        { "globalgameflags", T_GLOBALGAMEFLAGS },
    };

    while (1)
    {
        tokn = getatoken(script,grptokens,ARRAY_SIZE(grptokens));
        cmdtokptr = script->ltextptr;
        switch (tokn)
        {
        case T_LOADGRP:
        {
            char *fn;
            int opathsearchmode = pathsearchmode;

            pathsearchmode = 1;
            if (!scriptfile_getstring(script,&fn))
            {
                int32_t j = initgroupfile(fn);

                if (j == -1)
                    LOG_F(WARNING, "Could not find group file \"%s\".",fn);
                else
                {
                    LOG_F(INFO, "Using group file \"%s\".",fn);
                    if (!NoAutoLoad)
                        G_DoAutoload(fn);
                }

            }
            pathsearchmode = opathsearchmode;
        }
        break;
        case T_INCLUDE:
        {
            char *fn;
            if (!scriptfile_getstring(script,&fn))
                parsegroupfiles_include(fn, script, cmdtokptr);
            break;
        }
        break;
        case T_INCLUDEDEFAULT:
        {
            parsegroupfiles_include(G_DefaultDefFile(), script, cmdtokptr);
            break;
        }
        break;
        case T_NOAUTOLOAD:
            NoAutoLoad = 1;
            break;
        case T_GLOBALGAMEFLAGS:
        {
            if (scriptfile_getnumber(script,&duke3d_m32_globalflags)) break;
        }
        break;
        case T_EOF:
            return 0;
        default:
            break;
        }
    }
    return 0;
}

int loaddefinitions_game(const char *fn, int32_t preload)
{
    scriptfile *script;

    UNREFERENCED_PARAMETER(preload);

    script = scriptfile_fromfile(fn);
    if (script)
        parsegroupfiles(script);

    for (char const * m : g_defModules)
        parsegroupfiles_include(m, NULL, "null");

    if (script)
        scriptfile_close(script);

    scriptfile_clearsymbols();

    return 0;
}

int32_t parsetilegroups(scriptfile *script)
{
    int32_t tokn;
    char *cmdtokptr;

    tokenlist tgtokens[] =
    {
        { "include",         T_INCLUDE          },
        { "#include",        T_INCLUDE          },
        { "define",          T_DEFINE           },
        { "#define",         T_DEFINE           },

        { "dynamicremap",    T_DUMMY            },

        { "tilegroup",       T_TILEGROUP        },
        { "spritehotkey",    T_HOTKEY           },
        { "alphabet",        T_ALPHABET         },
    };

    while (1)
    {
        tokn = getatoken(script,tgtokens,ARRAY_SIZE(tgtokens));
        cmdtokptr = script->ltextptr;
        switch (tokn)
        {
        case T_HOTKEY:
        {
            int32_t i, j;
            if (scriptfile_getsymbol(script,&i)) break;
            if (scriptfile_getsymbol(script,&j)) break;
            if (i < 0 || i > 9 || j < 0 || j >= MAXTILES) break;
            prefixtiles[i] = j;
            break;
        }
        case T_INCLUDE:
        {
            char *fn;
            if (!scriptfile_getstring(script,&fn))
            {
                scriptfile *included;

                included = scriptfile_fromfile(fn);
                if (!included)
                {
                    LOG_F(WARNING, "%s:%d: warning: failed including %s",
                                   script->filename, scriptfile_getlinum(script, cmdtokptr)
                                   fn);
                }
                else
                {
                    parsetilegroups(included);
                    scriptfile_close(included);
                }
            }
            break;
        }
        case T_DEFINE:
        {
            char *name;
            int32_t number;

            if (scriptfile_getstring(script,&name)) break;
            if (scriptfile_getsymbol(script,&number)) break;
            if (scriptfile_addsymbolvalue(name,number) < 0)
                LOG_F(WARNING, "%s:%d: warning: symbol %s was NOT redefined to %d",
                               script->filename, scriptfile_getlinum(script, cmdtokptr)
                               name, number);
            break;
        }
        case T_TILEGROUP:
        {
            char *end, *name;
            int32_t i;

            if (tile_groups >= MAX_TILE_GROUPS) break;
            if (scriptfile_getstring(script,&name)) break;
            if (scriptfile_getbraces(script,&end)) break;

            TileGroup *const tileGrp = &s_TileGroups[tile_groups];

            tileGrp->pIds = (int32_t *)Xcalloc(MAX_TILE_GROUP_ENTRIES, sizeof(int32_t));
            tileGrp->szText = Xstrdup(name);

            while (script->textptr < end)
            {
                static const tokenlist tgtokens2[] =
                {
                    { "tilegroup",  T_TILEGROUP   },
                    { "tile",       T_TILE        },
                    { "tilerange",  T_TILERANGE   },
                    { "hotkey",     T_HOTKEY      },
                    { "tiles",      T_TILES       },
                    { "colors",     T_COLORS      },
                };

                const int32_t token = getatoken(script,tgtokens2,ARRAY_SIZE(tgtokens2));

                switch (token)
                {
                case T_TILE:
                {
                    if (scriptfile_getsymbol(script,&i)) break;
                    if (i >= 0 && i < MAXUSERTILES && tileGrp->nIds < MAX_TILE_GROUP_ENTRIES)
                        tileGrp->pIds[tileGrp->nIds++] = i;
//                    OSD_Printf("added tile %d to group %d\n",i,g);
                    break;
                }
                case T_TILERANGE:
                {
                    int32_t j;
                    if (scriptfile_getsymbol(script,&i)) break;
                    if (scriptfile_getsymbol(script,&j)) break;
                    if (i < 0 || i >= MAXTILES || j < 0 || j >= MAXTILES) break;
                    while (tileGrp->nIds < MAX_TILE_GROUP_ENTRIES && i <= j)
                    {
                        tileGrp->pIds[tileGrp->nIds++] = i++;
//                        OSD_Printf("added tile %d to group %d\n",i,g);
                    }
                    break;
                }
                case T_COLORS:
                {
                    int32_t j;
                    if (scriptfile_getsymbol(script, &i)) break;
                    if (scriptfile_getsymbol(script, &j)) break;
                    if (i < 0 || i >= 256 || j < 0 || j >= 256) break;
                    tileGrp->color1 = i;
                    tileGrp->color2 = j;
                    break;
                }
                case T_HOTKEY:
                {
                    char *c;
                    if (scriptfile_getstring(script,&c)) break;
                    tileGrp->key1 = Btoupper(c[0]);
                    tileGrp->key2 = Btolower(c[0]);
                    break;
                }
                case T_TILES:
                {
                    char *end2;
                    if (scriptfile_getbraces(script,&end2)) break;
                    while (script->textptr < end2-1)
                    {
                        if (!scriptfile_getsymbol(script,&i))
                        {
                            if (i >= 0 && i < MAXTILES && tileGrp->nIds < MAX_TILE_GROUP_ENTRIES)
                                tileGrp->pIds[tileGrp->nIds++] = i;
//                            OSD_Printf("added tile %d to group %d\n",i,g);
                        }
                    }
                    break;
                }
                }
            }

            tileGrp->pIds = (int32_t *)Xrealloc(tileGrp->pIds, tileGrp->nIds*sizeof(int32_t));
            tile_groups++;
            break;
        }
        case T_ALPHABET:
        {
            char *end;
            int32_t i, j, k;

            if (numalphabets >= MAX_ALPHABETS)
            {
                OSD_Printf("Too many alphabet definitions (max: %d).\n", MAX_ALPHABETS);
                break;
            }

            if (scriptfile_getbraces(script,&end)) break;

            for (i=0; i<NUMPRINTABLES; i++)
            {
                alphabets[numalphabets].pic[i] = -1;
                alphabets[numalphabets].xofs[i] = 0;
                alphabets[numalphabets].yofs[i] = 0;
            }

            while (script->textptr < end)
            {
                tokenlist alphtokens2[] =
                {
                    { "map",        T_MAP         },
                    { "mapa",       T_MAPA        },
                    { "maprange",   T_MAPRANGE    },
                    { "maprangea",  T_MAPRANGEA   },
                    { "offset",     T_OFFSET      },
                    { "offseta",    T_OFFSETA     },
                };

                int32_t token = getatoken(script,alphtokens2,ARRAY_SIZE(alphtokens2));
                switch (token)
                {
                case T_MAP:  // map <ascii num> <start tilenum>, e.g. map 46 3002
                {
                    if (scriptfile_getnumber(script,&i)) break;
                    if (scriptfile_getsymbol(script,&j)) break;

                    if (i>=33 && i<=126 && j>= 0 && j<MAXTILES)
                        alphabets[numalphabets].pic[i-33] = j;

                    break;
                }
                case T_MAPA:  // mapa <ascii string> <start tilenum>, e.g. map ".,!?" 3002
                {
                    char *s;
                    if (scriptfile_getstring(script,&s)) break;
                    if (scriptfile_getsymbol(script,&i)) break;

                    for (; *s; s++, i++)
                    {
                        if (*s>=33 && *s<=126 && i>= 0 && i<MAXTILES)
                            alphabets[numalphabets].pic[(*s)-33] = i;
                    }
                    break;
                }
                // maprange <start ascii num> <end ascii num> <start tilenum>, e.g. map 33 126 STARTALPHANUM
                // maprangea <start char> <end char> <start tilenum>, e.g. map "!" "~" STARTALPHANUM
                case T_MAPRANGE:
                case T_MAPRANGEA:
                {
                    if (token==T_MAPRANGE)
                    {
                        if (scriptfile_getnumber(script,&i)) break;
                        if (scriptfile_getnumber(script,&j)) break;
                    }
                    else
                    {
                        char *c1, *c2;
                        if (scriptfile_getstring(script,&c1)) break;
                        if (scriptfile_getstring(script,&c2)) break;
                        i=*c1;
                        j=*c2;
                    }
                    if (scriptfile_getsymbol(script,&k)) break;

                    if (i>126 || j<33) break;
                    for (; i<=j && k<MAXTILES; i++, k++)
                    {
                        if (i>=33 && i<=126)
                            alphabets[numalphabets].pic[i-33] = k;
                    }
                    break;
                }
                case T_OFFSET:  // offset <ascii num> <xoffset> <yoffset>
                {
                    if (scriptfile_getnumber(script, &i)) break;
                    if (scriptfile_getnumber(script, &j)) break;
                    if (scriptfile_getnumber(script, &k)) break;

                    if (i >= 33 && i <= 126)
                    {
                        alphabets[numalphabets].xofs[i-33] = j;
                        alphabets[numalphabets].yofs[i-33] = k;
                    }
                    break;
                }
                case T_OFFSETA:  // offseta <ascii string> <xoffset> <yoffset>
                {
                    char *s;
                    if (scriptfile_getstring(script, &s)) break;
                    if (scriptfile_getnumber(script, &i)) break;
                    if (scriptfile_getnumber(script, &j)) break;

                    for (; *s; s++)
                        if (*s >= 33 && *s <= 126)
                        {
                            alphabets[numalphabets].xofs[(*s)-33] = i;
                            alphabets[numalphabets].yofs[(*s)-33] = j;
                        }
                    break;
                }
                }
            }
            numalphabets++;
            break;
        }
        case T_EOF:
            return 0;
        default:
            break;
        }
    }
    return 0;
}

static int32_t loadtilegroups(const char *fn)
{
    int32_t i, j;
    scriptfile *script;
    TileGroup blank = { NULL, 0, NULL, 0, 0, 0, 0};

    script = scriptfile_fromfile(fn);
    if (!script) return -1;

    for (i=0; i<tile_groups; i++)
    {
        Xfree(s_TileGroups[i].pIds);
        Xfree(s_TileGroups[i].szText);
        Bmemcpy(&s_TileGroups[i], &blank, sizeof(blank));
    }
    tile_groups = 0;
#if 0
    // ---------- Init hardcoded tile group consisting of all named tiles
    s_TileGroups[0].szText = Xstrdup("All named");
    s_TileGroups[0].pIds = (int32_t *)Xmalloc(MAXTILES * sizeof(s_TileGroups[0].pIds[0]));

    j = 0;
    for (i=0; i<MAXTILES; i++)
        if (names[i][0])
            s_TileGroups[0].pIds[j++] = i;
    if (j)
    {
        s_TileGroups[0].nIds = j;
        s_TileGroups[0].key1 = 'Y';
        s_TileGroups[0].key2 = 'y';
        tile_groups++;
    }
    // --------------------
#endif
    parsetilegroups(script);

    scriptfile_close(script);
    scriptfile_clearsymbols();

    tilegroupItems = getTileGroup("Items");
    tilegroupActors = getTileGroup("Actors");

    // Apply 2d sprite colors as specified in tiles.cfg.
    for (i=0; i<tile_groups; i++)
    {
        // If the colors were specified...
        if (s_TileGroups[i].color1 && s_TileGroups[i].color2)
        {
            // Apply the colors to all tiles in the group...
            for (j = s_TileGroups[i].nIds-1; j >= 0 ; j--)
            {
                int const tilenum = s_TileGroups[i].pIds[j];

                // ... but for each tile, only if no color has been specified
                // for it previously.
                if (spritecol2d[tilenum][0] == 0)
                {
                    spritecol2d[tilenum][0] = s_TileGroups[i].color1;
                    spritecol2d[tilenum][1] = s_TileGroups[i].color2;
                }
            }
        }
    }

    return 0;
}

void ExtPreLoadMap(void)
{
}

/// ^^^

static void m32script_interrupt_handler(int signo)
{
    if (signo==SIGINT)
    {
        vm.flags |= VMFLAG_ERROR;
        OSD_Printf("M32 script execution interrupted.\n");
        Bmemset(aEventEnabled, 0, sizeof(aEventEnabled));
    }
}

static void M32_HandleMemErr(int32_t line, const char *file, const char *func)
{
    LOG_F(ERROR, "Out of memory in %s:%d (%s)", file, line, func);
    osdcmd_quit(NULL);
}

int32_t ExtInit(void)
{
    int32_t rv = 0;

    OSD_SetParameters(0, 2, 0, 0, 4, 0, OSD_ERROR, OSDTEXT_YELLOW, OSDTEXT_RED, 0);

    set_memerr_handler(&M32_HandleMemErr);

    if (!g_useCwd)
        G_AddSearchPaths();

    G_ExtInit();

    bpp = 32;

//#ifdef USE_OPENGL
    if (Bstrcmp(setupfilename, SETUPFILENAME))
        LOG_F(INFO, "Using config file \"%s\".",setupfilename);

    if (loadsetup(setupfilename) < 0)
        LOG_F(INFO, "Configuration file not found, using defaults."), rv = 1;
//#endif
    Bmemcpy(buildkeys, default_buildkeys, NUMBUILDKEYS);   //Trick to make build use setup.dat keys

    kensplayerheight = 55; //32
    zmode = 2;
    zlock = kensplayerheight<<8;

    showinvisibility = 1;

    defaultspritecstat = CSTAT_SPRITE_YCENTER;
    g_visibility = 800;

    getmessageleng = 0;
    getmessagetimeoff = 0;

    Bsprintf(apptitle, "NMapedit %s", s_buildRev);
    autosavetimer = (int)totalclock+120*autosave;

    registerosdcommands();

    {
        char *ptr = Xstrdup(setupfilename), *p = strtok(ptr,".");
        if (!Bstrcmp(setupfilename, SETUPFILENAME))
            Bsprintf(tempbuf, "m32_settings.cfg");
        else Bsprintf(tempbuf,"%s_m32_settings.cfg",p);
        OSD_Exec(tempbuf);
        Xfree(ptr);
    }

    // backup pathsearchmode so that a later open
    // will hopefully be the same file
    pathsearchmode_oninit = pathsearchmode;

    //G_ScanGroups();

    signal(SIGINT, m32script_interrupt_handler);

    return rv;
}

void TextFontModify(void)
{
    extern char textfont[];
    char CheckBox[2][8] = {
        { 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF },
        { 0xFF, 0xC3, 0xA5, 0x99, 0x99, 0xA5, 0xC3, 0xFF }
    };
    char RadioButton[2][8] = {
        { 0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C },
        { 0x3C, 0x42, 0x99, 0xBD, 0xBD, 0x99, 0x42, 0x3C }
    };

    Bmemcpy(&textfont[2*8], CheckBox[0], 8);
    Bmemcpy(&textfont[3*8], CheckBox[1], 8);
    Bmemcpy(&textfont[4*8], RadioButton[0], 8);
    Bmemcpy(&textfont[5*8], RadioButton[1], 8);
}

#ifdef USE_QHEAP
unsigned int nMaxAlloc = 0x4000000;
#endif

int32_t ExtPostStartupWindow(void)
{
    G_LoadGroups(!NoAutoLoad);

    if (!g_useCwd)
        G_CleanupSearchPaths();

#ifdef USE_QHEAP
    Resource::heap = new QHeap(nMaxAlloc);
#endif

    // NUKE-TODO: Add ability to specify custom RFF files
    gSysRes.Init("BLOOD.RFF");
    gGuiRes.Init("GUI.RFF");
    gSoundRes.Init("SOUNDS.RFF");

    HookReplaceFunctions();

    FillStringLists();
    TextFontModify();

    scrInit();
    trigInit(gSysRes);
    scrCreateStdColors();
    //sndInit();
    dbInit();

    artLoadFiles("TILES%03i.ART", MAXCACHE1DSIZE);
    //if (engineInit())
    //{
    //    LOG_F(ERROR, "There was a problem initializing the engine.");
    //    return -1;
    //}

    // NUKE-TODO:
    //G_LoadLookups();

    loadtilegroups(default_tiles_cfg);

    ReadHelpFile("m32help.hlp");

    //G_InitMultiPsky(CLOUDYOCEAN, MOONSKY1, BIGORBIT1, LA);

#ifdef LUNATIC
    if (Em_CreateState(&g_EmState) == 0)
    {
        extern const char luaJIT_BC__defs_editor[];

        int32_t i = L_RunString(&g_EmState, luaJIT_BC__defs_editor,
                                LUNATIC_DEFS_M32_BC_SIZE, "_defs_editor.lua");
        if (i != 0)
        {
            Em_DestroyState(&g_EmState);
            LOG_F(ERROR, "Lunatic: Error preparing global Lua state (code %d)", i);
            return -1;
        }
    }
#endif

    return 0;
}

static void (*keytimerstuff)(void) = NULL;

static void timerCallback()
{
    keytimerstuff();
    //gGameClock = totalclock;
}

void ExtPostInit(void)
{
    keytimerstuff = timerSetCallback(timerCallback);

    // InitCustomColors();

    //if (!(duke3d_m32_globalflags & DUKE3D_NO_PALETTE_CHANGES))
    //{
    //    // Make base shade table at shade 0 into the identity map.
    //    // (In the shade table of Duke3D's PALETTE.DAT, palookup[0][239]==143.)
    //    // This makes it possible to sensibly use Lunatic's engine.saveLookupDat().
    //    palookup[0][239] = 239;
    //}

    if (!(duke3d_m32_globalflags & DUKE3D_NO_HARDCODED_FOGPALS))
        paletteSetupDefaultFog();

    palettePostLoadLookups();
}

void ExtUnInit(void)
{
//    int32_t i;
    // setvmode(0x03);
    writesetup(setupfilename);

    // NUKE-TODO:
    //S_SoundShutdown();
    uninitgroupfile();
#if 0
    for (i = MAX_TILE_GROUPS-1; i >= 0; i--)
    {
        Xfree(s_TileGroups[i].pIds);
        Xfree(s_TileGroups[i].szText);
    }
    for (i = numhelppages-1; i >= 0; i--) Xfree(helppage[i]);
    Xfree(helppage);
#endif

   // Duke_CommonCleanup();
    memset(palookup, 0, sizeof(palookup));
    memset(blendtable, 0, sizeof(blendtable));
}

void ExtPreCheckKeys(void) // just before drawrooms
{
    int32_t i = 0, ii;

    if (in3dmode())
    {
        // NUKE-TODO:
#if 0
        if (shadepreview)
        {
            for (i=0; i<highlightsectorcnt; i++)
            {
                ii = highlightsector[i];
                sectorpals[ii][0] = sector[ii].floorpal;
                sectorpals[ii][1] = sector[ii].ceilingpal;

                sector[ii].floorpal = sector[ii].ceilingpal = 6;
            }

//            int32_t i = 0;
            for (i=0; i<MAXSPRITES; i++)
            {
                if (sprite[i].statnum==MAXSTATUS)
                    continue;

                if (sprite[i].picnum == SECTOREFFECTOR &&
                        (sprite[i].lotag == 12 || sprite[i].lotag == 3 || sprite[i].lotag == 4))
                {
                    int32_t w, isec=sprite[i].sectnum;
                    int32_t start_wall;
                    int32_t end_wall;

                    if (isec<0)
                        continue;

                    start_wall = sector[isec].wallptr;
                    end_wall = start_wall + sector[isec].wallnum;

                    for (w = start_wall; w < end_wall; w++)
                    {
                        if (!(wallflag[w>>3]&(1<<(w&7))))
                        {
                            wallshades[w] = wall[w].shade;
                            wallpals[w] = wall[w].pal;

                            wall[w].shade = sprite[i].shade;
                            wall[w].pal = sprite[i].pal;

                            wallflag[w>>3] |= (1<<(w&7));
                        }
                        // removed: same thing with nextwalls
                    }
                    sectorshades[isec][0] = sector[isec].floorshade;
                    sectorshades[isec][1] = sector[isec].ceilingshade;
                    sector[isec].floorshade = sprite[i].shade;
                    sector[isec].ceilingshade = sprite[i].shade;

                    sectorpals[isec][0] = sector[isec].floorpal;
                    sectorpals[isec][1] = sector[isec].ceilingpal;
                    sector[isec].floorpal = sprite[i].pal;
                    sector[isec].ceilingpal = sprite[i].pal;

                    for (w = headspritesect[isec]; w >= 0; w = nextspritesect[w])
                    {
                        if (w == i)
                            continue;

                        spriteshades[w] = sprite[w].shade;
                        spritepals[w] = sprite[w].pal;

                        sprite[w].shade = sprite[i].shade;
                        sprite[w].pal = sprite[i].pal;
                    }
                }
                else if (sprite[i].picnum == SECTOREFFECTOR && (sprite[i].lotag == 49 || sprite[i].lotag == 50))
                {
#ifdef POLYMER
                    if (sprite[i].lotag == 49)
                    {
                        if (videoGetRenderMode() == REND_POLYMER)
                        {
                            if (spritelightptr[i] == NULL)
                            {
#pragma pack(push,1)
                                _prlight mylight;
#pragma pack(pop)
                                addprlight_common1(&mylight, i);
                            }
                            else
                            {
                                if (Bmemcmp(&sprite[i], spritelightptr[i], sizeof(vec3_t)))
                                {
                                    Bmemcpy(spritelightptr[i], &sprite[i], sizeof(vec3_t));
                                    spritelightptr[i]->sector = sprite[i].sectnum;
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if (SHT(i) != spritelightptr[i]->range)
                                {
                                    spritelightptr[i]->range = SHT(i);
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if (check_prlight_colors(i))
                                    copy_prlight_colors(spritelightptr[i], i);
                                if ((int)!!(CS(i) & 128) != spritelightptr[i]->publicflags.negative)
                                {
                                    spritelightptr[i]->publicflags.negative = !!(CS(i) & 128);
                                }
                            }
                        }
                    }
                    if (sprite[i].lotag == 50)
                    {
                        if (videoGetRenderMode() == REND_POLYMER)
                        {
                            if (spritelightptr[i] == NULL)
                            {
#pragma pack(push,1)
                                _prlight mylight;
#pragma pack(pop)
                                mylight.radius = (256-(SS(i)+128))<<1;
                                mylight.faderadius = (int16_t)(mylight.radius * 0.75f);
                                mylight.tilenum = OW(i);
                                mylight.publicflags.emitshadow = !(CS(i) & 64);

                                addprlight_common1(&mylight, i);
                            }
                            else
                            {
                                if (Bmemcmp(&sprite[i], spritelightptr[i], sizeof(vec3_t)))
                                {
                                    Bmemcpy(spritelightptr[i], &sprite[i], sizeof(vec3_t));
                                    spritelightptr[i]->sector = sprite[i].sectnum;
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if (SHT(i) != spritelightptr[i]->range)
                                {
                                    spritelightptr[i]->range = SHT(i);
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if (check_prlight_colors(i))
                                    copy_prlight_colors(spritelightptr[i], i);
                                if (((256-(SS(i)+128))<<1) != spritelightptr[i]->radius)
                                {
                                    spritelightptr[i]->radius = (256-(SS(i)+128))<<1;
                                    spritelightptr[i]->faderadius = (int16_t)(spritelightptr[i]->radius * 0.75f);
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if (SA(i) != spritelightptr[i]->angle)
                                {
                                    spritelightptr[i]->angle = SA(i);
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if (SH(i) != spritelightptr[i]->horiz)
                                {
                                    spritelightptr[i]->horiz = SH(i);
                                    spritelightptr[i]->flags.invalidate = 1;
                                }
                                if ((int)!(CS(i) & 64) != spritelightptr[i]->publicflags.emitshadow)
                                {
                                    spritelightptr[i]->publicflags.emitshadow = !(CS(i) & 64);
                                }
                                if ((int)!!(CS(i) & 128) != spritelightptr[i]->publicflags.negative)
                                {
                                    spritelightptr[i]->publicflags.negative = !!(CS(i) & 128);
                                }
                                spritelightptr[i]->tilenum = OW(i);
                            }
                        }
                    }
#endif // POLYMER
                }
            }
        }

        if (floor_over_floor) SE40Code(pos.x,pos.y,pos.z,ang,horiz);
#endif
        if (purpleon) videoClearViewableArea(255);

        return;
    }


    videoBeginDrawing();  //{{{

    //    if (cursectornum >= 0)
    //        fillsector(cursectornum, 31);

    if (graphicsmode && (!m32_sideview || m32_sideelev == 512) && zoom >= 256)
    {
        for (i=ii=0; i<MAXSPRITES && ii < Numsprites; i++)
        {
            int32_t daang = 0, flags, shade;
            int32_t picnum, frames;
            int32_t xp1, yp1;

            if ((sprite[i].cstat & 48) != 0 || sprite[i].statnum == MAXSTATUS) continue;
            ii++;
            picnum = sprite[i].picnum;
            daang = flags = frames = shade = 0;

#if 0
            switch (picnum)
            {
                // 5-frame walk
            case 1550 :             // Shark
                frames=5;
                fallthrough__;
                // 2-frame walk
            case 1445 :             // duke kick
            case LIZTROOPDUCKING :
            case 2030 :            // pig shot
            case OCTABRAIN :
            case PIGCOPDIVE :
            case 2190 :            // liz capt shot
            case BOSS1SHOOT :
            case BOSS1LOB :
            case LIZTROOPSHOOT :
                if (frames==0) frames=2;
                fallthrough__;
                // 4-frame walk
            case 1491 :             // duke crawl
            case LIZTROOP :
            case LIZTROOPRUNNING :
            case PIGCOP :
            case LIZMAN :
            case BOSS1 :
            case BOSS2 :
            case BOSS3 :
            case BOSS4 :
            case NEWBEAST:
                if (frames==0) frames=4;
                fallthrough__;
            case LIZTROOPJETPACK :
            case DRONE :
            case COMMANDER :
            case TANK :
            case RECON :
                if (frames==0) frames = 10;
                fallthrough__;
            case CAMERA1:
            case APLAYER :
                if (frames==0) frames=1;
                fallthrough__;
            case GREENSLIME :
            case EGG :
            case PIGCOPSTAYPUT :
            case LIZMANSTAYPUT:
            case LIZTROOPSTAYPUT :
            case LIZMANSPITTING :
            case LIZMANFEEDING :
            case LIZMANJUMP :
            case NEWBEASTSTAYPUT :
            case BOSS1STAYPUT :
            {
                int32_t k;
                if (frames!=0)
                {
                    if (frames==10) frames=0;
                    k = 1536;//getangle(tspr->x-pos.x,tspr->y-pos.y);
                    k = (((sprite[i].ang+3072+128-k+(m32_sideview ? (2048 - m32_sideang) : 0))&2047)>>8)&7;
                    //This guy has only 5 pictures for 8 angles (3 are x-flipped)
                    if (k <= 4)
                    {
                        picnum += k;
                        daang = 0;
                        flags &= ~4;
                    }
                    else
                    {
                        picnum += 8-k;
                        daang = 1024;
                        flags |= 4;
                    }
                }

                if (graphicsmode == 2)
                {
                    if (frames==2) picnum+=((((4-(totalclock>>5)))&1)*5);
                    if (frames==4) picnum+=((((4-(totalclock>>5)))&3)*5);
                    if (frames==5) picnum+=(((totalclock>>5)%5))*5;
                }

                if (tilesiz[picnum].x == 0)
                    picnum -= 5;       //Hack, for actors
            }
            break;
            default:
                break;
            }
#endif
            shade = (i+16384 == pointhighlight) ? 7 - (M32_THROB>>1) : sprite[i].shade;

            if (m32_sideview)
            {
                editorGet2dScreenCoordinates(&xp1, &yp1, sprite[i].x-pos.x, sprite[i].y-pos.y, zoom);
                yp1 += midydim16 + getscreenvdisp(sprite[i].z-pos.z, zoom);
                yp1 -= mulscale14(tilesiz[picnum].y<<2, zoom);
                xp1 += halfxdim16;
            }
            else
                ovhscrcoords(sprite[i].x, sprite[i].y-(tilesiz[picnum].y<<2), &xp1, &yp1);

            ydim16 = ydim - STATUS2DSIZ2;  // XXX?

            int f = mulscale12(128, zoom);

            if (xp1 < -f || xp1 > xdim+f || yp1 < -f || yp1 > ydim16+f)
                continue;

            const int32_t oviewingrange=viewingrange, oyxaspect=yxaspect;
            renderSetAspect(yxaspect, divscale16(sprite[i].yrepeat, sprite[i].xrepeat));
            rotatesprite(xp1<<16,yp1<<16,zoom<<5,daang,picnum,
                         shade,sprite[i].pal,flags|1024,0,0,xdim-1,ydim16-1);
            renderSetAspect(oviewingrange, oyxaspect);
        }
    }

#if 0
    if (showambiencesounds)
    {
        for (ii=0; ii<numsectors; ii++)
            for (i=headspritesect[ii]; i>=0; i=nextspritesect[i])
            {
                int32_t radius, col;
                int32_t xp1, yp1;

                //if (sprite[i].picnum != MUSICANDSFX /*|| zoom < 256*/ || sprite[i].hitag < 1000)
                //    continue;

                if (showambiencesounds==1 && sprite[i].sectnum!=cursectnum)
                    continue;

                editorGet2dScreenCoordinates(&xp1,&yp1, sprite[i].x-pos.x,sprite[i].y-pos.y, zoom);
                if (m32_sideview)
                    yp1 += getscreenvdisp(sprite[i].z-pos.z, zoom);

                drawlinepat = 0xf0f0f0f0;

                col = editorcolors[6];
                if (i+16384 == pointhighlight)
                {
                    radius = mulscale14(sprite[i].hitag - (M32_THROB<<2), zoom);
                    col += M32_THROB>>1;
                }
                else radius = mulscale14(sprite[i].hitag, zoom);

                editorDraw2dCircle(halfxdim16+xp1, midydim16+yp1, radius, scalescreeny(16384), col);
                drawlinepat = 0xffffffff;
            }
    }
#endif

    if (zoom >= 256)
    {
        for (ii=0; ii<numsectors; ii++)
            for (i=headspritesect[ii]; i>=0; i=nextspritesect[i])
            {
                int32_t radius1, radius2, col1, col2;
                int32_t xp1, yp1, xp2, yp2;
                int16_t const angofs = m32_sideview ? m32_sideang : 0;

                editorGet2dScreenCoordinates(&xp1,&yp1, sprite[i].x-pos.x,sprite[i].y-pos.y, zoom);
                if (m32_sideview)
                    yp1 += getscreenvdisp(sprite[i].z-pos.z, zoom);

                //if (sprite[i].picnum != MUSICANDSFX /*|| zoom < 256*/ || sprite[i].hitag < 1000)
                //    continue;
                if (sprite[i].statnum == 10 && sprite[i].type == 3)
                {
                    int nSector = sprite[i].owner;
                    int nXSector = sector[nSector].extra;
                    dassert(nXSector > 0 && nXSector < kMaxXSectors);
                    int k = xsector[nXSector].marker1;
                    editorGet2dScreenCoordinates(&xp2,&yp2, sprite[k].x-pos.x,sprite[k].y-pos.y, zoom);
                    if (m32_sideview)
                        yp2 += getscreenvdisp(sprite[k].z-pos.z, zoom);
                    col2 = editorcolors[9];
                    if (pointhighlight >= 16384 &&
                        (i+16384 == pointhighlight ||
                        (!m32_sideview && (sprite[i].x == sprite[pointhighlight-16384].x &&
                            sprite[i].y == sprite[pointhighlight-16384].y))))
                    {
                        col2 -= M32_THROB>>2;
                    }
                    editorDraw2dLine(halfxdim16+xp1, midydim16+yp1, halfxdim16+xp2, midydim16+yp2, col2);
        
                    int ang = getangle(sprite[i].x-sprite[k].x, sprite[i].y-sprite[k].y);
                    int dx = mulscale30(zoom/64, Cos(ang+angofs+170));
                    int dy = mulscale30(zoom/64, Sin(ang+angofs+170));
                    dy = scalescreeny(dy);
                    editorDraw2dLine(halfxdim16+xp2, midydim16+yp2, halfxdim16+xp2+dx, midydim16+yp2+dy, col2);
                    dx = mulscale30(zoom/64, Cos(ang+angofs-170));
                    dy = mulscale30(zoom/64, Sin(ang+angofs-170));
                    dy = scalescreeny(dy);
                    editorDraw2dLine(halfxdim16+xp2, midydim16+yp2, halfxdim16+xp2+dx, midydim16+yp2+dy, col2);
                }

                if (showambiencesounds && sprite[i].statnum == 12)
                {
                    int nXSprite = sprite[i].extra;
                    if (nXSprite > 0)
                    {
                        dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
                        XSPRITE* pXSprite = &xsprite[nXSprite];
                        if (showambiencesounds==1 && sprite[i].sectnum!=cursectnum)
                            continue;

                        //drawlinepat = 0xf0f0f0f0;

                        col1 = editorcolors[14];
                        col2 = editorcolors[6];
                        if (i+16384 == pointhighlight)
                        {
                            radius1 = mulscale10(ClipLow(pXSprite->data1 - (M32_THROB<<2), 0), zoom);
                            radius2 = mulscale10(ClipLow(pXSprite->data2 - (M32_THROB<<2), 0), zoom);
                            col1 += M32_THROB>>2;
                            col2 += M32_THROB>>2;
                        }
                        else
                        {
                            radius1 = mulscale10(pXSprite->data1, zoom);
                            radius2 = mulscale10(pXSprite->data2, zoom);
                        }

                        editorDraw2dCircle(halfxdim16+xp1, midydim16+yp1, radius1, scalescreeny(16384), col1);
                        editorDraw2dCircle(halfxdim16+xp1, midydim16+yp1, radius2, scalescreeny(16384), col2);
                        //drawlinepat = 0xffffffff;
                    }
                }
            }

        gXTracker.Draw();
    }

    videoEndDrawing();  //}}}
}

void ExtAnalyzeSprites(int32_t ourx, int32_t oury, int32_t ourz, int32_t oura, int32_t smoothr)
{
    int32_t i, k;
    tspriteptr_t tspr;
    int32_t frames=0, sh;

    UNREFERENCED_PARAMETER(ourx);
    UNREFERENCED_PARAMETER(oury);
    UNREFERENCED_PARAMETER(oura);
    UNREFERENCED_PARAMETER(smoothr);

    for (i=0,tspr=&tsprite[0]; i<spritesortcnt; i++,tspr++)
    {
        frames=0;

        if ((nosprites==1||nosprites==3) && tspr->picnum<11)
            tspr->xrepeat=0;

        //if (nosprites==1||nosprites==3)
        //    switch (tspr->picnum)
        //    {
        //    case SEENINE :
        //        tspr->xrepeat=0;
        //    }

        if (showinvisibility && (tspr->cstat&32768))
        {
            tspr->pal = 6;
            tspr->cstat &= (uint16_t)~32768;
            tspr->cstat |= 2+512;
        }

        /* Shade preview rules (thanks to Gambini)
         *
         * 1st rule: Any pal value not equal to 0 in the floor of a sector will
         *           turn all the sprites within this sector to that pal value.
         *
         * 2nd rule: The shade of a sprite will be taken from the floor unless the
         *           ceiling is parallaxed, in which case will be taken from the
         *           ceiling. But not the pal which always follow the 1st rule.
         *
         * 3rd rule: relative to wall sprites will keep their own shade unless
         *           they're actors, but they will still retain the floor pal.
         */
        if (shadepreview)
        {
            if (tspr->sectnum<0)
                continue;

            const int32_t wallaligned = (tspr->cstat & 16);
            const int32_t fpal = sector[tspr->sectnum].floorpal;

            // 1st rule
            // Compare with game.c:G_MaybeTakeOnFloorPal()
            if (fpal && !g_noFloorPal[fpal])
                tspr->pal = fpal;

            // 2nd and 3rd rule minus "actor condition"
            if (!wallaligned && (tspr->cstat&CSTAT_SPRITE_NOSHADE)==0)
            {
                if (sector[tspr->sectnum].ceilingstat&1)
                    sh = sector[tspr->sectnum].ceilingshade;
                else
                    sh = sector[tspr->sectnum].floorshade;

                inpclamp(&sh, -127, 127);
                tspr->shade = sh;
            }
        }

        switch (tspr->picnum)
        {
#if 0
            // 5-frame walk
        case 1550 :             // Shark
            frames=5;
            fallthrough__;
            // 2-frame walk
        case 1445 :             // duke kick
        case LIZTROOPDUCKING :
        case 2030 :            // pig shot
        case OCTABRAIN :
        case PIGCOPDIVE :
        case 2190 :            // liz capt shot
        case BOSS1SHOOT :
        case BOSS1LOB :
        case LIZTROOPSHOOT :
            if (frames==0) frames=2;
            fallthrough__;
            // 4-frame walk
        case 1491 :             // duke crawl
        case LIZTROOP :
        case LIZTROOPRUNNING :
        case PIGCOP :
        case LIZMAN :
        case BOSS1 :
        case BOSS2 :
        case BOSS3 :
        case BOSS4 :
        case NEWBEAST:
            if (frames==0) frames=4;
            fallthrough__;
        case LIZTROOPJETPACK :
        case DRONE :
        case COMMANDER :
        case TANK :
        case RECON :
            if (frames==0) frames = 10;
            fallthrough__;
        case ROTATEGUN :
        case CAMERA1:
        case APLAYER :
            if (frames==0) frames=1;
            fallthrough__;
        case GREENSLIME :
        case PIGCOPSTAYPUT :
        case LIZMANSTAYPUT:
        case LIZTROOPSTAYPUT :
        case LIZMANSPITTING :
        case LIZMANFEEDING :
        case LIZMANJUMP :
        case NEWBEASTSTAYPUT :
        case BOSS1STAYPUT :
            if (skill!=4)
            {
                if (tspr->lotag>skill+1)
                {
                    tspr->xrepeat=0;
                    tspr->cstat=32768;
                    break;
                }
            }
            if (nosprites==2||nosprites==3)
            {
                tspr->xrepeat=0;
                tspr->cstat=32768;
            }
            //                else tspr->cstat&=32767;

#ifdef USE_OPENGL
            if (!usemodels || md_tilehasmodel(tspr->picnum,tspr->pal) < 0)
#endif
            {
                if (frames!=0)
                {
                    if (frames==10) frames=0;
                    k = getangle(tspr->x-pos.x,tspr->y-pos.y);
                    k = (((tspr->ang+3072+128-k)&2047)>>8)&7;
                    //This guy has only 5 pictures for 8 angles (3 are x-flipped)
                    if (k <= 4)
                    {
                        tspr->picnum += k;
                        tspr->cstat &= ~4;   //clear x-flipping bit
                    }
                    else
                    {
                        tspr->picnum += 8-k;
                        tspr->cstat |= 4;    //set x-flipping bit
                    }
                }

                if (frames==2) tspr->picnum += (((4-(totalclock>>5)))&1)*5;
                if (frames==4) tspr->picnum += (((4-(totalclock>>5)))&3)*5;
                if (frames==5) tspr->picnum += ((totalclock>>5)%5)*5;

                if (tilesiz[tspr->picnum].x == 0)
                    tspr->picnum -= 5;       //Hack, for actors
            }
            break;
#endif

        default:
            break;
        }
    }
}

#define MESSAGEX 3 // (xdimgame>>1)
#define MESSAGEY 3 // ((i/charsperline)<<3)+(ydimgame-(ydimgame>>3))-(((getmessageleng-1)/charsperline)<<3)

static void Keys2d3d(void)
{
    int32_t i;

#if M32_UNDO
    if (mapstate == NULL)
    {
        //        map_revision = 0;
        create_map_snapshot(); // initial map state
        //        Xfree(mapstate->next);
        //        mapstate = mapstate->prev;
    }
#endif

    // NUKE-TODO:
    //if (cursectnum>=0 && sector[cursectnum].lotag==2)
    //{
    //    if (sector[cursectnum].ceilingpicnum==FLOORSLIME)
    //        SetGamePalette(SLIMEPAL);
    //    else
    //        SetGamePalette(WATERPAL);
    //}
    //else SetGamePalette(BASEPAL);

    if (keystatus[KEYSC_F10])
    {
        keystatus[KEYSC_F10]=0;

        if (!m32_2d3d_resolutions_match())
            message("2d and 3d mode resolutions don't match!");

        else if (!in3dmode())
        {
            if (eitherSHIFT)
            {
                // shrinking the size while in a corner pulls the PIP into that corner
                if (m32_2d3d.x + XSIZE_2D3D >= xdim2d - 5)
                    m32_2d3d.x = 0xffff;

                if (m32_2d3d.y + YSIZE_2D3D >= ydim2d - 5 - STATUS2DSIZ2)
                    m32_2d3d.y = 0xffff;

                if (++m32_2d3dsize > 5)
                    m32_2d3dsize = 3;

                message("2d3d size %d", m32_2d3dsize);
            }
            else
            {
                m32_2d3dmode = !m32_2d3dmode;
                message("2d3d mode %s", m32_2d3dmode ? "enabled" : "disabled");
            }
        }
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(A)) // 'A
    {
        if (in3dmode())
            autosave = autosave?0:getnumber256("Autosave interval, in seconds: ",180,3600,0);
        else
            autosave = autosave?0:getnumber16("Autosave interval, in seconds: ",180,3600,0);

        if (autosave) message("Autosave enabled, interval: %d seconds",autosave);
        else message("Autosave disabled");
    }

    if (keystatus[KEYSC_QUOTE] && PRESSED_KEYSC(N)) // 'N
    {
        m32_clipping--;
        if (m32_clipping < 0)
            m32_clipping = 2;
        message("Clipping %s", m32_clipping==0 ? "disabled" :
                (m32_clipping==1 ? "non-masks only" : "enabled"));
    }

    if (eitherCTRL && PRESSED_KEYSC(N)) // CTRL+N
    {
        spnoclip = !spnoclip;
        message("Sprite clipping %s", spnoclip?"disabled":"enabled");
    }

    if (eitherCTRL)  //CTRL
    {
        if (!in3dmode())
            if (PRESSED_KEYSC(P)) // Ctrl-P: Map playtesting
                test_map(eitherALT);

        if (PRESSED_KEYSC(Z)) // CTRL+Z
        {
            if (!in3dmode() || m32_3dundo)
            {
                if (eitherSHIFT)
                {
                    if (map_undoredo(1))
                        message("Nothing to redo!");
                    else
                        message("Redo: restored revision %d", map_revision - 1);
                }
                else
                {
                    if (map_undoredo(0))
                        message("Nothing to undo!");
                    else
                        message("Undo: restored revision %d", map_revision - 1);
                }

                updatesectorz(pos.x, pos.y, pos.z, &cursectnum);

                // kick the user back to 2d mode if the sector they were in was deleted by the undo/redo operation
                if (cursectnum == -1 && (in3dmode() && !m32_is2d3dmode()))
                    keystatus[buildkeys[BK_MODE2D_3D]] = 1;
            }
        }

        if (keystatus[KEYSC_S]) // S
        {
            if (levelname[0])
            {
                keystatus[KEYSC_S] = 0;

                i = CheckMapCorruption(4, 0);
                if (i<4)
                {
                    Bsprintf(tempbuf, "Save to %s?", levelname);
                    if (!AskIfSure(tempbuf))
                    {
                        if (SaveBoard(levelname, M32_SB_ASKOV) != NULL)
                        {
                            message("Board saved to %s", levelname);
                            asksave = 0;
                            lastsave=(int)totalclock;
                        }
                    }
                }
                else
                    message("Map is heavily corrupted, not saving. See OSD for details.");
            }
        }
        if (keystatus[KEYSC_L]) // L
        {
            if (totalclock < (lastsave + 120*10) || !AskIfSure("Are you sure you want to load the last saved map?"))
            {
                int32_t sposx=pos.x,sposy=pos.y,sposz=pos.z,sang=ang;
                const char *f = GetSaveBoardFilename(levelname);

                lastsave=(int)totalclock;
                //  			  sectorhighlightstat = -1;
                //  			  newnumwalls = -1;
                //  			  joinsector[0] = -1;
                //  			  circlewall = -1;
                //  			  circlepoints = 7;

                if (LoadBoard(f, 0))
                    message("Invalid map format.");

                pos.x=sposx; pos.y=sposy; pos.z=sposz; ang=sang;
                updatesectorz(pos.x, pos.y, pos.z, &cursectnum);

                keystatus[KEYSC_L] = 0;
            }
        }
    }

    if (keystatus[buildkeys[BK_MODE2D_3D]])  // Enter
    {
        getmessageleng = 0;
        getmessagetimeoff = 0;
    }

    if (getmessageleng > 0)
    {
        if (!in3dmode())
            printmessage16("%s", getmessage);
        if ((int)totalclock > getmessagetimeoff)
            getmessageleng = 0;
    }

}

void ExtCheckKeys(void)
{
    static int32_t soundinit = 0;
    static int32_t lastbstatus = 0;

    gFrameTicks = totalclock - gFrameClock;
    gFrameClock += gFrameTicks;

    if (!soundinit)
    {
        SoundToggle = 1;
        NumVoices = 32;
        NumChannels = 2;
        MixRate = 44100;
        FXVolume = 255;
        sndInit();
        soundinit = 1;
    }
    sndProcess();

    // NUKE-TODO:
#if 0
    if (!soundinit)
    {
        g_numsounds = 0;

        loadconsounds(G_ConFile());

        if (g_numsounds > 0)
        {
            if (S_SoundStartup() != 0)
                S_SoundShutdown();
        }
        soundinit = 1;
    }
#endif

    // NUKE-TODO:
#if 0
    if (in3dmode() && shadepreview)
    {
        int32_t i = 0, ii;
        int32_t w, isec, start_wall, end_wall;

        for (i=0; i<highlightsectorcnt; i++)
        {
            ii = highlightsector[i];
            sector[ii].floorpal = sectorpals[ii][0];
            sector[ii].ceilingpal = sectorpals[ii][1];
        }

        for (i=0; i<MAXSPRITES; i++)
        {
            if (sprite[i].statnum==MAXSTATUS || sprite[i].picnum != SECTOREFFECTOR)
                continue;

            if (sprite[i].lotag != 12 && sprite[i].lotag != 3 && sprite[i].lotag != 4)
                continue;

            isec = sprite[i].sectnum;
            if (isec<0)
                continue;

            start_wall = sector[isec].wallptr;
            end_wall = start_wall + sector[isec].wallnum;

            for (w = start_wall; w < end_wall; w++)
            {
                if (wallflag[w>>3]&(1<<(w&7)))
                {
                    wall[w].shade = wallshades[w];
                    wall[w].pal = wallpals[w];
                    wallflag[w>>3] &= ~(1<<(w&7));
                }
                // removed: same thing with nextwalls
            }
            sector[isec].floorshade = sectorshades[isec][0];
            sector[isec].ceilingshade = sectorshades[isec][1];
            sector[isec].floorpal = sectorpals[isec][0];
            sector[isec].ceilingpal = sectorpals[isec][1];

            for (w=headspritesect[isec]; w>=0; w=nextspritesect[w])
            {
                if (w == i)
                    continue;
                sprite[w].shade = spriteshades[w];
                sprite[w].pal = spritepals[w];
            }

        }
    }
#endif

    lastbstatus = bstatus;
    bstatus = mouseReadButtons();

    Keys2d3d();

    // 2d3d mode
    if ((in3dmode() && !m32_is2d3dmode()) || m32_is2d3dmode())
    {
#ifdef USE_OPENGL
        int bakrendmode = rendmode;

        if (m32_is2d3dmode())
            rendmode = REND_CLASSIC;
#endif

        Keys3d();
        editinput();
        if (infobox&2)
            m32_showmouse();

#ifdef USE_OPENGL
        rendmode = bakrendmode;
#endif
    }
    else if (!in3dmode())
    {
        Keys2d();

        if (autocorruptcheck>0 && totalclock > corruptchecktimer)
        {
            if (CheckMapCorruption(3, 0)>=3)
                printmessage16("Corruption detected. See OSD for details.");
            corruptchecktimer = (int)totalclock + 120*autocorruptcheck;
        }
    }

    if (asksave == 1)
        asksave++;
    else if (asksave == 2 && (bstatus + lastbstatus) == 0
#if M32_UNDO
             && mapstate
#endif
        )
    {
        int32_t i;
        // check keys so that e.g. bulk deletions won't produce
        // as much revisions as deleted sprites
        for (i=ARRAY_SIZE(keystatus)-1; i>=0; i--)
            if (keystatus[i])
                break;
        if (i==-1)
        {
#if M32_UNDO
            create_map_snapshot();
#else
            CheckMapCorruption(5, 0);
#endif
            asksave++;
        }
    }
    else if (asksave == 3)
        asksave++;

    if (totalclock > autosavetimer && autosave)
    {
        if (asksave == 4)
        {
            if (CheckMapCorruption(5, 0)>=4)
            {
                if (SaveBoard("autosave_corrupt.map", M32_SB_NOEXT) != NULL)
                    message("Board autosaved to AUTOSAVE_CORRUPT.MAP");
            }
            else
            {
                if (SaveBoard("autosave.map", 0) != NULL)
                    message("Board autosaved to AUTOSAVE.MAP");
            }

            asksave++;
        }
        autosavetimer = (int)totalclock+120*autosave;
    }

    if (PRESSED_KEYSC(F12))   //F12
    {
#ifdef ENGINE_SCREENSHOT_DEBUG
        extern int32_t engine_screenshot;
        engine_screenshot = 1;
#else
        videoCaptureScreen("captxxxx.tga", eitherSHIFT);
        silentmessage("Saved screenshot");
#endif
    }
}

////

void faketimerhandler(void)
{
    timerUpdateClock();
}

void SetGamePalette(int32_t palid)
{
    if ((unsigned)palid >= MAXBASEPALS)
        palid = 0;

    videoSetPalette(GAMMA_CALC, palid, 2);
}

static void SearchSectors(int32_t dir)  // <0: backwards, >=0: forwards
{
    int32_t ii=0;
    dir = 1-2*(dir<0);

    if (cursector_lotag!=0)
    {
        if ((dir>0 && cursectornum<MAXSECTORS) || (dir<0 && cursectornum>=0))
            cursectornum += dir;

        for (ii=cursectornum; ii>=0 && ii<numsectors; ii+=dir)
        {
            if (sector[ii].lotag==cursector_lotag)
            {
                pos.x = wall[sector[ii].wallptr].x;
                pos.y = wall[sector[ii].wallptr].y;
                printmessage16("%s Sector search: found", dir<0?"<":">");
                cursectornum = ii;
                return;
            }
        }
    }
    printmessage16("%s Sector search: none found", dir<0?"<":">");
}


//////////////////// manual editing ////////////////////
// Build edit originally by Ed Coolidge <semicharm@earthlink.net>

static char med_disptext[80];
static char med_edittext[80];
static const char *med_typename="";
static int32_t med_dispwidth=24;
static int32_t med_editval=0;
static int32_t med_thenum=-1;

static void handlemed(int32_t dohex, const char *disp_membername, const char *edit_membername,
                      void *themember, int32_t thesizeof, int32_t themax, int32_t sign)
{
    int32_t i, val;
    int32_t flags = sign;
    sign &= 1;

    assert(!dohex || thesizeof == sizeof(int16_t));

    if (thesizeof==sizeof(int8_t))
    {
        if (sign)
            val = *(int8_t *)themember;
        else
            val = *(uint8_t *)themember;
    }
    else if (thesizeof==sizeof(int16_t))
    {
        val = *(int16_t *)themember;
        // Bug fix : Do not sign extend when dealing with hex values
        if (dohex)
            val &= 0xFFFF;
    }
    else //if (thesizeof==sizeof(int32_t))
        val = *(int32_t *)themember;

    if (dohex)
        i=Bsprintf(med_disptext,"%s: %x", disp_membername, val);
    else
        i=Bsprintf(med_disptext,"%s: %d", disp_membername, val);

    for (; i<med_dispwidth; i++)
        med_disptext[i] = ' ';

    if (med_editval)
    {
        // If editing a hex value and either SHIFT is pressed then toggle a specific bit
        if (SHIFTS_IS_PRESSED && dohex)
        {
            // Get highest bit in maximum value
            int32_t max_bit = 0;
            while (max_bit < 15 && 1<<(max_bit+1) < themax)
                max_bit++;

            Bsprintf(med_edittext, "Toggle %s %d %s Bit 0..%d: ", med_typename, med_thenum, edit_membername, max_bit);
            printmessage16("%s", med_edittext);

            const int32_t bit = getnumber16(med_edittext, -1, max_bit, 1);
            if (bit >= 0)
                *(int16_t *)themember = (int16_t)((1<<bit) ^ (*(int16_t *)themember));
        }
        // Else directly edit value
        else
        {
            Bsprintf(med_edittext,"%s %d %s: ", med_typename, med_thenum, edit_membername);

            printmessage16("%s", med_edittext);
            val = getnumber16(med_edittext, val, themax, flags);

            if (thesizeof==sizeof(int8_t))
            {
                if (sign)
                    *(int8_t *)themember = (int8_t)val;
                else
                    *(uint8_t *)themember = (uint8_t)val;
            }
            else if (thesizeof==sizeof(int16_t))
                *(int16_t *)themember = (int16_t)val;
            else //if (thesizeof==sizeof(int32_t))
                *(int32_t *)themember = val;
        }
    }
}

static void med_printcurline(int32_t xpos, int32_t ypos, int32_t row, int32_t selected)
{
    printext16(xpos,ypos+row*8,editorcolors[11],editorcolors[!!selected],med_disptext,0);
}

static void med_handlecommon(int32_t xpos, int32_t ypos, int32_t *row, int32_t rowmax)
{
    idle_waitevent();
    if (handleevents())
        quitevent = 0;

    _printmessage16("Edit mode, press <Esc> to exit");

    if (PRESSED_KEYSC(DOWN))
    {
        if (*row < rowmax)
        {
            med_printcurline(xpos, ypos, *row, 0);
            (*row)++;
        }
    }

    if (PRESSED_KEYSC(UP))
    {
        if (*row > 0)
        {
            med_printcurline(xpos, ypos, *row, 0);
            (*row)--;
        }
    }

    if (PRESSED_KEYSC(ENTER))
        med_editval = 1;
}

static void EditSectorData(int16_t sectnum)
{
    int32_t col=1, row=0, rowmax = 6;
    int32_t i = -1;
    int32_t xpos = 208, ypos = ydim-STATUS2DSIZ+48;

    med_editval = 0;

    med_dispwidth = 24;
    med_disptext[med_dispwidth] = 0;

    med_typename = "Sector";
    med_thenum = sectnum;

    drawgradient();

    showsectordata(sectnum, 0);

    while (keystatus[KEYSC_ESC] == 0)
    {
        med_handlecommon(xpos, ypos, &row, rowmax);

        if (PRESSED_KEYSC(LEFT))
        {
            if (col == 2)
            {
                med_printcurline(xpos, ypos, row, 0);
                col = 1;
                xpos = 208;
                med_disptext[med_dispwidth] = 0;
                if (row > rowmax) row = rowmax;
            }
        }
        if (PRESSED_KEYSC(RIGHT))
        {
            if (col == 1)
            {
                med_printcurline(xpos, ypos, row, 0);
                col = 2;
                xpos = 408;
                med_disptext[med_dispwidth] = 0;
                if (row > rowmax) row = rowmax;
            }
        }

#ifdef YAX_ENABLE
        if (med_editval)
        {
            if ((row==1 || row==3 || row==5) && yax_getbunch(sectnum, (col==2)) >= 0)
                med_editval = 0;
        }
#endif

        if (col == 1)
        {
            switch (row)
            {
            case 0:
#ifdef YAX_ENABLE__COMPAT
                i = sector[sectnum].ceilingstat&YAX_BIT;
#endif
                handlemed(1, "Flags (hex)", "Ceiling Flags", &sector[sectnum].ceilingstat,
                          sizeof(sector[sectnum].ceilingstat), 65535, 0);
#ifdef YAX_ENABLE__COMPAT
                sector[sectnum].ceilingstat &= ~YAX_BIT;
                sector[sectnum].ceilingstat |= i;
#endif
                break;
            case 1:
                i = Bsprintf(med_disptext,"(X,Y)pan: %d, %d",
                             TrackerCast(sector[sectnum].ceilingxpanning),
                             TrackerCast(sector[sectnum].ceilingypanning));
                for (; i < med_dispwidth; i++) med_disptext[i] = ' ';
                if (med_editval)
                {
                    Bsprintf(med_edittext,"Sector %d Ceiling X Pan: ",sectnum);
                    printmessage16("%s", med_edittext);
                    sector[sectnum].ceilingxpanning = (char)getnumber16(med_edittext,(int32_t)sector[sectnum].ceilingxpanning,255,0);
                    Bsprintf(med_edittext,"Sector %d Ceiling Y Pan: ",sectnum);
                    printmessage16("%s", med_edittext);
                    sector[sectnum].ceilingypanning = (char)getnumber16(med_edittext,(int32_t)sector[sectnum].ceilingypanning,255,0);
                }
                break;
            case 2:
                handlemed(0, "Shade byte", "Ceiling Shade", &sector[sectnum].ceilingshade,sizeof(sector[sectnum].ceilingshade), 128, 1);
                break;
            case 3:
                handlemed(0, "Z-coordinate", "Ceiling Z-coordinate", &sector[sectnum].ceilingz,
                          sizeof(sector[sectnum].ceilingz), BZ_MAX, 1);
                break;
            case 4:
                handlemed(0, "Tile number", "Ceiling Tile Number", &sector[sectnum].ceilingpicnum,
                          sizeof(sector[sectnum].ceilingpicnum), MAXTILES, 0);
                break;
            case 5:
                handlemed(0, "Ceiling heinum", "Ceiling Heinum", &sector[sectnum].ceilingheinum,
                          sizeof(sector[sectnum].ceilingheinum), BHEINUM_MAX, 1);
                setslope(sectnum, YAX_CEILING, sector[sectnum].ceilingheinum);
                break;
            case 6:
                handlemed(0, "Palookup number", "Ceiling Palookup Number", &sector[sectnum].ceilingpal,
                          sizeof(sector[sectnum].ceilingpal), M32_MAXPALOOKUPS, 0);
                break;
            }
        }
        else if (col == 2)
        {
            switch (row)
            {
            case 0:
#ifdef YAX_ENABLE__COMPAT
                i = sector[sectnum].ceilingstat&YAX_BIT;
#endif
                handlemed(1, "Flags (hex)", "Floor Flags", &sector[sectnum].floorstat,
                          sizeof(sector[sectnum].floorstat), 65535, 0);
#ifdef YAX_ENABLE__COMPAT
                sector[sectnum].ceilingstat &= ~YAX_BIT;
                sector[sectnum].ceilingstat |= i;
#endif
                break;

            case 1:
                i = Bsprintf(med_disptext,"(X,Y)pan: %d, %d",
                             TrackerCast(sector[sectnum].floorxpanning),
                             TrackerCast(sector[sectnum].floorypanning));
                for (; i < med_dispwidth; i++) med_disptext[i] = ' ';
                if (med_editval)
                {
                    Bsprintf(med_edittext,"Sector %d Floor X Pan: ",sectnum);
                    printmessage16("%s", med_edittext);
                    sector[sectnum].floorxpanning = (char)getnumber16(med_edittext,(int32_t)sector[sectnum].floorxpanning,255,0);
                    Bsprintf(med_edittext,"Sector %d Floor Y Pan: ",sectnum);
                    printmessage16("%s", med_edittext);
                    sector[sectnum].floorypanning = (char)getnumber16(med_edittext,(int32_t)sector[sectnum].floorypanning,255,0);
                }
                break;

            case 2:
                handlemed(0, "Shade byte", "Floor Shade", &sector[sectnum].floorshade,
                          sizeof(sector[sectnum].floorshade), 128, 1);
                break;
            case 3:
                handlemed(0, "Z-coordinate", "Floor Z-coordinate", &sector[sectnum].floorz,
                          sizeof(sector[sectnum].floorz), BZ_MAX, 1); //2147483647L,-2147483648L
                break;
            case 4:
                handlemed(0, "Tile number", "Floor Tile Number", &sector[sectnum].floorpicnum,
                          sizeof(sector[sectnum].floorpicnum), MAXTILES, 0);
                break;
            case 5:
                handlemed(0, "Floor heinum", "Floor Heinum", &sector[sectnum].floorheinum,
                          sizeof(sector[sectnum].floorheinum), BHEINUM_MAX, 1);
                setslope(sectnum, YAX_FLOOR, sector[sectnum].floorheinum);
                break;
            case 6:
                handlemed(0, "Palookup number", "Floor Palookup Number", &sector[sectnum].floorpal,
                          sizeof(sector[sectnum].floorpal), M32_MAXPALOOKUPS, 0);
                break;
            }
        }

        med_printcurline(xpos, ypos, row, 1);
        if (med_editval)
            med_editval = 0;

        videoShowFrame(1);
    }

    med_printcurline(xpos, ypos, row, 0);
    // printmessage16("");

    videoShowFrame(1);
    keystatus[KEYSC_ESC] = 0;
}

static void EditWallData(int16_t wallnum)
{
    int32_t row=0;
    int32_t i = -1;
    int32_t xpos = 208, ypos = ydim-STATUS2DSIZ+48;

    med_editval = 0;

    med_dispwidth = 24;
    med_disptext[med_dispwidth] = 0;

    med_typename = "Wall";
    med_thenum = wallnum;

    drawgradient();

    showwalldata(wallnum, 0);

    while (keystatus[KEYSC_ESC] == 0)
    {
        med_handlecommon(xpos, ypos, &row, 6);

        switch (row)
        {
        case 0:
#if !defined NEW_MAP_FORMAT
            i = wall[wallnum].cstat&YAX_NEXTWALLBITS;
#endif
            handlemed(1, "Flags (hex)", "Flags", &wall[wallnum].cstat,
                      sizeof(wall[wallnum].cstat), 65535, 0);
#if !defined NEW_MAP_FORMAT
            wall[wallnum].cstat &= ~YAX_NEXTWALLBITS;
            wall[wallnum].cstat |= i;
#endif
            break;
        case 1:
            handlemed(0, "Shade", "Shade", &wall[wallnum].shade,
                      sizeof(wall[wallnum].shade), 128, 1);
            break;
        case 2:
            handlemed(0, "Pal", "Pal", &wall[wallnum].pal,
                      sizeof(wall[wallnum].pal), M32_MAXPALOOKUPS, 0);
            break;
        case 3:
            i = Bsprintf(med_disptext,"(X,Y)repeat: %d, %d",
                         TrackerCast(wall[wallnum].xrepeat),
                         TrackerCast(wall[wallnum].yrepeat));
            for (; i < med_dispwidth; i++) med_disptext[i] = ' ';
            if (med_editval)
            {
                Bsprintf(med_edittext,"Wall %d X Repeat: ",wallnum);
                printmessage16("%s", med_edittext);
                wall[wallnum].xrepeat = (char)getnumber16(med_edittext,(int32_t)wall[wallnum].xrepeat,255,0);
                Bsprintf(med_edittext,"Wall %d Y Repeat: ",wallnum);
                printmessage16("%s", med_edittext);
                wall[wallnum].yrepeat = (char)getnumber16(med_edittext,(int32_t)wall[wallnum].yrepeat,255,0);
            }
            break;
        case 4:
            i = Bsprintf(med_disptext,"(X,Y)pan: %d, %d",
                         TrackerCast(wall[wallnum].xpanning),
                         TrackerCast(wall[wallnum].ypanning));
            for (; i < med_dispwidth; i++) med_disptext[i] = ' ';
            if (med_editval)
            {
                Bsprintf(med_edittext,"Wall %d X Pan: ",wallnum);
                printmessage16("%s", med_edittext);
                wall[wallnum].xpanning = (char)getnumber16(med_edittext,(int32_t)wall[wallnum].xpanning,255,0);
                Bsprintf(med_edittext,"Wall %d Y Pan: ",wallnum);
                printmessage16("%s", med_edittext);
                wall[wallnum].ypanning = (char)getnumber16(med_edittext,(int32_t)wall[wallnum].ypanning,255,0);
            }
            break;
        case 5:
            handlemed(0, "Tile number", "Tile number", &wall[wallnum].picnum,
                      sizeof(wall[wallnum].picnum), MAXTILES, 0);
            break;

        case 6:
            handlemed(0, "OverTile number", "OverTile number", &wall[wallnum].overpicnum,
                      sizeof(wall[wallnum].overpicnum), MAXTILES, 0);
            break;
        }

        med_printcurline(xpos, ypos, row, 1);
        if (med_editval)
        {
            med_editval = 0;
            //showwalldata(wallnum, 0);
            //// printmessage16("");
        }

        videoShowFrame(1);
    }

    med_printcurline(xpos, ypos, row, 0);
    // printmessage16("");

    videoShowFrame(1);
    keystatus[KEYSC_ESC] = 0;
}

static void EditSpriteData(int16_t spritenum)
{
    int32_t col=0, row=0, rowmax=4;
    int32_t xpos = 8, ypos = ydim-STATUS2DSIZ+48;

    med_editval = 0;

    med_dispwidth = 24;
    med_disptext[med_dispwidth] = 0;

    med_typename = "Sprite";
    med_thenum = spritenum;

    drawgradient();

    //    clearmidstatbar16();

    while (keystatus[KEYSC_ESC] == 0)
    {
        showspritedata(spritenum, 0);

        med_handlecommon(xpos, ypos, &row, rowmax);

        if (PRESSED_KEYSC(LEFT))
        {
            switch (col)
            {
            case 1:
            {
                med_printcurline(xpos, ypos, row, 0);
                col = 0;
                xpos = 8;
                rowmax = 4;
                med_dispwidth = 23;
                med_disptext[med_dispwidth] = 0;
                if (row > rowmax) row = rowmax;
            }
            break;
            case 2:
            {
                med_printcurline(xpos, ypos, row, 0);
                col = 1;
                xpos = 208;
                rowmax = 6;
                med_dispwidth = 24;
                med_disptext[med_dispwidth] = 0;
                if (row > rowmax) row = rowmax;
            }
            break;
            }
        }
        if (PRESSED_KEYSC(RIGHT))
        {
            switch (col)
            {
            case 0:
            {
                med_printcurline(xpos, ypos, row, 0);
                col = 1;
                xpos = 208;
                rowmax = 6;
                med_dispwidth = 24;
                med_disptext[med_dispwidth] = 0;
                if (row > rowmax) row = rowmax;
            }
            break;
            case 1:
            {
                med_printcurline(xpos, ypos, row, 0);
                col = 2;
                xpos = 408;
                rowmax = 6;
                med_dispwidth = 26;
                med_disptext[med_dispwidth] = 0;
                if (row > rowmax) row = rowmax;
            }
            break;
            }
        }

        switch (col)
        {
        case 0:
        {
            switch (row)
            {
            case 0:
                handlemed(0, "X-coordinate", "X-coordinate", &sprite[spritenum].x,
                          sizeof(sprite[spritenum].x), 131072, 1);
                break;
            case 1:
                handlemed(0, "Y-coordinate", "Y-coordinate", &sprite[spritenum].y,
                          sizeof(sprite[spritenum].y), 131072, 1);
                break;
            case 2:
                handlemed(0, "Z-coordinate", "Z-coordinate", &sprite[spritenum].z,
                          sizeof(sprite[spritenum].z), BZ_MAX, 1); //2147483647L,-2147483648L
                break;
            case 3:
            {
                int16_t i = sprite[spritenum].sectnum;
                handlemed(0, "Sectnum", "Sectnum", &sprite[spritenum].sectnum,
                          sizeof(sprite[spritenum].sectnum), numsectors-1, 0);
                if (i != sprite[spritenum].sectnum)
                {
                    swapshort(&i, &sprite[spritenum].sectnum);
                    changespritesect(spritenum,i);
                }
                break;
            }
            case 4:
            {
                int16_t i = sprite[spritenum].statnum;
                handlemed(0, "Statnum", "Statnum", &sprite[spritenum].statnum,
                          sizeof(sprite[spritenum].statnum), MAXSTATUS-1, 0);
                if (i != sprite[spritenum].statnum)
                {
                    swapshort(&i, &sprite[spritenum].statnum);
                    changespritestat(spritenum,i);
                }
                break;
            }
            }
        }
        break;
        case 1:
        {
            switch (row)
            {
            case 0:
                handlemed(1, "Flags (hex)", "Flags", &sprite[spritenum].cstat,
                          sizeof(sprite[spritenum].cstat), 65535, 0);
                break;
            case 1:
                handlemed(0, "Shade", "Shade", &sprite[spritenum].shade,
                          sizeof(sprite[spritenum].shade), 128, 1);
                break;
            case 2:
                handlemed(0, "Pal", "Pal", &sprite[spritenum].pal,
                          sizeof(sprite[spritenum].pal), M32_MAXPALOOKUPS, 0);
                break;
            case 3:
                handlemed(0, "Blend", "Blend", &sprite[spritenum].blend,
                          sizeof(sprite[spritenum].blend), MAXBLENDTABS, 0);
                break;
            case 4:
            {
                int32_t i = Bsprintf(med_disptext,"(X,Y)repeat: %d, %d",
                             TrackerCast(sprite[spritenum].xrepeat),
                             TrackerCast(sprite[spritenum].yrepeat));
                for (; i < med_dispwidth; i++) med_disptext[i] = ' ';
                if (med_editval)
                {
                    Bsprintf(med_edittext,"Sprite %d X Repeat: ",spritenum);
                    printmessage16("%s", med_edittext);
                    sprite[spritenum].xrepeat = (char)getnumber16(med_edittext,(int32_t)sprite[spritenum].xrepeat,255,0);
                    Bsprintf(med_edittext,"Sprite %d Y Repeat: ",spritenum);
                    printmessage16("%s", med_edittext);
                    sprite[spritenum].yrepeat = (char)getnumber16(med_edittext,(int32_t)sprite[spritenum].yrepeat,255,0);
                }
            }
            break;
            case 5:
            {
                int32_t i = Bsprintf(med_disptext,"(X,Y)offset: %d, %d",
                             TrackerCast(sprite[spritenum].xoffset),
                             TrackerCast(sprite[spritenum].yoffset));
                for (; i < med_dispwidth; i++) med_disptext[i] = ' ';
                if (med_editval)
                {
                    Bsprintf(med_edittext,"Sprite %d X Offset: ",spritenum);
                    printmessage16("%s", med_edittext);
                    sprite[spritenum].xoffset = (char)getnumber16(med_edittext,(int32_t)sprite[spritenum].xoffset,128,1);
                    Bsprintf(med_edittext,"Sprite %d Y Offset: ",spritenum);
                    printmessage16("%s", med_edittext);
                    sprite[spritenum].yoffset = (char)getnumber16(med_edittext,(int32_t)sprite[spritenum].yoffset,128,1);
                }
            }
            break;
            case 6:
                handlemed(0, "Tile number", "Tile number", &sprite[spritenum].picnum,
                          sizeof(sprite[spritenum].picnum), MAXTILES-1, 0+2);
                break;
            }
        }
        break;
        case 2:
        {
            switch (row)
            {
            case 0:
                handlemed(0, "Angle (2048 degrees)", "Angle", &sprite[spritenum].ang,
                          sizeof(sprite[spritenum].ang), 2048, 1);
                if (med_editval)
                    sprite[spritenum].ang &= 2047;
                break;
            case 1:
                handlemed(0, "X-Velocity", "X-Velocity", &sprite[spritenum].xvel,
                          sizeof(sprite[spritenum].xvel), 65535, 1);
                break;
            case 2:
                handlemed(0, "Y-Velocity", "Y-Velocity", &sprite[spritenum].yvel,
                          sizeof(sprite[spritenum].yvel), 65535, 1);
                break;
            case 3:
                handlemed(0, "Z-Velocity", "Z-Velocity", &sprite[spritenum].zvel,
                          sizeof(sprite[spritenum].zvel), 65535, 1);
                break;
            case 4:
                handlemed(0, "Owner", "Owner", &sprite[spritenum].owner,
                          sizeof(sprite[spritenum].owner), MAXSPRITES-1, 1);
                break;
            case 5:
                handlemed(0, "Clipdist", "Clipdist", &sprite[spritenum].clipdist,
                          sizeof(sprite[spritenum].clipdist), 255, 0);
                break;
            case 6:
                handlemed(0, "Extra", "Extra", &sprite[spritenum].extra,
                          sizeof(sprite[spritenum].extra), BTAG_MAX, 1);
                break;
            }
        }
        break;
        }

        med_printcurline(xpos, ypos, row, 1);
        if (med_editval)
            med_editval = 0;

        videoShowFrame(1);
    }

    med_printcurline(xpos, ypos, row, 0);
    // printmessage16("");
    videoShowFrame(1);

    keystatus[KEYSC_ESC] = 0;
}

#define TWENTYFIVE_BLANKS "                         "

static void GenericSpriteSearch(void)
{
    char disptext[80];
    char edittext[80];
    static int32_t col=0, row=0;
    int32_t i, j, k;
    int32_t rowmax[3]= {6,6,6}, dispwidth[3] = {24,24,28};
    int32_t xpos[3] = {8,200,400}, ypos = ydim-STATUS2DSIZ+48;

    static const char *labels[7][3] =
    {
        {"X-coordinate", "Flags (hex)", "Angle (2048 degrees)"},
        {"Y-coordinate", "Shade",       "X-Velocity"},
        {"Z-coordinate", "Pal",         "Y-Velocity"},
        {"Sectnum",      "Blend",       "Z-Velocity"},
        {"Statnum",      "(X/Y)repeat", "Owner"},
        {"Hitag",        "(X/Y)offset", "Clipdist"},
        {"Lotag",        "Tile number", "Extra"}
    };

    static int32_t maxval[7][3] =
    {
        { BXY_MAX     , 65535         , 2048 },
        { BXY_MAX     , 128           , 65535 },
        { BZ_MAX      , M32_MAXPALOOKUPS, 65535 },
        { MAXSECTORS-1, MAXBLENDTABS-1, 65535 },
        { MAXSTATUS-1 , 128           , MAXSPRITES-1 },
        { BTAG_MAX    , 128           , 256 },
        { BTAG_MAX    , MAXTILES-1    , BTAG_MAX }
    };

    static char sign[7][3] =
    {
        {1,   0,   1},
        {1,   1,   1},
        {1,   0,   1},
        {0,   0,   1},
        {0,   0,   0},
        {0+4, 1,   0},
        {0+4, 0+2, 1}
    };

    clearmidstatbar16();

    drawgradient();

    printext16(xpos[0], ypos-2*8, editorcolors[10], editorcolors[0], "Sprite search", 0);

    for (i=0; i<3; i++)
        for (j=0; j<=rowmax[i]; j++)
        {
            if (gs_spriteTagInterested[i][j])
                k=Bsprintf(disptext, "%s: %d", labels[j][i], gs_spriteTagValue[i][j]);
            else
                k=Bsprintf(disptext, "%s: ^7any", labels[j][i]);
//            for (; k<dispwidth[i]; k++) disptext[k] = 0;

            printext16(xpos[i], ypos+j*8, editorcolors[11], editorcolors[0], disptext, 0);
        }
    Bmemset(disptext, 0, sizeof(disptext));

    //    disptext[dispwidth[col]] = 0;
    //    showspritedata(spritenum, 0);
    wallsprite = 2;

    while (keystatus[KEYSC_ESC] == 0)
    {
        idle_waitevent();
        if (handleevents())
        {
            if (quitevent) quitevent = 0;
        }

        printmessage16("Sprite search, press <Esc> to exit");

        if (PRESSED_KEYSC(DOWN))
        {
            if (row < rowmax[col])
            {
                printext16(xpos[col],ypos+row*8,editorcolors[11],editorcolors[0],disptext,0);
                row++;
            }
        }
        if (PRESSED_KEYSC(UP))
        {
            if (row > 0)
            {
                printext16(xpos[col],ypos+row*8,editorcolors[11],editorcolors[0],disptext,0);
                row--;
            }
        }
        if (PRESSED_KEYSC(LEFT))
        {
            if (col > 0)
            {
                printext16(xpos[col],ypos+row*8,editorcolors[11],editorcolors[0],disptext,0);
                col--;
                disptext[dispwidth[col]] = 0;
                if (row > rowmax[col]) row = rowmax[col];
            }
        }
        if (PRESSED_KEYSC(RIGHT))
        {
            if (col < 2)
            {
                printext16(xpos[col],ypos+row*8,editorcolors[11],editorcolors[0],disptext,0);
                col++;
                disptext[dispwidth[col]] = 0;
                if (row > rowmax[col]) row = rowmax[col];
            }
        }
        if (PRESSED_KEYSC(ENTER))
        {
            Bsprintf(edittext, "%s: ", labels[row][col]);
            printmessage16("%s", edittext);
            i = getnumber16(edittext, gs_spriteTagInterested[col][row] ? gs_spriteTagValue[col][row] : 0,
                            maxval[row][col], sign[row][col]);
            if (col == 2 && row == 0) i = (i+2048)&2047;  // angle
            gs_spriteTagValue[col][row] = i;
            gs_spriteTagInterested[col][row] = 1;

            if (col == 1 && row == 6)  // picnum
            {
                printext16(xpos[1], ypos-2*8, editorcolors[14], editorcolors[0], TWENTYFIVE_BLANKS, 0);
                if (names[i][0])
                    printext16(xpos[1], ypos-2*8, editorcolors[14], editorcolors[0], names[i], 0);
            }
        }
        if (PRESSED_KEYSC(BS) || PRESSED_KEYSC(DELETE))
        {
            gs_spriteTagInterested[col][row] = 0;

            if (col == 1 && row == 6)  // picnum
                printext16(xpos[1], ypos-2*8, editorcolors[14], editorcolors[0], TWENTYFIVE_BLANKS, 0);
        }

        i = gs_spriteTagInterested[col][row];
        if (i)
        {
            if (col == 1 && row == 0)  // flags
                k = Bsprintf(disptext, "%s: %x", labels[row][col], gs_spriteTagValue[col][row]);
            else
                k = Bsprintf(disptext, "%s: %d", labels[row][col], gs_spriteTagValue[col][row]);
        }
        else
            k = Bsprintf(disptext, "%s: ^7any", labels[row][col]);
        //                      v-------^^
        for (; k<dispwidth[col]-2*i; k++) disptext[k] = ' ';
        disptext[k] = 0;

        printext16(xpos[col],ypos+row*8,editorcolors[11],editorcolors[1],disptext,0);

        videoShowFrame(1);
    }

    printext16(xpos[col],ypos+row*8,editorcolors[11],editorcolors[0],disptext,0);
    printmessage16("Search sprite");
    videoShowFrame(1);

    keystatus[KEYSC_ESC] = 0;
}

void netGetPackets(void)
{
}

void QuitGame(void)
{
    // NUKE-TODO:
    Bexit(0);
}


#ifndef __whaven_h__
#define __whaven_h__

#include "common_game.h"
#include "player.h"
#include "whdefs.h"
#include "names.h"

#ifndef APPNAME
#define APPNAME             "Witchaven"
#endif

#ifndef APPBASENAME
#define APPBASENAME         "witchaven"
#endif

enum cliptypes_t {
    NORMALCLIP = 0,
    PROJECTILECLIP,
    CLIFFCLIP
};

#define XDIM    320
#define YDIM    200
#define NUMOPTIONS     12
#define WHNUMKEYS      27
#define LOW            1
#define HIGH           2
#define INACTIVE       0
#define PATROL         1
#define CHASE          2
#define AMBUSH         3
#define ATTACK         6
#define STAND          8


#define MISSILE    100
#define FX         101
#define HEATSEEKER 102
#define YELL       103
#define CAST       104
#define PUSH       105
#define FALL       106
#define DIE        107
#define DEAD       108
#define FACE       109
#define SHOVE      110
#define SHATTER    111
#define FIRE       112
#define LIFTUP     113
#define LIFTDN     114
#define PENDULUM   115
#define RESURECT   116
#define BOB        117
#define SHOVER     118
#define TORCHER    119
#define MASPLASH   120
#define CHUNKOMEAT 121
#define FLEE       122
#define DORMANT    123
#define ACTIVE     124
#define ATTACK2    125
#define WITCHSIT   126
#define CHILL      127
#define SKIRMISH   128
#define FLOCK      129
#define FLOCKSPAWN 130
#define PAIN       131
#define WAR        132
#define TORCHLIGHT 133
#define GLOWLIGHT  134
#define BLOOD      135
#define DRIP       136
#define DEVILFIRE  137
#define FROZEN     138
#define PULLTHECHAIN  139
#define FLOCKCHIRP    140
#define CHUNKOWALL    141
#define FINDME        142
#define DRAIN         143
#define RATRACE       144
#define SMOKE         145
#define EXPLO         146
#define JAVLIN        147
#define ANIMLEVERUP   148
#define ANIMLEVERDN   149
#define BROKENVASE    150
#define CHARCOAL      151
#define WARPFX        152
#define KILLSECTOR    4444
#define GRAVITYCONSTANT  (4<<7);
#define JUMPVEL          (4<<10);
#define GROUNDBIT        1
#define PLATFORMBIT      2


extern int32_t g_commandSetup;
extern int32_t g_noSetup;

extern struct grpfile_t const* g_selectedGrp;

extern int loaddefinitions_game(const char* fn, int32_t preload);
void G_LoadGroupsInDir(const char* dirname);
void G_DoAutoload(const char* dirname);

extern int mouseaiming, aimmode, mouseflip;
extern int runkey_mode, auto_run;

extern int r_showfps;

int adjusthp(int hp);

// TODO / FIXME / CHECKME - old Build visibility variable. What happened it?
extern int32_t visibility;

extern int svga;
extern int delaycnt, engineinitflag, timerinitflag, videoinitflag;
extern int netgame;
extern int gametype;
extern int godmode;
extern int32_t followx, followy;
extern int32_t lockclock;
extern int32_t difficulty;
extern int32_t goreon;

extern int32_t brightness;
extern int32_t gbrightness;

extern char boardname[];
extern char tempboard[];
extern char loadgamename[];

extern int32_t* animateptr[];
extern int32_t animategoal[];
extern int32_t animatevel[];
extern int32_t animatecnt;

extern short ironbarsector[16];
extern short ironbarscnt;
extern int ironbarsgoal1[16], ironbarsgoal2[16];
extern short ironbarsdone[16], ironbarsanim[16];
extern int ironbarsgoal[16];

extern int swingcnt;

struct Delayitem {
    void (*func)(int);
    int  item;
    int  timer;
};

extern Delayitem delayitem[MAXSECTORS];

void crash(const char* fmt);
void drawscreen(Player *plr);
void doanimations(int32_t numtics);
void dodelayitems(int tics);
void setupboard(char* fname);
void shutdown();
int32_t setanimation(int32_t* animptr, int32_t thegoal, int32_t thevel);
int32_t getanimationgoal(int32_t* animptr);
void setdelayfunc(void (*func)(int), int item, int delay);
void intro();

extern int32_t synctics;
extern int32_t globhiz, globloz, globhihit, globlohit;
extern char option[];

// engine.cpp
void overwritesprite(int thex, int they, short tilenum, int8_t shade, char stat, char dapalnum);
void permanentwritesprite(int thex, int they, short tilenum, int8_t shade, int cx1, int cy1, int cx2, int cy2, char dapalnum);
void setbrightness(int32_t brightness);

#define kAngleMask	     0x7FF
#define kTimerTicks      120
#define kTicksPerFrame   (120/30)

inline int Sin(int angle)
{
    return sintable[angle & kAngleMask];
}

inline int Cos(int angle)
{
    return sintable[(angle + 512) & kAngleMask];
}

#endif

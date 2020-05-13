/***************************************************************************
 *   WHPLR.C  - code for player character                                  *
 *                                                                         *
 ***************************************************************************/

#include "witchaven.h"
#include "player.h"
#include "menu.h"
#include "sound.h"
#include "objects.h"
#include "tags.h"
#include "network.h"
#include "animation.h"
#include "input.h"
#include "network.h"
#include "scancodes.h"
#include "view.h"

int victor = 0;
int autohoriz = 0;

// WITCHAVEN 2
int dropshieldcnt = 0;
int droptheshield = 0;
int enchantedsoundhandle = -1;
extern short torchpattern[];
extern int expgained;

//#define   WHDEMO

int spelltime;
extern int loadedgame;
extern int scoretime;

int madeahit = 0;
extern int mapflag;

int pyrn;
int dahand = 0;
int mapon = 1;

int32_t neartagdist, neartaghitdist;

short neartagsector, neartagsprite, neartagwall;

Player player[MAXPLAYERS];

extern int justteleported;
extern int gameactivated;
extern short oldmousestatus;
extern int oldhoriz;

extern char tempbuf[];
extern char scorebuf[];
extern char healthbuf[];
extern char potionbuf[];

extern int weapontimex;
extern int weapontimey;
extern int weapontilenum;

extern int frames;

int weapondrop;
int weapondropgoal;
int weaponraise;

int currweapontics;
int currweaponframe;

int spellbooktics;
int spellbook;
int spellbookframe;
int spellbookflip;

int spiketics;
int spikeframe;
int currspikeframe;


#define MAXFRAMES 12

struct daweapons {
   int daweapontics;
   int daweaponframe;
   int currx;
   int curry;
};

struct daweapons spikeanimtics[5] =
{ {10,DIESPIKE,136,145},
  {10,DIESPIKE+1,136,124},
  {10,DIESPIKE+2,136,100},
  {10,DIESPIKE+3,136,70},
  {10,DIESPIKE+4,136,50} };

//
struct daweapons spellbookanim[MAXNUMORBS][9] =
{
    // SCARE
    { {8,SPELLBOOK8,121,161},    {8,SPELLBOOK8 + 1,121,161},
      {8,SPELLBOOK8 + 2,121,156},{8,SPELLBOOK8 + 3,121,158},
      {8,SPELLBOOK8 + 4,121,159},{8,SPELLBOOK8 + 5,121,161},
      {8,SPELLBOOK8 + 6,121,160},{8,SPELLBOOK8 + 7,121,161},
      {8,SPELLBOOK8 + 7,121,161}
    },
    // NIGHT VISION
    { {8,SPELLBOOK6,121,161},    {8,SPELLBOOK6 + 1,121,161},
      {8,SPELLBOOK6 + 2,121,156},{8,SPELLBOOK6 + 3,121,158},
      {8,SPELLBOOK6 + 4,121,159},{8,SPELLBOOK6 + 5,121,161},
      {8,SPELLBOOK6 + 6,121,160},{8,SPELLBOOK6 + 7,121,161},
      {8,SPELLBOOK6 + 7,121,161}
    },
    // FREEZE
    { {8,SPELLBOOK3,121,161},    {8,SPELLBOOK3 + 1,121,161},
      {8,SPELLBOOK3 + 2,121,156},{8,SPELLBOOK3 + 3,121,158},
      {8,SPELLBOOK3 + 4,121,159},{8,SPELLBOOK3 + 5,120,161},
      {8,SPELLBOOK3 + 6,120,160},{8,SPELLBOOK3 + 7,120,161},
      {8,SPELLBOOK3 + 7,121,161}
    },
    // MAGIC ARROW
    { {8,SPELLBOOKBLANK,121,161},    {8,SPELLBOOKBLANK + 1,121,161},
      {8,SPELLBOOKBLANK + 2,121,156},{8,SPELLBOOKBLANK + 3,121,158},
      {8,SPELLBOOKBLANK + 4,121,159},{8,SPELLBOOKBLANK + 5,121,161},
      {8,SPELLBOOKBLANK + 6,120,160},{8,SPELLBOOKBLANK + 7,121,161},
      {8,SPELLBOOKBLANK + 7,122,161}
    },
    // OPEN DOORS
    { {8,SPELLBOOK7,121,161},    {8,SPELLBOOK7 + 1,121,161},
      {8,SPELLBOOK7 + 2,121,156},{8,SPELLBOOK7 + 3,121,158},
      {8,SPELLBOOK7 + 4,121,159},{8,SPELLBOOK7 + 5,121,161},
      {8,SPELLBOOK7 + 6,121,160},{8,SPELLBOOK7 + 7,121,161},
      {8,SPELLBOOK7 + 7,121,161}
    },
    // FLY
    { {8,SPELLBOOK2,121,161},    {8,SPELLBOOK2 + 1,121,161},
      {8,SPELLBOOK2 + 2,121,156},{8,SPELLBOOK2 + 3,121,158},
      {8,SPELLBOOK2 + 4,121,159},{8,SPELLBOOK2 + 5,121,161},
      {8,SPELLBOOK2 + 6,121,160},{8,SPELLBOOK2 + 7,121,161},
      {8,SPELLBOOK2 + 7,121,161}
    },
    // FIRE BALL
    { {8,SPELLBOOK4,121,161},    {8,SPELLBOOK4 + 1,121,161},
      {8,SPELLBOOK4 + 2,121,156},{8,SPELLBOOK4 + 3,121,158},
      {8,SPELLBOOK4 + 4,121,159},{8,SPELLBOOK4 + 5,121,161},
      {8,SPELLBOOK4 + 6,121,160},{8,SPELLBOOK4 + 7,121,161},
      {8,SPELLBOOK4 + 7,121,161}
    },
    // NUKE!
    { {8,SPELLBOOK5,121,161},    {8,SPELLBOOK5 + 1,121,161},
      {8,SPELLBOOK5 + 2,121,156},{8,SPELLBOOK5 + 3,121,158},
      {8,SPELLBOOK5 + 4,121,159},{8,SPELLBOOK5 + 5,121,161},
      {8,SPELLBOOK5 + 6,121,160},{8,SPELLBOOK5 + 7,121,161},
      {8,SPELLBOOK5 + 7,121,161}
    }
};

struct daweapons sspellbookanim[MAXNUMORBS][9] =
{
    // SCARE
    { {8,SSPELLBOOK8,121,389},    {8,SSPELLBOOK8 + 1,121,377},
      {8,SSPELLBOOK8 + 2,121,383},{8,SSPELLBOOK8 + 3,121,385},
      {8,SSPELLBOOK8 + 4,121,389},{8,SSPELLBOOK8 + 5,121,387},
      {8,SSPELLBOOK8 + 6,121,389},{8,SSPELLBOOK8 + 7,121,389},
      {8,SSPELLBOOK8 + 7,121,389}
    },
    // NIGHT VISION
    { {8,SSPELLBOOK6,121,389},    {8,SSPELLBOOK6 + 1,121,377},
      {8,SSPELLBOOK6 + 2,121,383},{8,SSPELLBOOK6 + 3,121,385},
      {8,SSPELLBOOK6 + 4,121,389},{8,SSPELLBOOK6 + 5,121,387},
      {8,SSPELLBOOK6 + 6,121,389},{8,SSPELLBOOK6 + 7,121,389},
      {8,SSPELLBOOK6 + 7,121,389}
    },
    // FREEZE
    { {8,SSPELLBOOK3,121,389},    {8,SSPELLBOOK3 + 1,121,377},
      {8,SSPELLBOOK3 + 2,121,383},{8,SSPELLBOOK3 + 3,121,385},
      {8,SSPELLBOOK3 + 4,121,389},{8,SSPELLBOOK3 + 5,120,387},
      {8,SSPELLBOOK3 + 6,120,389},{8,SSPELLBOOK3 + 7,120,389},
      {8,SSPELLBOOK3 + 7,121,389}
    },
    // MAGIC ARROW
    { {8,SSPELLBOOKBLANK,121,389},    {8,SSPELLBOOKBLANK + 1,121,377},
      {8,SSPELLBOOKBLANK + 2,121,383},{8,SSPELLBOOKBLANK + 3,121,385},
      {8,SSPELLBOOKBLANK + 4,121,389},{8,SSPELLBOOKBLANK + 5,121,387},
      {8,SSPELLBOOKBLANK + 6,120,389},{8,SSPELLBOOKBLANK + 7,121,389},
      {8,SSPELLBOOKBLANK + 7,122,389}
    },
    // OPEN DOORS
    { {8,SSPELLBOOK7,121,389},    {8,SSPELLBOOK7 + 1,121,377},
      {8,SSPELLBOOK7 + 2,121,383},{8,SSPELLBOOK7 + 3,121,385},
      {8,SSPELLBOOK7 + 4,121,389},{8,SSPELLBOOK7 + 5,121,387},
      {8,SSPELLBOOK7 + 6,121,389},{8,SSPELLBOOK7 + 7,121,389},
      {8,SSPELLBOOK7 + 7,121,389}
    },
    // FLY
    { {8,SSPELLBOOK2,121,389},    {8,SSPELLBOOK2 + 1,121,377},
      {8,SSPELLBOOK2 + 2,121,383},{8,SSPELLBOOK2 + 3,121,385},
      {8,SSPELLBOOK2 + 4,121,389},{8,SSPELLBOOK2 + 5,121,387},
      {8,SSPELLBOOK2 + 6,121,389},{8,SSPELLBOOK2 + 7,121,389},
      {8,SSPELLBOOK2 + 7,121,389}
    },
    // FIRE BALL
    { {8,SSPELLBOOK4,121,389},    {8,SSPELLBOOK4 + 1,121,377},
      {8,SSPELLBOOK4 + 2,121,383},{8,SSPELLBOOK4 + 3,121,385},
      {8,SSPELLBOOK4 + 4,121,389},{8,SSPELLBOOK4 + 5,121,387},
      {8,SSPELLBOOK4 + 6,121,389},{8,SSPELLBOOK4 + 6,121,389},
      {8,SSPELLBOOK4 + 6,121,389}
    },
    // NUKE!
    { {8,SSPELLBOOK5,121,389},    {8,SSPELLBOOK5 + 1,121,377},
      {8,SSPELLBOOK5 + 2,121,383},{8,SSPELLBOOK5 + 3,121,385},
      {8,SSPELLBOOK5 + 4,121,389},{8,SSPELLBOOK5 + 5,121,387},
      {8,SSPELLBOOK5 + 6,121,389},{8,SSPELLBOOK5 + 6,121,389},
      {8,SSPELLBOOK5 + 6,121,389}
    }
};

struct daweapons throwanimtics[MAXNUMORBS][MAXFRAMES + 1] =
{
    // MUTWOHANDS
    { {10,MUTWOHANDS,19,155},{10,MUTWOHANDS + 1,0,128},{10,MUTWOHANDS + 2,0,93},
      {10,MUTWOHANDS + 3,0,83},{10,MUTWOHANDS + 4,0,72},{10,MUTWOHANDS + 5,0,83},
      {10,MUTWOHANDS + 6,10,96},{10,MUTWOHANDS + 7,43,109},{10,MUTWOHANDS + 8,69,113},
      {10,MUTWOHANDS + 9,65,115},{10,MUTWOHANDS + 10,64,117},{10,MUTWOHANDS + 11,63,117},
      {1,0,127,170}
    },
    // MUMEDUSA
    { {10,MUMEDUSA,0,177},{10,MUMEDUSA + 1,0,137},{10,MUMEDUSA + 2,48,82},
      {10,MUMEDUSA + 3,127,41},{10,MUMEDUSA + 4,210,9},{10,MUMEDUSA + 5,284,26},
      {10,MUMEDUSA + 6,213,63},{10,MUMEDUSA + 7,147,99},{10,MUMEDUSA + 8,91,136},
      {10,MUMEDUSA + 9,46,183},{1,0,127,170},{1,0,127,170},
      {1,0,127,170}
    },
    // BMUTWOHANDS
    { {10,MUTWOHANDS,19,155},{10,MUTWOHANDS + 1,0,128},{10,MUTWOHANDS + 2,0,93},
      {10,MUTWOHANDS + 3,0,83},{10,BMUTWOHANDS,0,74},{10,BMUTWOHANDS + 1,0,97},
      {10,BMUTWOHANDS + 2,10,109},{10,BMUTWOHANDS + 3,43,113},{10,BMUTWOHANDS + 4,69,115},
      {10,BMUTWOHANDS + 5,65,117},{10,BMUTWOHANDS + 6,64,117},{10,BMUTWOHANDS + 7,63,117},
      {1,0,127,170}
    },
    // MUTWOHANDS
    { {10,MUTWOHANDS,19,155},{10,MUTWOHANDS + 1,0,128},{10,MUTWOHANDS + 2,0,93},
      {10,MUTWOHANDS + 3,0,83},{10,MUTWOHANDS + 4,0,72},{10,MUTWOHANDS + 5,0,83},
      {10,MUTWOHANDS + 6,10,96},{10,MUTWOHANDS + 7,43,109},{10,MUTWOHANDS + 8,69,113},
      {10,MUTWOHANDS + 9,65,115},{10,MUTWOHANDS + 10,64,117},{10,MUTWOHANDS + 11,63,117},
      {1,0,127,170}
    },
    // MUTWOHANDS
    { {15,MUTWOHANDS,19,155},{15,MUTWOHANDS + 1,0,128},{15,MUTWOHANDS + 2,0,93},
      {15,MUTWOHANDS + 3,0,83},{15,MUTWOHANDS + 4,0,72},{15,MUTWOHANDS + 5,0,83},
      {15,MUTWOHANDS + 6,10,96},{15,MUTWOHANDS + 7,43,109},{15,MUTWOHANDS + 8,69,113},
      {15,MUTWOHANDS + 9,65,115},{15,MUTWOHANDS + 10,64,117},{15,MUTWOHANDS + 11,63,117},
      {1,0,127,170}
    },
    // MUMEDUSA
    { {10,MUMEDUSA,0,177},{10,MUMEDUSA + 1,0,137},{10,MUMEDUSA + 2,48,82},
      {10,MUMEDUSA + 3,127,41},{10,MUMEDUSA + 4,210,9},{10,MUMEDUSA + 5,284,26},
      {10,MUMEDUSA + 6,213,63},{10,MUMEDUSA + 7,147,99},{10,MUMEDUSA + 8,91,136},
      {10,MUMEDUSA + 9,46,183},{1,0,127,170},{1,0,127,170},
      {1,0,127,170}
    },
    // MUTWOHANDS
    { {10,MUTWOHANDS,19,155},{10,MUTWOHANDS + 1,0,128},{10,MUTWOHANDS + 2,0,93},
      {10,MUTWOHANDS + 3,0,83},{10,MUTWOHANDS + 4,0,72},{10,MUTWOHANDS + 5,0,83},
      {10,MUTWOHANDS + 6,10,96},{10,MUTWOHANDS + 7,43,109},{10,MUTWOHANDS + 8,69,113},
      {10,MUTWOHANDS + 9,65,115},{10,MUTWOHANDS + 10,64,117},{10,MUTWOHANDS + 11,63,117},
      {1,0,127,170}
    },
    // MUTWOHANDS
    { {10,MUTWOHANDS,19,155},{10,MUTWOHANDS + 1,0,128},{10,MUTWOHANDS + 2,0,93},
      {10,MUTWOHANDS + 3,0,83},{10,MUTWOHANDS + 4,0,72},{10,MUTWOHANDS + 5,0,83},
      {10,MUTWOHANDS + 6,10,96},{10,MUTWOHANDS + 7,43,109},{10,MUTWOHANDS + 8,69,113},
      {10,MUTWOHANDS + 9,65,115},{10,MUTWOHANDS + 10,64,117},{10,MUTWOHANDS + 11,63,117},
      {1,0,127,170}
    }
};

struct daweapons lefthandanimtics[5][MAXFRAMES] =
{
    { { 10,RFIST,15,121 },{10,RFIST + 1,17,114},{10,RFIST + 2,54,131},
      { 10,RFIST + 3,76,152 },{10,RFIST + 4,31,126},{10,RFIST + 5,26,135},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    // KNIFE
    { { 8,KNIFEATTACK2,0,113 },{8,KNIFEATTACK2 + 1,44,111},{8,KNIFEATTACK2 + 2,119,137},
      { 8,KNIFEATTACK2 + 3,187,159},{16,0,136,100},{8,KNIFEATTACK2 + 3,187,159},
      { 8,KNIFEATTACK2 + 2,119,137},{8,KNIFEATTACK2 + 1,44,111},{8,KNIFEATTACK2,0,113},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    // GOBLINATTACK
    { {10,GOBSWORDATTACK,243,92},{10,GOBSWORDATTACK + 1,255,68 },{10,GOBSWORDATTACK + 2,279,65},
      {10,GOBSWORDATTACK + 3,238,55},{10,GOBSWORDATTACK + 4,153,52 },{10,GOBSWORDATTACK + 5,129,152},
      {10,GOBSWORDATTACK + 6,90,184},{ 1,0,297,169 },{1,0,275,24},
      {1,0,275,24 },{ 1,0,275,24 },{1,0,275,24}
    },
    // MORNINGATTACK2
    { { 12,MORNINGATTACK2,38,142 },{12,MORNINGATTACK2 + 1,0,111},{12,MORNINGATTACK2 + 2,0,91},
      { 12,MORNINGATTACK2 + 3,0,47 },{12,MORNINGATTACK2 + 4,0,24},{1,0,0,24},
      { 1,0,0,24 },{1,0,0,24},{1,0,0,24},
      { 1,0,0,24 },{1,0,0,24},{1,0,0,24}
    },
    // GOBLINATTACK2
    { { 10,GOBSWORDATTACK2,236,99 },{10,GOBSWORDATTACK2 + 1,202,24},{10,GOBSWORDATTACK2 + 2,181,0},
      { 10,GOBSWORDATTACK2 + 3,52,12 },{10,GOBSWORDATTACK2 + 4,72,72},{10,GOBSWORDATTACK2 + 5,134,139},
      { 10,GOBSWORDATTACK2 + 6,297,169 },{1,0,275,24},{1,0,275,24 },
      { 1,0,275,24 },{1,0,275,24},{1,0,275,24}
    }
};

// #define LFIST   NULL

struct daweapons cockanimtics[MAXFRAMES + 1] =
{
    {24,0,10,10},
    {12,BOWREADYEND + 1,101,115},{12,BOWREADYEND + 2,112,0},
    {12,BOWREADYEND + 3,115,0},{12,BOWREADYEND + 4,75,13}
};

struct daweapons readyanimtics[MAXWEAPONS][MAXFRAMES + 1] =
{
    // FIST
    { { 10,RFIST,216,180 },{10,RFIST,216,170},{10,RFIST,216,160},
      { 10,RFIST,216,150 },{10,RFIST,216,140},{10,RFIST,216,130},
      { 10,RFIST,216,124 },{1,RFIST,216,124},{1,RFIST,216,124},
      { 1,RFIST,216,122 },{1,RFIST,216,122},{1,RFIST,216,122},
      {1,0,147,76}
    },
    // KNIFE
    { { 10,KNIFEREADY,69,171 },{10,KNIFEREADY + 1,11,146},{10,KNIFEREADY + 2,25,146},
      { 10,KNIFEREADY + 3,35,158 },{10,KNIFEREADY + 4,38,158},{10,KNIFEREADY + 5,16,157},
      { 10,KNIFEREADY + 6,37,102},{10,KNIFEREADY + 7,239,63},{10,KNIFEREADY + 8,214,85},
      { 10,KNIFEREADY + 9,206,110},{10,KNIFEREADY + 10,217,108},{10,KNIFEREADY + 11,204,95},
      {1,0,147,76}
    },
    // GOBSWORD
    { { 12,GOBSWORDPULL,79,169 },{12,GOBSWORDPULL + 1,95,115},{12,GOBSWORDPULL + 2,94,93},
      { 12,GOBSWORDPULL + 3,156,77 },{12,GOBSWORDPULL + 4,218,64},{12,GOBSWORDPULL + 5,224,57},
      { 8,GOBSWORDPULL + 6,251,54 },{1,GOBSWORDPULL + 7,243,92},{1,GOBSWORDPULL + 7,243,92},
      { 1,GOBSWORDPULL + 7,243,92 },{1,GOBSWORDPULL + 7,243,92},{1,GOBSWORDPULL + 7,243,92},
      {1,0,147,76}
    },
    // MORNINGSTAR
    { { 6,MORNINGSTAR,193,190 },{6,MORNINGSTAR,193,180},{6,MORNINGSTAR,193,170},
      { 6,MORNINGSTAR,193,160 },{6,MORNINGSTAR,193,150},{6,MORNINGSTAR,193,140},
      { 6,MORNINGSTAR,193,130 },{6,MORNINGSTAR,193,120},{6,MORNINGSTAR,193,110},
      { 6,MORNINGSTAR,193,100 },{6,MORNINGSTAR,193,90},{1,MORNINGSTAR,193,90},
      {1,0,147,76}
    },
    // SWORD
    { { 10,SWORDPULL,58,160 },{10,SWORDPULL + 1,81,111},{10,SWORDPULL + 2,19,88},
      { 10,SWORDPULL + 3,0,93 },{10,SWORDPULL + 4,104,0},{10,SWORDPULL + 5,169,0},
      { 10,SWORDPULL + 6,244,38 },{6,SWORDPULL + 7,225,121},{1,SWORDPULL + 7,225,121},
      { 1,SWORDPULL + 7,225,121 },{1,SWORDPULL + 7,225,121},{1,SWORDPULL + 7,225,121},
      {1,0,147,76}
    },
    { { 12,BIGAXEDRAW,71,108 },{12,BIGAXEDRAW + 1,17,58},{12,BIGAXEDRAW + 2,0,56},
      { 12,BIGAXEDRAW + 3,0,71 },{12,BIGAXEDRAW + 4,0,102},{12,BIGAXEDRAW + 5,0,11},
      { 12,BIGAXEDRAW + 6,33,0 },{12,BIGAXEDRAW + 7,69,0},{12,BIGAXEDRAW + 8,75,20},
      { 12,BIGAXEDRAW9,150,92 },{12,BIGAXEDRAW10,182,116},{1,0,200,122},
      {1,0,147,76}
    },
    // BOW
    { { 12,BOWREADY,0,0 },{12,BOWREADY + 1,0,20},{12,BOWREADY + 2,0,46},
      { 12,BOWREADY + 3,0,26 },{12,BOWREADY + 4,0,0},{12,BOWREADY + 5,71,0},
      { 8,BOWREADYEND,77,23 },{1,BOWREADYEND,77,23},{1,BOWREADYEND,77,23},
      { 1,BOWREADYEND,77,23 },{1,BOWREADYEND,77,23},{1,BOWREADYEND,77,23},
      {1,0,147,76}
    },
    { { 8,PIKEDRAW,0,156 },{8,PIKEDRAW + 1,15,98},{8,PIKEDRAW + 2,83,49},
      { 8,PIKEDRAW + 3,144,66 },{8,PIKEDRAW + 4,197,99},{8,PIKEDRAW + 5,216,131},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76},
      {1,0,147,76}
    },
    { { 12,EXCALDRAW,167,130 },{12,EXCALDRAW + 1,70,117},{12,EXCALDRAW + 2,0,128},
      { 12,EXCALDRAW + 3,0,150 },{12,EXCALDRAW + 4,4,72},{12,EXCALDRAW + 5,38,81},
      { 12,EXCALDRAW + 6,0,44 },{12,EXCALDRAW + 7,112,0},{12,EXCALDRAW + 8,224,0},
      { 12,EXCALDRAW + 9,198,84 },{12,EXCALDRAW + 10,186,120},{12,EXCALDRAW + 11,188,123},
      {1,0,147,76}
    },
    { { 12,HALBERDDRAW,183,62 },{12,HALBERDDRAW + 1,166,10},{12,HALBERDDRAW + 2,173,29},
      { 12,HALBERDDRAW + 3,114,35 },{1,HALBERDATTACK1,245,22},{1,HALBERDATTACK1,245,22},
      { 1,HALBERDATTACK1,245,22 },{1,HALBERDATTACK1,245,22},{1,HALBERDATTACK1,245,22},
      { 1,HALBERDATTACK1,245,22 },{1,HALBERDATTACK1,245,22},{1,HALBERDATTACK1,245,22},
      {1,0,147,76}
    }
};

struct daweapons weaponanimtics[MAXWEAPONS][MAXFRAMES] =
{
    // FIST
    { { 10,RFIST,216,120 },{10,RFIST + 1,166,113},{10,RFIST + 2,156,129},
      { 10,RFIST + 3,169,151 },{10,RFIST + 4,153,124},{10,RFIST + 5,224,133},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    // KNIFE
    { { 8,KNIFEATTACK,189,52 },{8,KNIFEATTACK + 1,254,68},{8,0,147,76},
      { 8,0,80,41 },{8,KNIFEATTACK + 2,254,69},{8,KNIFEATTACK + 3,218,80},
      { 8,KNIFEATTACK + 4,137,83},{8,KNIFEATTACK + 5,136,100},{8,KNIFEATTACK + 6,126,140},
      { 8,KNIFEATTACK + 5,136,100},{8,KNIFEATTACK + 4,137,83},{8,KNIFEATTACK,189,52}
    },
    // GOBLINATTACK
    { {10,GOBSWORDATTACK,243,92},{10,GOBSWORDATTACK + 1,255,68 },{10,GOBSWORDATTACK + 2,279,65},
      {10,GOBSWORDATTACK + 3,238,55},{10,GOBSWORDATTACK + 4,153,52 },{10,GOBSWORDATTACK + 5,129,152},
      {10,GOBSWORDATTACK + 6,90,184},{ 1,0,297,169 },{1,0,275,24},
      {1,0,275,24 },{ 1,0,275,24 },{1,0,275,24}
    },
    // MORNINGSTAR
    { { 12,MORNINGSTAR,193,90 },{12,MORNINGSTAR + 1,102,133},{12,MORNINGSTAR + 2,77,164},
      { 12,MORNINGSTAR + 3,239,86 },{12,0,299,86},{12,0,107,52},
      { 12,MORNINGSTAR + 4,197,24 },{12,MORNINGSTAR + 5,125,124},{12,MORNINGSTAR + 6,109,191 },
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    // SWORD
    { { 8,SWORDATTACK,229,123},{8,SWORDATTACK + 1,221,87},{8,SWORDATTACK + 2,193,21},
      { 8,SWORDATTACK + 3,173,0},{8,SWORDATTACK + 4,61,0},{8,SWORDATTACK + 5,33,48},
      { 8,SWORDATTACK + 6,126,131},{8,SWORDATTACK + 7,297,164},{3,0,147,76},
      { 3,0,80,41 },{3,0,107,52},{3,0,147,76}
    },
    { { 12,BIGAXEATTACK,184,123 },{12,BIGAXEATTACK + 1,223,112},{12,BIGAXEATTACK + 2,63,151},
      { 12,BIGAXEATTACK + 3,91,133 },{12,BIGAXEATTACK + 4,127,138},{12,BIGAXEATTACK + 5,106,128},
      { 12,BIGAXEATTACK + 6,117,49 },{12,BIGAXEATTACK + 7,140,0},{12,BIGAXEATTACK + 8,152,47},
      { 12,BIGAXEATTACK + 9,166,143 },{12,0,107,52},{1,0,147,76}
    },
    // BOW
    { { 8,BOWWALK,75,13},{8,BOWWALK + 1,90,0},{8,BOWWALK + 2,70,0},
      { 8,BOWWALK + 3,70,0},{6,BOWWALK + 4,70,0},{4,BOWWALK + 5,70,0},
      { 1,0,126,131},{1,0,297,164},{1,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    { { 10,PIKEDRAW + 5,216,131},{10 ,0,80,41 },{10,0,107,52},{10,0,147,76},
      { 10,PIKEATTACK1,0,47 },{10,PIKEATTACK1 + 1,0,0},{10,PIKEATTACK1 + 2,0,0},
      { 10,PIKEATTACK1 + 3,73,0 },{10,PIKEATTACK1 + 4,130,27},{10,PIKEATTACK1 + 5,138,125},
      { 12,0,80,41 },{1,0,107,52}
    },
    { { 8,EXCALATTACK1,98,133},{8,EXCALATTACK1 + 1,123,130 },{8,EXCALATTACK1 + 2,125,128},
      { 8,EXCALATTACK1 + 3,115,82},{ 8,EXCALATTACK1 + 4,115,6 },{8,EXCALATTACK1 + 5,178,0},
      { 8,EXCALATTACK1 + 6,155,0},{ 8,EXCALATTACK1 + 7,143,0 },{8,EXCALATTACK1 + 8,90,91},
      { 8,EXCALATTACK1 + 9,30,159},{ 1,0,80,41 },{1,0,107,52}
    },
    { { 12,HALBERDATTACK1,245,22 },{12,0,107,52},{12,0,147,76},
      { 12,HALBERDATTACK1 + 1,249,45 },{12,HALBERDATTACK1 + 2,161,60},{12,HALBERDATTACK1 + 3,45,88},
      { 12,0,80,41 },{12,HALBERDATTACK1 + 3,45,88},{12,HALBERDATTACK1 + 2,161,60},
      { 12,HALBERDATTACK1 + 1,249,45 },{12,0,107,52},{1,0,147,76}
    }
};

struct daweapons weaponanimtics2[MAXWEAPONS][MAXFRAMES] =
{
    // FIST
    { { 10,RFIST,216,120 },{10,RFIST + 1,166,113},{10,RFIST + 2,156,129},
      { 10,RFIST + 3,169,151 },{10,RFIST + 4,153,124},{10,RFIST + 5,224,133},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    // KNIFE
    { { 8,KNIFEATTACK,189,52 },{8,KNIFEATTACK + 1,254,68},{16,0,147,76},
      { 8,KNIFEATTACK2,206,114 },{8,KNIFEATTACK2 + 1,107,112},{8,KNIFEATTACK2 + 2,22,138},
      { 8,KNIFEATTACK2 + 3,0,161},{16,0,136,100},{8,KNIFEATTACK2 + 3,0,161},
      { 8,KNIFEATTACK2 + 2,22,138},{8,KNIFEATTACK2 + 1,107,112},{8,KNIFEATTACK2,206,114}
    },
    // GOBLINATTACK
    { { 10,GOBSWORDATTACK2,236,99 },{10,GOBSWORDATTACK2 + 1,202,24},{10,GOBSWORDATTACK2 + 2,181,0},
      { 10,GOBSWORDATTACK2 + 3,52,12 },{10,GOBSWORDATTACK2 + 4,72,72},{10,GOBSWORDATTACK2 + 5,134,139},
      { 10,GOBSWORDATTACK2 + 6,297,169 },{1,0,275,24},{1,0,275,24 },
      { 1,0,275,24 },{1,0,275,24},{1,0,275,24}
    },
    // MORNINGATTACK2
    { { 12,MORNINGATTACK2,85,136 },{12,MORNINGATTACK2 + 1,34,110},{12,MORNINGATTACK2 + 2,32,91},
      { 12,MORNINGATTACK2 + 3,186,47 },{12,MORNINGATTACK2 + 4,275,24},{1,0,275,24},
      { 1,0,275,24 },{1,0,275,24},{1,0,275,24 },
      { 1,0,275,24 },{1,0,275,24},{1,0,275,24}
    },
    // SWORD
    { { 8,SWORDATTACK2 + 1,195,63},{8,SWORDATTACK2 + 2,250,54},{8,SWORDATTACK2 + 3,275,37},
      {16,0,61,0},{ 8,SWORDATTACK2 + 4,229,66},{8,SWORDATTACK2 + 5,185,0},
      { 8,SWORDATTACK2 + 6,158,115},{8,SWORDATTACK2 + 7,57,163},{1,0,57,163},
      { 1,0,57,163 },{1,0,57,163},{1,0,57,163}
    },
    { { 12,BIGAXEATTACK2,200,111 },{12,BIGAXEATTACK2 + 1,5,136},{12,BIGAXEATTACK2 + 2,69,162},
      { 12,BIGAXEATTACK2 + 3,147,164 },{12,BIGAXEATTACK2 + 4,76,152},{12,BIGAXEATTACK2 + 5,33,95},
      { 12,BIGAXEATTACK2 + 6,0,91 },{12,BIGAXEATTACK2 + 7,0,98},{12,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    // BOW
    { { 8,BOWWALK,75,13},{8,BOWWALK + 1,90,0},{8,BOWWALK + 2,70,0},
      { 8,BOWWALK + 3,70,0},{6,BOWWALK + 4,70,0},{4,BOWWALK + 5,70,0},
      { 1,0,126,131},{1,0,297,164},{1,0,147,76},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    { { 10,PIKEATTACK2,266,147 },{10,PIKEATTACK2 + 1,182,117},{10,PIKEATTACK2 + 2,123,84},
      { 10,PIKEATTACK2 + 3,7,48 },{10,PIKEATTACK2 + 4,0,83},{10,PIKEATTACK2 + 5,0,158},
      { 10,PIKEATTACK2 + 6,25,117 },{10,PIKEATTACK2 + 7,139,93},{10,PIKEATTACK2 + 8,234,75},
      { 8,0,80,41 },{1,0,107,52},{1,0,147,76}
    },
    { { 8,EXCALATTACK2,0,143},{8,EXCALATTACK2 + 1,0,103 },{8,EXCALATTACK2 + 2,0,70},
      { 8,EXCALATTACK2 + 3,48,0},{ 8,EXCALATTACK2 + 4,67,0 },{8,EXCALATTACK2 + 5,78,21},
      { 8,EXCALATTACK2 + 6,165,107},{ 8,EXCALATTACK2 + 7,260,168 },{1,0,130,27},
      { 1,0,138,125},{ 1,0,80,41 },{1,0,107,52}
    },
    { { 12,HALBERDATTACK1,245,22 },{12,HALBERDATTACK2,114,35},{12,HALBERDATTACK2 + 1,105,87},
      { 12,HALBERDATTACK2 + 2,54,107 },{12,HALBERDATTACK2 + 3,48,102},{1,HALBERDATTACK2 + 3,48,102},
      { 1,HALBERDATTACK2 + 3,48,102 },{12,HALBERDATTACK2 + 2,54,107},{12,HALBERDATTACK2 + 1,105,87},
      { 1,0,80,41 },{1,0,107,52},{1,0,147,76}
    }
};

// fist
// bracers
// hammer
// sword
// bow
// axe
// morning star
// bfg item


void playerdead(Player* plr)
{
    int i;
    int spellbookpage;
    int exit = 0;

    int32_t clockgoal = (int)totalclock + 240;

    plr->playerdie = true;
    currspikeframe = 0;

    if (plr->spiked)
    {
        spiketics = spikeanimtics[0].daweapontics;

        playsound_loc(S_GORE1, plr->x, plr->y);
        SND_Sound(S_HEARTBEAT);
    }

    SND_PlaySound(S_PLRDIE1, 0, 0, 0, 0);

    netsendmove();

    while ((int)totalclock < clockgoal)
    {
        handleevents();

        if (plr->horiz < 100 + (YDIM >> 1))
        {
            plr->horiz += (synctics << 1);
        }

        drawscreen(plr);
        animateobjs(plr);
        animatetags(plr);
        doanimations(synctics);
        dodelayitems(synctics);

        videoNextPage();
    }

    int32_t goaltime = (int)totalclock + 240;

    while (!exit)
    {
        handleevents();

        if ((int)totalclock > goaltime)
            exit = 1;

        if (keystatus[sc_Space] > 0)
            exit = 1;
    }

    SND_CheckLoops();

    keystatus[sc_Space] = 0;

    plr->vampiretime = 0;
    plr->shieldpoints = 0;
    plr->playerdie = false;
    plr->spiked = false;
    plr->shockme = -1;
    plr->poisoned = 0;
    plr->poisontime = 0;
    currspikeframe = 0;
    spellbookflip = 0;

    StatusMessageDisplayTime(0);

    plr->oldsector = plr->sector;
    plr->horiz = 100;
    plr->zoom = 256;
    plr->dimension = 3;
    plr->height = PLAYERHEIGHT;

    if (difficulty > 1)
    {
        for (i = 0; i <= 9; i++)
        {
            plr->ammo[i] = 0;
            plr->weapon[i] = 0;
            if (i < 9)
            {
                plr->orb[i] = 0;
                plr->orbammo[i] = 0;
            }
        }
    }
    else
    {
        for (i = 0; i <= 9; i++)
        {
            if (i < 5)
            {
                plr->ammo[i] = 40;
                plr->weapon[i] = 1;
            }
            if (i < 9)
            {
                plr->orb[i] = 0;
                plr->orbammo[i] = 0;
            }
        }
    }

    if (difficulty > 1)
    {
        plr->weapon[0] = plr->weapon[1] = 1;
        plr->ammo[0] = 32000;
        plr->ammo[1] = 45;
    }

    for (i = 0; i < MAXPOTIONS; i++)
        plr->potion[i] = 0;

    for (i = 0; i < MAXTREASURES; i++)
        plr->treasure[i] = 0;

    plr->hasshot = false;
    plr->orbshot = false;
    oldmousestatus = 0;

    plr->lvl = 1;
    plr->score = 0;
    plr->health = 100;
    plr->maxhealth = 100;
    plr->armor = 0;
    plr->armortype = 0;
    plr->currentorb = 0;
    plr->currentpotion = 0;

    for (i = 0; i < MAXNUMORBS; i++)
        plr->orbactive[i] = -1;

    lockclock = (int)totalclock;

    if (difficulty > 1)
        plr->currweapon = plr->selectedgun = 1;
    else
        plr->currweapon = plr->selectedgun = 4;

    plr->currweaponfired = 3;
    plr->currweaponflip = false;
    plr->currweaponanim = 0;

    plr->helmettime = -1;

    if (svga == 0)
    {
        if (plr->screensize <= 320)
        {
            spellbookpage = spellbookanim[plr->currentorb][8].daweaponframe;
            overwritesprite(121, 161, spellbookpage, 0, 0, 0);
        }
    }
    if (svga == 1)
    {
        if (plr->screensize == 320)
        {
            spellbookpage = sspellbookanim[plr->currentorb][8].daweaponframe;
            overwritesprite(121 << 1, 389, spellbookpage, 0, 0, 0);
        }
    }

    justteleported = 1;

    if (netgame == 0)
    {
        loadnewlevel(mapon);
    }
    else
    {
        netrestartplayer(plr);
    }

    StatusMessageDisplayTime(-1);
    plr->shadowtime = -1;
    plr->helmettime = -1;
    plr->nightglowtime = -1;
    plr->strongtime = -1;
    plr->invisibletime = -1;
    plr->svgahealth = -1;

//    updatepics();
}

void spikeheart(Player* plr)
{
    char dabits = 0x02;
    char dashade = sector[plr->sector].ceilingshade;
    int  dax, day;

    spiketics -= synctics;

    if (spiketics < 0)
    {
        currspikeframe++;

        if (currspikeframe > 4)
            currspikeframe = 4;

        spiketics = spikeanimtics[currspikeframe].daweapontics;
        spikeframe = spikeanimtics[currspikeframe].daweaponframe;
    }
    else
        currweaponframe = spikeanimtics[currspikeframe].daweaponframe;

    dax = spikeanimtics[currspikeframe].currx;
    day = spikeanimtics[currspikeframe].curry;

    overwritesprite(dax, day, spikeframe, dashade, dabits, 0);
    startredflash(10);
}

void updateloadedplayer(int i)
{
    Player* plr = &player[pyrn];

    plr->playerdie = false;
    plr->spiked = false;

    plr->oldsector = plr->sector;
    plr->horiz = 100;
    plr->zoom = 256;
    plr->dimension = 3;
    plr->height = PLAYERHEIGHT;
    plr->spritenum = i;

    sprite[plr->spritenum].x = plr->x;
    sprite[plr->spritenum].y = plr->y;
    sprite[plr->spritenum].z = plr->z + (plr->height << 8);
    sprite[plr->spritenum].cstat = 1;
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
    sprite[plr->spritenum].sectnum = plr->sector;

    vec3_t pos;
    pos.x = sprite[i].x;
    pos.y = sprite[i].y;
    pos.z = sprite[i].z;

    setsprite(i, &pos);
}

void initplayersprite()
{
    int i;
    int spellbookpage;

    Player* plr = &player[pyrn];

    plr->vampiretime = 0;
    plr->shieldpoints = 0;
    plr->playerdie = false;
    plr->spiked = false;
    plr->shockme = -1;
    plr->poisoned = 0;
    plr->poisontime = -1;

    if (mapflag == 0)
        mapon = 1;

    plr->oldsector = plr->sector;
    plr->horiz = 100;
    plr->zoom = 256;
    plr->dimension = 3;
    plr->height = PLAYERHEIGHT;
    plr->z = sector[plr->sector].floorz - (plr->height << 8);

    plr->spritenum = insertsprite(plr->sector, 0);

    plr->onsomething = 1;

    sprite[plr->spritenum].x = plr->x;
    sprite[plr->spritenum].y = plr->y;
    sprite[plr->spritenum].z = plr->z + (plr->height << 8);
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

    if (loadedgame == 0)
    {
        plr->selectedgun = 0;

        if (difficulty > 1)
        {
            for (i = 0; i <= 9; i++)
            {
                plr->ammo[i] = 0;
                plr->weapon[i] = 0;
                if (i < 9)
                {
                    plr->orb[i] = 0;
                    plr->orbammo[i] = 0;
                }
            }
        }
        else
        {
            for (i = 0; i <= 9; i++)
            {
                if (i < 5)
                {
                    plr->ammo[i] = 40;
                    plr->weapon[i] = 1;
                }
                if (i < 9)
                {
                    plr->orb[i] = 0;
                    plr->orbammo[i] = 0;
                }
            }
        }

        if (difficulty > 1)
        {
            plr->weapon[0] = plr->weapon[1] = 1;
            plr->ammo[0] = 32000;
            plr->ammo[1] = 45;
        }

        for (i = 0; i < MAXPOTIONS; i++)
            plr->potion[i] = 0;

        for (i = 0; i < MAXTREASURES; i++)
            plr->treasure[i] = 0;

        plr->lvl = 1;
        plr->score = 0;
        plr->health = 100;
        plr->maxhealth = 100;
        plr->armor = 0;
        plr->armortype = 0;
        plr->currentorb = 0;
        plr->currentpotion = 0;

        if (difficulty > 1)
            plr->currweapon = plr->selectedgun = 1;
        else
            plr->currweapon = plr->selectedgun = 4;

        plr->currweaponfired = 3;
        plr->currweaponflip = false;

        plr->currweaponanim = 0;
        plr->currweaponattackstyle = 0;

        for (i = 0; i < MAXNUMORBS; i++)
            plr->orbactive[i] = -1;

        lockclock = (int)totalclock;

        spellbookflip = 0;

        if (svga == 0)
        {
            if (plr->screensize <= 320)
            {
                spellbookpage = spellbookanim[plr->currentorb][8].daweaponframe;
                overwritesprite(121, 161, spellbookpage, 0, 0, 0);
            }
        }
        if (svga == 1)
        {
            if (plr->screensize == 320)
            {
                spellbookpage = sspellbookanim[plr->currentorb][8].daweaponframe;
                overwritesprite(121 << 1, 389, spellbookpage, 0, 0, 0);
            }
        }

        plr->invincibletime = -1;
        plr->manatime = -1;
        plr->hasshot = false;;
        plr->orbshot = false;
        oldmousestatus = 0;

        StatusMessageDisplayTime(-1);
        plr->shadowtime = -1;
        plr->helmettime = -1;
        plr->nightglowtime = -1;
        plr->strongtime = -1;
        plr->invisibletime = -1;
        plr->svgahealth = -1;
    }

    //updatepics();
}

void autoweaponchange(int dagun)
{
    Player* plr = &player[pyrn];

    if (plr->currweaponanim > 0)
        return;

    if (dagun > plr->selectedgun)
    {
        plr->selectedgun = dagun;
        plr->hasshot = false;
        plr->currweaponfired = 2; // drop weapon

        switch (plr->selectedgun)
        {
            case 1:
            weapondropgoal = 100;
            weapondrop = 0;
            break;
            case 2:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
            case 3:
            weapondropgoal = 100;
            weapondrop = 0;
            //levelpic();
            break;
            case 4:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
            case 5:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
            case 6:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
            case 7:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
            case 8:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
            case 9:
            weapondropgoal = 40;
            weapondrop = 0;
            //levelpic();
            break;
        }
    }
}

void weaponchange()
{
    Player* plr = &player[pyrn];

    if (plr->currweaponanim == 0 && !plr->currweaponflip)
    {
        for (int i = sc_1; i <= sc_0; i++)
        {
            if (keystatus[i] > 0 && plr->weapon[i - sc_1] > 0)
            {
                keystatus[i] = 0;

                plr->selectedgun = i - sc_1;
                plr->hasshot = false;
                plr->currweaponfired = 2; // drop weapon

                //levelpic();

                switch (plr->selectedgun)
                {
                    case 0:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    weapondropgoal = 40;
                    weapondrop = 0;
                    break;
                    case 1:
                    case 2:
                    case 3:
                    weapondropgoal = 100;
                    weapondrop = 0;
                    break;
                }
            }
        }
    }

    if (!plr->currweaponflip)
    {
        for (int i = sc_F1; i <= sc_F8; i++)
        {
            if (keystatus[i] > 0)
            {
                if (plr->selectedgun > 0)
                {
                    plr->hasshot = false;
                    plr->currweaponfired = 2; // drop weapon
                    weapondropgoal = 100;
                    weapondrop = 0;
                    plr->selectedgun = 0;
                    //selectedgun=spellcasthands
                    //levelpic();
                }
                plr->currentorb = i - sc_F1;
                if (spellbookflip == 0)
                {
                    spellbook = 0;
                    spellbooktics = 10;
                    spellbookflip = 1;

                    SND_PlaySound(S_PAGE, 0, 0, 0, 0);
                    displayspelltext();
                    spelltime = 360;
                }
                plr->orbshot = false;
            }
            keystatus[i] = 0;
//            orbpic(plr->currentorb);
        }
    }

    for (int i = 0; i < MAXNUMORBS; i++)
    {
        if (plr->orbactive[i] > -1) {
            plr->orbactive[i] -= synctics;
        }
    }

    if (keystatus[sc_OpenBracket] > 0)
    {
        plr->currentpotion--;
        if (plr->currentpotion < 0)
            plr->currentpotion = 4;
        keystatus[sc_OpenBracket] = 0;

        SND_PlaySound(S_BOTTLES, 0, 0, 0, 0);
        setpotion(plr->currentpotion);
        potiontext();
    }
    if (keystatus[sc_CloseBracket] > 0)
    {
        plr->currentpotion++;
        if (plr->currentpotion > 4) // MAXPOTIONS
            plr->currentpotion = 0;
        keystatus[sc_CloseBracket] = 0;

        SND_PlaySound(S_BOTTLES, 0, 0, 0, 0);
        setpotion(plr->currentpotion);
        potiontext();
    }
}

void potiontext()
{
    Player* plr = &player[pyrn];

    if (plr->potion[plr->currentpotion] > 0)
    {
        switch (plr->currentpotion)
        {
            case 0:
            StatusMessage(240, "Health Potion");
            break;
            case 1:
            StatusMessage(240, "Strength Potion");
            break;
            case 2:
            StatusMessage(240, "Cure Poison Potion");
            break;
            case 3:
            StatusMessage(240, "Resist Fire Potion");
            break;
            case 4:
            StatusMessage(240, "Invisiblity Potion");
            break;
        }
    }
}

void swingdacrunch()
{
    Player* plr = &player[pyrn];

    switch (plr->currweapon)
    {
        case 0: //fist
        case 3: //morningstar
        playsound_loc(S_SOCK1 + (rand() % 4), plr->x, plr->y);
        break;
        case 1: //dagger
        if (rand() % 2)
            playsound_loc(S_GORE1 + (rand() % 4), plr->x, plr->y);
        break;
        case 2: //short sword
        playsound_loc(S_SWORD2 + (rand() % 3), plr->x, plr->y);
        break;
        case 4: //broad sword
        playsound_loc(S_SWORD1 + (rand() % 3), plr->x, plr->y);
        break;
        case 5: //battle axe
        case 7: //pike
        case 9: //halberd
        if (rand() % 2)
            playsound_loc(S_SOCK1 + (rand() % 4), plr->x, plr->y);
        else
            playsound_loc(S_SWORD1 + (rand() % 3), plr->x, plr->y);
        break;
        case 6: //bow
        break;

        case 8: //two handed sword
        playsound_loc(S_SWORD1 + (rand() % 2), plr->x, plr->y);
        break;
    }
}

void swingdasound()
{
    Player* plr = &player[pyrn];

    switch (plr->currweapon)
    {
        case 0: // fist
        SND_PlaySound(S_PLRWEAPON0, 0, 0, 0, 0);
        break;
        case 1:  // knife
        SND_PlaySound(S_PLRWEAPON1, 0, 0, 0, 0);
        break;
        case 2:  // short sword
        SND_PlaySound(S_PLRWEAPON4, 0, 0, 0, 0);
        break;
        case 3:  // mace
        SND_PlaySound(S_PLRWEAPON2, 0, 0, 0, 0);
        break;
        case 4:  //
        SND_PlaySound(S_PLRWEAPON4, 0, 0, 0, 0);
        break;
        case 5:  // sword
        SND_PlaySound(S_PLRWEAPON4, 0, 0, 0, 0);
        break;
        case 6:  // bow
        SND_PlaySound(S_PLRWEAPON3, 0, 0, 0, 0);
        break;
        case 7:  //
        SND_PlaySound(S_PLRWEAPON4, 0, 0, 0, 0);
        break;
        case 8:  //
        SND_PlaySound(S_PLRWEAPON4, 0, 0, 0, 0);
        break;
        case 9:  //
        SND_PlaySound(S_PLRWEAPON4, 0, 0, 0, 0);
        break;
    }
}

void swingdaweapon(Player* plr)
{
    short daang = plr->ang;

    if (currweaponframe == BOWWALK + 5 && plr->ammo[6] > 0)
    {
        plr->currweaponfired = 5;
        oldmousestatus = 0; // NEW
        plr->currweaponanim = 0;
    }
    if (currweaponframe == BOWWALK + 5 && plr->ammo[6] <= 0)
    {
        plr->currweaponfired = 0;
        oldmousestatus = 0;
        plr->currweaponanim = 0;
        return;
    }

    if (currweaponframe == PIKEATTACK1 + 4
        //|| currweaponframe == PIKEATTACK2+4
        && plr->weapon[7] == 2
        && plr->ammo[7] > 0)
    {
        shootgun(plr, daang, 10);
        playsound_loc(S_THROWPIKE, plr->x, plr->y);
        plr->hasshot = true;
        return;
    }

    switch (plr->selectedgun)
    {
        case 0:  // fist  & close combat weapons
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 1:  // knife
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 2:  // shortsword
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 3:  // morningstar
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 4:  // sword
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 5: //  battleaxe
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 6: // bow
        shootgun(plr, daang, 1);
        plr->hasshot = true;
        break;
        case 7: // pike
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 8: // two handed
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
        case 9: // halberd
        shootgun(plr, daang, 0);
        plr->hasshot = true;
        break;
    }
}

int weaponuseless=0;

void plrfireweapon(Player* plr)
{
    int i;

    if (plr->currweaponfired == 4)
    {
        currweapontics = throwanimtics[plr->currentorb][0].daweapontics;
        return;
    }

    if (plr->ammo[plr->selectedgun] <= 0)
    {
        if (plr->currweapon == 6)
        {
            for (i = 0; i < MAXWEAPONS; i++)
            {
                if (plr->ammo[i] > 0 && plr->weapon[i] == 1)
                {
                    plr->selectedgun = i;
                    plr->hasshot = false;
                    plr->currweaponfired = 2; // drop weapon
                    weapondropgoal = 100;
                    weapondrop = 0;
                    //levelpic();
                }
            }
        }
        return;
    }
    else
    {
        madeahit = 0;
        plr->ammo[plr->selectedgun]--;
        if (plr->ammo[plr->selectedgun] <= 0 || plr->ammo[plr->selectedgun] == 10)
        {
            switch (plr->selectedgun)
            {
                case 0: //fist
                plr->ammo[0] = 9999;
                break;
                case 1: //knife
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Dagger is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[1] = 0;
                    plr->weapon[1] = 0;
                    StatusMessage(360, "Dagger is Useless");
                    weaponuseless = 1;
                }
                break;
                case 2: //short sword
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Short Sword is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[2] = 0;
                    plr->weapon[2] = 0;
                    StatusMessage(360, "Short Sword is Useless");
                    weaponuseless = 1;
                }
                break;
                case 3: //mace
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Morning Star is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[3] = 0;
                    plr->weapon[3] = 0;
                    StatusMessage(360, "Morning Star is Useless");
                    weaponuseless = 1;
                }
                break;

                case 4: //sword
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Sword is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[4] = 0;
                    plr->weapon[4] = 0;
                    StatusMessage(360, "Sword is Useless");
                    weaponuseless = 1;
                }
                break;
                case 5: //battle axe
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Battle axe is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[5] = 0;
                    plr->weapon[5] = 0;
                    StatusMessage(360, "Battle axe is Useless");
                    weaponuseless = 1;
                }
                break;
                case 6: //bow
                break;
                case 7: //pike
                if (plr->weapon[7] == 1)
                {
                    if (plr->ammo[plr->selectedgun] == 10)
                    {
                        StatusMessage(360, "Pike is damaged");
                    }
                    if (plr->ammo[plr->selectedgun] <= 0)
                    {
                        plr->ammo[7] = 0;
                        plr->weapon[7] = 0;
                        StatusMessage(360, "Pike is Useless");
                        weaponuseless = 1;
                    }
                }
                if (plr->weapon[7] == 2 && plr->ammo[7] <= 0)
                {
                    plr->weapon[7] = 1;
                    plr->ammo[7] = 30;
                }
                break;
                case 8: // two handed sword
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Magic Sword is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[8] = 0;
                    plr->weapon[8] = 0;
                    StatusMessage(360, "Magic Sword is Useless");
                    weaponuseless = 1;
                }
                break;
                case 9: //halberd
                if (plr->ammo[plr->selectedgun] == 10)
                {
                    StatusMessage(360, "Halberd is damaged");
                }
                if (plr->ammo[plr->selectedgun] <= 0)
                {
                    plr->ammo[9] = 0;
                    plr->weapon[9] = 0;
                    StatusMessage(360, "Halberd is Useless");
                    weaponuseless = 1;
                }
                break;
            }
        }
    }

    if (weaponuseless == 1)
    {
        for (i = 0; i < MAXWEAPONS; i++)
        {
            if (plr->weapon[i] > 0 && plr->ammo[i] > 0)
            {
                plr->currweapon = plr->selectedgun = i;
                //hasshot=0;
                //currweaponfired=2; // drop weapon
                plr->currweaponfired = 3; // ready weapon
                //weapondropgoal=100;
                //weapondrop=0;
                plr->currweaponflip = false;
                weaponuseless = 0;
                //autoweaponchange(i);
                //levelpic();
            }
        }
    }
    else
    {
        plr->currweaponfired = 1;
    }

//    if (plr->currweapon == 6 || plr->selectedgun == 6)
        //levelpic();

 //   if (plr->currweapon == 7 || plr->selectedgun == 7)
        //levelpic();

    plr->currweapon = plr->selectedgun;

    // start from the beginning to cycle
    //currweaponfired=1;

    plr->currweaponattackstyle = krand() % 2;

    if (plr->weapon[7] == 2 && plr->currweapon == 7)
    {
        plr->currweaponattackstyle = 0;
    }

    if (plr->currweapon == 9)
    {
        if (krand() % 100 > 80)
            plr->currweaponattackstyle = 0;
        else
            plr->currweaponattackstyle = 1;
    }

    if (plr->currweaponanim > 11)
    {
        currweapontics = weaponanimtics[plr->currweapon][0].daweapontics;
    }
}

void activatedaorb(Player* plr)
{
    if (plr->orbammo[plr->currentorb] <= 0)
        return;

    switch (plr->currentorb)
    {
        case 0: // SCARE
           //shadowtime=1200+(plr->lvl*120);
        break;
        case 1: // NIGHT VISION
           //nightglowtime=2400+(plr->lvl*600);
        break;
        case 2: // FREEZE
        plr->orbactive[plr->currentorb] = -1;
        break;
        case 3: // MAGIC ARROW
        plr->orbactive[plr->currentorb] = -1;
        break;
        case 4: // OPEN DOORS
        plr->orbactive[plr->currentorb] = -1;
        break;
        case 5: // FLY
           //plr->orbactive[currentorb]=3600+(plr->lvl*600);
        break;
        case 6: // FIREBALL
        plr->orbactive[plr->currentorb] = -1;
        break;
        case 7: // NUKE
        plr->orbactive[plr->currentorb] = -1;
        break;
    }

    if (plr->orbammo[plr->currentorb] <= 0)
    {
        plr->orb[plr->currentorb] = 0;
        return;
    }
    else
        plr->orbammo[plr->currentorb]--;

    plr->currweaponfired = 4;
    currweapontics = throwanimtics[plr->currentorb][0].daweapontics;
}

void plruse(Player* plr)
{
    short daang2, daang;
    int32_t daz2;
    int32_t hitx, hity, hitz;

    neartag(plr->x, plr->y, plr->z, plr->sector, plr->ang, &neartagsector, &neartagwall, &neartagsprite, &neartaghitdist, 1024, 3, NULL);

    if (neartagsector >= 0)
    {
        if (sector[neartagsector].hitag == 0)
        {
            operatesector(neartagsector);
        }
        else
        {
            daang = plr->ang;
            daang2 = daang + 2048;
            daz2 = (100 - plr->horiz) * 2000;

            vec3_t pos;
            pos.x = plr->x;
            pos.y = plr->y;
            pos.z = plr->z;

            hitdata_t hitinfo;

            hitscan(&pos, plr->sector,   // Start position
                Cos(daang2 + 2048),      // X vector of 3D ang
                Sin(daang2 + 2048),      // Y vector of 3D ang
                daz2,                    // Z vector of 3D ang
                &hitinfo, CLIPMASK1); // CHECKME - is CLIPMASK1 correct for version of this function that hadn't got cliptype param?

            hitx = hitinfo.pos.x;
            hity = hitinfo.pos.y;
            hitz = hitinfo.pos.z;

            if (hitinfo.wall >= 0)
            {
                if ((labs(plr->x - hitx) + labs(plr->y - hity) < 512) && (labs((plr->z >> 8) - ((hitz >> 8) - (64))) <= (512 >> 3)))
                {
                    switch (wall[hitinfo.wall].picnum)
                    {
                        case PENTADOOR1:
                        case PENTADOOR2:
                        case PENTADOOR3:
                        case PENTADOOR4:
                        case PENTADOOR5:
                        case PENTADOOR6:
                        case PENTADOOR7:
                        StatusMessage(360, "find door trigger");
                        break;
                    }
                }
            }
            playsound_loc(S_PUSH1 + (rand() % 2), plr->x, plr->y);
        }
    }

    if (neartagsprite >= 0)
    {
        if (sprite[neartagsprite].lotag == 1)
        {
            switch (sprite[neartagsprite].picnum)
            {
                case PULLCHAIN1:
                sprite[neartagsprite].lotag = 0;
                newstatus(neartagsprite, PULLTHECHAIN);
                break;
                case LEVERUP:
                sprite[neartagsprite].lotag = 0;
                newstatus(neartagsprite, ANIMLEVERUP);
                break;
            }
            for (int i = 0; i < numsectors; i++)
            {
                if (sector[i].hitag == sprite[neartagsprite].hitag)
                {
                    operatesector(i);
                }
            }
        }
        else
        {
            operatesprite(neartagsprite);
        }
    }
}

void loadnewlevel(int mapon)
{
    char mapbuf[BMAX_PATH];

    Player* plr = &player[pyrn];

    sprintf(mapbuf, "level%d.map", mapon);

    SND_SongFlush();
    SND_StartMusic(mapon - 1);

    #ifdef WHDEMO
    if (mapon > 3)
    {
        victory();
    }
    #endif

    setupboard(mapbuf);
}

void victory()
{
    int exit;

    int32_t goaltime = (int)totalclock + 360;
    victor = 1;

    if (svga == 1)
    {
        SND_Sound(S_PICKUPFLAG);
        permanentwritesprite(0, 0, SVGAVICTORYA1, 0, 0, 0, 639, 239, 0);
        permanentwritesprite(0, 240, SVGAVICTORYA2, 0, 0, 240, 639, 479, 0);

        videoNextPage();

        exit = 0;

        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;

        videoNextPage();

        exit = 0;
        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;

        SND_Sound(S_DROPFLAG);
        permanentwritesprite(0, 0, SVGAVICTORYB1, 0, 0, 0, 639, 239, 0);
        permanentwritesprite(0, 240, SVGAVICTORYB2, 0, 0, 240, 639, 479, 0);

        videoNextPage();

        exit = 0;
        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;

        SND_Sound(S_WISP2);
        permanentwritesprite(0, 0, SVGAVICTORYC1, 0, 0, 0, 639, 239, 0);
        permanentwritesprite(0, 240, SVGAVICTORYC2, 0, 0, 240, 639, 479, 0);

        videoNextPage();

        exit = 0;
        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;

    }
    else
    {
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;
        SND_Sound(S_PICKUPFLAG);
        exit = 0;
        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
            overwritesprite(0, 0, VICTORYA, 0, 0, 0);
            videoNextPage();
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;
        SND_Sound(S_DROPFLAG);

        exit = 0;

        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
            overwritesprite(0, 0, VICTORYB, 0, 0, 0);
            videoNextPage();
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;

        SND_Sound(S_WISP2);

        exit = 0;

        while (!exit)
        {
            if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                exit = 1;
            overwritesprite(0, 0, VICTORYC, 0, 0, 0);
            videoNextPage();
        }
        keystatus[sc_Space] = 0;
        keystatus[sc_Escape] = 0;
        exit = 0;

    }
    shutdown();
}

void drawweapons(Player* plr)
{
    int dax, day;
    int32_t snakex = 0;
    int32_t snakey = 0;
    char dabits;
    char dashade;
    char dapalnum;

    if (spelltime > 0)
        spelltime -= synctics;

    if (spellbook == 8 && spelltime > 0 && plr->screensize > 320)
    {
        if (svga == 1)
        {
            spellbookframe = sspellbookanim[plr->currentorb][8].daweaponframe;
            dax = sspellbookanim[plr->currentorb][8].currx;
            day = sspellbookanim[plr->currentorb][8].curry;
        }
        else
        {
            spellbookframe = spellbookanim[plr->currentorb][8].daweaponframe;
            dax = spellbookanim[plr->currentorb][8].currx;
            day = spellbookanim[plr->currentorb][8].curry;
        }
        if (svga == 1)
        {
            overwritesprite(dax << 1, day, spellbookframe, 0, 0, 0);
            sprintf(tempbuf, "%d", plr->orbammo[plr->currentorb]);
            fancyfont(126 << 1, 439, SSCOREFONT - 26, tempbuf, 0);
        }
        else
        {
            itemtoscreen(dax, day, spellbookframe, 0, 0);
            //overwritesprite(dax, day, spellbookframe, 0, 0, 0);
            sprintf(tempbuf, "%d", plr->orbammo[plr->currentorb]);
            fancyfont(126, 181, SCOREFONT - 26, tempbuf, 0);
        }
    }

    if (plr->shadowtime > 0)
        dashade = 31, dapalnum = 0;
    else
        dashade = sector[plr->sector].ceilingshade, dapalnum = 0;

    if (plr->invisibletime > 0)
        dabits = 0x06;
    else
        dabits = 0x02;

    if (plr->currweaponflip)
        dabits += 0x08;

    if ((plr->currweapon == 0) && (dahand == 0))
        if (rand() % 2 == 0)
            dahand = 1;
        else
            dahand = 2;

    switch (plr->currweaponfired)
    {
        case 6:
        switch (plr->currweapon)
        {
            case 1: // knife
            if (currweaponframe == KNIFEATTACK2 + 1)
                if ((plr->currweaponanim == 2 || plr->currweaponanim == 10) && currweapontics == 8)
                    swingdasound();
            break;
            case 3: // morning
            if (currweaponframe == MORNINGATTACK2 + 3)
                if (plr->currweaponanim == 3 && currweapontics == 12)
                    swingdasound();
            break;
        }
        if (currweaponframe == RFIST + 5
            || currweaponframe == KNIFEATTACK + 6
            || currweaponframe == MORNINGSTAR + 5
            || currweaponframe == SWORDATTACK + 7
            || currweaponframe == BOWWALK + 5
            || currweaponframe == KNIFEATTACK2 + 2
            || currweaponframe == SWORDATTACK2 + 6
            || currweaponframe == MORNINGATTACK2 + 3
            || currweaponframe == HALBERDATTACK1 + 3
            || currweaponframe == HALBERDATTACK2 + 3
            || currweaponframe == BIGAXEATTACK + 7
            || currweaponframe == BIGAXEATTACK2 + 6
            || currweaponframe == PIKEATTACK1 + 4
            || currweaponframe == PIKEATTACK2 + 4
            || currweaponframe == EXCALATTACK1 + 7
            || currweaponframe == EXCALATTACK2 + 5
            || currweaponframe == GOBSWORDATTACK2 + 4
            || currweaponframe == GOBSWORDATTACK + 4)
            swingdaweapon(plr);

        currweapontics -= synctics;

        if (plr->helmettime > 0)
            currweapontics--;

        if (currweapontics < 0)
        {
            plr->currweaponanim++;

            if (plr->currweaponanim > 11)
            {
                plr->currweaponanim = 0;
                plr->currweaponfired = 0;
                plr->currweaponflip = false;
                plr->currweapon = plr->selectedgun;
                oldmousestatus = 0;

                if (dahand > 0)
                    dahand = 0;
            }

            currweapontics = lefthandanimtics[plr->currweapon][plr->currweaponanim].daweapontics;
            currweaponframe = lefthandanimtics[plr->currweapon][plr->currweaponanim].daweaponframe;
            dax = lefthandanimtics[plr->currweapon][plr->currweaponanim].currx;
            day = lefthandanimtics[plr->currweapon][plr->currweaponanim].curry + 8;
        }

        else
        {
            currweaponframe = lefthandanimtics[plr->currweapon][plr->currweaponanim].daweaponframe;
            dax = lefthandanimtics[plr->currweapon][plr->currweaponanim].currx;
            day = lefthandanimtics[plr->currweapon][plr->currweaponanim].curry + 8;
        }
        if (plr->currweapon == 0 && currweaponframe != NULL)
        {
            if (dahand == 1)
                overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            else if (dahand == 2)
            {
                dax = lefthandanimtics[0][plr->currweaponanim].currx;
                day = lefthandanimtics[0][plr->currweaponanim].curry + 8;
                overwritesprite(dax, day + 5, currweaponframe + 6, dashade, dabits, dapalnum);
            }
        }
        else
        {
            if (currweaponframe != 0)
            {
                dax = lefthandanimtics[plr->currweapon][plr->currweaponanim].currx;
                overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            }
        }

        if (plr->currweapon == 0 && currweaponframe == 0)
        {
            dahand = 0;
            oldmousestatus = 0;
            plr->currweaponanim = 0;
            plr->currweaponfired = 0;
        }

        if (plr->selectedgun == 4 && currweaponframe == 0)
        {
            plr->currweaponanim = 0;
            plr->currweaponfired = 0;
            plr->currweaponflip = false;
            plr->currweapon = plr->selectedgun;
            oldmousestatus = 0;
        }

        break;
        case 1: // fire
        switch (plr->currweapon)
        {
            case 0: // fist
            if (currweaponframe == RFIST + 5)
                if (plr->currweaponanim == 5 && currweapontics == 10)
                    swingdasound();
            break;
            case 1: // knife
            if (currweaponframe == KNIFEATTACK + 6)
                if (plr->currweaponanim == 8 && currweapontics == 8)
                    swingdasound();
            if (currweaponframe == KNIFEATTACK2 + 2)
                if ((plr->currweaponanim == 5 || plr->currweaponanim == 9) && currweapontics == 8)
                    swingdasound();
            break;
            case 2: // shortsword
            if (currweaponframe == GOBSWORDATTACK + 4)
                if (plr->currweaponanim == 4 && currweapontics == 10)
                    swingdasound();
            if (currweaponframe == GOBSWORDATTACK2 + 4)
                if (plr->currweaponanim == 4 && currweapontics == 10)
                    swingdasound();
            break;
            case 3: // morning
            if (currweaponframe == MORNINGSTAR + 5)
                if (plr->currweaponanim == 7 && currweapontics == 12)
                    swingdasound();
            if (currweaponframe == MORNINGATTACK2 + 3)
                if (plr->currweaponanim == 3 && currweapontics == 12)
                    swingdasound();
            break;
            case 4: // sword
            if (currweaponframe == SWORDATTACK + 7)
                if (plr->currweaponanim == 7 && currweapontics == 8)
                    swingdasound();
            if (currweaponframe == SWORDATTACK2 + 6)
                if (plr->currweaponanim == 6 && currweapontics == 8)
                    swingdasound();
            break;
            case 5: // battleaxe
            if (currweaponframe == BIGAXEATTACK + 7)
                if (plr->currweaponanim == 7 && currweapontics == 12)
                    swingdasound();
            if (currweaponframe == BIGAXEATTACK2 + 6)
                if (plr->currweaponanim == 6 && currweapontics == 12)
                    swingdasound();
            break;
            case 6: //bow
            if (currweaponframe == BOWWALK + 4)
                if (plr->currweaponanim == 4 && currweapontics == 6)
                    swingdasound();
            break;
            case 7: // pike
            if (currweaponframe == PIKEATTACK1 + 4)
                if (plr->currweaponanim == 8 && currweapontics == 10)
                    swingdasound();
            if (currweaponframe == PIKEATTACK2 + 4)
                if (plr->currweaponanim == 4 && currweapontics == 10)
                    swingdasound();
            break;
            case 8: // two handed sword
            if (currweaponframe == EXCALATTACK1 + 7)
                if (plr->currweaponanim == 7 && currweapontics == 8)
                    swingdasound();
            if (currweaponframe == EXCALATTACK2 + 5)
                if (plr->currweaponanim == 5 && currweapontics == 8)
                    swingdasound();
            break;
            case 9: // halberd
            if (currweaponframe == HALBERDATTACK1 + 3)
                if (plr->currweaponanim == 7 && currweapontics == 12)
                    swingdasound();
            if (currweaponframe == HALBERDATTACK2 + 3)
                if (plr->currweaponanim == 4 && currweapontics == 12)
                    swingdasound();
            break;
        }

        if (currweaponframe == RFIST + 5
            || currweaponframe == KNIFEATTACK + 6
            || currweaponframe == MORNINGSTAR + 5
            || currweaponframe == SWORDATTACK + 7
            || currweaponframe == BOWWALK + 5
            || currweaponframe == KNIFEATTACK2 + 2
            || currweaponframe == SWORDATTACK2 + 6
            || currweaponframe == MORNINGATTACK2 + 3
            || currweaponframe == HALBERDATTACK1 + 3
            || currweaponframe == HALBERDATTACK2 + 3
            || currweaponframe == BIGAXEATTACK + 7
            || currweaponframe == BIGAXEATTACK2 + 6
            || currweaponframe == PIKEATTACK1 + 4
            || currweaponframe == PIKEATTACK2 + 4
            || currweaponframe == EXCALATTACK1 + 7
            || currweaponframe == EXCALATTACK2 + 5
            || currweaponframe == GOBSWORDATTACK2 + 4
            || currweaponframe == GOBSWORDATTACK + 4)
        {
            swingdaweapon(plr);
        }

        currweapontics -= synctics;
        if (plr->helmettime > 0)
            currweapontics--;

        if ((currweaponframe == SWORDATTACK + 7
            || currweaponframe == SWORDATTACK2 + 7)
            && currweapontics < 0 && plr->shieldpoints <= 0)
        {
            if (krand() % 2 == 0)
            {
                /*
                if( plr->lvl <= 3 ) {
                    if( krand()%2 == 0 )
                        currweapon=3;
                    else
                        currweapon=4;
                    currweapontics=6;
                    currweaponanim=0;
                    currweaponfired=6;
                    currweaponflip=1;
                }
                else*/
                if (plr->lvl == 4 && plr->ammo[1] > 0)
                {
                    plr->currweapon = 1;
                    currweapontics = 6;
                    plr->currweaponanim = 0;
                    plr->currweaponfired = 6;
                    plr->currweaponflip = true;
                }
                else if (plr->lvl >= 5 && plr->ammo[3] > 0)
                {
                    plr->currweapon = 3;
                    currweapontics = 6;
                    plr->currweaponanim = 0;
                    plr->currweaponfired = 6;
                    plr->currweaponflip = true;
                }
            }
            /*
            else {
            //else if( krand()%100 > 50) {
                    if( krand()%2 == 0 )
                        currweapon=3;
                    else
                        currweapon=4;
                    currweapontics=6;
                    currweaponanim=0;
                    currweaponfired=6;
                    currweaponflip=1;
            }
            */
        }
        if (currweapontics < 0)
        {
            plr->currweaponanim++;
            if (plr->currweaponanim > 11)
            {
                //impact=0;
                plr->currweaponanim = 0;
                plr->currweaponfired = 0;
                oldmousestatus = 0; // NEW
                if (dahand > 0)
                    dahand = 0;
            }
            if (plr->currweaponattackstyle == 0)
            {
                currweapontics = weaponanimtics[plr->currweapon][plr->currweaponanim].daweapontics;
                currweaponframe = weaponanimtics[plr->currweapon][plr->currweaponanim].daweaponframe;
                dax = weaponanimtics[plr->currweapon][plr->currweaponanim].currx + 8;
                day = weaponanimtics[plr->currweapon][plr->currweaponanim].curry + 4;
            }
            else
            {
                currweapontics = weaponanimtics2[plr->currweapon][plr->currweaponanim].daweapontics;
                currweaponframe = weaponanimtics2[plr->currweapon][plr->currweaponanim].daweaponframe;
                dax = weaponanimtics2[plr->currweapon][plr->currweaponanim].currx + 8;
                day = weaponanimtics2[plr->currweapon][plr->currweaponanim].curry + 4;
            }
        }
        else
        {
            if (plr->currweaponattackstyle == 0)
            {
                //flip
                currweaponframe = weaponanimtics[plr->currweapon][plr->currweaponanim].daweaponframe;
                dax = weaponanimtics[plr->currweapon][plr->currweaponanim].currx;
                day = weaponanimtics[plr->currweapon][plr->currweaponanim].curry + 4;
            }
            else
            {
                //flip
                currweaponframe = weaponanimtics2[plr->currweapon][plr->currweaponanim].daweaponframe;
                dax = weaponanimtics2[plr->currweapon][plr->currweaponanim].currx;
                day = weaponanimtics2[plr->currweapon][plr->currweaponanim].curry + 4;
            }
        }
        if (plr->currweapon == 0 && currweaponframe != 0)
        {
            if (dahand == 1)
                overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            else if (dahand == 2)
            {
                dax = lefthandanimtics[0][plr->currweaponanim].currx;
                day = lefthandanimtics[0][plr->currweaponanim].curry + 8;
                overwritesprite(dax, day + 5, currweaponframe + 6, dashade, dabits, dapalnum);
            }
        }
        else
        {
            if (currweaponframe != 0)
            {
                if (plr->currweaponattackstyle == 0)
                    //flip
                    dax = weaponanimtics[plr->currweapon][plr->currweaponanim].currx;
                else
                    //flip
                    dax = weaponanimtics2[plr->currweapon][plr->currweaponanim].currx;

                overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            }
        }

        if (plr->currweapon == 0 && currweaponframe == 0)
        {
            dahand = 0;
            oldmousestatus = 0; // NEW
            plr->currweaponanim = 0;
            plr->currweaponfired = 0;
        }
        break;

        case 0: //walking

        currweapontics = weaponanimtics[plr->currweapon][0].daweapontics;

        if (plr->currweapon == 6 && plr->ammo[6] <= 0)
            currweaponframe = BOWREADYEND;
        else
            currweaponframe = weaponanimtics[plr->currweapon][0].daweaponframe;

        if (vel != 0)
        {
            snakex = Sin(lockclock << 4) >> 12;
            snakey = Sin((int)totalclock << 4) >> 12;

            if (plr->screensize <= 320)
            {
                if (currweaponframe == BOWREADYEND)
                {
                    day = readyanimtics[plr->currweapon][6].curry + snakey + 16;
                    dax = readyanimtics[plr->currweapon][6].currx + snakex + 16;
                }
                else
                {
                    day = weaponanimtics[plr->currweapon][0].curry + snakey + 16;
                    dax = weaponanimtics[plr->currweapon][0].currx + snakex + 16;
                }
            }
            else
            {
                if (currweaponframe == BOWREADYEND)
                {
                    day = readyanimtics[plr->currweapon][6].curry + snakey + 8;
                    dax = readyanimtics[plr->currweapon][6].currx + snakex + 8;
                }
                else
                {
                    day = weaponanimtics[plr->currweapon][0].curry + snakey + 8;
                    dax = weaponanimtics[plr->currweapon][0].currx + snakex + 8;
                }
            }
        }
        else
        {
            if (currweaponframe == BOWREADYEND)
            {
                day = readyanimtics[plr->currweapon][6].curry + 3;
                dax = readyanimtics[plr->currweapon][6].currx + 3;
            }
            else
            {
                dax = weaponanimtics[plr->currweapon][0].currx + 3;
                day = weaponanimtics[plr->currweapon][0].curry + 3;
            }
        }
        if (plr->currweapon == 0 && currweaponframe != 0)
        {
            overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            overwritesprite(0, day + 8, currweaponframe + 6, dashade, dabits, dapalnum);
        }
        else
        {
            if (currweaponframe != 0)
            {
                overwritesprite(dax + snakex, day, currweaponframe, dashade, dabits, dapalnum);
            }
        }
        break;
        case 2: // unready
        if (plr->currweapon == 1)
            weapondrop += synctics << 1;
        else
            weapondrop += synctics;
        if (weapondrop > weapondropgoal)
        {
            plr->currweaponfired = 3;
            weaponraise = 40;
            plr->currweapon = plr->selectedgun;
            weaponuseless = 0;
            //hasshot=0;//just in case of bugg
                        //make hasshot=1;
        }

        currweapontics = weaponanimtics[plr->currweapon][0].daweapontics;

        if (plr->currweapon == 6 && plr->ammo[6] <= 0)
            currweaponframe = BOWREADYEND;
        else
            currweaponframe = weaponanimtics[plr->currweapon][0].daweaponframe;

        if (currweaponframe == BOWREADYEND)
        {
            day = readyanimtics[plr->currweapon][6].curry + (weapondrop);
            dax = readyanimtics[plr->currweapon][6].currx;
        }
        else
        {
            dax = weaponanimtics[plr->currweapon][0].currx;
            day = weaponanimtics[plr->currweapon][0].curry + (weapondrop);
        }

        if (plr->currweapon == 0 && currweaponframe != 0)
        {
            overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            overwritesprite(0, day, currweaponframe + 6, dashade, dabits, dapalnum);
        }

        else
        {
            if (currweaponframe != 0)
            {
                dax = weaponanimtics[plr->currweapon][0].currx;
                overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            }
        }
        break;
        case 3: // ready
        currweapontics -= synctics;
        if (currweapontics < 0)
        {
            plr->currweaponanim++;
            if (plr->currweaponanim == 12)
            {
                plr->currweaponanim = 0;
                plr->currweaponfired = 0;

                currweaponframe = readyanimtics[plr->currweapon][11].daweaponframe;
                dax = readyanimtics[plr->currweapon][11].currx;
                day = readyanimtics[plr->currweapon][11].curry + 8;

                if (plr->currweapon == 0 && currweaponframe != 0)
                {
                    overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
                    overwritesprite(0, day, currweaponframe + 6, dashade, dabits, dapalnum);
                }
                else
                {
                    if (currweaponframe != 0)
                    {
                        overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
                    }
                }
                break;
                if (dahand > 0)
                    dahand = 0;
            }
            currweapontics = readyanimtics[plr->currweapon][plr->currweaponanim].daweapontics;
            currweaponframe = readyanimtics[plr->currweapon][plr->currweaponanim].daweaponframe;
            dax = readyanimtics[plr->currweapon][plr->currweaponanim].currx;
            day = readyanimtics[plr->currweapon][plr->currweaponanim].curry + 8;
        }
        else
        {
            currweaponframe = readyanimtics[plr->currweapon][plr->currweaponanim].daweaponframe;
            dax = readyanimtics[plr->currweapon][plr->currweaponanim].currx;
            day = readyanimtics[plr->currweapon][plr->currweaponanim].curry + 8;
        }
        if (plr->currweapon == 0 && currweaponframe != 0)
        {
            overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            overwritesprite(0, day, currweaponframe + 6, dashade, dabits, dapalnum);
        }
        else
        {
            if (currweaponframe != 0)
            {
                overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
            }
        }
        break;

        case 5: //cock
        currweapontics -= synctics;

        if (currweapontics < 0)
        {
            plr->currweaponanim++;
            if (plr->currweaponanim == 4)
            {
                plr->currweaponanim = 0;
                plr->currweaponfired = 0;

                currweaponframe = cockanimtics[3].daweaponframe;
                dax = cockanimtics[3].currx + 3;
                day = cockanimtics[3].curry + 3;

                if (currweaponframe != 0)
                {
                    overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
                }
                break;
            }
            currweapontics = cockanimtics[plr->currweaponanim].daweapontics;
            currweaponframe = cockanimtics[plr->currweaponanim].daweaponframe;
            dax = cockanimtics[plr->currweaponanim].currx;
            day = cockanimtics[plr->currweaponanim].curry + 8;
        }
        else
        {
            currweaponframe = cockanimtics[plr->currweaponanim].daweaponframe;
            dax = cockanimtics[plr->currweaponanim].currx;
            day = cockanimtics[plr->currweaponanim].curry + 8;
        }
        if (currweaponframe != 0)
        {
            overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
        }

        break;

        case 4: // throw the orb

        if (currweaponframe == 0)
        {
            castaorb(plr);
        }

        currweapontics -= synctics;

        if (currweapontics < 0)
        {
            plr->currweaponanim++;
            if (plr->currweaponanim > 12)
            {
                plr->currweaponanim = 0;
                plr->currweaponfired = 0;
                plr->orbshot = false;

                currweaponframe = throwanimtics[plr->currentorb][plr->currweaponanim].daweaponframe;
                dax = throwanimtics[plr->currentorb][plr->currweaponanim].currx;
                day = throwanimtics[plr->currentorb][plr->currweaponanim].curry + 8;

                if (plr->currweapon == 0 && currweaponframe != 0)
                {
                    overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
                    //overwritesprite(0,day,currweaponframe+6,dashade,dabits,0);
                }
                else if (currweaponframe != 0)
                {
                    overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
                }

                break;
            }
            currweapontics = throwanimtics[plr->currentorb][plr->currweaponanim].daweapontics;
            currweaponframe = throwanimtics[plr->currentorb][plr->currweaponanim].daweaponframe;
            dax = throwanimtics[plr->currentorb][plr->currweaponanim].currx;
            day = throwanimtics[plr->currentorb][plr->currweaponanim].curry + 8;
        }
        else
        {
            currweaponframe = throwanimtics[plr->currentorb][plr->currweaponanim].daweaponframe;
            dax = throwanimtics[plr->currentorb][plr->currweaponanim].currx;
            day = throwanimtics[plr->currentorb][plr->currweaponanim].curry + 8;
        }
        if (currweaponframe != 0)
        {
            overwritesprite(dax, day, currweaponframe, dashade, dabits, dapalnum);
        }
        break;
    }

    // shield stuff
    if (plr->shieldpoints > 0
        && (plr->currweaponfired == 0 || plr->currweaponfired == 1)
        && plr->selectedgun > 0
        && plr->selectedgun < 5)
    {

        if (plr->currweaponfired == 1)
        {
            snakex = Sin(lockclock << 4) >> 12;
            snakey = Sin((int)totalclock << 4) >> 12;
        }

        if (plr->shieldpoints > 75)
        {
            overwritesprite(-40 + snakex, 70 + snakey, GRONSHIELD, dashade, dabits, dapalnum);
        }
        else if (plr->shieldpoints > 50 && plr->shieldpoints < 76)
        {
            overwritesprite(-40 + snakex, 70 + snakey, GRONSHIELD + 1, dashade, dabits, dapalnum);
        }
        else if (plr->shieldpoints > 25 && plr->shieldpoints < 51)
        {
            overwritesprite(-40 + snakex, 70 + snakey, GRONSHIELD + 2, dashade, dabits, dapalnum);
        }
        else
        {
            overwritesprite(-40 + snakex, 70 + snakey, GRONSHIELD + 3, dashade, dabits, dapalnum);
        }
    }

    //
    // spellbook
    //
    if (spellbookflip == 1)
    {
        if (svga == 0)
        {
            if (plr->screensize <= 320)
            {
                //overwritesprite(122, 154, SPELLBOOKBACK, 0, 0, 0);
                itemtoscreen(122, 154, SPELLBOOKBACK, 0, 0);
            }
        }
        if (svga == 1)
        {
            if (plr->screensize == 320)
            {
                itemtoscreen(120 << 1, 372, SSPELLBACK, 0, 0);
                //overwritesprite(120 << 1, 372, SSPELLBACK, 0, 0, 0);
            }
        }

        spellbooktics -= synctics;

        if (spellbooktics < 0)
        {
            spellbook++;
            if (spellbook > 8)
                spellbook = 8;

            if (spellbook == 8)
            {
                spellbooktics = spellbookanim[plr->currentorb][8].daweapontics;
                if (svga == 1)
                {
                    spellbookframe = sspellbookanim[plr->currentorb][8].daweaponframe;
                    dax = sspellbookanim[plr->currentorb][8].currx;
                    day = sspellbookanim[plr->currentorb][8].curry;
                }
                else
                {
                    spellbookframe = spellbookanim[plr->currentorb][8].daweaponframe;
                    dax = spellbookanim[plr->currentorb][8].currx;
                    day = spellbookanim[plr->currentorb][8].curry;
                }
                if (svga == 1)
                {
                    itemtoscreen(dax << 1, day, spellbookframe, 0, 0);
                    //overwritesprite(dax << 1, day, spellbookframe, 0, 0, 0);
                }
                else
                {
                    itemtoscreen(dax, day, spellbookframe, 0, 0);
                    //overwritesprite(dax, day, spellbookframe, 0, 0, 0);
                }

                spellbookflip = 0;
                return;
            }
            else
            {
                spellbooktics = spellbookanim[plr->currentorb][spellbook].daweapontics;

                if (svga == 1)
                {
                    spellbookframe = sspellbookanim[plr->currentorb][spellbook].daweaponframe;
                    dax = sspellbookanim[plr->currentorb][spellbook].currx;
                    day = sspellbookanim[plr->currentorb][spellbook].curry;

                }
                else
                {
                    spellbookframe = spellbookanim[plr->currentorb][spellbook].daweaponframe;
                    dax = spellbookanim[plr->currentorb][spellbook].currx;
                    day = spellbookanim[plr->currentorb][spellbook].curry;
                }
                if (svga == 1)
                {
                    itemtoscreen(dax << 1, day, spellbookframe, 0, 0);
                    //overwritesprite(dax << 1, day, spellbookframe, 0, 0, 0);
                }
                else
                {
                    itemtoscreen(dax, day, spellbookframe, 0, 0);
                    //overwritesprite(dax, day, spellbookframe, 0, 0, 0);
                }
            }
        }
        else
        {

            if (svga == 1)
            {
                spellbookframe = sspellbookanim[plr->currentorb][spellbook].daweaponframe;
                dax = sspellbookanim[plr->currentorb][spellbook].currx;
                day = sspellbookanim[plr->currentorb][spellbook].curry;
            }
            else
            {
                spellbookframe = spellbookanim[plr->currentorb][spellbook].daweaponframe;
                dax = spellbookanim[plr->currentorb][spellbook].currx;
                day = spellbookanim[plr->currentorb][spellbook].curry;
            }
            if (svga == 1)
            {
                itemtoscreen(dax << 1, day, spellbookframe, 0, 0);
                //overwritesprite(dax << 1, day, spellbookframe, 0, 0, 0);
            }
            else
            {
                itemtoscreen(dax, day, spellbookframe, 0, 0);
                //overwritesprite(dax, day, spellbookframe, 0, 0, 0);
            }
        }
    }
}

void castaorb(Player* plr)
{
    short daang;

    switch (plr->currentorb)
    {
        case 0: // SCARE
        plr->shadowtime = (plr->lvl * 120) << 2;
        break;
        case 1: // NIGHTVISION
        plr->nightglowtime = 3600 + (plr->lvl * 120);
        break;
        case 2: // FREEZE
        daang = plr->ang;
        shootgun(plr, daang, 6);
        playsound_loc(S_SPELL1, plr->x, plr->y);
        break;
        case 3: // MAGIC ARROW
        daang = (plr->ang - 36) & kAngleMask;
        for (int k = 0; k < 10; k++)
        {
            daang = (daang + (k << 1)) & kAngleMask;
            shootgun(plr, daang, 2);
        }
        playsound_loc(S_SPELL1, plr->x, plr->y);
        break;
        case 4: // OPEN DOORS
        daang = plr->ang;
        shootgun(plr, daang, 7);
        playsound_loc(S_SPELL1, plr->x, plr->y);
        break;
        case 5: // FLY
        plr->orbactive[plr->currentorb] = 3600 + (plr->lvl * 120);
        playsound_loc(S_SPELL1, plr->x, plr->y);
        break;
        case 6: // FIREBALL
        daang = plr->ang;
        shootgun(plr, daang, 3);
        playsound_loc(S_SPELL1, plr->x, plr->y);
        break;
        case 7: // NUKE
        daang = plr->ang;
        shootgun(plr, daang, 4);
        playsound_loc(S_SPELL1, plr->x, plr->y);
        break;
    }
}

void chunksofmeat(Player* plr, short hitsprite, int hitx, int hity, int hitz, short hitsect, short daang)
{
    short j;
    short k;
    short zgore;
    int chunk = REDCHUNKSTART;
    int newchunk;

    if (goreon == 0)
        return;

    if (sprite[hitsprite].picnum == JUDY
        || sprite[hitsprite].picnum == JUDYATTACK1
        || sprite[hitsprite].picnum == JUDYATTACK2)
        return;

    switch (plr->selectedgun)
    {
        case 1:
        case 2:
        case 6:
        zgore = 1;
        break;
        case 3:
        case 4:
        case 7:
        zgore = 2;
        break;
        case 5:
        case 8:
        case 9:
        zgore = 3;
        break;
    }

    if (sprite[hitsprite].picnum == RAT)
        zgore = 1;

    if (sprite[hitsprite].picnum == WILLOW
        || sprite[hitsprite].picnum == WILLOWEXPLO
        || sprite[hitsprite].picnum == WILLOWEXPLO + 1
        || sprite[hitsprite].picnum == WILLOWEXPLO + 2
        || sprite[hitsprite].picnum == GUARDIAN
        || sprite[hitsprite].picnum == GUARDIANATTACK)
        return;

    if (sprite[hitsprite].picnum == SKELETON
        || sprite[hitsprite].picnum == SKELETONATTACK
        || sprite[hitsprite].picnum == SKELETONDIE)
    {
        playsound_loc(S_SKELHIT1 + (rand() % 2), sprite[hitsprite].x, sprite[hitsprite].y);
    }
    else
    {
        if (rand() % 100 > 60)
            playsound_loc(S_GORE1 + (rand() % 4), sprite[hitsprite].x, sprite[hitsprite].y);
    }

    if ((hitsprite >= 0) && (sprite[hitsprite].statnum < MAXSTATUS))
    {
        for (k = 0; k < zgore; k++)
        {
            newchunk = 0;
            j = insertsprite(hitsect, CHUNKOMEAT);
            sprite[j].x = hitx;
            sprite[j].y = hity;
            sprite[j].z = hitz;
            sprite[j].cstat = 0;
            if (rand() % 100 > 50)
            {
                switch (sprite[hitsprite].picnum)
                {
                    case GRONHAL:
                    case GRONHALATTACK:
                    case GRONHALPAIN:
                    case GRONMU:
                    case GRONMUATTACK:
                    case GRONMUPAIN:
                    case GRONSW:
                    case GRONSWATTACK:
                    case GRONSWPAIN:
                    chunk = REDCHUNKSTART + (rand() % 8);
                    break;
                    case KOBOLD:
                    case KOBOLDATTACK:
                    if (sprite[hitsprite].pal == 0)
                        chunk = BROWNCHUNKSTART + (rand() % 8);
                    if (sprite[hitsprite].pal == 4)
                        chunk = GREENCHUNKSTART + (rand() % 8);
                    if (sprite[hitsprite].pal == 7)
                        chunk = REDCHUNKSTART + (rand() % 8);
                    break;
                    case DRAGON:
                    case DRAGONATTACK2:
                    case DRAGONATTACK:
                    chunk = GREENCHUNKSTART + (rand() % 8);
                    break;
                    case DEVILSTAND:
                    case DEVIL:
                    case DEVILATTACK:
                    chunk = REDCHUNKSTART + (rand() % 8);
                    break;
                    case FREDSTAND:
                    case FRED:
                    case FREDATTACK:
                    case FREDPAIN:
                    chunk = BROWNCHUNKSTART + (rand() % 8);
                    break;
                    case GOBLINSTAND:
                    case GOBLIN:
                    case GOBLINCHILL:
                    case GOBLINATTACK:
                    case GOBLINPAIN:
                    if (sprite[hitsprite].pal == 0)
                        chunk = GREENCHUNKSTART + (rand() % 8);
                    if (sprite[hitsprite].pal == 4)
                        chunk = BROWNCHUNKSTART + (rand() % 8);
                    if (sprite[hitsprite].pal == 5)
                        chunk = TANCHUNKSTART + (rand() % 8);
                    break;
                    case MINOTAUR:
                    case MINOTAURATTACK:
                    case MINOTAURPAIN:
                    chunk = TANCHUNKSTART + (rand() % 8);
                    break;
                    case SPIDER:
                    chunk = GREYCHUNKSTART + (rand() % 8);
                    break;
                    case SKULLY:
                    case SKULLYATTACK:
                    case FATWITCH:
                    case FATWITCHATTACK:
                    case JUDYSIT:
                    case JUDYSTAND:
                    case JUDY:
                    case JUDYATTACK1:
                    case JUDYATTACK2:
                    chunk = REDCHUNKSTART + (rand() % 8);
                    break;
                }
            }
            else
            {
                newchunk = 1;
                chunk = NEWCHUNK + (rand() % 9);
            }
            if (sprite[hitsprite].picnum == SKELETON
                || sprite[hitsprite].picnum == SKELETONATTACK
                || sprite[hitsprite].picnum == SKELETONDIE)
                chunk = BONECHUNK1 + (rand() % 9);

            sprite[j].picnum = chunk;
            sprite[j].shade = -16;
            sprite[j].xrepeat = 64;
            sprite[j].yrepeat = 64;
            sprite[j].clipdist = 16;
            sprite[j].ang = ((rand() & 1023) - 1024) & kAngleMask;
            sprite[j].xvel = ((rand() & 1023) - 512);
            sprite[j].yvel = ((rand() & 1023) - 512);
            sprite[j].zvel = ((rand() & 1023) - 512);

            if (newchunk == 1)
                sprite[j].zvel <<= 1;

            sprite[j].owner = 4096;
            sprite[j].lotag = 512;
            sprite[j].hitag = 0;
            sprite[j].pal = 0;

            movesprite(j, (Cos(sprite[j].ang) * synctics) << 3, (Sin(sprite[j].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);
        }
    }
}

void swingdapunch()
{
    Player* plr = &player[pyrn];

    switch (plr->currweapon)
    {
        case 0: // hands
            SND_Sound(S_SOCK4);
            SND_Sound(S_PLRPAIN1 + (rand() % 2));
            sethealth(-1);
            startredflash(10);
        break;
        case 1: // knife
        case 2: // mace
        case 4: // sword
        case 5:
        case 6:
        case 7:
        case 8:
            SND_PlaySound(S_WALLHIT1, 0, 0, 0, 0);
        break;
        case 3: //arrow
        break;
    }
}

void shootgun(Player *plr, short daang, char guntype)
{
    short daang2, k;
    int32_t i, j, daz2;

    int32_t hitsprite = -1;
    int32_t hitsect = -1;
    int32_t hitwall = -1;
    int32_t hitx = -1;
    int32_t hity = -1;
    int32_t hitz = -1;
    vec3_t pos;
    hitdata_t hitinfo;

    switch (guntype)
    {
        case 0:
        {
            daang2 = daang + 2048;
            daz2 = (100 - plr->horiz) * 2000;

            pos.x = plr->x;
            pos.y = plr->y;
            pos.z = plr->z;

            hitscan(&pos, plr->sector, // Start position
                Cos(daang2 + 2048), // X vector of 3D ang
                Sin(daang2 + 2048), // X vector of 3D ang
                daz2,               // Z vector of 3D ang
                &hitinfo, CLIPMASK1); // CHECKME - is CLIPMASK1 correct for vesion of this function that hadn't got cliptype param?

            hitsprite = hitinfo.sprite;
            hitsect = hitinfo.sect;
            hitwall = hitinfo.wall;
            hitx = hitinfo.pos.x;
            hity = hitinfo.pos.y;
            hitz = hitinfo.pos.z;

            if (hitsprite >= 0)
                madeahit = 1;

            if (hitwall >= 0)
            {
                if ((labs(plr->x - hitx) + labs(plr->y - hity) < 512) && (labs((plr->z >> 8) - ((hitz >> 8) - (64))) <= (512 >> 3)))
                {
                    madeahit = 1;

                    switch (plr->currweapon)
                    {
                        case 0: // fist
                        if (currweaponframe == RFIST + 5)
                            if (plr->currweaponanim == 5 && currweapontics == 10)
                                swingdapunch();
                        break;
                        case 1: // knife
                        if (currweaponframe == KNIFEATTACK + 6)
                            if (plr->currweaponanim == 8 && currweapontics == 8)
                                swingdapunch();
                        if (currweaponframe == KNIFEATTACK2 + 2)
                            if ((plr->currweaponanim == 5 || plr->currweaponanim == 9) && currweapontics == 8)
                                swingdapunch();
                        break;
                        case 2: // short sword
                        if (currweaponframe == GOBSWORDATTACK + 4)
                            if (plr->currweaponanim == 4 && currweapontics == 10)
                                swingdapunch();
                        if (currweaponframe == GOBSWORDATTACK + 4)
                            if (plr->currweaponanim == 4 && currweapontics == 10)
                                swingdapunch();
                        break;
                        case 3: // morning
                        if (currweaponframe == MORNINGSTAR + 5)
                            if (plr->currweaponanim == 7 && currweapontics == 12)
                                swingdapunch();
                        if (currweaponframe == MORNINGATTACK2 + 3)
                            if (plr->currweaponanim == 3 && currweapontics == 12)
                                swingdapunch();
                        break;
                        case 4: // sword
                        if (currweaponframe == SWORDATTACK + 7)
                            if (plr->currweaponanim == 7 && currweapontics == 8)
                            {
                                swingdapunch();
                                madenoise(2, plr->x, plr->y, plr->z);
                            }
                        if (currweaponframe == SWORDATTACK2 + 6)
                            if (plr->currweaponanim == 6 && currweapontics == 8)
                            {
                                swingdapunch();
                                madenoise(2, plr->x, plr->y, plr->z);
                            }
                        break;
                        case 5: // battleaxe
                        if (currweaponframe == BIGAXEATTACK + 7)
                            if (plr->currweaponanim == 7 && currweapontics == 12)
                                swingdapunch();
                        if (currweaponframe == BIGAXEATTACK2 + 6)
                            if (plr->currweaponanim == 6 && currweapontics == 12)
                                swingdapunch();
                        break;
                        case 6: // bow
                        if (currweaponframe == BOWWALK + 4)
                            if (plr->currweaponanim == 4 && currweapontics == 6)
                                swingdapunch();
                        break;
                        case 7: // pike
                        if (currweaponframe == PIKEATTACK1 + 4)
                            if (plr->currweaponanim == 8 && currweapontics == 10)
                                swingdapunch();
                        if (currweaponframe == PIKEATTACK2 + 4)
                            if (plr->currweaponanim == 4 && currweapontics == 10)
                                swingdapunch();
                        break;
                        case 8: // two handed sword
                        if (currweaponframe == EXCALATTACK1 + 7)
                            if (plr->currweaponanim == 7 && currweapontics == 8)
                                swingdapunch();
                        if (currweaponframe == EXCALATTACK2 + 5)
                            if (plr->currweaponanim == 5 && currweapontics == 8)
                                swingdapunch();
                        break;
                        case 9: // halberd
                        if (currweaponframe == HALBERDATTACK1 + 3)
                            if (plr->currweaponanim == 6 && currweapontics == 12)
                                swingdapunch();
                        if (currweaponframe == HALBERDATTACK2 + 3)
                            if (plr->currweaponanim == 4 && currweapontics == 12)
                                swingdapunch();
                        break;
                    }
                }
            }
            if (checkweapondist(hitsprite, plr->x, plr->y, plr->z, guntype))
            {
                switch (sprite[hitsprite].picnum)
                {
                    case BARREL:
                    case VASEA:
                    case VASEB:
                    case VASEC:
                    newstatus(hitsprite, BROKENVASE);
                    break;
                    case GRONHAL:
                    case GRONHALATTACK:
                    case GRONHALPAIN:
                    case GRONMU:
                    case GRONMUATTACK:
                    case GRONMUPAIN:
                    case GRONSW:
                    case GRONSWATTACK:
                    case GRONSWPAIN:
                    case KOBOLD:
                    case KOBOLDATTACK:
                    case DRAGON:
                    case DRAGONATTACK2:
                    case DRAGONATTACK:
                    case DEVILSTAND:
                    case DEVIL:
                    case DEVILATTACK:
                    case FREDSTAND:
                    case FRED:
                    case FREDATTACK:
                    case FREDPAIN:
                    case SKELETON:
                    case SKELETONATTACK:
                    case GOBLINSTAND:
                    case GOBLIN:
                    case GOBLINCHILL:
                    //case GOBLINSURPRISE:
                    case GOBLINATTACK:
                    case GOBLINPAIN:
                    case MINOTAUR:
                    case MINOTAURATTACK:
                    case MINOTAURPAIN:
                    case SPIDER:
                    case SKULLY:
                    case SKULLYATTACK:
                    case FATWITCH:
                    case FATWITCHATTACK:
                    case FISH:
                    case RAT:
                    case GUARDIAN:
                    case GUARDIANATTACK:
                    case WILLOW:
                    case JUDYSIT:
                    case JUDYSTAND:
                    case JUDY:
                    case JUDYATTACK1:
                    case JUDYATTACK2:

                    if (netgame) {
                        netshootgun(hitsprite, plr->currweapon);
                    }

                    if (plr->invisibletime > 0)
                    {
                        if ((krand() & 32) > 15) {
                            plr->invisibletime = -1;
                        }
                    }

                    switch (plr->selectedgun)
                    {
                        case 0: // fist
                        k = (krand() & 5) + 1;
                        break;
                        case 1: // dagger
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 5) + 10;
                        else
                            k = (krand() & 3) + 5;
                        break;
                        case 2: // short sword
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 10) + 10;
                        else
                            k = (krand() & 6) + 10;
                        break;
                        case 3: // morning star
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 8) + 10;
                        else
                            k = (krand() & 8) + 15;
                        break;
                        case 4: // broad sword
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 5) + 20;
                        else
                            k = (krand() & 5) + 15;
                        break;
                        case 5: // battle axe
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 5) + 25;
                        else
                            k = (krand() & 5) + 20;
                        break;
                        case 6: // bow
                            k = (krand() & 15) + 5;
                        break;
                        case 7: // pike axe
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 15) + 10;
                        else
                            k = (krand() & 15) + 5;
                        break;
                        case 8: // two handed sword
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 15) + 45;
                        else
                            k = (krand() & 15) + 40;
                        break;
                        case 9: // halberd
                        if (plr->currweaponattackstyle == 0)
                            k = (krand() & 15) + 25;
                        else
                            k = (krand() & 15) + 15;
                        break;
                    }

                    k += plr->lvl;

                    if (plr->vampiretime > 0)
                    {
                        if (plr->health <= plr->maxhealth)
                            sethealth((rand() % 10) + 1);
                    }
                    if (plr->helmettime > 0)
                    {
                        k <<= 1;
                    }

                    if (plr->strongtime > 0)
                    {
                        k += k >> 1;

                        switch (plr->currweapon)
                        {
                            case 0: // fist
                            if (currweaponframe == RFIST + 5)
                                if (plr->currweaponanim == 5 && currweapontics == 10)
                                    swingdacrunch();
                            break;
                            case 1: // knife
                            if (currweaponframe == KNIFEATTACK + 6)
                                if (plr->currweaponanim == 8 && currweapontics == 8)
                                    swingdacrunch();
                            if (currweaponframe == KNIFEATTACK2 + 2)
                                if (plr->currweaponanim == 5 || plr->currweaponanim == 9 && currweapontics == 8)
                                    swingdacrunch();
                            break;
                            case 2: // short sword
                            if (currweaponframe == GOBSWORDATTACK + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 10)
                                    swingdacrunch();
                            if (currweaponframe == GOBSWORDATTACK2 + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 10)
                                    swingdacrunch();
                            break;
                            case 3: // morning
                            if (currweaponframe == MORNINGSTAR + 5)
                                if (plr->currweaponanim == 7 && currweapontics == 12)
                                    swingdacrunch();
                            if (currweaponframe == MORNINGATTACK2 + 3)
                                if (plr->currweaponanim == 3 && currweapontics == 12)
                                    swingdacrunch();
                            break;
                            case 4: // sword
                            if (currweaponframe == SWORDATTACK + 7)
                                if (plr->currweaponanim == 7 && currweapontics == 8)
                                    swingdacrunch();
                            if (currweaponframe == SWORDATTACK2 + 6)
                                if (plr->currweaponanim == 6 && currweapontics == 8)
                                    swingdacrunch();
                            break;
                            case 5: // battleaxe
                            if (currweaponframe == BIGAXEATTACK + 7)
                                if (plr->currweaponanim == 7 && currweapontics == 12)
                                    swingdacrunch();
                            if (currweaponframe == BIGAXEATTACK2 + 6)
                                if (plr->currweaponanim == 6 && currweapontics == 12)
                                    swingdacrunch();
                            break;
                            case 6: // bow
                            if (currweaponframe == BOWWALK + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 6)
                                    swingdacrunch();
                            break;
                            case 7: // pike
                            if (currweaponframe == PIKEATTACK1 + 4)
                                if (plr->currweaponanim == 8 && currweapontics == 10)
                                    swingdacrunch();
                            if (currweaponframe == PIKEATTACK2 + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 10)
                                    swingdacrunch();
                            break;
                            case 8: // two handed sword
                            if (currweaponframe == EXCALATTACK1 + 7)
                                if (plr->currweaponanim == 7 && currweapontics == 8)
                                    swingdacrunch();
                            if (currweaponframe == EXCALATTACK2 + 5)
                                if (plr->currweaponanim == 5 && currweapontics == 8)
                                    swingdacrunch();
                            break;
                            case 9: // halberd
                            if (currweaponframe == HALBERDATTACK1 + 3)
                                if (plr->currweaponanim == 6 && currweapontics == 12)
                                    swingdacrunch();
                            if (currweaponframe == HALBERDATTACK2 + 3)
                                if (plr->currweaponanim == 4 && currweapontics == 12)
                                    swingdacrunch();
                            break;
                        }

                        sprite[hitsprite].hitag -= (k << 1);

                        if (plr->currweapon != 0)
                        {
                            //JSA GORE1 you have strong time
                            if (rand() % 100 > 50)
                            {
                                if (sprite[hitsprite].picnum == SKELETON
                                    || sprite[hitsprite].picnum == SKELETONATTACK
                                    || sprite[hitsprite].picnum == SKELETONDIE)
                                    playsound_loc(S_SKELHIT1 + (rand() % 2), sprite[hitsprite].x, sprite[hitsprite].y);
                            }

                            switch (plr->currweapon)
                            {
                                case 0: // fist
                                break;
                                case 1: // knife
                                if (currweaponframe == KNIFEATTACK + 6)
                                {
                                    if (plr->currweaponanim == 8 && currweapontics == 8) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                if (currweaponframe == KNIFEATTACK2 + 2)
                                {
                                    if (plr->currweaponanim == 5 || plr->currweaponanim == 9 && currweapontics == 8) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                break;
                                case 2: // short sword
                                if (currweaponframe == GOBSWORDATTACK + 4)
                                {
                                    if (plr->currweaponanim == 4 && currweapontics == 10) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                if (currweaponframe == GOBSWORDATTACK2 + 4)
                                {
                                    if (plr->currweaponanim == 4 && currweapontics == 10) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                break;
                                case 3: // morning
                                if (currweaponframe == MORNINGSTAR + 5)
                                {
                                    if (plr->currweaponanim == 7 && currweapontics == 12) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                if (currweaponframe == MORNINGATTACK2 + 3)
                                {
                                    if (plr->currweaponanim == 3 && currweapontics == 12) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                break;
                                case 4: // sword
                                if (currweaponframe == SWORDATTACK + 7)
                                {
                                    if (plr->currweaponanim == 7 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                }
                                if (currweaponframe == SWORDATTACK2 + 6)
                                {
                                    if (plr->currweaponanim == 6 && currweapontics == 8)
                                        break;
                                }
                                chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                fallthrough__;
                                case 5: // battleaxe
                                {
                                    if (currweaponframe == BIGAXEATTACK + 7)
                                        if (plr->currweaponanim == 7 && currweapontics == 12)
                                            chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                }
                                if (currweaponframe == BIGAXEATTACK2 + 6)
                                {
                                    if (plr->currweaponanim == 6 && currweapontics == 12)
                                        break;
                                }
                                chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                fallthrough__;
                                case 6: // bow
                                if (currweaponframe == BOWWALK + 4)
                                {
                                    if (plr->currweaponanim == 4 && currweapontics == 6) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                break;
                                case 7: // pike
                                if (currweaponframe == PIKEATTACK1 + 4)
                                {
                                    if (plr->currweaponanim == 8 && currweapontics == 10) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                if (currweaponframe == PIKEATTACK2 + 4)
                                {
                                    if (plr->currweaponanim == 4 && currweapontics == 10) {
                                        break;
                                    }
                                }
                                chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                fallthrough__;
                                case 8: // two handed sword
                                if (currweaponframe == EXCALATTACK1 + 7)
                                {
                                    if (plr->currweaponanim == 7 && currweapontics == 8) {
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                    }
                                }
                                if (currweaponframe == EXCALATTACK2 + 5)
                                {
                                    if (plr->currweaponanim == 5 && currweapontics == 8) {
                                        break;
                                    }
                                }
                                chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                fallthrough__;
                                case 9: // halberd
                                if (currweaponframe == HALBERDATTACK1 + 3)
                                    if (plr->currweaponanim == 6 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == HALBERDATTACK2 + 3)
                                    if (plr->currweaponanim == 4 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                            }
                            //ENDOFHERE
                            //chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                        }
                    }
                    else
                    {
                        switch (plr->currweapon)
                        {
                            case 0: // fist
                            if (currweaponframe == RFIST + 5)
                                if (plr->currweaponanim == 5 && currweapontics == 10)
                                    swingdacrunch();
                            break;
                            case 1: // knife
                            if (currweaponframe == KNIFEATTACK + 6)
                                if (plr->currweaponanim == 8 && currweapontics == 8)
                                    swingdacrunch();
                            if (currweaponframe == KNIFEATTACK2 + 2)
                                if (plr->currweaponanim == 5 || plr->currweaponanim == 9 && currweapontics == 8)
                                    swingdacrunch();
                            break;
                            case 2: // SHORT SWORD
                            if (currweaponframe == GOBSWORDATTACK + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 10)
                                    swingdacrunch();
                            if (currweaponframe == GOBSWORDATTACK2 + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 10)
                                    swingdacrunch();
                            break;
                            case 3: // morning
                            if (currweaponframe == MORNINGSTAR + 5)
                                if (plr->currweaponanim == 7 && currweapontics == 12)
                                    swingdacrunch();
                            if (currweaponframe == MORNINGATTACK2 + 3)
                                if (plr->currweaponanim == 3 && currweapontics == 12)
                                    swingdacrunch();
                            break;
                            case 4: // sword
                            if (currweaponframe == SWORDATTACK + 7)
                                if (plr->currweaponanim == 7 && currweapontics == 8)
                                    swingdacrunch();
                            if (currweaponframe == SWORDATTACK2 + 6)
                                if (plr->currweaponanim == 6 && currweapontics == 8)
                                    swingdacrunch();
                            break;
                            case 5: // battleaxe
                            if (currweaponframe == BIGAXEATTACK + 7)
                                if (plr->currweaponanim == 7 && currweapontics == 12)
                                    swingdacrunch();
                            if (currweaponframe == BIGAXEATTACK2 + 6)
                                if (plr->currweaponanim == 6 && currweapontics == 12)
                                    swingdacrunch();
                            break;
                            case 6: // bow
                            if (currweaponframe == BOWWALK + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 6)
                                    swingdacrunch();
                            break;

                            case 7: // pike
                            if (currweaponframe == PIKEATTACK1 + 4)
                                if (plr->currweaponanim == 8 && currweapontics == 10)
                                    swingdacrunch();
                            if (currweaponframe == PIKEATTACK2 + 4)
                                if (plr->currweaponanim == 4 && currweapontics == 10)
                                    swingdacrunch();
                            break;
                            case 8: // two handed sword
                            if (currweaponframe == EXCALATTACK1 + 7)
                                if (plr->currweaponanim == 7 && currweapontics == 8)
                                    swingdacrunch();
                            if (currweaponframe == EXCALATTACK2 + 5)
                                if (plr->currweaponanim == 5 && currweapontics == 8)
                                    swingdacrunch();
                            break;
                            case 9: // halberd
                            if (currweaponframe == HALBERDATTACK1 + 3)
                                if (plr->currweaponanim == 6 && currweapontics == 12)
                                    swingdacrunch();
                            if (currweaponframe == HALBERDATTACK2 + 3)
                                if (plr->currweaponanim == 4 && currweapontics == 12)
                                    swingdacrunch();
                            break;
                        }

                        sprite[hitsprite].hitag -= k;

                        if (plr->currweapon != 0)
                        {
                            //JSA GORE normal
                            if (rand() % 100 > 50)
                            {
                                if (sprite[hitsprite].picnum == SKELETON
                                    || sprite[hitsprite].picnum == SKELETONATTACK
                                    || sprite[hitsprite].picnum == SKELETONDIE)
                                    playsound_loc(S_SKELHIT1 + (rand() % 2), sprite[hitsprite].x, sprite[hitsprite].y);
                            }
                            //HERE
                            switch (plr->currweapon)
                            {
                                case 0: // fist
                                break;
                                case 1: // knife
                                if (currweaponframe == KNIFEATTACK + 6)
                                    if (plr->currweaponanim == 8 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == KNIFEATTACK2 + 2)
                                    if (plr->currweaponanim == 5 || plr->currweaponanim == 9 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 2: // short sword
                                if (currweaponframe == GOBSWORDATTACK + 4)
                                    if (plr->currweaponanim == 4 && currweapontics == 10)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == GOBSWORDATTACK2 + 4)
                                    if (plr->currweaponanim == 4 && currweapontics == 10)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 3: // morning
                                if (currweaponframe == MORNINGSTAR + 5)
                                    if (plr->currweaponanim == 7 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == MORNINGATTACK2 + 3)
                                    if (plr->currweaponanim == 3 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 4: // sword
                                if (currweaponframe == SWORDATTACK + 7)
                                    if (plr->currweaponanim == 7 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == SWORDATTACK2 + 6)
                                    if (plr->currweaponanim == 6 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 5: // battleaxe
                                if (currweaponframe == BIGAXEATTACK + 7)
                                    if (plr->currweaponanim == 7 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == BIGAXEATTACK2 + 6)
                                    if (plr->currweaponanim == 6 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 6: // bow
                                if (currweaponframe == BOWWALK + 4)
                                    if (plr->currweaponanim == 4 && currweapontics == 6)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 7: // pike
                                if (currweaponframe == PIKEATTACK1 + 4)
                                    if (plr->currweaponanim == 8 && currweapontics == 10)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == PIKEATTACK2 + 4)
                                    if (plr->currweaponanim == 4 && currweapontics == 10)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 8: // two handed sword
                                if (currweaponframe == EXCALATTACK1 + 7)
                                    if (plr->currweaponanim == 7 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == EXCALATTACK2 + 5)
                                    if (plr->currweaponanim == 5 && currweapontics == 8)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                                case 9: // halberd
                                if (currweaponframe == HALBERDATTACK1 + 3)
                                    if (plr->currweaponanim == 6 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                if (currweaponframe == HALBERDATTACK2 + 3)
                                    if (plr->currweaponanim == 4 && currweapontics == 12)
                                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                                break;
                            }
                            //ENDOFHERE

                            //chunksomeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                        }
                    }

                    if (netgame)
                    {
                        break;
                    }

                    if (sprite[hitsprite].hitag <= 0)
                    {
                        if (plr->selectedgun > 1)
                        {
                            //JSA GORE on death ?
                            //RAF ans:death
                            chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);

                            if (sprite[hitsprite].picnum == SKELETON
                                || sprite[hitsprite].picnum == SKELETONATTACK
                                || sprite[hitsprite].picnum == SKELETONDIE)
                                playsound_loc(S_SKELHIT1 + (rand() % 2), sprite[hitsprite].x, sprite[hitsprite].y);
                        }
                        newstatus(hitsprite, DIE);
                    }
                    sprite[hitsprite].ang = plr->ang + ((krand() & 32) - 64);
                    if (sprite[hitsprite].hitag > 0)
                    {
                        newstatus(hitsprite, PAIN);
                    }
                    break;
                    case GRONHALDIE:
                    case GRONMUDIE:
                    case GRONSWDIE:
                    case KOBOLDDIE:
                    case DRAGONDIE:
                    case DEVILDIE:
                    case FREDDIE:
                    case SKELETONDIE:
                    case GOBLINDIE:
                    case MINOTAURDIE:
                    case SKULLYDIE:
                    case SPIDERDIE:
                    case FATWITCHDIE:
                    case JUDYDIE:
                    //case FISH:
                    //case RAT:
                    //case WILLOW:
                    if (sprite[hitsprite].pal == 6)
                    {
                        //JSA_NEW
                        SND_PlaySound(S_SOCK1 + (rand() % 4), 0, 0, 0, 0);
                        playsound_loc(S_FREEZEDIE, hitx, hity);
                        for (k = 0; k < 32; k++)
                            icecubes(hitsprite, hitx, hity, hitz, hitsprite);
                        deletesprite(hitsprite);
                    }
                } // switch
            } // if weapondist
            if (madeahit == 0)
            {
                plr->ammo[plr->currweapon]++;
                madeahit = 1;
            }
        }
    break;
    case 1:
    {
        daang2 = (daang + 2048) & kAngleMask;
        daz2 = (100 - plr->horiz) * 2000;

        pos.x = plr->x;
        pos.y = plr->y;
        pos.z = plr->z;

        hitscan(&pos, plr->sector,                   // Start position
            Cos(daang2 + 2048), // X vector of 3D ang
            Sin(daang2 + 2048), // X vector of 3D ang
            daz2,               // Z vector of 3D ang
            &hitinfo, CLIPMASK1); // CHECKME - is CLIPMASK1 correct for vesion of this function that hadn't got cliptype param?

        hitsprite = hitinfo.sprite;
        hitsect = hitinfo.sect;
        hitwall = hitinfo.wall;
        hitx = hitinfo.pos.x;
        hity = hitinfo.pos.y;
        hitz = hitinfo.pos.z;

        if (hitwall > 0 && hitsprite < 0)
        {
            neartag(hitx, hity, hitz, hitsect, daang,
                &neartagsector, &neartagwall, &neartagsprite, &neartaghitdist, 1024, 3, NULL);

            if (neartagsector < 0)
            {
                if (sector[neartagsector].lotag == 0)
                {
                    j = insertsprite(hitsect, 0);
                    sprite[j].x = hitx;
                    sprite[j].y = hity;
                    sprite[j].z = hitz + (8 << 8);
                    sprite[j].cstat = 17;//was16
                    sprite[j].picnum = WALLARROW;
                    sprite[j].shade = 0;
                    sprite[j].pal = 0;
                    sprite[j].xrepeat = 16;
                    sprite[j].yrepeat = 48;
                    sprite[j].ang = ((daang + 2048) - 512 + ((rand() & 128) - 64)) & kAngleMask;
                    sprite[j].xvel = 0;
                    sprite[j].yvel = 0;
                    sprite[j].zvel = 0;
                    sprite[j].owner = 4096;
                    sprite[j].lotag = 32;
                    sprite[j].hitag = 0;
                    playsound_loc(S_ARROWHIT, sprite[j].x, sprite[j].y);
                }
            }

            if (netgame)
            {
                netshootgun(-1, 5);
            }
        }
        if (hitwall > 0 && hitsprite > 0)
        {
            j = insertsprite(hitsect, FX);
            sprite[j].x = hitx;
            sprite[j].y = hity;
            sprite[j].z = hitz + (8 << 8);
            sprite[j].cstat = 2;
            sprite[j].picnum = PLASMA;
            sprite[j].shade = -32;
            sprite[j].pal = 0;
            sprite[j].xrepeat = 32;
            sprite[j].yrepeat = 32;
            sprite[j].ang = daang;
            sprite[j].xvel = 0;
            sprite[j].yvel = 0;
            sprite[j].zvel = 0;
            sprite[j].owner = 4096;
            sprite[j].lotag = 32;
            sprite[j].hitag = 0;

            movesprite(j, (Cos(sprite[j].ang) * synctics) << 3, (Sin(sprite[j].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);
        }
        if ((hitsprite >= 0) && (sprite[hitsprite].statnum < MAXSTATUS))
        {
            switch (sprite[hitsprite].picnum)
            {
                case VASEA:
                case VASEB:
                case VASEC:
                newstatus(hitsprite, BROKENVASE);
                break;
                case GRONHAL:
                case GRONHALATTACK:
                case GRONHALPAIN:
                case GRONMU:
                case GRONMUATTACK:
                case GRONMUPAIN:
                case GRONSW:
                case GRONSWATTACK:
                case GRONSWPAIN:
                case KOBOLD:
                case KOBOLDATTACK:
                case DRAGON:
                case DRAGONATTACK2:
                case DRAGONATTACK:
                case DEVILSTAND:
                case DEVIL:
                case DEVILATTACK:
                case FREDSTAND:
                case FRED:
                case FREDATTACK:
                case FREDPAIN:
                case SKELETON:
                case SKELETONATTACK:
                case GOBLINSTAND:
                case GOBLIN:
                case GOBLINCHILL:
                //case GOBLINSURPRISE:
                case GOBLINATTACK:
                case GOBLINPAIN:
                case MINOTAUR:
                case MINOTAURATTACK:
                case MINOTAURPAIN:
                case SPIDER:
                case SKULLY:
                case SKULLYATTACK:
                case FATWITCH:
                case FATWITCHATTACK:
                case FISH:
                case RAT:
                case WILLOW:
                case GUARDIAN:
                case GUARDIANATTACK:
                case JUDYSIT:
                case JUDYSTAND:
                case JUDY:
                case JUDYATTACK1:
                case JUDYATTACK2:

                if (netgame)
                {
                    netshootgun(hitsprite, plr->currweapon);
                    break;
                }

                sprite[hitsprite].hitag -= (krand() & 15) + 15;
                if (sprite[hitsprite].hitag <= 0)
                {
                    newstatus(hitsprite, DIE);
                    if (sprite[hitsprite].picnum == RAT)
                        chunksofmeat(plr, hitsprite, hitx, hity, hitz, hitsect, daang);
                }
                else
                {
                    sprite[hitsprite].ang = (getangle(plr->x - sprite[hitsprite].x, plr->y - sprite[hitsprite].y) & kAngleMask);
                    newstatus(hitsprite, PAIN);
                }
                break;
                // SHATTER FROZEN CRITTER
                case GRONHALDIE:
                case GRONMUDIE:
                case GRONSWDIE:
                case KOBOLDDIE:
                case DRAGONDIE:
                case DEVILDIE:
                case FREDDIE:
                case SKELETONDIE:
                case GOBLINDIE:
                case MINOTAURDIE:
                case SKULLYDIE:
                case SPIDERDIE:
                case FATWITCHDIE:
                case JUDYDIE:
                //case FISH:
                //case RAT:
                //case WILLOW:
                if (sprite[hitsprite].pal == 6)
                {
                    //JSA_NEW
                    SND_PlaySound(S_SOCK1 + (rand() % 4), 0, 0, 0, 0);
                    playsound_loc(S_FREEZEDIE, hitx, hity);
                    for (k = 0; k < 32; k++)
                    {
                        icecubes(hitsprite, hitx, hity, hitz, hitsprite);
                    }
                    deletesprite(hitsprite);
                }
            } // switch
        }
    }
    break;

    case 6: // MEDUSA
    {
        for (i = 0; i < MAXSPRITES; i++)
        {
            //cansee
            if (i != plr->spritenum)
            {
                if (sprite[i].picnum == FRED
                    || sprite[i].picnum == KOBOLD
                    || sprite[i].picnum == GOBLIN
                    || sprite[i].picnum == MINOTAUR
                    || sprite[i].picnum == SPIDER
                    || sprite[i].picnum == SKELETON
                    || sprite[i].picnum == GRONHAL
                    || sprite[i].picnum == GRONMU
                    || sprite[i].picnum == GRONSW)
                {
                    if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1)
                    {
                        //distance check
                        if (checkmedusadist(i, plr->x, plr->y, plr->z, plr->lvl))
                            //medusa
                            medusa(i);
                    }
                }
            }
        }
    }
    break;
    case 7:    //KNOCKSPELL
    {
        daang2 = (daang + 2048) & kAngleMask;
        daz2 = (100 - plr->horiz) * 2000;

        pos.x = plr->x;
        pos.y = plr->y;
        pos.z = plr->z;

        hitscan(&pos, plr->sector,                   //Start position
            Cos(daang2 + 2048), // X vector of 3D ang
            Sin(daang2 + 2048), // X vector of 3D ang
            daz2,               // Z vector of 3D ang
            &hitinfo, CLIPMASK1); // CHECKME - is CLIPMASK1 correct for vesion of this function that hadn't got cliptype param?

        hitsprite = hitinfo.sprite;
        hitsect = hitinfo.sect;
        hitwall = hitinfo.wall;
        hitx = hitinfo.pos.x;
        hity = hitinfo.pos.y;
        hitz = hitinfo.pos.z;

        if (hitsect < 0 && hitsprite < 0 || hitwall >= 0)
        {
            neartag(hitx, hity, hitz, hitsect, daang,
                &neartagsector, &neartagwall, &neartagsprite, &neartaghitdist, 1024, 3, NULL);

            if (neartagsector >= 0)
            {
                if (sector[neartagsector].lotag >= 60 && sector[neartagsector].lotag <= 69)
                {
                    sector[neartagsector].lotag = 6;
                    sector[neartagsector].hitag = 0;
                }
                if (sector[neartagsector].lotag >= 70 && sector[neartagsector].lotag <= 79)
                {
                    sector[neartagsector].lotag = 7;
                    sector[neartagsector].hitag = 0;
                }
                operatesector(neartagsector);
            }
        }
        break;
    case 10: //throw a pike axe
    {
        if (currweaponframe == PIKEATTACK1 + 4)
        {
            if (plr->currweaponanim == 8 && currweapontics == 10)
            {

                if (netgame)
                {
                    netshootgun(-1, 15);
                }

                //j=insertsprite(plr->sector,JAVLIN);
                j = insertsprite(plr->sector, MISSILE);

                sprite[j].x = plr->x;
                sprite[j].y = plr->y;
                sprite[j].z = plr->z + (16 << 8);

                //sprite[j].cstat=17;
                sprite[j].cstat = 21;

                sprite[j].picnum = THROWPIKE;
                sprite[j].ang = ((plr->ang + 2048 + 96) - 512) & kAngleMask;
                sprite[j].xrepeat = 24;
                sprite[j].yrepeat = 24;
                sprite[j].clipdist = 24;

                sprite[j].extra = plr->ang;
                sprite[j].shade = -15;
                sprite[j].xvel = ((krand() & 256) - 128);
                sprite[j].yvel = ((krand() & 256) - 128);
                //sprite[j].zvel=((krand()&256)-128);
                sprite[j].zvel = ((100 - plr->horiz) << 4);
                sprite[j].owner = 4096;
                sprite[j].lotag = 1024;
                sprite[j].hitag = 0;
                sprite[j].pal = 0;
            }
        }
        if (currweaponframe == PIKEATTACK2 + 4)
        {
            if (plr->currweaponanim == 4 && currweapontics == 10)
            {
                //j=insertsprite(plr->sector,JAVLIN);
                j = insertsprite(plr->sector, MISSILE);

                sprite[j].x = plr->x;
                sprite[j].y = plr->y;
                sprite[j].z = plr->z;

                //sprite[j].cstat=17;
                sprite[j].cstat = 21;

                sprite[j].picnum = THROWPIKE;
                sprite[j].ang = ((plr->ang + 2048) - 512) & kAngleMask;
                sprite[j].xrepeat = 24;
                sprite[j].yrepeat = 24;
                sprite[j].clipdist = 24;

                sprite[j].extra = plr->ang;
                sprite[j].shade = -15;
                sprite[j].xvel = ((krand() & 256) - 128);
                sprite[j].yvel = ((krand() & 256) - 128);
                sprite[j].zvel = ((krand() & 256) - 128);
                sprite[j].owner = 4096;
                sprite[j].lotag = 1024;
                sprite[j].hitag = 0;
                sprite[j].pal = 0;
            }
        }
    }

    break;
    case 2: // parabolic trajectory
    {
        if (netgame)
        {
            netshootgun(-1, 12);
        }

        j = insertsprite(plr->sector, MISSILE);
        sprite[j].x = plr->x;
        sprite[j].y = plr->y;
        sprite[j].z = plr->z + (8 << 8) + ((krand() & 10) << 8);
        sprite[j].cstat = 0;
        sprite[j].picnum = PLASMA;
        sprite[j].shade = -32;
        sprite[j].pal = 0;
        sprite[j].xrepeat = 16;
        sprite[j].yrepeat = 16;
        sprite[j].ang = daang;
        sprite[j].xvel = Cos(daang + 2048) >> 5;
        sprite[j].yvel = Sin(daang + 2048) >> 5;
        sprite[j].zvel = ((100 - plr->horiz) << 4);
        sprite[j].owner = 4096;
        sprite[j].lotag = 256;
        sprite[j].hitag = 0;
        sprite[j].clipdist = 48;

        movesprite(j, (Cos(sprite[j].ang) * synctics) << 3, (Sin(sprite[j].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);

        //vec3_t pos;
        pos.x = sprite[j].x;
        pos.y = sprite[j].y;
        pos.z = sprite[j].z;

        setsprite(j, &pos);
    }
    break;
    case 3:
    {
        if (netgame)
        {
            netshootgun(-1, 13);
        }

        j = insertsprite(plr->sector, MISSILE);
        sprite[j].x = plr->x;
        sprite[j].y = plr->y;
        sprite[j].z = plr->z + (8 << 8);
        sprite[j].cstat = 0;        //Hitscan does not hit other bullets
        sprite[j].picnum = MONSTERBALL;
        sprite[j].shade = -32;
        sprite[j].pal = 0;
        sprite[j].xrepeat = 64;
        sprite[j].yrepeat = 64;
        sprite[j].ang = plr->ang;
        sprite[j].xvel = Cos(daang + 2048) >> 7;
        sprite[j].yvel = Sin(daang + 2048) >> 7;
        sprite[j].zvel = ((100 - plr->horiz) << 4);
        sprite[j].owner = 4096;
        sprite[j].lotag = 256;
        sprite[j].hitag = 0;
        sprite[j].clipdist = 64;

        movesprite(j, (Cos(sprite[j].ang) * synctics) << 3, (Sin(sprite[j].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);

        //vec3_t pos;
        pos.x = sprite[j].x;
        pos.y = sprite[j].y;
        pos.z = sprite[j].z;

        setsprite(j, &pos);
    }
    break;
    case 4:
    {
        if (netgame)
        {
            netshootgun(-1, 14);
        }

        for (j = 0; j < MAXSPRITES; j++)
        {
            switch (sprite[j].picnum)
            {
                case SPIDER:
                case KOBOLD:
                case KOBOLDATTACK:
                case DEVIL:
                case DEVILATTACK:
                case GOBLIN:
                case GOBLINATTACK:
                case GOBLINCHILL:
                case MINOTAUR:
                case MINOTAURATTACK:
                case SKELETON:
                case SKELETONATTACK:
                case GRONHAL:
                case GRONHALATTACK:
                case GRONMU:
                case GRONMUATTACK:
                case GRONSW:
                case GRONSWATTACK:
                case DRAGON:
                case DRAGONATTACK:
                case DRAGONATTACK2:
                case GUARDIAN:
                case GUARDIANATTACK:
                case FATWITCH:
                case FATWITCHATTACK:
                case SKULLY:
                case SKULLYATTACK:
                case JUDY:
                case JUDYATTACK1:
                case JUDYATTACK2:
                case WILLOW:
                {
                    if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[j].x, sprite[j].y, sprite[j].z - (tilesiz[sprite[j].picnum].y << 7), sprite[j].sectnum) == 1)
                    {
                        if (checkmedusadist(j, plr->x, plr->y, plr->z, 12))
                        {
                            nukespell(j);
                        }
                    }
                }
                break;
            }
        }
    }
    }
    break;
    }
}

void singleshot(short bstatus)
{
    Player* plr = &player[pyrn];

    plr->hasshot = false;

    #if 0
    if (selectedgun != 1 && oldmousestatus == 0)
    {
        keystatus[keys[KEYFIRE]] = keystatus[sc_RightControl] = 0;
        hasshot = 0;
    }
    #endif
}

void setpotion(int nPotion)
{
    Player* plr = &player[pyrn];

    plr->currentpotion = nPotion;
}

void drawpotionpic()
{
    int i;
    int tilenum;

    Player* plr = &player[pyrn];

    if (WH2 && plr->playerdie) {
        return;
    }

    if (netgame)
        return;

    if (svga == 1)
    {
        itemtoscreen(260 << 1, 387, SPOTIONBACKPIC, 0, 0);
        itemtoscreen((260 << 1) - 4, 380, SPOTIONARROW + plr->currentpotion, 0, 0);
    }
    else
    {
        itemtoscreen(260, 161, POTIONBACKPIC, 0, 0);
        itemtoscreen(260 - 2, 161 - 4, POTIONARROW + plr->currentpotion, 0, 0);
    }

    for (i = 0; i < MAXPOTIONS; i++)
    {
        if (plr->potion[i] < 0) {
            plr->potion[i] = 0;
        }

        if (plr->potion[i] > 0)
        {
            switch (i)
            {
                case 0:
                if (svga == 1)
                    tilenum = SFLASKBLUE;
                else
                    tilenum = FLASKBLUE;
                break;
                case 1:
                if (svga == 1)
                    tilenum = SFLASKGREEN;
                else
                    tilenum = FLASKGREEN;
                break;
                case 2:
                if (svga == 1)
                    tilenum = SFLASKOCHRE;
                else
                    tilenum = FLASKOCHRE;
                break;
                case 3:
                if (svga == 1)
                    tilenum = SFLASKRED;
                else
                    tilenum = FLASKRED;
                break;
                case 4:
                if (svga == 1)
                    tilenum = SFLASKTAN;
                else
                    tilenum = FLASKTAN;
                break;
            }

            if (svga == 1) {
                itemtoscreen((262 << 1) + (i * 20), 406, tilenum, 0, 0);
            }
            else {
                itemtoscreen(262 + (i * 10), 169, tilenum, 0, 0);
            }

            sprintf(tempbuf, "%d", plr->potion[i]);

            if (svga == 1)
                fancyfont((266 << 1) + (i * 20), 394, SPOTIONFONT - 26, potionbuf, 0);
            else
                fancyfont(266 + (i * 10), 164, SMFONT - 26, potionbuf, 0);
        }
        else
        {
            if (svga == 1)
            {
                if (WH2) {
                    rotatesprite(((262 << 1) + (i * 20)) << 16, 406 << 16, 65536, 0, SFLASKBLACK, 0, 0, 8 + 16, 0, 0, xdim - 1, ydim - 1);
                }
                else {
                    itemtoscreen((262 << 1) + (i * 10), 406, SFLASKBLACK, 0, 0);
                }
            }
            else {
                itemtoscreen(262 + (i * 10), 169, FLASKBLACK, 0, 0);
            }
        }
    }
}

void usapotion(Player* plr)
{
    if (WH2 && plr->playerdie) {
        return;
    }

    if (plr->currentpotion == 0 && plr->health >= plr->maxhealth)
        return;

    if (plr->currentpotion == 2 && plr->poisoned == 0)
        return;

    if (plr->potion[plr->currentpotion] <= 0)
        return;
    else
        plr->potion[plr->currentpotion]--;

    switch (plr->currentpotion)
    {
        case 0: // health potion
        {
            if (plr->health + 25 > plr->maxhealth)
            {
                if (WH2) {
                    plr->health = plr->maxhealth;
                }
                else {
                    plr->health = 0;
                }

                SND_PlaySound(S_DRINK, 0, 0, 0, 0);

                if (WH2) {
                    sethealth(0);
                }
                else {
                    sethealth(plr->maxhealth);
                }
            }
            else
            {
                SND_PlaySound(S_DRINK, 0, 0, 0, 0);
                sethealth(25);
            }

            startblueflash(10);
        }
        break;
        case 1: // strength
        plr->strongtime = 3200;
        SND_PlaySound(S_DRINK, 0, 0, 0, 0);

        if (WH2) {
            startblueflash(10);
        }
        else {
            startredflash(10);
        }
        break;
        case 2: // anti venom
        {
            SND_PlaySound(S_DRINK, 0, 0, 0, 0);
            plr->poisoned = 0;
            plr->poisontime = 0;
            startwhiteflash(10);
            StatusMessage(360, "poison cured");
            sethealth(0);
        }
        break;
        case 3: // fire resist
        {
            SND_PlaySound(S_DRINK, 0, 0, 0, 0);
            plr->manatime = 3200;
            startwhiteflash(10);
            if (lavasnd != -1)
            {
                SND_StopLoop(lavasnd);
                lavasnd = -1;
            }

            if (WH2) {
                addarmoramount(0);
            }
        }
        break;
        case 4: // invisi
        {
            SND_PlaySound(S_DRINK, 0, 0, 0, 0);
            plr->invisibletime = 3200;
            startgreenflash(10);
        }
        break;
    }

    setpotion(plr->currentpotion);
}

void draworbpic()
{
    char tempbuf[8];
    int  spellbookpage;

    Player* plr = &player[pyrn];

    if (WH2 && plr->playerdie) {
        return;
    }

    if (plr->orbammo[plr->currentorb] < 0)
        plr->orbammo[plr->currentorb] = 0;

    sprintf(tempbuf, "%d", plr->orbammo[plr->currentorb]);

 //   if (svga == 1)
 //   {
 //       spellbookpage = sspellbookanim[plr->currentorb][8].daweaponframe;
 //   }
 //   else
    {
        spellbookpage = spellbookanim[plr->currentorb][8].daweaponframe;
    }

    #if 0
    if (svga == 1)
    {
        if (WH2)
        {
            #if 0
            itemtoscreen(120 << 1, 372, SSPELLBACK, 0, 0);
            spellbookframe = sspellbookanim[currentorb][8].daweaponframe;
            dax = sspellbookanim[currentorb][8].currx;
            day = sspellbookanim[currentorb][8].curry;
            itemtoscreen(dax << 1, day, spellbookframe, 0, 0);
            sprintf(tempbuf, "%d", plr->orbammo[plr->currentorb]);
            fancyfont(126 << 1, 439, SSCOREFONT - 26, tempbuf, 0);
            #endif
        }
        else
        {
            overwritesprite(121 << 1, 389, spellbookpage, 0, 0, 0);
        }

        fancyfont(126 << 1, 439, SSCOREFONT - 26, tempbuf, 0);
    }
    else
    {
        if (WH2)
        {
            itemtoscreen(122, 155, SPELLBOOKBACK, 0, 0);
            itemtoscreen(121, 161, spellbookpage, 0, 0);
        }
        else
        {
            overwritesprite(121, 161, spellbookpage, 0, 0, 0);
        }

        fancyfont(126, 181, SCOREFONT - 26, tempbuf, 0);
    }
    #else
    
    itemtoscreen(121, 161, spellbookpage, 0, 0);
    //overwritesprite(121, 161, spellbookpage, 0, 0, 0);
    fancyfont(126, 181, SCOREFONT - 26, tempbuf, 0);
    #endif
}

void sethealth(int hp)
{
    Player* plr = &player[pyrn];

    if (WH2)
    {
        switch (difficulty)
        {
            case 1:
            hp >>= 1;
            break;
            case 3:
            hp <<= 1;
            break;
        }
    }

    plr->health += hp;

    if (plr->health < 0) {
        plr->health = 0;
    }
}

void healthpic()
{
    Player* plr = &player[pyrn];

    if (WH2)
    {
        if (plr->playerdie) {
            return;
        }

        if (godmode == 1)
        {
            plr->health = 200;
            if (svga == 0)
            {
                if (plr->screensize <= 320)
                {
                    itemtoscreen(72, 168, HEALTHBACKPIC, 0, 0);
                    fancyfont(74, 170, BGZERO - 26, healthbuf, 0);
                }
            }
            else if (svga == 1)
            {
                if (plr->screensize == 320)
                {
                    itemtoscreen(74 << 1, 406, SHEALTHBACK, 0, 0);
                    fancyfont(76 << 1, 409, SHEALTHFONT - 26, healthbuf, 0);
                }
            }
            return;
        }
    }

    sprintf(healthbuf, "%d", plr->health);

    if (svga == 0)
    {
        if (plr->screensize <= 320)
        {
            if (!WH2 && plr->poisoned == 1)
            {
                itemtoscreen(72, 168, HEALTHBACKPIC, 0, 6);
                fancyfont(74, 170, BGZERO - 26, healthbuf, 6);
            }
            else
            {
                itemtoscreen(72, 168, HEALTHBACKPIC, 0, 0);
                fancyfont(74, 170, BGZERO - 26, healthbuf, 0);
            }
        }
    }
    else if (svga == 1)
    {
        if (plr->screensize == 320)
        {
            if (!WH2 && plr->poisoned == 1)
            {
                itemtoscreen(74 << 1, 406, SHEALTHBACK, 0, 6);
                fancyfont(76 << 1, 409, SHEALTHFONT - 26, healthbuf, 6);
            }
            else
            {
                itemtoscreen(74 << 1, 406, SHEALTHBACK, 0, 0);
                fancyfont(76 << 1, 409, SHEALTHFONT - 26, healthbuf, 0);
            }
        }
    }
}

void addarmoramount(int nArmor)
{
    Player* plr = &player[pyrn];

    if (WH2 && plr->playerdie) {
        return;
    }

    plr->armor += nArmor;

    if (plr->armor < 0)
    {
        plr->armor = 0;
        plr->armortype = 0;
    }
}

//void armorpic(int arm)
void drawarmor()
{
    char armorbuf[5];
    Player* plr = &player[pyrn];

    #if 0
    plr->armor += arm;

    if (plr->armor < 0)
    {
        plr->armor = 0;
        plr->armortype = 0;
    }
    #endif

    sprintf(armorbuf, "%d", plr->armor);

    if (svga == 0)
    {
        if (plr->screensize <= 320)
        {
            itemtoscreen(197, 168, HEALTHBACKPIC, 0, 0);
            fancyfont(200 + 1, 170, BGZERO - 26, armorbuf, 0);
        }
    }
    else
    {
        if (plr->screensize == 320)
        {
            itemtoscreen(200 << 1, 406, SHEALTHBACK, 0, 0);
            fancyfont(204 << 1, 409, SHEALTHFONT - 26, armorbuf, 0);
        }
    }
}

void levelpic()
{
    int tilenum;
    char textbuf[20];

    Player* plr = &player[pyrn];

    if (WH2 && plr->playerdie) {
        return;
    }

    if (plr->selectedgun == 6)
    {
        if (plr->ammo[6] < 0)
            plr->ammo[6] = 0;

        sprintf(textbuf, "%d", plr->ammo[6]);
        #if 0
        if (svga == 0)
        {
            itemtoscreen(3, 181, ARROWS, 0, 0);
            fancyfont(42, 183, SCOREFONT - 26, textbuf, 0);
        }
        else
        #endif
        {
            itemtoscreen(3 << 1, 434, SARROWS, 0, 0);
            fancyfont(42 << 1, 439, SSCOREFONT - 26, textbuf, 0);
        }
    }
    else if (plr->selectedgun == 7 && plr->weapon[7] == 2)
    {
        if (plr->ammo[7] < 0)
            plr->ammo[7] = 0;

        sprintf(textbuf, "%d", plr->ammo[7]);

        // need pike pic
        if (svga == 0)
        {
            itemtoscreen(3, 181, PIKES, 0, 0);
            fancyfont(42, 183, SCOREFONT - 26, textbuf, 0);
        }
        else
        {
            itemtoscreen(3 << 1, 434, SPIKES, 0, 0);
            fancyfont(42 << 1, 439, SSCOREFONT - 26, textbuf, 0);
        }
    }
    else
    {
        #if 1
        //if (svga == 0)
            tilenum = PLAYERLVL + (plr->lvl - 1);
        //else
        //#endif
        //    tilenum = SPLAYERLVL + (plr->lvl - 1);
            #endif

        #if 1
        //if (svga == 0)
            itemtoscreen(3, 181, tilenum, 0, 0);
        //else
        #endif
            //itemtoscreen(3/* << 1*/, 436, tilenum, 0, 0);
//        itemtoscreen(3, 181, tilenum, 0, 0);
    }
}

void addscoreamount(int nScore)
{
    Player* plr = &player[pyrn];

    if (WH2 && plr->playerdie) {
        return;
    }

    plr->score += nScore;
    expgained  += nScore; // Witchaven 2

    goesupalevel(plr);
}

void drawscore()
{
    int tilenum;
    char tempbuf[8];
    Player* plr = &player[pyrn];

    tilenum = SCOREFONT - 26;

    sprintf(tempbuf, "%d", plr->score);

    if (svga == 1)
        itemtoscreen(6, 394, SSCOREBACKPIC, 0, 0);
    else
        itemtoscreen(29, 163, SCOREBACKPIC, 0, 0);

    strcpy(scorebuf, tempbuf);

    if (svga == 1)
    {
        if (plr->screensize == 320)
            fancyfont(60, 396 + 1, SSCOREFONT - 26, scorebuf, 0);
    }
    else
    {
        if (plr->screensize <= 320)
            fancyfont(30, 165, SCOREFONT - 26, scorebuf, 0);
    }

//    goesupalevel(plr);
}

void goesupalevel(Player* plr)
{
    if (WH2)
    {
        switch (plr->lvl)
        {
            case 0:
            case 1:
            if (plr->score > 9999)
            {
                StatusMessage(360, "thou art a warrior");
                plr->lvl = 2;
                plr->maxhealth = 120;
                //levelpic();
            }
            break;
            case 2:
            if (plr->score > 19999)
            {
                StatusMessage(360, "thou art a swordsman");
                plr->lvl = 3;
                plr->maxhealth = 140;
                //levelpic();
            }
            break;
            case 3:
            if (plr->score > 29999)
            {
                StatusMessage(360, "thou art a hero");
                plr->lvl = 4;
                plr->maxhealth = 160;
                //levelpic();
            }
            break;
            case 4:
            if (plr->score > 39999)
            {
                StatusMessage(360, "thou art a champion");
                plr->lvl = 5;
                plr->maxhealth = 180;
                //levelpic();
            }
            break;
            case 5:
            if (plr->score > 49999)
            {
                StatusMessage(360, "thou art a superhero");
                plr->lvl = 6;
                plr->maxhealth = 200;
                //levelpic();
            }
            break;
            case 6:
            if (plr->score > 59999)
            {
                StatusMessage(360, "thou art a lord");
                plr->lvl = 7;
                //levelpic();
            }
        }
    }
    else
    {
        if (plr->score > 2250 && plr->score < 4499 && plr->lvl < 2)
        {
            StatusMessage(360, "thou art 2nd level");
            plr->lvl = 2;
            plr->maxhealth = 120;
            //levelpic();
        }
        else if (plr->score > 4500 && plr->score < 8999 && plr->lvl < 3)
        {
            StatusMessage(360, "thou art 3rd level");
            plr->lvl = 3;
            plr->maxhealth = 140;
            //levelpic();
        }
        else if (plr->score > 9000 && plr->score < 17999 && plr->lvl < 4)
        {
            StatusMessage(360, "thou art 4th level");
            plr->lvl = 4;
            plr->maxhealth = 160;
            //levelpic();
        }
        else if (plr->score > 18000 && plr->score < 35999 && plr->lvl < 5)
        {
            StatusMessage(360, "thou art 5th level");
            plr->lvl = 5;
            plr->maxhealth = 180;
            //levelpic();
        }
        else if (plr->score > 36000 && plr->score < 74999 && plr->lvl < 6)
        {
            StatusMessage(360, "thou art 6th level");
            plr->lvl = 6;
            plr->maxhealth = 200;
            //levelpic();
        }
        else if (plr->score > 75000 && plr->score < 179999 && plr->lvl < 7)
        {
            StatusMessage(360, "thou art 7th level");
            plr->lvl = 7;
            //levelpic();
        }
        else if (plr->score > 180000 && plr->score < 279999 && plr->lvl < 8)
        {
            StatusMessage(360, "thou art 8th level");
            plr->lvl = 8;
            //levelpic();
        }
        else if (plr->score > 280000 && plr->score < 379999 && plr->lvl < 9)
        {
            StatusMessage(360, "thou art hero");
            plr->lvl = 9;
            //levelpic();
        }
    }
}

int checkweapondist(short i, int x, int y, int z, char guntype)
{
    Player* plr = &player[pyrn];

    int length;

    switch (plr->selectedgun)
    {
        default:
        case 0:
        case 1:
        case 7:
        length = 1024;
        break;
        case 2:
        case 3:
        case 4:
        case 5:
        length = 1536;
        break;
        case 6:
        case 8:
        case 9:
        length = 2048;
        break;
    }

    if ((labs(x - sprite[i].x) + labs(y - sprite[i].y) < length) && (labs((z >> 8) - ((sprite[i].z >> 8) - (tilesiz[sprite[i].picnum].y >> 1))) <= (length >> 3)))
        return 1;
    else
        return 0;
}

void updatepics()
{
//    score(0);

    if (netgame != 0)
    {
        if (gametype >= 1)
            captureflagpic();
        else
            fragspic();
    }
    else {
// TODO        setpotion(plr->currentpotion);
    }

    //levelpic();
//    healthpic();
//    armorpic(0);
    drawpotionpic();
// TODO    orbpic(currentorb);
    keyspic();
}

extern short teamscore[];
extern short teaminplay[];

struct capt
{
    int x;
    int y;
    int palnum;
};

void captureflagpic()
{
    struct capt flag[4] = {{ 260, 161, 0},
                           { 286, 161, 10},
                           { 260, 176, 11},
                           { 286, 176, 12}};

    struct capt sflag[4] = {{ 260, 387, 0},
                            { 286, 387, 10},
                            { 260, 417, 11},
                            { 286, 417, 12}};

    if (svga == 1)
    {
        // overwritesprite(260<<1,387,SPOTIONBACKPIC,0,0,0);
        itemtoscreen(260 << 1, 387, SPOTIONBACKPIC, 0, 0);
    }
    else
    {
        // overwritesprite(260,161,POTIONBACKPIC,0,0,0);
        itemtoscreen(260, 161, POTIONBACKPIC, 0, 0);
    }

    for (int i = 0; i < 4; i++)
    {
        if (svga == 1)
        {
            if (teaminplay[i])
            {
                overwritesprite((sflag[i].x << 1) + 6, sflag[i].y + 8, STHEFLAG, 0, 0, sflag[i].palnum);
                sprintf(tempbuf, "%d", teamscore[i]);
                fancyfont((sflag[i].x << 1) + 16, sflag[i].y + 16, SPOTIONFONT - 26, tempbuf, 0);
            }
        }
        else
        {
            if (teaminplay[i])
            {
                if (WH2)
                {
                    rotatesprite((flag[i].x + 3) << 16, (flag[i].y + 3) << 16,
                        65536, 0, THEFLAG, 0, flag[i].palnum, 8 + 16,
                        0, 0, xdim - 1, ydim - 1);
                }
                else
                {
                    overwritesprite(flag[i].x + 3, flag[i].y + 3, THEFLAG, 0, 0, flag[i].palnum);
                }

                sprintf(tempbuf, "%d", teamscore[i]);
                fancyfont(flag[i].x + 6, flag[i].y + 6, SMFONT - 26, tempbuf, 0);
            }
        }
    }
}

void fragspic()
{
    Player* plr = &player[pyrn];

    if (svga == 1)
    {
        if (plr->screensize == 320)
        {
            // overwritesprite(260<<1,387,SPOTIONBACKPIC,0,0,0);
            itemtoscreen(260 << 1, 387, SPOTIONBACKPIC, 0, 0);
            sprintf(tempbuf, "%d", teamscore[pyrn]);
            //overwritesprite(74<<1,406,SHEALTHBACK,0,0,0);
            fancyfont((260 << 1) + 10, 387 + 10, SHEALTHFONT - 26, tempbuf, 0);
        }
    }
    else
    {
        if (plr->screensize <= 320)
        {
            // overwritesprite(260,161,POTIONBACKPIC,0,0,0);
            itemtoscreen(260, 161, POTIONBACKPIC, 0, 0);
            sprintf(tempbuf, "%d", teamscore[pyrn]);
            //overwritesprite(72,168,HEALTHBACKPIC,0,0,0);
            fancyfont(260 + 15, 161 + 5, BGZERO - 26, tempbuf, 0);
        }
    }
}

void keyspic()
{
    Player* plr = &player[pyrn];

    if (svga == 1)
    {
        if (plr->treasure[14] == 1)
            // overwritesprite(242<<1,387,SKEYBRASS,0,0,0);
            itemtoscreen(242 << 1, 387, SKEYBRASS, 0, 0);
        else
            // overwritesprite(242<<1,387,SKEYBLANK,0,0,0);
            itemtoscreen(242 << 1, 387, SKEYBLANK, 0, 0);

        if (plr->treasure[15] == 1)
            // overwritesprite(242<<1,408,SKEYBLACK,0,0,0);
            itemtoscreen(242 << 1, 408, SKEYBLACK, 0, 0);
        else
            // overwritesprite(242<<1,408,SKEYBLANK,0,0,0);
            itemtoscreen(242 << 1, 408, SKEYBLANK, 0, 0);

        if (plr->treasure[16] == 1)
            // overwritesprite(242<<1,430,SKEYGLASS,0,0,0);
            itemtoscreen(242 << 1, 430, SKEYGLASS, 0, 0);
        else
            // overwritesprite(242<<1,430,SKEYBLANK,0,0,0);
            itemtoscreen(242 << 1, 430, SKEYBLANK, 0, 0);

        if (plr->treasure[17] == 1)
            // overwritesprite(242<<1,452,SKEYIVORY,0,0,0);
            itemtoscreen(242 << 1, 452, SKEYIVORY, 0, 0);
        else
            // overwritesprite(242<<1,452,SKEYBLANK,0,0,0);
            itemtoscreen(242 << 1, 452, SKEYBLANK, 0, 0);
    }
    else
    {
        if (plr->treasure[14] == 1)
            // overwritesprite(242,160,KEYBRASS,0,0,0);
            itemtoscreen(242, 160, KEYBRASS, 0, 0);
        else
            // overwritesprite(242,160,KEYBLANK,0,0,0);
            itemtoscreen(242, 160, KEYBLANK, 0, 0);

        if (plr->treasure[15] == 1)
            // overwritesprite(242,169,KEYBLACK,0,0,0);
            itemtoscreen(242, 169, KEYBLACK, 0, 0);
        else
            // overwritesprite(242,169,KEYBLANK,0,0,0);
            itemtoscreen(242, 169, KEYBLANK, 0, 0);

        if (plr->treasure[16] == 1)
            // overwritesprite(242,178,KEYGLASS,0,0,0);
            itemtoscreen(242, 178, KEYGLASS, 0, 0);
        else
            // overwritesprite(242,178,KEYBLANK,0,0,0);
            itemtoscreen(242, 178, KEYBLANK, 0, 0);

        if (plr->treasure[17] == 1)
            // overwritesprite(242,187,KEYIVORY,0,0,0);
            itemtoscreen(242, 187, KEYIVORY, 0, 0);
        else
            // overwritesprite(242,187,KEYBLANK,0,0,0);
            itemtoscreen(242, 187, KEYBLANK, 0, 0);
    }
}

int adjustscore(int score)
{
    float factor;

    factor = (krand() % 20) / 100;

    if (krand() % 100 > 50)
        return(score * (factor + 1));
    else
        return(score - (score * (factor)));
}

int lvlspellcheck(Player* plr)
{
    int legal = 0;

    if (WH2)
    {
        legal = 1;
        return legal;
    }

    switch (plr->currentorb)
    {
        case 0:
            legal = 1;
        break;
        case 1:
            legal = 1;
        break;
        case 2:
            if (plr->lvl > 1)
                legal = 1;
            else 
                StatusMessage(360, "must attain 2nd level");
        break;
        case 3:
            if (plr->lvl > 1)
                legal = 1;
            else 
                StatusMessage(360, "must attain 2nd level");
        break;
        case 4:
        case 5:
            if (plr->lvl > 2)
                legal = 1;
            else
                StatusMessage(360, "must attain 3rd level");
        break;
        case 6:
            if (plr->lvl > 3)
                legal = 1;
            else
                StatusMessage(360, "must attain 4th level");
        break;
        case 7:
            if (plr->lvl > 4)
                legal = 1;
            else
                StatusMessage(360, "must attain 5th level");
        break;
    }

    return legal;
}

void gronmissile(int s)
{
    Player* plr = &player[pyrn];

    short daang = (sprite[s].ang - 36) & kAngleMask;

    for (int k = 0; k < 10; k++)
    {
        daang = (daang + (k << 1)) & kAngleMask;

        int32_t j = insertsprite(sprite[s].sectnum, MISSILE);
        sprite[j].x = sprite[s].x;
        sprite[j].y = sprite[s].y;
        sprite[j].z = sprite[s].z + (8 << 8) + ((krand() & 10) << 8);
        sprite[j].cstat = 0;
        sprite[j].picnum = PLASMA;
        sprite[j].shade = -32;
        sprite[j].pal = 0;
        sprite[j].xrepeat = 16;
        sprite[j].yrepeat = 16;
        sprite[j].ang = daang;
        sprite[j].xvel = Cos(daang + 2048) >> 5;
        sprite[j].yvel = Sin(daang + 2048) >> 5;

        int32_t discrim = ksqrt((plr->x - sprite[s].x) * (plr->x - sprite[s].x)
            + (plr->y - sprite[s].y) * (plr->y - sprite[s].y));

        if (discrim == 0)
            discrim = 1;

        sprite[j].zvel = (((plr->z + (8 << 8)) - sprite[s].z) << 7) / discrim;
        sprite[j].owner = s;
        sprite[j].lotag = 256;
        sprite[j].hitag = 0;
        sprite[j].clipdist = 48;
    }
}

void displayspelltext()
{
    Player* plr = &player[pyrn];

    switch (plr->currentorb)
    {
        case 0:
        StatusMessage(360, "scare spell");
        break;
        case 1:
        StatusMessage(360, "night vision spell");
        break;
        case 2:
        StatusMessage(360, "freeze spell");
        break;
        case 3:
        StatusMessage(360, "magic arrow spell");
        break;
        case 4:
        StatusMessage(360, "open door spell");
        break;
        case 5:
        StatusMessage(360, "fly spell");
        break;
        case 6:
        StatusMessage(360, "fireball spell");
        break;
        case 7:
        StatusMessage(360, "nuke spell");
        break;
    }
}

void painsound(int xplc, int yplc)
{
    playsound_loc(S_BREATH1+(rand()%6),xplc,yplc);
}

int inView(Player* plr, int i)
{
    short a = getangle(sprite[i].x - plr->x, sprite[i].y - plr->y);

    if ((a < plr->ang && plr->ang - a < 256) ||
        (a >= plr->ang && ((plr->ang + a) & kAngleMask) < 256))
    {
        return 1;
    }
    return 0;
}

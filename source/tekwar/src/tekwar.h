/***************************************************************************
 *   TEKWAR.H  -  for stuff that probably all modules would like to        *
 *                know about                                               *
 *                                                                         *
 ***************************************************************************/

#define TEKWAR 

#define BETA   1
#define DEBUG  1

#define CLKIPS      120
#define MSGBUFSIZE  40             
#define DOOMGUY     999
#define KENSPLAYERHEIGHT    34

#define TICSPERFRAME 3
#define MOVEFIFOSIZ 256

#define TEKTEMPBUFSIZE  256

#define GAMECVARSGLOBAL
#define NETNAMES

typedef struct
{
     int x, y, z;
} point3d;

extern int vel, svel, angvel;
extern int vel2, svel2, angvel2;

extern volatile int recsnddone, recsndoffs;
extern int recording;


#define   NUMOPTIONS          8
#define   NUMKEYS             32
#define   MAXMOREOPTIONS      21
#define   MAXTOGGLES          16
#define   MAXGAMESTUFF        16
extern int fullscreen, xdimgame, ydimgame, bppgame;
extern int forcesetup;
extern unsigned char option[NUMOPTIONS];
extern unsigned char keys[NUMKEYS];
extern unsigned char moreoptions[MAXMOREOPTIONS];
extern char toggles[MAXTOGGLES];
extern int  gamestuff[MAXGAMESTUFF];

extern char frame2draw[MAXPLAYERS];
extern int frameskipcnt[MAXPLAYERS];
extern char gundmost[320];

     //Shared player variables
extern int posx[MAXPLAYERS], posy[MAXPLAYERS], posz[MAXPLAYERS];
extern int horiz[MAXPLAYERS], zoom[MAXPLAYERS], hvel[MAXPLAYERS];
extern short ang[MAXPLAYERS], cursectnum[MAXPLAYERS], ocursectnum[MAXPLAYERS];
extern short playersprite[MAXPLAYERS], deaths[MAXPLAYERS];
extern int lastchaingun[MAXPLAYERS];
extern int health[MAXPLAYERS], score[MAXPLAYERS], saywatchit[MAXPLAYERS];
extern short numbombs[MAXPLAYERS], oflags[MAXPLAYERS];
extern char dimensionmode[MAXPLAYERS];
extern char revolvedoorstat[MAXPLAYERS];
extern short revolvedoorang[MAXPLAYERS], revolvedoorrotang[MAXPLAYERS];
extern int revolvedoorx[MAXPLAYERS], revolvedoory[MAXPLAYERS];

     //Local multiplayer variables
extern int locselectedgun;
extern signed char locvel, olocvel;
extern short locsvel, olocsvel;                          // Les 09/30/95
extern short locangvel, olocangvel;                      // Les 09/30/95
extern short locbits, olocbits;

     //Local multiplayer variables for second player
extern int locselectedgun2;
extern signed char locvel2, olocvel2;
extern short locsvel2, olocsvel2;                        // Les 09/30/95
extern short locangvel2, olocangvel2;                    // Les 09/30/95
extern short locbits2, olocbits2;

  //Multiplayer syncing variables
extern signed char fsyncvel[MAXPLAYERS], osyncvel[MAXPLAYERS], syncvel[MAXPLAYERS];
extern short fsyncsvel[MAXPLAYERS], osyncsvel[MAXPLAYERS], syncsvel[MAXPLAYERS];  // Les 09/30/95
extern short fsyncangvel[MAXPLAYERS], osyncangvel[MAXPLAYERS], syncangvel[MAXPLAYERS]; // Les 09/30/95
extern unsigned short fsyncbits[MAXPLAYERS], osyncbits[MAXPLAYERS], syncbits[MAXPLAYERS];

extern char frameinterpolate, detailmode, ready2send;
extern int ototalclock, gotlastpacketclock, smoothratio;
extern int oposx[MAXPLAYERS], oposy[MAXPLAYERS], oposz[MAXPLAYERS];
extern int ohoriz[MAXPLAYERS], ozoom[MAXPLAYERS];
extern short oang[MAXPLAYERS];

extern point3d osprite[MAXSPRITESONSCREEN];

extern int movefifoplc, movefifoend;
extern signed char baksyncvel[MOVEFIFOSIZ][MAXPLAYERS];
extern short baksyncsvel[MOVEFIFOSIZ][MAXPLAYERS];       // Les 09/30/95
extern short baksyncangvel[MOVEFIFOSIZ][MAXPLAYERS];     // Les 09/30/95
extern short baksyncbits[MOVEFIFOSIZ][MAXPLAYERS];

     //GAME.C sync state variables
extern short syncstat;
extern int syncvalplc, othersyncvalplc;
extern int syncvalend, othersyncvalend;
extern int syncvalcnt, othersyncvalcnt;
extern short syncval[MOVEFIFOSIZ], othersyncval[MOVEFIFOSIZ];

extern int crctable[256];
#define updatecrc16(dacrc,dadat) dacrc = (((dacrc<<8)&65535)^crctable[((((unsigned short)dacrc)>>8)&65535)^dadat])
extern char playerreadyflag[MAXPLAYERS];

     //Game recording variables
extern int reccnt, recstat;
extern signed char recsyncvel[16384][2];
extern short recsyncsvel[16384][2];                      // Les 09/30/95
extern short recsyncangvel[16384][2];                    // Les 09/30/95
extern short recsyncbits[16384][2];

     //Miscellaneous variables
extern unsigned char tempbuf[];
extern char boardfilename[80];
extern short screenpeek, oldmousebstatus, brightness;
extern short screensize, screensizeflag;
extern short neartagsector, neartagwall, neartagsprite;
extern int lockclock, neartagdist, neartaghitdist;
extern int masterslavetexttime;
extern int pageoffset, ydim16, chainnumpages;
extern int globhiz, globloz, globhihit, globlohit;
extern int stereofps, stereowidth, stereopixelwidth;

     //Board animation variables
extern short rotatespritelist[16], rotatespritecnt;
extern short warpsectorlist[64], warpsectorcnt;
extern short xpanningsectorlist[16], xpanningsectorcnt;
extern short ypanningwalllist[64], ypanningwallcnt;
extern short floorpanninglist[64], floorpanningcnt;
extern short dragsectorlist[16], dragxdir[16], dragydir[16], dragsectorcnt;
extern int dragx1[16], dragy1[16], dragx2[16], dragy2[16], dragfloorz[16];
extern short swingcnt, swingwall[32][5], swingsector[32];
extern short swingangopen[32], swingangclosed[32], swingangopendir[32];
extern short swingang[32], swinganginc[32];
extern int swingx[32][8], swingy[32][8];
extern short revolvesector[4], revolveang[4], revolvecnt;
extern int revolvex[4][16], revolvey[4][16];
extern int revolvepivotx[4], revolvepivoty[4];
extern short subwaytracksector[4][128], subwaynumsectors[4], subwaytrackcnt;
extern int subwaystop[4][8], subwaystopcnt[4];
extern int subwaytrackx1[4], subwaytracky1[4];
extern int subwaytrackx2[4], subwaytracky2[4];
extern int subwayx[4], subwaygoalstop[4], subwayvel[4], subwaypausetime[4];
extern short waterfountainwall[MAXPLAYERS], waterfountaincnt[MAXPLAYERS];
extern short slimesoundcnt[MAXPLAYERS];

     //Variables that let you type messages to other player
extern char getmessage[162], getmessageleng;
extern int getmessagetimeoff;
extern char typemessage[162];
extern int typemessageleng, typemode;
extern char scantoasc[128];
extern char scantoascwithshift[128];

     //These variables are for animating x, y, or z-coordinates of sectors,
     //walls, or sprites (They are NOT to be used for changing the [].picnum's)
     //See the setanimation(), and getanimategoal() functions for more details.
#define MAXANIMATES 512
extern int *animateptr[MAXANIMATES], animategoal[MAXANIMATES];
extern int animatevel[MAXANIMATES], animateacc[MAXANIMATES], animatecnt;


#define   FT_FULLSCREEN   0
#define   FT_DATALINK     1

#define   MAXHEALTH      1000
#define   MAXSTUN        1000
#define   MAXAMMO        100

#define   CLASS_NULL               0
#define   CLASS_FCIAGENT           1
#define   CLASS_CIVILLIAN          2
#define   CLASS_SPIDERDROID        3
#define   CLASS_COP                4
#define   CLASS_MECHCOP            5
#define   CLASS_TEKBURNOUT         6
#define   CLASS_TEKGOON            7
#define   CLASS_ASSASSINDROID      8
#define   CLASS_SECURITYDROID      9
#define   CLASS_TEKBOSS            10
#define   CLASS_TEKLORD            11

#define   MAPXT        1

struct    picattribtype  {
     unsigned  char      numframes;
     unsigned  char      animtype;
     signed    char      ycenteroffset,xcenteroffset;
     unsigned  char      animspeed;
};

#ifdef __GNUC__
#  if __GNUC__ == 4 && __GNUC_MINOR__ >= 7
#    define TPACK __attribute__ ((packed, gcc_struct))
#  else
#    define TPACK __attribute__ ((packed))
#  endif
#else
#define TPACK
#endif

#ifdef _MSC_VER
#pragma pack(1)
#endif

#ifdef __WATCOMC__
#pragma pack(push,1);
#endif

struct TPACK spriteextension {
     unsigned  char      class;
     signed    char      hitpoints;
     unsigned  short     target;
     unsigned  short     fxmask;  
     unsigned  short     aimask;
     short     basestat;
     unsigned  short     basepic;
     unsigned  short     standpic;
     unsigned  short     walkpic;
     unsigned  short     runpic;
     unsigned  short     attackpic;
     unsigned  short     deathpic;
     unsigned  short     painpic;
     unsigned  short     squatpic;
     unsigned  short     morphpic;
     unsigned  short     specialpic;
     unsigned  char      lock;
     unsigned  char      weapon;
     short     ext2;
     };

#ifdef _MSC_VER
#pragma pack()
#endif

#ifdef __WATCOMC__
#pragma pack(pop)
#endif

#undef TPACK

// use this function whenever you need to verify your 
// extension index
extern    int       seconds;
extern    int       minutes;
extern    int       hours;
extern    int       messageon;
extern    unsigned  criticalerror;
extern    char      messagebuf[MSGBUFSIZE];
extern    char      tektempbuf[TEKTEMPBUFSIZE];
extern    char      notininventory;
extern    struct    picattribtype       *picinfoptr;
extern    sectortype          *sectptr[MAXSECTORS];
extern    spritetype          *sprptr[MAXSPRITES];
extern    walltype            *wallptr[MAXWALLS];
extern    struct    spriteextension     spriteXT[MAXSPRITES];
extern    struct    spriteextension     *sprXTptr[MAXSPRITES];

// additional player arrays
extern    int      fallz[],stun[];
extern    int      invredcards[MAXPLAYERS], invbluecards[MAXPLAYERS];
extern    int      invaccutrak[MAXPLAYERS];
extern    int       fireseq[MAXPLAYERS];
extern    short     ammo1[MAXPLAYERS];
extern    short     ammo2[MAXPLAYERS];
extern    short     ammo3[MAXPLAYERS];
extern    short     ammo4[MAXPLAYERS];
extern    short     ammo5[MAXPLAYERS];
extern    short     ammo6[MAXPLAYERS];
extern    short     ammo7[MAXPLAYERS];
extern    short     ammo8[MAXPLAYERS];
extern    int      weapons[MAXPLAYERS];
extern    char      dead[MAXPLAYERS];
extern    int      firedonetics[MAXPLAYERS];
extern    int      lastgun[MAXPLAYERS];
extern    int       drawweap[MAXPLAYERS];

extern    int      flags32[];                                                  

#define   TOGGLE_RETICULE     0
#define   TOGGLE_TIME         1
#define   TOGGLE_SCORE        2
#define   TOGGLE_SOUND        3
#define   TOGGLE_MUSIC        4
#define   TOGGLE_REARVIEW     5
#define   TOGGLE_UPRT         6
#define   TOGGLE_GODMODE      7
#define   TOGGLE_HEALTH       8
#define   TOGGLE_OVERSCAN     9
#define   TOGGLE_INVENTORY    10

#define   GUN1FLAG            1
#define   GUN2FLAG            2
#define   GUN3FLAG            3
#define   GUN4FLAG            4
#define   GUN5FLAG            5
#define   GUN6FLAG            6
#define   GUN7FLAG            7
#define   GUN8FLAG            8

#define   BASESONG      0

#define   ST_NULL                  0x0000
#define   ST_IMMEDIATE             0x0001
#define   ST_UPDATE                0x0002
#define   ST_NOUPDATE              0x0004
#define   ST_UNIQUE                0x0008
#define   ST_DELAYED               0x0010
#define   ST_VARPITCH              0x0020
#define   ST_BACKWARDS             0x0040
#define   ST_AMBUPDATE             0x0060
#define   ST_VEHUPDATE             0x0080
#define   ST_TOGGLE                0x0100

#define   MAXSOUNDS                     16
#define   TOTALSOUNDS                   208

#define   S_WATERFALL                   0
#define   S_ALARM                       1

#define   S_SEWERLOOP1                  2          //sewer sounds
#define   S_SEWERLOOP2                  3 
#define   S_SEWERLOOP3                  4                  
#define   S_SEWERLOOP4                  5                  
#define   S_SEWERSPLASH1                6
#define   S_SEWERSPLASH2                7
#define   S_SEWERSPLASH3                8
#define   S_SPLASH2                     9
#define   S_SPLASH3                     10
#define   S_SPLASH                      11

#define   S_MATRIX1                     12        //matrix sounds
#define   S_MATRIX2                     13
#define   S_MATRIX3                     14
#define   S_MATRIX4                     15
#define   S_MATRIX5                     16
#define   S_MATRIX6                     17
#define   S_MATRIX7                     18
#define   S_MATRIX8                     19
#define   S_MATRIX9                     20
#define   S_MATRIX10                    21
#define   S_MATRIX11                    22
#define   S_MATRIX12                    23

#define   S_WH_SWITCH                   24        //warehouse
#define   S_WH_GRINDING                 25             
#define   S_WH_TRACKSTOP                26
#define   S_WH_STEAMPRESS               27
#define   S_WH_CLANK                    28
#define   S_WH_6                        29
#define   S_WH_7                        30
#define   S_WH_8                        31

#define   S_WAVES1                      32        //beach and city
#define   S_WAVES2                      33
#define   S_BIRDIES                     34

#define   S_STEAM                       35
#define   S_FIRELOOP                    36
#define   S_SPINNING_DOOR               37

#define   S_TRUCKHORN                   38
#define   S_CITY_AMBIENCE               39
#define   S_WIND                        40
#define   S_AMBULANCE                   41

#define   S_ROBOT_ACCESS                42        //matrix voices
#define   S_ROBOT_INTRUDER              43
#define   S_ROBOT_VIRUS                 44

#define   S_SUBWAY_TRACK                45
#define   S_ALARM2                      46
#define   S_ALARM3                      47

#define   S_STATUS1                     48        //menus and status       
#define   S_STATUS2                     49   
#define   S_HEALTHMONITOR               50   
#define   S_REARMONITOR                 51
#define   S_MENUSOUND1                  52
#define   S_MENUSOUND2                  53 
#define   S_BEEP                        54
#define   S_BOOP                        55

#define   S_MATRIXDOOR1                 56      //doors
#define   S_MATRIXDOOR2                 57
#define   S_MATRIXDOOR3                 58   
#define   S_SIDEDOOR1                   59
#define   S_SIDEDOOR2                   60
#define   S_BIGSWINGOP                  61 
#define   S_BIGSWINGCL                  62 
#define   S_FLOOROPEN                   63
#define   S_BAYDOOR_OPEN                64  
#define   S_BAYDOOR_CLOSE               65
#define   S_BAYDOORLOOP                 66
#define   S_UPDOWNDR2_OP                67
#define   S_UPDOWNDR2_CL                68   
#define   S_DOORKLUNK                   69
#define   S_AIRDOOR                     70
#define   S_AIRDOOR_OPEN                71 
#define   S_AIRDOOR_CLOSE               72 
#define   S_ELEVATOR_DOOR               73

#define   S_SUBSTATIONLOOP              74        //vehicles     
#define   S_SUBWAYLOOP                  75
#define   S_SUBWAYSTART                 76 
#define   S_SUBWAYSTOP                  77
#define   S_TRUCKLOOP                   78
#define   S_TRUCKSTART                  79
#define   S_TRUCKSTOP                   80 
#define   S_TRAMBUSLOOP                 81
#define   S_BOATLOOP                    82
#define   S_CARTLOOP                    83
#define   S_FORKLIFTLOOP                84               


#define   S_RIC1                        85        //guns 
#define   S_RIC2                        86  
#define   S_WEAPON1                     87  
#define   S_WEAPON2                     88  
#define   S_WEAPON3                     89

#define   S_WEAPON4                     90
#define   S_WEAPON5                     91
#define   S_WEAPON6                     92
#define   S_WEAPON7                     93
#define   S_WEAPON8                     94



#define   S_ENEMYGUN1                   95                  
#define   S_ENEMYGUN2                   96
#define   S_ENEMYGUN3                   97
#define   S_ENEMYGUN4                   98
#define   S_WITCH                       99  
#define   S_PLATFORMSTART               100
#define   S_PLATFORMSTOP                101
#define   S_PLATFORMLOOP                102
#define   S_FORCEFIELD1                 103
#define   S_FORCEFIELD2                 104
#define   S_AUTOGUN                     105
#define   S_AUTOGUNEXPLODE              106
#define   S_BUSHIT                      107
#define   S_FORCEFIELDHUMLOOP           108
#define   S_KEYCARDBLIP                 109 
#define   S_PICKUP_BONUS                110
#define   S_JUMP                        111
#define   S_GLASSBREAK1                 112
#define   S_GLASSBREAK2                 113                                
#define   S_EXPLODE1                    114
#define   S_EXPLODE2                    115
#define   S_SMALLGLASS1                 116
#define   S_SMALLGLASS2                 117
#define   S_GORE1                       118
#define   S_GORE2                       119
#define   S_GORE3                       120
#define   S_FLUSH                       121
#define   S_REVOLVEDOOR                 122
#define   S_PAIN1                       123
#define   S_PAIN2                       124
#define   S_SCREAM1                     125
#define   S_SCREAM2                     126
#define   S_SCREAM3                     127
#define   S_PLAYERDIE                   128
#define   S_MANDIE1                     129
#define   S_MANDIE2                     130
#define   S_MANDIE3                     131
#define   S_MANDIE4                     132
#define   S_MANDIE5                     133
#define   S_MANDIE6                     134
#define   S_MANDIE7                     135
#define   S_MANDIE8                     136
#define   S_MANDIE9                     137
#define   S_MANDIE10                    138
#define   S_GIRLDIE1                    139
#define   S_GIRLDIE2                    140
#define   S_GIRLDIE3                    141
#define   S_GIRLDIE4                    142
#define   S_GIRLDIE5                    143
#define   S_GIRLDIE6                    144
#define   S_ANDROID_DIE                 145
#define   S_MATRIX_ATTACK               146
#define   S_MATRIX_ATTACK2              147
#define   S_MATRIX_DIE1                 148
#define   S_MATRIX_DIE2                 149
#define   S_TRANSITION                  150
#define   S_MALE_COMEONYOU              151
#define   S_MALE_TAKEYOUINSLEEP         152
#define   S_MALE_YOULOSER               153
#define   S_MALE_YOUPUNK                154
#define   S_MALE_YOUMORON               155
#define   S_MALE_LIKESHOOTINGDUX        156
#define   S_MALE_SCRAPEBOTBARREL        157
#define   S_MALE_DONTHURT               158
#define   S_MALE_DONTSHOOT              159
#define   S_MALE_PLEASEDONTSHOOT        160 
#define   S_MALE_DONTSHOOT2             161
#define   S_MALE_PLEASEDONTSHOOT2       162
#define   S_MALE_OHMYGOD                163
#define   S_MALE_GETDOWNTAKECOVER       164 
#define   S_MALE_HESGOTGUN              165
#define   S_MALE_IDONTBELIEVE           166
#define   S_MALE_RUNAWAY                167
#define   S_MALE_TAKECOVER              168
#define   S_MALE_HESGOTGUN2             169
#define   S_MALE_ICANTBELIEVE           170
#define   S_MALE_CALLTHEPOLICE          171
#define   S_MALE_HELPCALLPOLICE         172
#define   S_MALE_HEYBACKOFF             173
#define   S_MALE_HESGOTAGUN             174 
#define   S_FEM_RUNHEGOTGUN             175
#define   S_FEM_RUN                     176
#define   S_FEM_EVRYRUNHEGOTGUN         177
#define   S_FEM_CALLACOP                178      
#define   S_FEM_OHNO                    179
#define   S_FEM_MOVEHEKILLUS            180
#define   S_FEM_HESGOTAGUN1             181
#define   S_FEM_EEKRUN                  182
#define   S_FEM_PSYCHOGOTGUN            183
#define   S_FEM_OHMYGOD                 184
#define   S_FEM_HURRYOUTAMYWAY          185
#define   S_FEM_CALLAMBULANCE           186
#define   S_DIANE_DONTSHOOTP            187
#define   S_DIANE_HELPMEPLEASE          188
#define   S_DIANE_OHH                   189
#define   S_DIANE_PLEASEDONTSHOOT       190
#define   S_DIANE_DONTSHOOTP2           191
#define   S_KATIE_DONTSHOOT             192
#define   S_KATIE_PLEASEDONTSHOOT       193
#define   S_MAR_THINKYOUTAKE            194
#define   S_MAR_ISTHATALL               195
#define   S_MAR_KISSYOURASS             196
#define   S_GRD_WHATDOINGHERE           197
#define   S_GRD_IDLEAVE                 198
#define   S_GRD_HANDSUP                 199
#define   S_GRD_HOLDIT                  200
#define   S_GRD_DROPIT                  201
#define   S_DIM_WANTSOMETHIS            202
#define   S_DIM_THINOUCANTAKEME         203
#define   S_DIM_LAUGHTER                204
#define   S_DIM_CANTSTOPTEKLORDS        205
#define   S_DIM_TEKRULES                206
#define   S_HOLOGRAMDIE                 207


#define   AI_NULL             0x0000
#define   AI_FRIEND           0x0001
#define   AI_FOE              0x0002
#define   AI_JUSTSHOTAT       0x0004
#define   AI_CRITICAL         0x0008
#define   AI_WASDRAWN         0x0010
#define   AI_WASATTACKED      0x0020
#define   AI_GAVEWARNING      0x0040
#define   AI_ENCROACHMENT     0x0080
#define   AI_DIDFLEESCREAM    0x0100
#define   AI_DIDAMBUSHYELL    0x0200
#define   AI_DIDHIDEPLEA      0x0400
#define   AI_TIMETODODGE      0x0800


#define   lm(_str_) printf(" %s...\n", _str_);

#define fillsprite(newspriteindex2,x2,y2,z2,cstat2,shade2,pal2,            \
          clipdist2,xrepeat2,yrepeat2,xoffset2,yoffset2,picnum2,ang2,      \
          xvel2,yvel2,zvel2,owner2,sectnum2,statnum2,lotag2,hitag2,extra2) \
{                                                                          \
     spritetype *spr2;                                                     \
     spr2 = &sprite[newspriteindex2];                                      \
     spr2->x = x2; spr2->y = y2; spr2->z = z2;                             \
     spr2->cstat = cstat2; spr2->shade = shade2;                           \
     spr2->pal = pal2; spr2->clipdist = clipdist2;                         \
     spr2->xrepeat = xrepeat2; spr2->yrepeat = yrepeat2;                   \
     spr2->xoffset = xoffset2; spr2->yoffset = yoffset2;                   \
     spr2->picnum = picnum2; spr2->ang = ang2;                             \
     spr2->xvel = xvel2; spr2->yvel = yvel2; spr2->zvel = zvel2;           \
     spr2->owner = owner2;                                                 \
     spr2->lotag = lotag2; spr2->hitag = hitag2; spr2->extra = -1;         \
     copybuf(&spr2->x,&osprite[newspriteindex2].x,3);                      \
}                                                                          \


// b5compat.c

void overwritesprite(int thex, int they, short tilenum, signed char shade,
                     char stat, unsigned char dapalnum);
void permanentwritesprite(int thex, int they, short tilenum, signed char shade,
        int cx1, int cy1, int cx2, int cy2, unsigned char dapalnum);
void permanentwritespritetile(int thex, int they, short tilenum, signed char shade,
        int cx1, int cy1, int cx2, int cy2, unsigned char dapalnum);
void printext(int x, int y, char *buffer, short tilenum, char invisiblecol);
void precache();
void resettiming();

// config.c
int loadsetup(const char *fn);
int writesetup(const char *fn);

// tekcdr.c

int cdpreinit(void);
void cduninit(void);

// tekchng.c

int changehealth(short snum, short deltahealth);
void changescore(short snum, short deltascore);
void tekchangefallz(short snum,int loz,int hiz);
void tekhealstun(short snum);

// tekgame.c

#define   MAXNAMESIZE    11

extern int       dbgflag;
extern int       dbgcolumn;
extern FILE      *dbgfp;

extern short     biasthreshhold;
extern char      biasthreshholdon;
extern int      headbob;
extern char      localname[MAXNAMESIZE];
extern char      netnames[MAXPLAYERS][MAXNAMESIZE];

void checkmasterslaveswitch();
void doanimations();
void domovethings();
void drawscreen(short snum, int dasmoothratio);
void getpackets();
void playback();
int setanimation(int *animptr, int thegoal, int thevel, int theacc);

// tekgun.c

extern short dieframe[MAXPLAYERS];
extern int       goreflag;

void gunstatuslistcode(void);
void killscore(short hs, short snum, char guntype);
void playerpainsound(int p);
void restockammo(int snum);
void tekanimweap(int gun,short p);
void tekdrawgun(int gun,short p);
int tekexplodebody(int i);
void tekfiregun(int gun,short p);
int tekgundamage(int gun,int x,int y,int z,int hitsprite);
void tekgunload(int fil);
void tekgunsave(int fil);
int tekhasweapon(int gun,short snum);
int validplayer(int snum);

// tekldsv.c

int loadgame(int loadno);
int savegame(int saveno);

// tekmap.c

void drawoverheadmap(int cposx, int cposy, int czoom, short cang);

// tekmsc.c

#define   MAXLOADSAVEOPTS     5
#define   MAXLOADSAVESIZE     12
#define   TOTALMAPS      32

extern int       activemenu;
extern int       autocenter[MAXPLAYERS];
extern char      debrief;
extern int       difficulty;
extern char      dofadein;
extern char      gameover;
extern char      generalplay;
extern char      loadsavenames[MAXLOADSAVEOPTS][MAXLOADSAVESIZE];
extern char      *mapnames[TOTALMAPS];
extern int       mission;
extern int       mousesensitivity;
extern int       nochaseflag;
extern int       noenemiesflag;
extern int       noguardflag;
extern int       nostalkflag;
extern int       nostrollflag;
extern char      rearviewdraw;
extern char      singlemapmode;

int accessiblemap(int mn);
void bonusflash();
int choosemap();
int choosemission();
void copyrightscreen();
void criticalflash();
void debriefing();
void depositsymbol(int snum);
void domenu(void);
void domenuinput(void);
void fadein(int start, int end, int steps);
void fadeout(int start, int end, int red, int green, int blue, int steps);
void finishpaletteshifts(void);
void holyoff();
void holyon();
void initmenu(void);
void initmoreoptions();
int initpaletteshifts(void);
void missionaccomplished(int  sn);
int missionfailed();
void newgame(char *mapname);
void newmap(int mapno);
void nextnetlevel();
void rearview(int snum);
void redrawbackfx(void);
void teksavemissioninfo(int fil);
void setup3dscreen();
void showmessage(char *fmt,...);
void tekargv(int argc,char const * const argv[]);
void tekgamestarted(void);
void tekloadmissioninfo(int fil);
void tekloadmoreoptions(int fil);
int tekprivatekeys(void);
void teksavemoreoptions(int fil);
int tekscreenfx(void);
void tektime(void);
void woundflash();

// tekprep.c

extern int  coopmode;
extern int      startx,starty,startz;
extern short starta,starts;
extern int      switchlevelsflag;
extern int       subwaysound[4];

void initplayersprite(short snum);
void netstartspot(int *x, int *y,short *sectnum);
void placerandompic(int picnum);
void prepareboard(char *daboardfilename);
void tekinitmultiplayers(int argc, char const * const argv[]);
void tekloadsetup();
void teknetpickmap(void);
int tekpreinit(void);
void tekrestoreplayer(short snum);
void teksavesetup(void);
void tekview(int *x1,int *y1, int *x2,int *y2);

// teksmk.c

void smkclosemenu();
void smkmenuframe(int fn);
void smkopenmenu(char *name);
void smkplayseq(char *name);
void smkshowmenu();

// teksnd.c

extern int          digiloopflag;

void initsb(char option1,char option2,int digihz,char option7a,char option7b,int val,char option7c);
void menusong(int insubway);
void musicfade();
void musicoff(void);
int playsound(int sn, int sndx,int sndy, int loop, short type);
void songmastervolume(int vol);
void soundmastervolume(int vol);
void startmusic(int level);
void stopallsounds();
void stopsound(int i);
void uninitsb(void);
void updatesounds(int snum);
void updatevehiclesnds(int i, int sndx, int sndy);

// tekspr.c

void analyzesprites(int dax, int day);
void checktouchsprite(short snum, short sectnum);
short floatmovesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype);
short flymovesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype);
short kenmovesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype);
short movesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype);
void playerdropitems(int snum);

// tekstat.c

#define   NORMALCLIP          0
#define   PROJECTILECLIP      1
#define   CLIFFCLIP           2

extern spritetype     pickup;

void ambushyell(short sn, short ext);
void androidexplosion(int i);
int attachvirus(short i, int pic);
void blastmark(int i);
void bloodonwall(int wn, int x,int y,int z, short sect, short daang, int hitx, int hity, int hitz);
void bombexplosion(int i);
int damagesprite(int hitsprite, int points);
void dosectorflash();
void forceexplosion(int i);
void genexplosion1(int i);
void genexplosion2(int i);
void givewarning(short i, short ext);
int initsprites();
int initspriteXTs();
int isahologram(int i);
int isanandroid(int i);
short jsdeletesprite(short spritenum);
short jsinsertsprite(short sect, short stat);
void newstatus(short sn, int  seq);
int pickupsprite(short sn);
void playergunshot(int snum);
int playerhit(int hitsprite, int *pnum);
int playervirus(short pnum, int pic);
void sectorflash(short s);
void sectortriggersprites(short snum);
int spewblood(int sprnum, int hitz, short daang);
void statuslistcode();
void tekstatload(int fh);
void tekstatsave(int fh);
void toss(short snum);
int validext(int ext);

// tektag.c

extern int       headbobon;
extern char onelev[MAXPLAYERS];

void checkmapsndfx(short p);
int krand_intercept(char *stg);
void movedoors(int d);
void movefloordoor(int d);
void movesprelevs(int e);
void movevehicles(int v);
void operatesector(short dasector);
void operatesprite(short dasprite);
void setanimpic(short *pic,short tics,short frames);
void tagcode();
void tekdoanimpic(void);
void tekdodelayfuncs(void);
void tekheadbob(void);
void teknewsector(short p);
void tekoperatesector(short dasector);
void tekpreptags();
void teksetdelayfunc(void (*delayfunc)(short),int tics,short parm);
void tekswitchtrigger(short snum);
void tektagcode(void);
void tektagload(int fil);
void tektagsave(int fil);
int testneighborsectors(short sect1, short sect2);
void warp(int *x, int *y, int *z, short *daang, short *dasector);
void warpsprite(short spritenum);

// tektxt.c

extern char      bypasscdcheck;

void crash(char *s,...);

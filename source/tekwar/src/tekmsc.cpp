/***************************************************************************
 *   TEKMSC.C  -   screenfx, menus, flic stuff etc. for Tekwar             *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "names.h"
#include "pragmas.h"
#include "mmulti.h"
#include "baselayer.h"

#include "tekwar.h"

#define   NUMWHITESHIFTS      3
#define   WHITESTEPS          20
#define   WHITETICS           6
#define   NUMREDSHIFTS        4
#define   REDSTEPS            8

#define   MENUQUITGAME        4
#define   MENULOADGAME        6
#define   MENUSAVEGAME        7
#define   MENUSOUNDMUS        8
#define   MENUMOUSEADJ        9
#define   MENUSCREENADJ       10
#define   HELPSCREEN          99

#define   HCSCALE             100
#define   AMMOSCALE           10

#define   AVERAGEFRAMES       16
#define   MFONTYSTEP          12
#define   MAXSUBOPTIONS       6

char      headbobstr[13]={"HEAD BOB ON"};
struct menu {
     short x,y;
     char proportional,shade,pal;
     char *label;
     struct menu *nextopt;
     char tomenu,backmenu;
} menu[][MAXSUBOPTIONS]={
    {{-1,1,1,0,0,NULL,NULL,0,0},                        // 0
     {-1,1,1,0,0,NULL,NULL,0,0},
     {-1,1,1,0,0,NULL,NULL,0,0},
     {-1,1,1,0,0,NULL,NULL,0,0},
     {-1,1,1,0,0,NULL,NULL,0,0},
     {-1,1,1,0,0,NULL,NULL,0,0}},
    {{-1,1,1,0,0,"TEKWAR MENU",&menu[1][1],0,0},        // 1 MAIN MENU
     {-1,3,1,0,2,"NEW GAME",&menu[1][2],2,0},
     {-1,4,1,2,2,"OPTIONS",&menu[1][3],3,0},
     {-1,5,1,4,2,"LOAD GAME",&menu[1][4],6,0},
     {-1,6,1,6,2,"SAVE GAME",&menu[1][5],7,0},
     {-1,7,1,8,2,"  ABORT  ",NULL,4,0}},
    {{-1,1,1,0,0,"DIFFICULTY",&menu[2][1],0,1},         // 2 DIFFICULTY LEVEL
     {-1,3,1,0,2,"EASY",&menu[2][2],0,1},
     {-1,4,1,0,2,"MEDIUM",&menu[2][3],0,1},
     {-1,5,1,0,2,"HARD",NULL,0,1},
     {-1,6,1,0,2,NULL,NULL,0,1},
     {-1,7,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,0,"OPTIONS MENU",&menu[3][1],0,1},       // 3 OPTIONS MENU
     //{-1,3,1,0,2,"SELECT MISSION",&menu[3][2],5,1},
     {-1,4,1,0,2,"MOUSE SENSITIVITY",&menu[3][2],9,1},
     {-1,5,1,0,2,"SOUND/MUSIC VOLUME",&menu[3][3],8,1},
     {-1,6,1,0,2,&headbobstr[0],NULL,0,1},
     {-1,3,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,5,1,0,3,"ABORT MISSION Y/N?",NULL,0,1},        // 4 QUIT TO DOS
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,0,"SELECT LOCATIONS",&menu[5][1],0,3},// 5 DAY/NIGHT MISSION
     {-1,3,1,0,2,"CITY",&menu[5][2],0,3},
     {-1,4,1,0,2,"HOSPITAL",&menu[5][3],0,3},
     {-1,5,1,0,2,"POLICE STATION",&menu[5][4],0,3},
     {-1,6,1,0,2,"WAREHOUSE",NULL,0,3},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,3,"LOAD GAME MENU",NULL,0,1},            // 6 LOAD GAME MENU
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,3,"SAVE GAME MENU",NULL,0,1},            // 7 SAVE GAME MENU
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,3,"SOUND/MUSIC VOLUME",&menu[8][1],0,3}, // 8 SOUND EFFECTS VOLUME
     {-1,3,1,0,2,"SOUND VOLUME",&menu[8][2],0,3},
     {-1,6,1,0,2,"MUSIC VOLUME",NULL,0,3},
     {-1,0,1,0,2,NULL,NULL,0,3},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,3,"MOUSE SENSITIVITY",NULL,0,3},         // 9 MOUSE SENSITIVITY
     {-1,0,1,0,2,NULL,NULL,0,3},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}},
    {{-1,1,1,0,3,"SCREEN SIZE",NULL,0,3},               // 10 SCREEN SIZE
     {-1,0,1,0,2,NULL,NULL,0,3},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1},
     {-1,0,1,0,2,NULL,NULL,0,1}}
};
int  lastselopt[16]={
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

int      framecnt,frameval[AVERAGEFRAMES];
char     blink=0xFF;
int      menuspincnt=0L;
int      activemenu=0;
char     requesttoquit;
char     dofadein=0;
unsigned char whiteshifts[NUMREDSHIFTS][768];
unsigned char redshifts[NUMREDSHIFTS][768];
int      redcount,whitecount;
char     palshifted;
char     depresscount;
int      selopt=1;
char     otoggles[MAXTOGGLES];
char     tektempbuf[TEKTEMPBUFSIZE];
char     gameover=0;
char     demowon=0;
char     outofsync=0;
char     rearviewdraw;
int      timedinv;
char     loadsavenames[MAXLOADSAVEOPTS][MAXLOADSAVESIZE];
char     lockeybuf[MAXLOADSAVESIZE];
int      invredcards[MAXPLAYERS], invbluecards[MAXPLAYERS];
short    symbols[7];
short    symbolsdeposited[7];
int      invaccutrak[MAXPLAYERS];
int      noenemiesflag;
int      noguardflag;
int      nostalkflag;
int      nochaseflag;
int      nostrollflag;
int      nobriefflag;
char     messagebuf[MSGBUFSIZE];
char     notininventory;
char     redrawborders;
int      curblink,loadnewgame;
int      locmessagelen,loctypemode;
int      mousesensitivity,musicv=16,soundv=16;
int      curblinkclock;
int      messagex;
short    hcpos,wppos,rvpos;
int      autocenter[MAXPLAYERS],menudirect;
int      hcmoving,rvmoving,wpmoving;
short    winner=-1;
char     rvonemotime;
char     wponemotime;
char     hconemotime;
int      lastsec;
int      lastscore;
char     lasttimetoggle;
char     lastscoretoggle;
char     lastinvtoggle;
int      lastwx2;
int      lastinvr,lastinvb,lastinvacc;
int      fortieth;
int      seconds=0;
int      minutes=0;
int      hours=0;
int      messageon=0;
int      difficulty;
int      currentmapno=0;
int      warpretang,warpretsect;
int      warpretx,warprety,warpretz;
int      mission=0;
char     numlives=0;
char     mission_accomplished=0;
int      civillianskilled=0;
char     generalplay;
char     novideoid;
char     singlemapmode;
int      newnetleader=0,oldnetleader=0;
int      netclock;
int      allsymsdeposited=0;
int      killedsonny=0;


void
fadeout(int UNUSED(start), int UNUSED(end), int UNUSED(red), int UNUSED(green), int UNUSED(blue), int UNUSED(steps))
{
//   int       i,j,orig,delta;
//   char      *origptr,*newptr;

     finishpaletteshifts();
/*
     asmwaitvrt(1);
     getpalette(&palette1[0][0]);
     memcpy(palette2,palette1,768);

     for( i=0; i<steps; i++ ) {
          origptr=&palette1[start][0];
          newptr=&palette2[start][0];
          for( j=start; j<=end; j++ ) {
               orig = *origptr++;
               delta = red-orig;
               *newptr++ = orig + delta * i / steps;
               orig = *origptr++;
               delta = green-orig;
               *newptr++ = orig + delta * i / steps;
               orig = *origptr++;
               delta = blue-orig;
               *newptr++ = orig + delta * i / steps;
          }

          asmwaitvrt(1);
          asmsetpalette(&palette2[0][0]);
     }
*/
 return;
}

void
clearkeys(void)
{
     memset(keystatus, 0, sizeof(keystatus));

 return;
}

void
fadein(int UNUSED(start), int UNUSED(end), int UNUSED(steps))
{
/*   int  i,j,delta;

     if( steps == 0 ) {
          return;
     }

     asmwaitvrt(1);
     getpalette(&palette1[0][0]);
     memcpy(&palette2[0][0],&palette1[0][0],sizeof(palette1));

     start *= 3;
     end = end*3+2;

     // fade through intermediate frames
     for( i=0; i<steps; i++ ) {
          for( j=start; j<=end; j++ ) {
               delta=palette[j]-palette1[0][j];
               palette2[0][j]=palette1[0][j] + delta * i / steps;
          }

          asmwaitvrt(1);
          asmsetpalette(&palette2[0][0]);
     }

     // final color
     asmwaitvrt(1);
     asmsetpalette(palette);
*/
     dofadein=0;
     clearkeys();
}

void
fog()
{
     unsigned char      *temp;

     temp=palookup[0];
     palookup[0]=palookup[255];
     palookup[255]=temp;
}

int
initpaletteshifts(void)
{
     unsigned char *workptr,*baseptr;
     int       i,j,delta;

     for( i=1; i<=NUMREDSHIFTS; i++ ) {
          workptr=&redshifts[i-1][0];
          baseptr=&palette[0];
          for( j=0; j<=255; j++) {
               delta=64-*baseptr;
               *workptr++=*baseptr++ + delta * i / REDSTEPS;
               delta=-*baseptr;
               *workptr++=*baseptr++ + delta * i / REDSTEPS;
               delta=-*baseptr;
               *workptr++=*baseptr++ + delta * i / REDSTEPS;
          }
     }

     for( i=1; i<=NUMWHITESHIFTS; i++ ) {
          workptr=&whiteshifts[i-1][0];
          baseptr=&palette[0];
          for( j=0; j<=255; j++ ) {
               delta = 64-*baseptr;
               *workptr++ = *baseptr++ + delta * i / WHITESTEPS;
               delta = 62-*baseptr;
               *workptr++ = *baseptr++ + delta * i / WHITESTEPS;
               delta = 0-*baseptr;
               *workptr++ = *baseptr++ + delta * i / WHITESTEPS;
          }
     }

     return 0;
}

int
startredflash(int damage)
{
     if( redcount != 0 )
          return(0);

     redcount=0;

     redcount+=damage;

     if( redcount < 0 ) {
          redcount=0;
     }

     return 0;
}

int
startwhiteflash(int bonus)
{
     whitecount = 0;

     whitecount+=bonus;

     if( whitecount < 0 ) {
          whitecount=0;
     }

     return 0;
}

void
woundflash()
{
     startredflash(4);
}

void
criticalflash()
{
     startredflash(32);
}

void
bonusflash()
{
     startwhiteflash(8);
}

void
burnflash()
{
     startredflash(8);
}

void
updatepaletteshifts(void)
{
     int   red,white;

     if (whitecount)
     {
          white = whitecount/WHITETICS +1;
          if (white>NUMWHITESHIFTS)
               white = NUMWHITESHIFTS;
          whitecount -= TICSPERFRAME;
          if (whitecount < 0)
               whitecount = 0;
     }
     else {
          white = 0;
     }

     if (redcount)
     {
          red = redcount/10 +1;
          if (red>NUMREDSHIFTS)
               red = NUMREDSHIFTS;
          redcount -= TICSPERFRAME;
          if (redcount < 0)
               redcount = 0;
     }
     else {
          red = 0;
     }


     if( red ) {
          //asmsetpalette(redshifts[red-1]);
          debugprintf("updatepaletteshifts redshifts[%d]\n", red-1);
          palshifted = 1;
     }
     else if( white ) {
          //asmsetpalette(whiteshifts[white-1]);
          debugprintf("updatepaletteshifts whiteshifts[%d]\n", white-1);
          palshifted = 1;
     }
     else if( palshifted ) {
          //asmsetpalette(&palette[0]);     // back to normal
          debugprintf("updatepaletteshifts reset\n");
//          setbrightness(brightness);
          palshifted = 0;
     }

 return;
}

void
setup3dscreen()
{
     int      i, dax, day, dax2, day2;

     if( screensize > xdim ) {
          dax = 0; day = 0;
          dax2 = xdim-1; day2 = ydim-1;
     }
     else {
          dax = (xdim>>1)-(screensize>>1);
          dax2 = dax+screensize-1;
          day = ((ydim-32)>>1)-(((screensize*(ydim-32))/xdim)>>1);
          day2 = day + ((screensize*(ydim-32))/xdim)-1;
          tekview(&dax,&day,&dax2,&day2);
          setview(dax,day>>detailmode,dax2,day2>>detailmode);
     }
     if( screensize <= xdim ) {
          permanentwritespritetile(0L,0L,BACKGROUND,0,0L,0L,xdim-1,ydim-1,0);
          permanentwritesprite((xdim-320)>>1,ydim-32,STATUSBAR,0,0,0,xdim-1,ydim-1,0);
          i = ((xdim-320)>>1);
          while (i >= 8) i -= 8, permanentwritesprite(i,ydim-32,STATUSBARFILL8,0,0,0,xdim-1,ydim-1,0);
          if (i >= 4) i -= 4, permanentwritesprite(i,ydim-32,STATUSBARFILL4,0,0,0,xdim-1,ydim-1,0);
          i = ((xdim-320)>>1)+320;
          while (i <= xdim-8) permanentwritesprite(i,ydim-32,STATUSBARFILL8,0,0,0,xdim-1,ydim-1,0), i += 8;
          if (i <= xdim-4) permanentwritesprite(i,ydim-32,STATUSBARFILL4,0,0,0,xdim-1,ydim-1,0), i += 4;
     }
}

void
finishpaletteshifts(void)
{
     if( palshifted == 1 ) {
          palshifted = 0;
          //asmsetpalette(&palette[0]);
          debugprintf("finishpaletteshifts\n");
     }

 return;
}

void
showmessage(char *fmt,...)
{
     va_list   vargs;

     memset(messagebuf,   '\0', MSGBUFSIZE);

     va_start(vargs,fmt);
     vsprintf(messagebuf,fmt,vargs);
     va_end(vargs);

     messagebuf[MSGBUFSIZE-1]='\0';
     messagex=(xdim>>1)-( ((strlen(messagebuf))>>1)*8 );
     if( messagex < 0 )
          messagex=0;
     messageon=1;
}

#ifdef DOANNOYINGTITLESCREEN
void
tektitlescreen()
{
     int  i,j,k,l;
     char *ptr;

     setgamemode(0, vesares[option[6]&15][0],vesares[option[6]&15][1], 8);
     setview(0L,0L,xdim-1,ydim-1);
     loadtile(TITLESCRPIC);
     loadtile(BACKGROUND);
     i=0;
     j=1621;
     for( k=0; k<256; k++ ) {
          for( l=0; l<256; l++ ) {
               ptr  = (char *)(waloff[TITLESCRPIC]+i);
               *ptr =*(char *)(waloff[BACKGROUND]+i);
               i = (i+j)&65535;
               j = (j+4)&65535;
          }
          rotatesprite(0<<16, 0<<16, 65536l, 0, TITLESCRPIC, 0, 0, 0, 2+8+16, 0, 0, xdim-1, ydim-1);
          nextpage();
     }
}
#endif

void
tekfirstpass()
{
     setbrightness(brightness, palette, 0);
}

void
tekgamestarted(void)
{
     hcpos=-tilesizx[HCDEVICE];
     wppos=-tilesizx[WPDEVICE];
     rvpos=-tilesizx[RVDEVICE];
     seconds=minutes=hours=0;
     show2dsprite[playersprite[myconnectindex]>>3]|=
          (1<<(playersprite[myconnectindex]&7));
     songmastervolume(musicv<<3);
     soundmastervolume(soundv<<11);
}

int
tekprivatekeys(void)
{
     if( activemenu != 0 ) {
          return(0);
     }

     // alt hot keys - non mappable
     /*
     if( (keystatus[56] != 0) && (keystatus[25] != 0) ) {
          keystatus[56]=0;
          keystatus[25]=0;
          cd_play();
     }
     if( (keystatus[56] != 0) && (keystatus[19] != 0) ) {
          keystatus[56]=0;
          keystatus[19]=0;
          cd_resetdisc();
     }
     if( (keystatus[56] != 0) && (keystatus[31] != 0) ) {
          keystatus[56]=0;
          keystatus[31]=0;
          cd_stopplay();
     }
     if( (keystatus[56] != 0) && (keystatus[33] != 0) ) {
          keystatus[56]=0;
          keystatus[33]=0;
          cd_advancetrack();
     }
     if( (keystatus[56] != 0) && (keystatus[48] != 0) ) {
          keystatus[56]=0;
          keystatus[48]=0;
          cd_retardtrack();
     }
     */
     if( (keystatus[56] != 0) && (keystatus[16] != 0) ) {
          keystatus[56]=0;
          keystatus[16]=0;
          gameover=1;
     }

     // cheat num lock keys
     if( (keystatus[56] != 0) && (keystatus[42] != 0) && (option[4] == 0) ) {
          if( keystatus[34] != 0 ) {
               toggles[TOGGLE_GODMODE]^=0x01;
               if( toggles[TOGGLE_GODMODE] ) {
                    holyon();
               }
               else {
                    holyoff();
               }
               keystatus[34]=0;
          }
          if( keystatus[35] != 0 ) {
               changehealth(screenpeek, 200);
               keystatus[35]=0;
          }
          if( keystatus[36] != 0 ) {
               symbols[0]=1;
               symbols[1]=1;
               symbols[2]=1;
               symbols[3]=1;
               symbols[4]=1;
               symbols[5]=1;
               symbols[6]=1;
               keystatus[36]=0;
          }
          if( keystatus[17] != 0 ) {
               ammo1[screenpeek]=MAXAMMO;
               ammo2[screenpeek]=MAXAMMO;
               ammo3[screenpeek]=MAXAMMO;
               ammo4[screenpeek]=MAXAMMO;
               ammo5[screenpeek]=MAXAMMO;
               ammo6[screenpeek]=MAXAMMO;
               ammo7[screenpeek]=MAXAMMO;
               ammo8[screenpeek]=MAXAMMO;
               invredcards[screenpeek]=1;
               invbluecards[screenpeek]=1;
               invaccutrak[screenpeek]=1;
               weapons[screenpeek] =(flags32[GUN1FLAG]|flags32[GUN2FLAG]|flags32[GUN3FLAG]|flags32[GUN4FLAG]);
               weapons[screenpeek]|=(flags32[GUN5FLAG]|flags32[GUN6FLAG]|flags32[GUN7FLAG]|flags32[GUN8FLAG]);
               keystatus[17]=0;
          }
          keystatus[69]=0;
     }

     // local game keys
     if( keystatus[keys[23]] != 0 ) {
          keystatus[keys[23]]=0;
          toggles[TOGGLE_RETICULE]^=0x01;
     }
     if( keystatus[keys[24]] != 0 ) {
          keystatus[keys[24]]=0;
          toggles[TOGGLE_TIME]^=0x01;
     }
     if( keystatus[keys[25]] != 0 ) {
          keystatus[keys[25]]=0;
          toggles[TOGGLE_SCORE]^=0x01;

     }
     if( keystatus[keys[20]] != 0 ) {
          keystatus[keys[20]]=0;
          toggles[TOGGLE_REARVIEW]^=0x01;
          playsound( S_REARMONITOR,0,0,0, ST_IMMEDIATE);
          rvmoving=1;
     }
     if( keystatus[keys[21]] != 0 ) {
          keystatus[keys[21]]=0;
          toggles[TOGGLE_UPRT]^=0x01;
          playsound(S_STATUS1+toggles[TOGGLE_UPRT],0,0,0, ST_IMMEDIATE);
          wpmoving=1;
     }
     if (keystatus[keys[22]] != 0) {
          keystatus[keys[22]]=0;
          toggles[TOGGLE_HEALTH]^=0x01;
          playsound( S_HEALTHMONITOR,0,0,0, ST_IMMEDIATE);
          hcmoving=1;
     }
     if( keystatus[keys[26]] != 0 ) {
          keystatus[keys[26]]=0;
          toggles[TOGGLE_INVENTORY]^=0x01;
     }

     // non mappable function keys
     if (keystatus[59] != 0) {
          activemenu=HELPSCREEN;
          menudirect=1;
     }
     else if (keystatus[60] != 0) {
          if( option[4] == 0 ) {
               activemenu=MENUSAVEGAME;
               menudirect=1;
          }
     }
     else if (keystatus[61] != 0) {
          if( option[4] == 0 ) {
               activemenu=MENULOADGAME;
               menudirect=1;
          }
     }
     else if (keystatus[62] != 0) {
          activemenu=MENUQUITGAME;
          menudirect=1;
     }
     else if (keystatus[64] != 0) {
          activemenu=MENUMOUSEADJ;
          menudirect=1;
          selopt=1;
     }
     else if (keystatus[66] != 0) {
          activemenu=MENUSOUNDMUS;
          menudirect=1;
          selopt=1;
     }
     if( keystatus[65] != 0 ) {
          keystatus[65]=0;
          toggles[TOGGLE_OVERSCAN]^=0x01;
     }

     // esc non mappable
     if( keystatus[1] != 0 ) {
          keystatus[1]=0;
          if( activemenu ) {
               activemenu=0;
          }
          else {
               activemenu=1;
               playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
          }
     }

     return 0;
}

void
redrawbackfx(void)
{
     memmove(otoggles,toggles,MAXTOGGLES);
}

void
holyon()
{
     if( screensize <= xdim ) {
          printext((xdim>>1)-16,4,"HOLY",ALPHABET2,255);
     }
}

void
holyoff()
{
     if( screensize <= xdim ) {
          permanentwritesprite((xdim>>1)-16,4,TIMERESTORE,
                               0,(xdim>>1)-16,4,xdim-1,ydim-1,0);
     }
}

void
showtime()
{
     int   alphabet=ALPHABET+(xdim > 360);

     if( (screensize > xdim) || (dimensionmode[screenpeek] == 2) ) {
          if( toggles[TOGGLE_TIME] ) {
               sprintf(tektempbuf,"%02d:%02d:%02d", hours,minutes,seconds);
               printext(xdim-72,ydim-12,tektempbuf,alphabet,255);
          }
          lastsec=0;
     }
     else {
          if( toggles[TOGGLE_TIME] == 0 ) {
               if( lasttimetoggle != toggles[TOGGLE_TIME] ) {
                    permanentwritesprite(xdim-80,ydim-12,TIMERESTORE,
                                         0,xdim-80,ydim-12,xdim-1,ydim-1,0);
               }
               lasttimetoggle=toggles[TOGGLE_TIME];
               lastsec=0;
          }
          else {
               if( lastsec != seconds ) {
                    permanentwritesprite(xdim-80,ydim-12,TIMERESTORE,
                                         0,xdim-80,ydim-12,xdim-1,ydim-1,0);
                    sprintf(tektempbuf,"%02d:%02d:%02d", hours,minutes,seconds);
                    printext(xdim-72,ydim-12,tektempbuf,alphabet,255);
                    lastsec=seconds;
               }
               lasttimetoggle=toggles[TOGGLE_TIME];
          }
     }
}

void
showscore()
{
     int   alphabet=ALPHABET+(xdim > 360);

     if( (screensize > xdim) || (dimensionmode[screenpeek] == 2) ) {
          if( toggles[TOGGLE_SCORE] ) {
               if( score[screenpeek] == 1 ) {
                    sprintf(tektempbuf,"%08d", 0);
               }
               else {
                    sprintf(tektempbuf,"%08d", score[screenpeek]);
               }
               printext(xdim-160,ydim-12,tektempbuf,alphabet,255);
          }
          lastscore=0;
     }
     else {
          if( toggles[TOGGLE_SCORE] == 0 ) {
               if( (lastscoretoggle != toggles[TOGGLE_SCORE]) ) {
                    permanentwritesprite(xdim-160,ydim-12,SCORERESTORE,
                                         0,xdim-160,ydim-12,xdim-1,ydim-1,0);
               }
               lastscoretoggle=toggles[TOGGLE_SCORE];
               lastscore=0;
          }
          else {
               if( (score[screenpeek]==0) || (lastscore != score[screenpeek]) ) {
                    permanentwritesprite(xdim-160,ydim-12,SCORERESTORE,
                                         0,xdim-160,ydim-12,xdim-1,ydim-1,0);
                    if( score[screenpeek] == 1 ) {
                         sprintf(tektempbuf,"%08d", 0);
                    }
                    else {
                         sprintf(tektempbuf,"%08d", score[screenpeek]);
                    }
                    printext(xdim-160,ydim-12,tektempbuf,alphabet,255);
                    lastscore=score[screenpeek];
                    if( score[screenpeek]==0 )
                         score[screenpeek]=1;
               }
               lastscoretoggle=toggles[TOGGLE_SCORE];
          }
     }
}

void
showinv(int snum)
{
     char      ti=toggles[TOGGLE_INVENTORY];
     char      shade;

     if( (screensize < xdim) || (toggles[TOGGLE_INVENTORY] == 0) ) {
          goto skipsyms;
     }
     if( symbols[0] ) {
          shade=0;
          if( symbolsdeposited[0] )
               shade=32;
          rotatesprite((windowx2-(30*8))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL1PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }
     if( symbols[1] ) {
          shade=0;
          if( symbolsdeposited[1] )
               shade=32;
          rotatesprite((windowx2-(30*7))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL2PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }
     if( symbols[2] ) {
          shade=0;
          if( symbolsdeposited[2] )
               shade=32;
          rotatesprite((windowx2-(30*6))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL3PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }
     if( symbols[3] ) {
          shade=0;
          if( symbolsdeposited[3] )
               shade=32;
          rotatesprite((windowx2-(30*5))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL4PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }
     if( symbols[4] ) {
          shade=0;
          if( symbolsdeposited[4] )
               shade=32;
          rotatesprite((windowx2-(30*4))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL5PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }
     if( symbols[5] ) {
          shade=0;
          if( symbolsdeposited[5] )
               shade=32;
          rotatesprite((windowx2-(30*3))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL6PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }
     if( symbols[6] ) {
          shade=0;
          if( symbolsdeposited[6] )
               shade=32;
          rotatesprite((windowx2-(30*2))<<16, (windowy2-32)<<16, 65536l, 0, SYMBOL7PIC,
               shade, 0, 8+16, windowx1, windowy1, windowx2, windowy2);
     }

skipsyms:

     if( ti ) {
          timedinv--;
          if( timedinv == 0 )
               toggles[TOGGLE_INVENTORY]=0;
     }

     if( (windowx2 >= (xdim-24)) && ti ) {
          if( invbluecards[snum] != 0 ) {
               rotatesprite((xdim-24)<<16, ((ydim>>1)-14)<<16, 65536l, 0, 483,
                    0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          }
          if( invredcards[snum] != 0 ) {
               rotatesprite((xdim-24)<<16, ((ydim>>1)-2)<<16, 65536l, 0, 484,
                    0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          }
          if( invaccutrak[snum] != 0 ) {
               rotatesprite((xdim-24)<<16, ((ydim>>1)+10)<<16, 65536l, 0, 485,
                    0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          }
          lastinvtoggle=1;
          return;
     }
}

#define   NETWINSCORE    1200

void
nextnetlevel()
{
     int  i,j,len,other,readyplayers,playerreadyflag[MAXPLAYERS];
     int lastpacketclock,packets=0L;

     if( strcmp(boardfilename,"NET1.MAP") == 0) {
          strcpy(boardfilename,"NET2.MAP");
     }
     else if( strcmp(boardfilename,"NET2.MAP") == 0) {
          strcpy(boardfilename,"NET3.MAP");
     }
     else if( strcmp(boardfilename,"NET3.MAP") == 0) {
          strcpy(boardfilename,"NET4.MAP");
     }
     else if( strcmp(boardfilename,"NET4.MAP") == 0) {
          strcpy(boardfilename,"NET6.MAP");
     }
     else if( strcmp(boardfilename,"NET6.MAP") == 0) {
          strcpy(boardfilename,"NET7.MAP");
     }
     else if( strcmp(boardfilename,"NET7.MAP") == 0) {
          strcpy(boardfilename,"NET5.MAP");
     }
     else if( strcmp(boardfilename,"NET5.MAP") == 0) {
          strcpy(boardfilename,"NET1.MAP");
     }
     initpaletteshifts();
     prepareboard(boardfilename);
     precache();
     for( i=connecthead ; i >= 0 ; i=connectpoint2[i] ) {
          initplayersprite((short)i);
     }
//** Les START - 10/02/95
     for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
          playerreadyflag[i]=0;
     }
#if 0
     for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
          syncvel[i]=fsyncvel[i]=osyncvel[i]=0;
          syncsvel[i]=fsyncsvel[i]=osyncsvel[i]=0;
          syncangvel[i]=fsyncangvel[i]=osyncangvel[i]=0;
          syncbits[i]=fsyncbits[i]=osyncbits[i]=0;
          for (j=0 ; j < MOVEFIFOSIZ ; j++) {
               baksyncvel[j][i]=0;
               baksyncsvel[j][i]=0;
               baksyncangvel[j][i]=0;
               baksyncbits[j][i]=0;
          }
     }
     movefifoplc=movefifoend=0;
     syncvalplc=othersyncvalplc=0;
     syncvalend=othersyncvalend=0;
     syncvalcnt=othersyncvalcnt=0L;
     olocvel=olocvel2=0;
     olocsvel=olocsvel2=0;
     olocangvel=olocangvel2=0;
     olocbits=olocbits2=0;
#endif
     randomseed=17L;
     keystatus[0x01]=0;
     memset(playerreadyflag,0,sizeof(int)*MAXPLAYERS);
     readyplayers=0;
     if (myconnectindex == connecthead) {
          playerreadyflag[myconnectindex]=1;
          do {
               if ((len=getpacket(&other,tempbuf)) > 0) {
                    playerreadyflag[other]=1;
               }
               readyplayers=0;
               for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
                    if (playerreadyflag[i]) {
                         readyplayers++;
                    }
               }
               showmessage("%d OF %d PLAYERS READY..",readyplayers,numplayers);
               drawscreen(screenpeek,0L);
          } while (readyplayers < numplayers && keystatus[0x01] == 0);
          if (keystatus[0x01]) {
               keystatus[0x01]=0;
               crash("Multiplayer game aborted!");
          }
          if (dbgflag) {
               fprintf(dbgfp,"\nNEW LEVEL\n\n");
          }
     }
     else {
          showmessage("ONE MOMENT...");
          if (dbgflag) {
               fprintf(dbgfp,"\nNEW LEVEL\n\n");
          }
     }
//** Les END   - 10/02/95
}

#ifdef WONMESSAGE
void
postwonmessage(int nw)
{
     int       len;

     clearview();
     printext((xdim>>1)-40,(ydim>>1)-48,"TEK MASTER",ALPHABET2,255);
     len=strlen(netnames[nw]);
     len<<=2;
     sprintf(tektempbuf,"%s",netnames[nw]);
     printext((xdim>>1)-len,(ydim>>1)-32,tektempbuf,ALPHABET,255);
     nextpage();
     nextnetlevel();
}
#endif

void
netstats()
{
     int       icnt,i;

    #ifdef NETNAMES
     if( option[4] != 0 ) {
          icnt=connecthead;
          for( i=connecthead ; i >= 0 ; i=connectpoint2[i] ) {
               icnt=i;
              #if 0
               if( score[i] > score[newnetleader] ) {
                    if( score[newnetleader] < NETWINSCORE ) {
                         newnetleader=i;
                    }
               }
              #endif
               if( (toggles[TOGGLE_SCORE]) && (screensize >= xdim) ) {
                    sprintf(tektempbuf,"%2d %10s %6d",i,netnames[i],score[i]);
                    printext(12,(windowy1+32)+(i<<3),tektempbuf,ALPHABET,255);
               }
          }
         #if 0
          if( newnetleader != oldnetleader ) {
               oldnetleader=newnetleader;
          }
          else if( (totalclock-netclock) > 7200 ) {
               netclock=totalclock;
               sprintf(tektempbuf,"%2d %s LEADS",newnetleader,netnames[newnetleader],score[newnetleader]);
               showmessage(tektempbuf);
          }
         #endif
     }
    #endif
}

int
tekscreenfx(void)
{
     int  ammo,n;
     int i;
     static short hcpic,rvpic,wppic;

     updatepaletteshifts();
     updatesounds(screenpeek);

    //#define COMMITTEE
    #ifdef  COMMITTEE
     printext((xdim>>1)-25,windowy1+24,"THURS 6PM",ALPHABET,255);
    #endif

    #ifdef MATRIXTIMELIMIT
     if( mission == 7 ) {
          switch( minutes ) {
          case 1:
               if( (seconds == 0) && (fortieth < 10) ) {
                    showmessage("1 MINUTE LEFT");
                    playsound(S_BEEP,0,0,0,ST_IMMEDIATE);
               }
               break;
          case 2:
               if( (seconds == 0) && (fortieth < 10) ) {
                    showmessage("MATRIX TIMED OUT"); break;
                    playsound(S_ALARM3,0,0,0,ST_IMMEDIATE);
               }
               else if( seconds == 3 ) {
                    gameover=1;
               }
               break;
          }
     }
    #endif

     if( (messageon == 0) && notininventory ) {
          showmessage("NOT IN INVENTORY");
          notininventory=0;
     }
     if( otoggles[TOGGLE_REARVIEW] ) {
          if (rvpos < 0) {
               rvpos+=(TICSPERFRAME<<2);
               if (rvpos >= 0) {
                    rvpos=0;
                    rvpic=RVDEVICEON;
               }
               else {
                    n=tilesizx[RVDEVICE]/(RVDEVICEON-RVDEVICE);
                    n=(tilesizx[RVDEVICE]-abs(rvpos))/n;
                    rvpic=RVDEVICE+n;
               }
          }
          rotatesprite(rvpos<<16, 0<<16, 65536l, 0, rvpic,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          if (rvpos == 0) {
               rearview(screenpeek);
          }
     }
     else if (abs(rvpos) < tilesizx[RVDEVICE]) {
          rvpos-=(TICSPERFRAME<<2);
          if (abs(rvpos) > tilesizx[RVDEVICE]) {
               rvpos=-tilesizx[RVDEVICE];
               rvpic=RVDEVICE;
          }
          else {
               n=tilesizx[RVDEVICE]/(RVDEVICEON-RVDEVICE);
               n=(tilesizx[RVDEVICE]-abs(rvpos))/n;
               rvpic=RVDEVICE+n;
          }
          rotatesprite(rvpos<<16, 0<<16, 65536l, 0, rvpic,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
     }
     if( otoggles[TOGGLE_UPRT] ) {
          if (wppos < 0) {
               wppos+=(TICSPERFRAME<<2);
               if (wppos >= 0) {
                    wppos=0;
                    wppic=WPDEVICEON;
               }
               else {
                    n=tilesizx[WPDEVICE]/(WPDEVICEON-WPDEVICE);
                    n=(tilesizx[WPDEVICE]-abs(wppos))/n;
                    wppic=WPDEVICE+n;
               }
          }
          if (wppos == 0) {
               switch (locselectedgun+1) {
              #ifdef GUNINWPDEV
               case GUN1FLAG:
                    wppic=WPDEVICEGUN2;
                    break;
               case GUN2FLAG:
                    wppic=WPDEVICEGUN4;
                    break;
               case GUN3FLAG:
                    wppic=WPDEVICEGUN6;
                    break;
              #endif
               default:
                    wppic=WPDEVICEON;
                    break;
               }
          }
          rotatesprite((xdim-1-(tilesizx[WPDEVICE]+wppos))<<16, 0<<16, 65536l, 0, WPDEVICE,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
     }
     else if (abs(wppos) < tilesizx[WPDEVICE]) {
          wppos-=(TICSPERFRAME<<2);
          if (abs(wppos) > tilesizx[WPDEVICE]) {
               wppos=-tilesizx[WPDEVICE];
               wppic=WPDEVICE;
          }
          else {
               n=tilesizx[WPDEVICE]/(WPDEVICEON-WPDEVICE);
               n=(tilesizx[WPDEVICE]-abs(wppos))/n;
               wppic=WPDEVICE+n;
          }
          //overwritesprite(wppos,0,wppic,0,0,0);
          rotatesprite((xdim-1-(tilesizx[WPDEVICE]+wppos))<<16, 0<<16, 65536l, 0, WPDEVICE,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
     }
    #define  SCOREANDTIMEONWPDEVICE
    #ifdef   SCOREANDTIMEONWPDEVICE
     if( (wppic == WPDEVICEON) && (activemenu == 0) ) {
          sprintf(tektempbuf,"%02d:%02d:%02d", hours,minutes,seconds);
          printext(xdim-74,8,tektempbuf,ALPHABET,255);
          sprintf(tektempbuf,"%08d", score[screenpeek]);
          printext(xdim-74,18,tektempbuf,ALPHABET,255);
     }
    #endif
    #ifdef SCOREANDTIMEATBOTTOM
     if ((activemenu == 0) && !(biasthreshholdon)) {
          showtime();
          showscore();
     }
    #endif
     if( activemenu == 0 ) {
          showinv(screenpeek);
     }
     if( toggles[TOGGLE_GODMODE] && (screensize > xdim) ) {
           printext((xdim>>1)-16,4,"HOLY",ALPHABET2,255);
     }
     if (otoggles[TOGGLE_HEALTH]) {
          if (hcpos < 0) {
               hcpos+=(TICSPERFRAME<<2);
               if (hcpos >= 0) {
                    hcpos=0;
                    hcpic=HCDEVICEON;
               }
               else {
                    n=tilesizx[HCDEVICE]/(HCDEVICEON-HCDEVICE);
                    n=(tilesizx[HCDEVICE]-abs(hcpos))/n;
                    hcpic=HCDEVICE+n;
               }
          }
          rotatesprite(hcpos<<16, (ydim-tilesizy[hcpic])<<16, 65536l, 0, hcpic,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          if (hcpic == HCDEVICEON) {
               for (n=0 ; n < health[screenpeek]/HCSCALE ; n++) {
                    rotatesprite((hcpos+34+(n*5))<<16, (ydim-tilesizy[hcpic]+7)<<16, 65536l, 0, GREENLIGHTPIC,
                         (health[screenpeek]/HCSCALE)-n, 0, 8+16, 0, 0, xdim-1, ydim-1);
               }
               for (n=0 ; n < stun[screenpeek]/HCSCALE ; n++) {
                    rotatesprite((hcpos+34+(n*5))<<16, (ydim-tilesizy[hcpic]+13)<<16, 65536l, 0, YELLOWLIGHTPIC,
                         (stun[screenpeek]/HCSCALE)-n, 0, 8+16, 0, 0, xdim-1, ydim-1);
               }
               switch (locselectedgun) {
               case 0:
                    ammo=ammo1[screenpeek];
                    break;
               case 1:
                    ammo=ammo2[screenpeek];
                    break;
               case 2:
                    ammo=ammo3[screenpeek];
                    break;
               case 3:
                    ammo=ammo4[screenpeek];
                    break;
               case 4:
                    ammo=ammo5[screenpeek];
                    break;
               case 5:
                    ammo=ammo6[screenpeek];
                    break;
               case 6:
                    ammo=ammo7[screenpeek];
                    break;
               case 7:
                    ammo=ammo8[screenpeek];
                    break;
               default:
                    ammo=MAXAMMO;
                    break;
               }
               if( ammo > MAXAMMO ) {
                    ammo =MAXAMMO-1;
               }
               for (n=0 ; n < ammo/AMMOSCALE ; n++) {
                    rotatesprite((hcpos+34+(n*5))<<16, (ydim-tilesizy[hcpic]+19)<<16, 65536l, 0, BLUELIGHTPIC,
                         (ammo/AMMOSCALE)-n, 0, 8+16, 0, 0, xdim-1, ydim-1);
               }
          }
     }
     else if (abs(hcpos) < tilesizx[HCDEVICE]) {
          hcpos-=(TICSPERFRAME<<2);
          if (abs(hcpos) > tilesizx[HCDEVICE]) {
               hcpos=-tilesizx[HCDEVICE];
               hcpic=HCDEVICE;
          }
          else {
               n=tilesizx[HCDEVICE]/(HCDEVICEON-HCDEVICE);
               n=(tilesizx[HCDEVICE]-abs(hcpos))/n;
               hcpic=HCDEVICE+n;
          }
          rotatesprite(hcpos<<16, (ydim-tilesizy[hcpic])<<16, 65536l, 0, hcpic,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
     }

    #ifdef FRAMECNT
     i=totalclock;
     if (i != frameval[framecnt]) {
          sprintf(tempbuf,"%ld",(CLKIPS*AVERAGEFRAMES)/(i-frameval[framecnt]));
          printext256(windowx1,windowy1,31,-1,tempbuf,1);
          frameval[framecnt]=i;
     }
     framecnt=((framecnt+1)&(AVERAGEFRAMES-1));
    #endif

     if( (activemenu == 0) && (option[4] != 0) ) {
          netstats();
     }

     if( biasthreshholdon ) {
          sprintf((char *)tempbuf,"SET BIAS THRESHHOLD %3d", biasthreshhold);
          printext((xdim>>1)-96,windowy2-10,(char *)tempbuf,ALPHABET2,255);
     }
     else if( (activemenu == 0) && messageon ) {
          if( messagex > windowx1 )
                printext(messagex,windowy2-32,messagebuf,ALPHABET2,255);
     }
     if( (activemenu == 0) && (toggles[TOGGLE_HEALTH] == 0) && (hcpos == -tilesizx[HCDEVICE]) && (screensize > 140) ) {
          if (!(biasthreshholdon)) {
               if( health[screenpeek] < 0 ) {
                    sprintf(tektempbuf,"%4d", 0);
               }
               else if( health[screenpeek] > MAXHEALTH ) {
                    sprintf(tektempbuf,"%4d", 1000);
               }
               else {
                    sprintf(tektempbuf,"%4d", health[screenpeek]);
               }
               printext(windowx1+6,windowy2-10,tektempbuf,ALPHABET2,255);
          }
     }
     if (activemenu) {
          domenu();
     }

    return 0;
}

void
tektime(void)
{
     fortieth++;
     if( fortieth == 40 ) {
          fortieth=0;
          seconds++;
     }
     if( seconds == 60 ) {
          minutes++;
          seconds=0;
     }
     if( minutes == 60 ) {
          hours++;
          minutes=0;
     }
     if( hours > 99 ) {
          hours=0;
     }
     if( messageon ) {
          messageon++;
          if( messageon == 160 ) {
               messageon=0;
          }
     }
}

void
initmenu(void)
{
     gameover=0;
     activemenu=0;
}

void
newgame(char *mapname)
{
     int  i;

     if( option[4] != 0 ) {
          return;
     }

     ready2send=0;

     stopallsounds();
     if (strcmp(boardfilename,mapname) != 0) {
          strcpy(boardfilename,mapname);
     }
     initpaletteshifts();
     prepareboard(boardfilename);
     precache();
     for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
          initplayersprite((short)i);
     }

     ready2send=1;
}

char *mapnames[TOTALMAPS] = {
     "subway0.map",    // 0
     "subway1.map",    // 1
     "subway2.map",    // 2
     "subway3.map",    // 3
     "level1.map",     // 4
     "level2.map",     // 5
     "",               // 6
     "",               // 7
     "city1.map",      // 8
     "",               // 9
     "",               // 10
     "beach1.map",     // 11
     "park1.map",      // 12
     "",               // 13
     "",               // 14
     "mid1.map",       // 15
     "mid2.map",       // 16
     "mid3.map",       // 17
     "sewer1.map",     // 18
     "sewer2.map",     // 19
     "inds1.map",      // 20
     "",               // 21
     "free1.map",      // 22
     "free2.map",      // 23
     "",               // 24
     "ware1.map",      // 25
     "ware2.map",      // 26
     "ware3.map",      // 27
     "",               // 28
     "",               // 29
     "final1.map",     // 30
     ""                // 31
};

#define   TOTALMISSIONS       7
#define   MAXMAPSPERMISSION   6
int       missionset[TOTALMISSIONS][MAXMAPSPERMISSION] = {
     //SUB  M1 M2 M3 M4 MTRX
     {  0,  4, 5,-1,10, -1 },    // mission 0
     {  0,  8,-1,-1,-1, -1 },    // mission 1
     {  1, 11,12,-1,-1, -1 },    // mission 2
     {  1, 15,16,17,-1, -1 },    // mission 3
     {  2, 18,19,20,-1, -1 },    // mission 4
     {  2, 22,23,-1,-1, -1 },    // mission 5
     {  3, 25,26,27,-1, -1 },    // mission 6
};

int
accessiblemap(int mn)
{
     int       i;

     if( (mn < 0) || (mn >= TOTALMAPS) ) {
          return(0);
     }
     if( strlen(mapnames[mn]) < 2 ) {
          return(0);
     }
     for( i=0; i<MAXMAPSPERMISSION; i++ ) {
          if( missionset[mission][i] == mn ) {
               return(1);
          }
     }
     return(0);
}

int
mapreturn(int cmap)
{
/*
     "level1.map",     // 4
     "level2.map",     // 5
     "city1.map",      // 8

     "beach1.map",     // 11
     "park1.map",      // 12
     "mid1.map",       // 15
     "mid2.map",       // 16
     "mid3.map",       // 17

     "sewer1.map",     // 18
     "sewer2.map",     // 19
     "inds1.map",      // 20
     "free1.map",      // 22
     "free2.map",      // 23

     "ware1.map",      // 25
     "ware2.map",      // 26
     "ware3.map",      // 27
*/
     int       rv;

     switch( cmap ) {
     case 4:  rv=0; warpretx=-27200; warprety=21500; warpretang= 512; warpretsect=  351; break;
     case 5:  rv=0; warpretx=  7295; warprety=21500; warpretang= 512; warpretsect=  349; break;
     case 8:  rv=0; warpretx=-11902; warprety=39300; warpretang=1536; warpretsect=  353; break;
     case 11: rv=1; warpretx=-27200; warprety=21500; warpretang= 512; warpretsect=  489; break;
     case 12: rv=1; warpretx=  7295; warprety=21500; warpretang= 512; warpretsect=  492; break;
     case 15: rv=1; warpretx=-11904; warprety=39300; warpretang=1536; warpretsect=  488; break;
     case 16: rv=1; warpretx= 24322; warprety=39300; warpretang=1536; warpretsect=  486; break;
     case 17: rv=1; warpretx= 57346; warprety=39300; warpretang=1536; warpretsect=  483; break;
     case 18: rv=2; warpretx=-27200; warprety=21500; warpretang= 512; warpretsect=  486; break;
     case 19: rv=2; warpretx=  7295; warprety=21500; warpretang= 512; warpretsect=  488; break;
     case 20: rv=2; warpretx= 41600; warprety=21500; warpretang= 512; warpretsect=  490; break;
     case 22: rv=2; warpretx=-11904; warprety=39300; warpretang=1536; warpretsect=  483; break;
     case 23: rv=2; warpretx= 24380; warprety=39300; warpretang=1536; warpretsect=  482; break;
     case 25: rv=3; warpretx=-27200; warprety=21500; warpretang= 512; warpretsect=  477; break;
     case 26: rv=3; warpretx=  7295; warprety=21500; warpretang= 512; warpretsect=  472; break;
     case 27: rv=3; warpretx= 41600; warprety=21500; warpretang= 512; warpretsect=  494; break;
     default: rv=-1;break;
     }

     return(rv);
}

void
newmap(int mapno)
{
     int       i,sn,p;
     int       savhealth;
     int       savestun;
     int       savereds;
     int       saveblues;
     int       saveaccutrk;
     int       newmap;

     if( option[4] != 0 ) {
          return;
     }
     if( mapno >= TOTALMAPS ) {
          return;
     }

     switch( mapno ) {
     case 0:
     case 1:
     case 2:
     case 3:
          newmap=mapreturn(currentmapno);
          if( newmap != mapno ) {
               crash("bad return map");
          }
          break;
     }

     drawscreen(screenpeek,0);
     printext((xdim>>1)-44,(ydim>>1)-32,"LOADING MAP",ALPHABET2,255);
     nextpage();

     initpaletteshifts();
     p=screenpeek;

    #if 0
     switch( currentmapno ) {
     case 0:
     case 1:
     case 2:
     case 3:
          warpretx=posx[p];
          warprety=posy[p];
          warpretz=posz[p];
          warpretang=ang[p];
          warpretsect=cursectnum[p];
          break;
     default:
          break;
     }
    #endif

     savhealth=health[p];
     savestun=stun[p];
     savereds=invredcards[p];
     saveblues=invbluecards[p];
     saveaccutrk=invaccutrak[p];

     newgame(mapnames[mapno]);

     health[p]=savhealth;
     stun[p]=savestun;
     invredcards[p]=savereds;
     invbluecards[p]=saveblues;
     invaccutrak[p]=saveaccutrk;

    #if 0
     switch( mapno ) {
     case 0:
     case 1:
     case 2:
     case 3:
          sn=playersprite[p];
          sprite[sn].x=warpretx;
          sprite[sn].y=warprety;
          sprite[sn].z=warpretz;
          changespritesect(sn,warpretsect);
          posx[p]=sprite[sn].x;
          posy[p]=sprite[sn].y;
          posz[p]=sprite[sn].z;
          ang[p]=((warpretang+1024)&2047);
          cursectnum[p]=sprite[sn].sectnum;
          updatesector(posx[p],posy[p],cursectnum[p]);
          break;
     default:
          break;
     }
    #endif

     switch( mapno ) {
     case 0:
     case 1:
     case 2:
     case 3:
          sn=playersprite[p];

          sprite[sn].x=warpretx;
          sprite[sn].y=warprety;
          sprite[sn].ang=warpretang;
          copybuf(&sprite[sn].x,&osprite[sn].x,3);
          changespritesect(sn,warpretsect);

          posx[p]=sprite[sn].x;
          posy[p]=sprite[sn].y;
          ang[p]=sprite[sn].ang;
          cursectnum[p]=sprite[sn].sectnum;

          //updatesector(posx[p],posy[p],cursectnum[p]);
          //pushmove(&posx[p],&posy[p],&posz[p],&cursectnum[p],128L,4<<8,4<<8,CLIPMASK0);
          break;
     default:
          break;
     }

     vel=0;
     svel=0;
     angvel=0;

     currentmapno=mapno;
     strcpy(tektempbuf, mapnames[mapno]);
     showmessage(Bstrupr(tektempbuf));

     musicfade();
     if( mapno <= 3 ) {
          menusong(1);
     }
     else {
          startmusic(mission);
     }

}

void
getloadsavenames(void)
{
     int  fil,i;

     for (i=0 ; i < MAXLOADSAVEOPTS ; i++) {
          sprintf((char *)tempbuf,"savegam%d.tek",i+1);
          if (access((char *)tempbuf,F_OK) == 0) {
               fil=open((char *)tempbuf,O_BINARY|O_RDONLY,S_IREAD);
               read(fil,&loadsavenames[i],MAXLOADSAVESIZE);
               close(fil);
          }
          else {
               strncpy(loadsavenames[i],"-EMPTY-",MAXLOADSAVESIZE);
          }
     }
}

void
mprintf(short x,short y,char prop,char shade,char palnum,char *stg,...)
{
     int  i,n,pic,propx;
     va_list vargs;

     va_start(vargs,stg);
     vsprintf((char *)tempbuf,stg,vargs);
     va_end(vargs);
     Bstrupr((char *)tempbuf);
     n=strlen((char *)tempbuf);
     if (x == -1) {
          if (prop) {
               pic=MFONT_A;
               propx=0;
               for (i=0 ; i < n ; i++) {
                    if (isalpha(tempbuf[i])) {
                         pic=MFONT_A+tempbuf[i]-'A';
                    }
                    else if (isdigit(tempbuf[i])) {
                         pic=MFONT_0+tempbuf[i]-'0';
                    }
                    else if (tempbuf[i] > ' ' && tempbuf[i] < '0') {
                         pic=MFONT_SPECIAL1+tempbuf[i]-'!';
                    }
                    propx+=tilesizx[pic];
               }
               x=(xdim>>1)-(propx>>1);
          }
          else {
               x=(xdim>>1)-(n<<3);
          }
     }
     if (y == -1) {
          y=(ydim>>1)-(MFONTYSTEP>>1);
     }
     else {
          y=MFONTYSTEP*y;
          if (ydim >= 400) {
               y<<=1;
          }
     }
     pic=MFONT_A;
     for (i=0 ; i < n ; i++) {
          if (isalpha(tempbuf[i])) {
               pic=MFONT_A+tempbuf[i]-'A';
               rotatesprite(x<<16, y<<16, 65536l, 0, pic, shade, palnum, 8+16, 0, 0, xdim-1, ydim-1);
          }
          else if (isdigit(tempbuf[i])) {
               pic=MFONT_0+tempbuf[i]-'0';
               rotatesprite(x<<16, y<<16, 65536l, 0, pic, shade, palnum, 8+16, 0, 0, xdim-1, ydim-1);
          }
          else if (tempbuf[i] > ' ' && tempbuf[i] < '0') {
               pic=MFONT_SPECIAL1+tempbuf[i]-'!';
               rotatesprite(x<<16, y<<16, 65536l, 0, pic, shade, palnum, 8+16, 0, 0, xdim-1, ydim-1);
          }
          if (prop) {
               x+=tilesizx[pic];
          }
          else {
               if (tempbuf[i] == ' ') {
                    x+=8;
               }
               else {
                    x+=16;
               }
          }
     }
}

void
showmenu(void)
{
    if (xdim < 640) {
        rotatesprite(160<<16,100<<16, 65536, 0, MENUSTATION, 0, 0, 2+8, 0, 0, xdim-1, ydim-1);
        rotatesprite(160<<16,100<<16, 65536, 0, MENUGLASS, 0, 0, 1+2+8, 0, 0, xdim-1, ydim-1);
    } else {
        rotatesprite(160<<16,50<<16, divscale15(200, tilesizy[MENUPANEL4801]), 0, MENUPANEL4801, 0, 0, 2+8, 0, 0, xdim-1, ydim-1);
        rotatesprite(160<<16,150<<16, divscale15(200, tilesizy[MENUPANEL4802]), 0, MENUPANEL4802, 0, 0, 2+8, 0, 0, xdim-1, ydim-1);
    }
}

void
showhelpscreen(void)
{
    if (xdim < 640) {
        rotatesprite(160<<16,160<<16, 65536, 0, HELPSCREENPIC, 0, 0, 2+8, 0, 0, xdim-1, ydim-1);
    } else {
        rotatesprite(160<<16,50<<16, divscale15(200, tilesizy[HELPSCREEN4801]), 0, HELPSCREEN4801, 0, 0, 2+8, 0, 0, xdim-1, ydim-1);
        rotatesprite(160<<16,150<<16, divscale15(200, tilesizy[HELPSCREEN4802]), 0, HELPSCREEN4802, 0, 0, 2+8, 0, 0, xdim-1, ydim-1);
    }
}

void
domenu(void)
{
     char pal;
     int  i,stepy;
     int dax,dax2,day,day2;
     struct menu *mptr;
     static int firstpass,odiff,osoundv,omusicv,omousesens,oheadb;

     if (activemenu == HELPSCREEN) {
          showhelpscreen();
          return;
     }
     if (firstpass == 0) {
          firstpass=1;
          getloadsavenames();
          odiff=difficulty;
          osoundv=soundv;
          omusicv=musicv;
          omousesens=mousesensitivity;
          oheadb=headbobon;
          if (headbobon) {
               strcpy(headbobstr,"HEAD BOB ON");
          }
          else {
               strcpy(headbobstr,"HEAD BOB OFF");
          }
     }
     if (activemenu == 255) {
         #ifdef DYNAMICSAVESETUP
          if (odiff != difficulty || osoundv != soundv
             || omusicv != musicv || omousesens != mousesensitivity
             || oheadb != headbobon) {
               teksavesetup();
          }
         #endif
          playsound( S_MENUSOUND2 ,0,0,0,ST_IMMEDIATE);
          setup3dscreen();
          activemenu=0;
          firstpass=0;
          return;
     }
     vel=svel=angvel=0;
     mptr=&menu[activemenu][0];
     if (redrawborders) {
          dax=windowx1;
          dax2=windowx2;
          day=windowy1;
          day2=windowy2;
          if (dax2-dax < xdim-1 || day2-day < ydim-1) {
               setup3dscreen();
          }
          redrawborders=0;
     }
     if (selopt == 0) {
          switch (activemenu) {
          case MENULOADGAME:
          case MENUSAVEGAME:
               selopt=1;
               break;
          }
     }
     showmenu();
     for (i=0 ; i < MAXSUBOPTIONS ; i++) {
          if (mptr->label != NULL) {
               if (selopt == i) {
                    pal=4;
               }
               else {
                    pal=mptr->pal;
               }
               mprintf(mptr->x,mptr->y,mptr->proportional,mptr->shade,pal,
                    mptr->label);
               if (mptr->nextopt != NULL) {
                    mptr=mptr->nextopt;
               }
               else {
                    break;
               }
          }
     }
     if (activemenu == 5 && loadnewgame) {   // select a mission
          switch (loadnewgame) {
          case 1:   // day mission
               newgame("city.map");
               break;
          case 2:   // night mission
               newgame("hospital.map");
               break;
          case 3:
               newgame("police.map");
               break;
          case 4:
               newgame("warehse.map");
               break;
          }
          loadnewgame=0;
          activemenu=255;
     }
     if (activemenu == 6 || activemenu == 7) {    // load/save a game
          dax=(xdim>>1)-((MAXLOADSAVESIZE*tilesizx[MFONT_A])>>1);
          dax-=tilesizx[MFONT_A];
          for (i=1 ; i <= MAXLOADSAVEOPTS ; i++) {
               if (selopt == i) {
                    pal=4;
                    if (loctypemode == 0) {
                         mprintf(dax,2+i,0,0,pal,"%d %s",i,loadsavenames[i-1]);
                    }
                    else {
                         mprintf(dax,2+i,0,0,pal,"%d %s",i,lockeybuf);
                         if (totalclock > curblinkclock) {
                              curblinkclock=totalclock+(CLKIPS>>2);
                              curblink^=1;
                         }
                         if (curblink) {
                              day=((2+i)*MFONTYSTEP)+3;
                              if (ydim >= 400) {
                                   day<<=1;
                              }
                              rotatesprite((dax+((strlen(lockeybuf)+1)*16)+5)<<16, day<<16, 65536l, 0, YELLOWLIGHTPIC,
                                   0, 0, 8+16, 0, 0, xdim-1, ydim-1);
                         }
                    }
               }
               else {
                    pal=1;
                    mprintf(dax,2+i,0,0,pal,"%d %s",i,loadsavenames[i-1]);
               }
          }
     }
     else if (activemenu == 8) {   // sound/music volume
          dax=(xdim>>1)-(tilesizx[SLIDERBARPIC]>>1);
          day=50;
          if (ydim >= 400) {
               day<<=1;
          }
          rotatesprite(dax<<16, day<<16, 65536l, 0, SLIDERBARPIC,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          rotatesprite((dax+10+(soundv*7))<<16, day<<16, 65536l, 0, SLIDERKNOBPIC,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          day=86;
          if (ydim >= 400) {
               day<<=1;
          }
          rotatesprite(dax<<16, day<<16, 65536l, 0, SLIDERBARPIC,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          rotatesprite((dax+10+(musicv*7))<<16, day<<16, 65536l, 0, SLIDERKNOBPIC,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
     }
     else if (activemenu == 9) {   // mouse sensitivity
          dax=(xdim>>1)-(tilesizx[SLIDERBARPIC]>>1);
          day=26;
          if (ydim >= 400) {
               day<<=1;
          }
          rotatesprite(dax<<16, day<<16, 65536l, 0, SLIDERBARPIC,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
          dax+=(10+(mousesensitivity*7));
          rotatesprite(dax<<16, day<<16, 65536l, 0, SLIDERKNOBPIC,
               0, 0, 8+16, 0, 0, xdim-1, ydim-1);
     }
}

void
domenuinput(void)
{
     int c,keystate;
     int  tries;
     struct menu *mptr = 0;

     if (activemenu == 255) {
          return;
     }
     if (activemenu == HELPSCREEN) {
          if (keystatus[1]) {
               keystatus[1]=0;
               playsound( S_BEEP ,0,0,0,ST_IMMEDIATE);
               if (menudirect) {
                    activemenu=255;
                    menudirect=0;
               }
          }
          return;
     }
     if (loctypemode) {
          while (keyfifoplc != keyfifoend) {
               c=keyfifo[keyfifoplc];
               keystate=keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
               keyfifoplc=((keyfifoplc+2)&(KEYFIFOSIZ-1));
               if (keystate != 0) {
                    if (c == 0x01) {              // ESC key
                         keystatus[1]=0;
                         locmessagelen=0;
                         loctypemode=0;
                    }
                    if (c == 0x0E) {              // backspace key
                         keystatus[14]=0;
                         if (locmessagelen == 0) {
                              break;
                         }
                         locmessagelen--;
                         lockeybuf[locmessagelen]=0;
                    }
                    if (c == 0x1C || c == 0x9C) { // enter keys
                         keystatus[0x1C]=keystatus[0x9C]=0;
                         if (locmessagelen > 0) {
                              strncpy(loadsavenames[selopt-1],lockeybuf,
                                   MAXLOADSAVESIZE);
                              locmessagelen=0;
                         }
                         loctypemode=0;
                         if( option[4] == 0 ) {
                              savegame(selopt);
                         }
                         activemenu=255;
                         break;
                    }
                    if (locmessagelen < (MAXLOADSAVESIZE-1) && c < 128) {
                         if (keystatus[0x2A] || keystatus[0x36]) {
                              c=scantoascwithshift[c];
                         }
                         else {
                              c=scantoasc[c];
                         }
                         if (c != 0) {
                              if (isalpha(c) || isdigit(c) || c == ' ') {
                                   lockeybuf[locmessagelen++]=toupper(c);
                                   lockeybuf[locmessagelen]=0;
                              }
                         }
                    }
               }
          }
          return;
     }
     mptr=&menu[activemenu][selopt];
     if (keystatus[1]) {
          keystatus[1]=0;
          playsound( S_BEEP ,0,0,0,ST_IMMEDIATE);
          if (menudirect) {
               activemenu=255;
               menudirect=0;
          }
          else {
               activemenu=mptr->backmenu;
          }
          selopt=lastselopt[activemenu];
          if (activemenu == 0) {
               activemenu=255;
          }
     }
     else if (keystatus[keys[0]]) {     // up arrow
          keystatus[keys[0]]=0;
          playsound( S_BOOP ,0,0,0,ST_IMMEDIATE);
          tries=0;
          while (tries < MAXSUBOPTIONS) {
               selopt--;
               if (selopt <= 0) {
                    selopt=5;
               }
               lastselopt[activemenu]=selopt;
               if (menu[activemenu][selopt].label != NULL) {
                    break;
               }
               tries++;
          }
     }
     else if (keystatus[keys[1]]) {     // down arrow
          keystatus[keys[1]]=0;
          playsound( S_BOOP ,0,0,0,ST_IMMEDIATE);
          tries=0;
          while (tries < MAXSUBOPTIONS) {
               selopt++;
               if (selopt > 5) {
                    selopt=1;
               }
               lastselopt[activemenu]=selopt;
               if (menu[activemenu][selopt].label != NULL) {
                    break;
               }
               tries++;
          }
     }
     else if (activemenu == 4) {
          if (keystatus[21] != 0) {                    // "Y" key
               switch (activemenu) {
               case 4:
                    gameover=1;
                    //playsound( ??? ,0,0,0,ST_IMMEDIATE);
                    break;
               case 5:
                    //endgame=1;
                    activemenu=255;
                    break;
               }
          }
          else if (keystatus[49] != 0) {               // "N" key
               activemenu=255;
          }
     }
     else if (keystatus[0x1C] || keystatus[0x9C]) {    // enter key
          keystatus[0x1C]=keystatus[0x9C]=0;
          playsound( S_MENUSOUND2 ,0,0,0,ST_IMMEDIATE);
          if (mptr->tomenu != 0) {
               activemenu=mptr->tomenu;
               selopt=lastselopt[activemenu];
               redrawborders=1;
          }
          else {
               switch (activemenu) {
               case 1:
                    break;
               case 2:
                    difficulty=selopt;
                    newgame(boardfilename);
                    activemenu=255;
                    break;
               case 3:
                    if (selopt == 3) {
                         if (strcmp(headbobstr,"HEAD BOB ON") == 0) {
                              strcpy(headbobstr,"HEAD BOB OFF");
                              headbobon=0;
                         }
                         else {
                              strcpy(headbobstr,"HEAD BOB ON");
                              headbobon=1;
                         }
                    }
                    break;
               case 5:                                 // day/night mission
                    loadnewgame=selopt;
                    break;
               case 6:                                 // load game
                    stopallsounds();
                    if( option[4] == 0 ) {
                         loadgame(selopt);
                    }
                    activemenu=255;
                    break;
               case 7:                                 // save game
                    keystatus[0x1C]=keystatus[0x9C]=0;
                    loctypemode=1;
                    keyfifoplc=keyfifoend;
                    if (strcmp(loadsavenames[selopt-1],"-EMPTY-") != 0) {
                         strncpy(lockeybuf,loadsavenames[selopt-1],
                              MAXLOADSAVESIZE);
                         locmessagelen=strlen(lockeybuf);
                    }
                    else {
                         memset(lockeybuf,0,sizeof(lockeybuf));
                         locmessagelen=0;
                    }
                    break;
               }
          }
     }
     else if (activemenu == 8 || activemenu == 9) {
          if (keystatus[0x4B] || keystatus[0xCB]) {    // left arrow key
               keystatus[0x4B]=keystatus[0xCB]=0;
               switch (selopt) {
               case 1:
                    if (activemenu == 8) {
                         if (soundv > 0) {
                              soundv--;
                              soundmastervolume(soundv<<11);
                         }
                    }
                    else {
                         if (mousesensitivity > 0) {
                              mousesensitivity--;
                         }
                    }
                    break;
               case 2:
                    if (musicv > 0) {
                         musicv--;
                         songmastervolume(musicv<<3);
                    }
                    break;
               }
               playsound( S_MENUSOUND1 ,0,0,0,ST_IMMEDIATE);
          }
          else if (keystatus[0x4D] || keystatus[0xCD]) {    // right arrow key
               keystatus[0x4D]=keystatus[0xCD]=0;
               switch (selopt) {
               case 1:
                    if (activemenu == 8) {
                         if (soundv < 16) {
                              soundv++;
                              soundmastervolume(soundv<<11);
                         }
                    }
                    else {
                         if (mousesensitivity < 16) {
                              mousesensitivity++;
                         }
                    }
                    break;
               case 2:
                    if (musicv < 16) {
                         musicv++;
                         songmastervolume(musicv<<3);
                    }
                    break;
               }
               playsound( S_MENUSOUND1 ,0,0,0,ST_IMMEDIATE);
          }
     }
     else if (keystatus[16]) {
          keystatus[16]=0;
          gameover=1;
     }
}

void
rearview(int snum)
{
     int      cposx, cposy, cposz, choriz, czoom;
     short     cang;
     short     plrang,plrhoriz;
     int       oldwx1,oldwx2,oldwy1,oldwy2;

     if( toggles[TOGGLE_REARVIEW] == 0 ) {
          return;
     }

     oldwx1=windowx1; oldwx2=windowx2;
     oldwy1=windowy1; oldwy2=windowy2;

     plrang=ang[snum];
     plrhoriz=ohoriz[snum];
     setview(67,9,130,40);

     oang[snum]=(plrang+1024)&2047;
     ohoriz[snum]=(200-plrhoriz);

     cposx = oposx[snum]+mulscale(posx[snum]-oposx[snum],0,16);
     cposy = oposy[snum]+mulscale(posy[snum]-oposy[snum],0,16);
     cposz = oposz[snum]+mulscale(posz[snum]-oposz[snum],0,16);
     if (frameinterpolate == 0)
          { cposx = posx[snum]; cposy = posy[snum]; cposz = posz[snum]; }
     choriz = ohoriz[snum]+mulscale(horiz[snum]-ohoriz[snum],0,16);
     czoom = ozoom[snum]+mulscale(zoom[snum]-ozoom[snum],0,16);
     cang = oang[snum]+mulscale(((ang[snum]+1024-oang[snum])&2047)-1024,0,16);

     drawrooms(cposx,cposy,cposz,cang,choriz,cursectnum[snum]);
     rearviewdraw=1;
     analyzesprites(posx[snum],posy[snum]);
     rearviewdraw=0;
     drawmasks();

     oang[snum]=plrang;
     ohoriz[snum]=plrhoriz;
     setview(oldwx1,oldwy1, oldwx2,oldwy2);
}

void
usage()
{
     printf("\ntekwar [option1] [option2] .... [optionN]\n");
     printf("options:    practice\n");
     printf("            nogore\n");
     printf("            nobriefs\n");
     printf("            netname [NAME]\n");
     printf("            noenemies\n");
}

void
tekargv(int argc, char const * const argv[])
{
     int       p;
     char      argmatch=0;

     if( (argc >= 2) ) {
          if( strchr(argv[1],'.') != 0 ) {
               strcpy(boardfilename,argv[1]);
               if( (strcmp(boardfilename,"matrix.map") == 0) || (strcmp(boardfilename,"MATRIX.MAP") == 0) ) {
                    mission=7;
               }
               singlemapmode=1;
               nobriefflag=1;
          }
     }
     else {
          strcpy(boardfilename,"subway0.map");
     }

     goreflag=1;

     for( p=1 ; p < argc ; p++ ) {
          if (strcasecmp(argv[p],"PRACTICE") == 0) {
               generalplay=1;
               argmatch++;
          }
          if (strcasecmp(argv[p],"NOVIDEOID") == 0) {
               novideoid=1;
               argmatch++;
          }
          if (strcasecmp(argv[p],"NETNAME") == 0) {
               bypasscdcheck=1;
               if( (p+1) < argc ) {
                    memset(localname,0,sizeof(localname));
                    strncpy(localname,argv[p+1],sizeof(localname)-1);
                    Bstrupr(localname);
               }
               argmatch++;
          }
          if (strcasecmp(argv[p],"NOGORE") == 0) {
               argmatch++;
               goreflag=0;
          }
          if (strcasecmp(argv[p],"NOENEMIES") == 0) {
               argmatch++;
               noenemiesflag=1;
          }
          if (strcasecmp(argv[p],"NOGUARD") == 0) {
               argmatch++;
               noguardflag=1;
          }
          if (strcasecmp(argv[p],"NOSTALK") == 0) {
               argmatch++;
               nostalkflag=1;
          }
          if (strcasecmp(argv[p],"NOCHASE") == 0) {
               argmatch++;
               nochaseflag=1;
          }
          if (strcasecmp(argv[p],"NOSTROLL") == 0) {
               argmatch++;
               nostrollflag=1;
          }
          if (strcasecmp(argv[p],"DIGILOOPS") == 0) {
               argmatch++;
               digiloopflag=1;
          }
          if (strcasecmp(argv[p],"NOBRIEFS") == 0) {
               argmatch++;
               nobriefflag=1;
          }
          if (strcasecmp(argv[p],"DEBUG") == 0) {
               argmatch++;
               dbgflag=1;
          }
          if (strcasecmp(argv[p],"COOP") == 0) {
               argmatch++;
               coopmode=1;
          }
          if (strcasecmp(argv[p],"SWITCHLEVELS") == 0) {
               argmatch++;
               switchlevelsflag=1;
          }
     }

     if( singlemapmode ) {
          if( access(boardfilename, F_OK) != 0 ) {
               printf("\ncant find %s\n", boardfilename);
               exit(-1);
          }
     }

     if( (argmatch == 0) && (singlemapmode != 1) && (argc > 1) ) {
          usage();
          exit(-1);
     }
}

void
tekloadmoreoptions(int fil)
{
     int       rv;

     rv=read(fil,&moreoptions[0],MAXMOREOPTIONS);
     rv=read(fil,&toggles[0],MAXTOGGLES);
     rv=read(fil,&gamestuff[0],MAXGAMESTUFF<<2);

     toggles[TOGGLE_GODMODE]=0;
     if (toggles[TOGGLE_REARVIEW]) {
          rvmoving=1;
     }
     if (toggles[TOGGLE_UPRT]) {
          wpmoving=1;
     }
     if (toggles[TOGGLE_HEALTH]) {
          hcmoving=1;
     }
}

void
initmoreoptions()
{
     difficulty=moreoptions[8];
     soundv=moreoptions[9];
     musicv=moreoptions[10];
     mousesensitivity=moreoptions[11];
     headbobon=moreoptions[12];

     screensize=gamestuff[2];
     brightness=gamestuff[3];
     biasthreshhold=gamestuff[4];

     if( option[4] != 0 ) {
          difficulty=2;
     }
}

void
teksavemoreoptions(int fil)
{
     moreoptions[8]=difficulty;
     moreoptions[9]=soundv;
     moreoptions[10]=musicv;
     moreoptions[11]=mousesensitivity;
     moreoptions[12]=headbobon;

     gamestuff[0]=-1;
     gamestuff[1]=-1;
     gamestuff[2]=( int)screensize;
     gamestuff[3]=brightness;
     gamestuff[4]=biasthreshhold;

     write(fil,&moreoptions[0],MAXMOREOPTIONS);
     write(fil,&toggles[0],MAXTOGGLES);
     write(fil,&gamestuff[0],MAXGAMESTUFF<<2);
}

void
tekendscreen()
{
     if( demowon )
         return;

     memset(keystatus, 0, sizeof(keystatus));
     if( xdim == 640 ) {
          setview(0L,0L,xdim-1,ydim-1);
          loadtile(ES1A_SVGA);
          overwritesprite(0L,0L,ES1A_SVGA,0,0,0);
          loadtile(ES1B_SVGA);
          overwritesprite(0L,239L,ES1B_SVGA,0,0,0);
          nextpage();
          fadein(0,255,50);
          while( (keystatus[1] == 0) && (keystatus[57] == 0) && (keystatus[28] == 0) ) {
          }
          memset(keystatus, 0, sizeof(keystatus));
          loadtile(ES2A_SVGA);
          overwritesprite(0L,0L,ES2A_SVGA,0,0,0);
          loadtile(ES2B_SVGA);
          overwritesprite(0L,239L,ES2B_SVGA,0,0,0);
          nextpage();
          while( (keystatus[1] == 0) && (keystatus[57] == 0) && (keystatus[28] == 0) ) {
          }
     }
     else if( xdim == 320 ) {
          setview(0L,0L,xdim-1,ydim-1);
          loadtile(ES1_VGA);
          overwritesprite(0L,0L,ES1_VGA,0,0,0);
          nextpage();
          fadein(0,255,50);
          while( (keystatus[1] == 0) && (keystatus[57] == 0) && (keystatus[28] == 0) ) {
          }
          memset(keystatus, 0, sizeof(keystatus));
          loadtile(ES2_VGA);
          overwritesprite(0L,0L,ES2_VGA,0,0,0);
          nextpage();
          while( (keystatus[1] == 0) && (keystatus[57] == 0) && (keystatus[28] == 0) ) {
          }
     }
     else {
          setview(0L,0L,xdim-1,ydim-1);
          loadtile(ES1_VGA);
          overwritesprite(0,0,ES1_VGA,0,0x02,0);
          nextpage();
          fadein(0,255,50);
          while( (keystatus[1] == 0) && (keystatus[57] == 0) && (keystatus[28] == 0) ) {
          }
          memset(keystatus, 0, sizeof(keystatus));
          loadtile(ES2_VGA);
          overwritesprite(0,0,ES2_VGA,0,0x02,0);
          nextpage();
          while( (keystatus[1] == 0) && (keystatus[57] == 0) && (keystatus[28] == 0) ) {
          }
     }
}

char      debrief=0;

int
choosemission()
{
     int       lastmission,before7;
     int      clock,helpclock;
     char      nogo,missiondone;
     char      onlymission8=0;
     char      cdstopped=0;

     if( generalplay ) {
          return(choosemap());
     }
     if( singlemapmode ) {
          if( gameover == 1 ) {
               return(0);
          }
          else {
               newgame(boardfilename);
               return(1);
          }
     }

     musicfade();
     stopallsounds();
     /*if( cdplaying > 0 ) {
          cd_stop();
          cdstopped=1;
     }*/

     if( debrief ) {
          debriefing();
          debrief=0;
     }

     fadeout(0,255,0,0,0,25);

     if( allsymsdeposited == 1 ) {
          smkplayseq("FINALB");
          allsymsdeposited=2;
     }
     if( allsymsdeposited == 2) {
          mission=9;
          allsymsdeposited=3;
          goto donewgame;
     }
     if( allsymsdeposited == 3) {
          if( killedsonny == 1 ) {
               smkplayseq("FINALDB");
               smkplayseq("CREDITS");
               return(0);
          }
          else {
               return(0);
          }
     }

     //playsound(S_TRANSITION,0,0,0,ST_IMMEDIATE);
     menusong(0);
     smkopenmenu("smkmm.smk");
     mission=0;
     lastmission=mission;
     keystatus[1]=0;
     smkmenuframe(1);
     activemenu=0;

     if( symbols[0] && symbols[1] && symbols[2] && symbols[3] && symbols[4] &&
         symbols[5] && symbols[6] ) {
         lastmission=mission=7;
         onlymission8=1;
     }

choosingmission:

     if( lastmission != mission ) {
          switch( lastmission ) {
          case 0: smkmenuframe(3); break;
          case 1: smkmenuframe(7); break;
          case 2: smkmenuframe(11); break;
          case 3: smkmenuframe(15); break;
          case 4: smkmenuframe(19); break;
          case 5: smkmenuframe(23); break;
          case 6: smkmenuframe(27); break;
          case 7: smkmenuframe(31); break;
          }
     }
     switch( mission ) {
     case 0: smkmenuframe(5); break;
     case 1: smkmenuframe(9); break;
     case 2: smkmenuframe(13); break;
     case 3: smkmenuframe(17); break;
     case 4: smkmenuframe(21); break;
     case 5: smkmenuframe(25); break;
     case 6: smkmenuframe(29); break;
     case 7: smkmenuframe(33); break;
     }
     if( symbols[0] ) {
          smkmenuframe(35);
     }
     if( symbols[1] ) {
          smkmenuframe(37);
     }
     if( symbols[2] ) {
          smkmenuframe(39);
     }
     if( symbols[3] ) {
          smkmenuframe(41);
     }
     if( symbols[4] ) {
          smkmenuframe(43);
     }
     if( symbols[5] ) {
          smkmenuframe(45);
     }
     if( symbols[6] ) {
          smkmenuframe(47);
     }
     lastmission=mission;

     helpclock=totalclock;
     while( (keystatus[1]   == 0) &&
            (keystatus[28]  == 0) &&
            (keystatus[57]  == 0) &&
            (keystatus[38]  == 0) &&
            (keystatus[203] == 0) &&
            (keystatus[205] == 0) &&
            (keystatus[200] == 0) &&
            (keystatus[35]  == 0) &&
            (keystatus[208] == 0)  ) {
          if( (totalclock - helpclock) > 1024 ) {
               keystatus[35]=1;
          }
         smkshowmenu();
         if (handleevents() && quitevent) {
             return 0;
         }
     };

     if( (keystatus[203] != 0) &&(!onlymission8) ) {         // LF
          playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
nextmissionleft:
          missiondone=0;
          mission--;
          if( mission < 0 ) mission=6;
          switch( mission ) {
          case 0: if( symbols[0] ) missiondone=1; break;
          case 1: if( symbols[1] ) missiondone=1; break;
          case 2: if( symbols[2] ) missiondone=1; break;
          case 3: if( symbols[3] ) missiondone=1; break;
          case 4: if( symbols[4] ) missiondone=1; break;
          case 5: if( symbols[5] ) missiondone=1; break;
          case 6: if( symbols[6] ) missiondone=1; break;
          }
          if( missiondone ) {
               goto nextmissionleft;
          }
     }
     else if( (keystatus[205] != 0) && (!onlymission8) ) {    // RT
          playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
nextmissionright:
          missiondone=0;
          mission++;
          if( mission > 6 ) mission=0;
          switch( mission ) {
          case 0: if( symbols[0] ) missiondone=1; break;
          case 1: if( symbols[1] ) missiondone=1; break;
          case 2: if( symbols[2] ) missiondone=1; break;
          case 3: if( symbols[3] ) missiondone=1; break;
          case 4: if( symbols[4] ) missiondone=1; break;
          case 5: if( symbols[5] ) missiondone=1; break;
          case 6: if( symbols[6] ) missiondone=1; break;
          }
          if( missiondone ) {
               goto nextmissionright;
          }
     }
     else if( (keystatus[200] != 0) && (!onlymission8) ) {    // UP
          if( (symbols[0] == 0) && (symbols[1] == 0) && (symbols[2] == 0) && (symbols[3] == 0) &&
              (symbols[4] == 0) && (symbols[5] == 0) && (symbols[6] == 0) ) {
               playsound(S_BOOP,0,0,0,ST_IMMEDIATE);
               smkmenuframe(53);
               clock=totalclock;
               while( (totalclock-clock) < 128 ) {
                    smkshowmenu();
                    handleevents();
               }
               smkmenuframe(1);
               smkshowmenu();
          }
          else {
               playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
               before7=lastmission;
               mission=7;
          }
     }
     else if( (keystatus[208] != 0) && (!onlymission8) ) {    // DN
          if( mission == 7 ) {
               playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
               mission=before7;
               lastmission=7;
          }
     }
     keystatus[203]=0;
     keystatus[205]=0;
     keystatus[200]=0;
     keystatus[208]=0;

    #if STATS
     if( keystatus[31] != 0 ) {
          playsound(S_KEYCARDBLIP,0,0,0,ST_IMMEDIATE);
          smkmenuframe(49);
          keystatus[31]=0;
          while( (keystatus[1]  == 0) &&
                 (keystatus[28] == 0) &&
                 (keystatus[57] == 0) ) {
              smkshowmenu();
              writestats();
              if (handleevents() && quitevent) {
                  return 0;
              }
          }
          smkmenuframe(1);
          smkshowmenu();
          keystatus[1]=0;
          keystatus[28]=0;
          keystatus[57]=0;
     }
    #endif

     if( keystatus[1] != 0 ) {
          playsound(S_PICKUP_BONUS,0,0,0,ST_IMMEDIATE);
          smkmenuframe(51);
          keystatus[1]=0;
          while( (keystatus[1]  == 0) &&
                 (keystatus[21] == 0) &&
                 (keystatus[49] == 0) ) {
              smkshowmenu();
              if (handleevents() && quitevent) {
                  return 0;
              }
          }
          if( (keystatus[49] == 1) || (keystatus[1] == 1) ) {
               keystatus[49]=0;
               keystatus[1]=0;
               smkmenuframe(1);
               smkshowmenu();
          }
          else {
               fadeout(0,255,0,0,0,50);
               return(0);
          }
     }

     if( keystatus[35] != 0 ) {
          playsound(S_PICKUP_BONUS,0,0,0,ST_IMMEDIATE);
          keystatus[35]=0;
          smkmenuframe(55);
          keystatus[1]=0;
          helpclock=totalclock;
          while( (keystatus[1]  == 0) &&
                 (keystatus[28] == 0) &&
                 (keystatus[57] == 0) ) {
                 if( (totalclock-helpclock) > 2048 ) {
                    keystatus[1]=1;
                 }
              smkshowmenu();
              if (handleevents() && quitevent) {
                  return 0;
              }
          }
          keystatus[28]=0;
          keystatus[57]=0;
          keystatus[1]=0;
          smkmenuframe(1);
          smkshowmenu();
     }

     if( (keystatus[1] == 0)  && (keystatus[28] == 0) &&
         (keystatus[57] == 0) && (keystatus[38] == 0) ) {
          goto choosingmission;
     }

     if( keystatus[38] != 0 ) {
          mission=8;
          keystatus[38]=0;
          activemenu=6;
     }

     nogo=0;
     switch( mission ) {
     case 0: if( symbols[0] ) nogo=1; break;
     case 1: if( symbols[1] ) nogo=1; break;
     case 2: if( symbols[2] ) nogo=1; break;
     case 3: if( symbols[3] ) nogo=1; break;
     case 4: if( symbols[4] ) nogo=1; break;
     case 5: if( symbols[5] ) nogo=1; break;
     case 6: if( symbols[6] ) nogo=1; break;
     }
     if( nogo ) {
          playsound(S_BEEP,0,0,0,ST_IMMEDIATE);
          goto choosingmission;
     }

     keystatus[1] =0;
     keystatus[28]=0;
     keystatus[57]=0;
     smkclosemenu();

     musicfade();
     fadeout(0,255,0,0,0,50);

     switch( mission ) {
     case 2:
          smkplayseq("ROSSI1");
          strcpy(boardfilename,"subway1.map");
          break;
     case 1:
          smkplayseq("DIMARCO1");
          strcpy(boardfilename,"subway0.map");
          break;
     case 5:
          smkplayseq("CONNOR1");
          strcpy(boardfilename,"subway2.map");
          break;
     case 4:
          smkplayseq("SONNY1");
          strcpy(boardfilename,"subway2.map");
          break;
     case 6:
          smkplayseq("JANUS1");
          strcpy(boardfilename,"subway3.map");
          break;
     case 3:
          smkplayseq("LOWELL1");
          strcpy(boardfilename,"subway1.map");
          break;
     case 0:
          smkplayseq("DOLLAR1");
          strcpy(boardfilename,"subway0.map");
          break;
     case 8:
          strcpy(boardfilename,"load.map");
          break;
     }

donewgame:

     gameover=0;
     numlives=0;
     civillianskilled=0;
     mission_accomplished=0;

     fadeout(0,255,0,0,0,50);

     clearview(0);
     setbrightness(brightness, palette, 0);
     clearview(0);
     switch( mission ) {
     case 0:
     case 1:
          newgame("subway0.map");
          break;
     case 2:
     case 3:
          newgame("subway1.map");
          break;
     case 4:
     case 5:
          newgame("subway2.map");
          break;
     case 6:
          newgame("subway3.map");
          break;
     case 7:
          newgame("matrix.map");
          locselectedgun=7;
          keystatus[keys[6]]=1;
          break;
     case 8:
          newgame("load.map");
          break;
     case 9:
          newgame("final1.map");
          break;
     }
     clearview(0);
     //memcpy(palette, palette1, 768);
     dofadein=32;
     initpaletteshifts();

     if( mission == 7 ) {
         #ifdef MUSICINMATRIX
          startmusic(rand()%7);
         #endif
     }
     else {
          menusong(1);
     }
     /*if( cdstopped ) {
          cd_play();
     }*/

     // if matrix, reset time
     if( mission == 7 ) {
          seconds=minutes=hours=0;
     }

     return(1);
}

void
teksavemissioninfo(int fil)
{
     int       rv;

     rv=write(fil,symbols,sizeof(symbols));
     rv=write(fil,symbolsdeposited,sizeof(symbolsdeposited));
     rv=write(fil,&difficulty,sizeof(difficulty));
     rv=write(fil,&currentmapno,sizeof(currentmapno));
    #if 0
     rv=write(fil,&warpretang,sizeof(warpretang));
     rv=write(fil,&warpretsect,sizeof(warpretsect));
     rv=write(fil,&warpretx,sizeof(warpretx));
     rv=write(fil,&warprety,sizeof(warprety));
     rv=write(fil,&warpretz,sizeof(warpretz));
    #endif
     rv=write(fil,&mission,sizeof(mission));
     rv=write(fil,&numlives,sizeof(numlives));
     rv=write(fil,&mission_accomplished,sizeof(mission_accomplished));
     rv=write(fil,&civillianskilled,sizeof(civillianskilled));
     rv=write(fil,&generalplay,sizeof(generalplay));
     rv=write(fil,&singlemapmode,sizeof(singlemapmode));
     rv=write(fil,&allsymsdeposited,sizeof(allsymsdeposited));
     rv=write(fil,&killedsonny,sizeof(killedsonny));
}

void
tekloadmissioninfo(int fil)
{
     int       rv;

     musicfade();

     rv=read(fil,symbols,sizeof(symbols));
     rv=read(fil,symbolsdeposited,sizeof(symbolsdeposited));
     rv=read(fil,&difficulty,sizeof(difficulty));
     rv=read(fil,&currentmapno,sizeof(currentmapno));
    #if 0
     rv=read(fil,&warpretang,sizeof(warpretang));
     rv=read(fil,&warpretsect,sizeof(warpretsect));
     rv=read(fil,&warpretx,sizeof(warpretx));
     rv=read(fil,&warprety,sizeof(warprety));
     rv=read(fil,&warpretz,sizeof(warpretz));
    #endif
     rv=read(fil,&mission,sizeof(mission));
     rv=read(fil,&numlives,sizeof(numlives));
     rv=read(fil,&mission_accomplished,sizeof(mission_accomplished));
     rv=read(fil,&civillianskilled,sizeof(civillianskilled));
     rv=read(fil,&generalplay,sizeof(generalplay));
     rv=read(fil,&singlemapmode,sizeof(singlemapmode));
     rv=read(fil,&allsymsdeposited,sizeof(allsymsdeposited));
     rv=read(fil,&killedsonny,sizeof(killedsonny));

     if( generalplay == 1 ) {
          startmusic(rand()%7);
     }
     else {
          startmusic(mission);
     }
}

void
teknetmenu()
{
     initpaletteshifts();

     //memcpy(palette1, palette, 768);
     //memset(palette, 0, 768);

     clearview(0);
     strcpy(boardfilename,"NET1.MAP");
     prepareboard(boardfilename);
     precache();

     clearview(0);
     //memcpy(palette, palette1, 768);
     fadein(0,255,16);
}

void
copyrightscreen()
{
     clearview(0);
     smkopenmenu("smkgm.smk");
     smkmenuframe(81);
     fadein(0,255,10);
     while( (keystatus[1]   == 0) &&
            (keystatus[28]  == 0) &&
            (keystatus[57]  == 0) &&
            (keystatus[38]  == 0) &&
            (keystatus[203] == 0) &&
            (keystatus[205] == 0) &&
            (keystatus[200] == 0) &&
            (keystatus[31]  == 0) &&
            (keystatus[35]  == 0) &&
            (keystatus[208] == 0)  ) {
         smkshowmenu();
         if (handleevents() && quitevent) {
             break;
         }
     };
     smkclosemenu();
}

int
choosemap()
{
     int       lastmap,map,set;
     int      clock,helpclock,stall;

     musicfade();

     fadeout(0,255,0,0,0,25);

     stopallsounds();
     menusong(0);

     smkopenmenu("smkgm.smk");
     lastmap=map=0;
     keystatus[1]=0;
     activemenu=0;
     set=0;
     smkmenuframe(1);

choosingmap:

     if( lastmap != map ) {
          if( set == 0 ) {
               switch( lastmap ) {
               case  0: smkmenuframe( 5); break;
               case  1: smkmenuframe( 9); break;
               case  2: smkmenuframe(13); break;
               case  3: smkmenuframe(17); break;
               case  4: smkmenuframe(21); break;
               case  5: smkmenuframe(25); break;
               case  6: smkmenuframe(29); break;
               case  7: smkmenuframe(33); break;
               case  8: smkmenuframe(37); break;
               }
          }
          else {
               switch( lastmap ) {
               case  0: smkmenuframe(41); break;
               case  1: smkmenuframe(45); break;
               case  2: smkmenuframe(49); break;
               case  3: smkmenuframe(53); break;
               case  4: smkmenuframe(57); break;
               case  5: smkmenuframe(61); break;
               case  6: smkmenuframe(65); break;
               case  7: smkmenuframe(69); break;
               case  8: smkmenuframe(73); break;
               }
          }
     }
     if( set == 0 ) {
          switch( map ) {
          case  0: smkmenuframe( 7); break;
          case  1: smkmenuframe(11); break;
          case  2: smkmenuframe(15); break;
          case  3: smkmenuframe(19); break;
          case  4: smkmenuframe(23); break;
          case  5: smkmenuframe(27); break;
          case  6: smkmenuframe(31); break;
          case  7: smkmenuframe(35); break;
          case  8: smkmenuframe(39); break;
          }
     }
     else {
          switch( map ) {
          case  0: smkmenuframe(43); break;
          case  1: smkmenuframe(47); break;
          case  2: smkmenuframe(51); break;
          case  3: smkmenuframe(55); break;
          case  4: smkmenuframe(59); break;
          case  5: smkmenuframe(63); break;
          case  6: smkmenuframe(67); break;
          case  7: smkmenuframe(71); break;
          case  8: smkmenuframe(75); break;
          }
     }
     smkshowmenu();
     lastmap=map;

     helpclock=totalclock;
     while( (keystatus[1]   == 0) &&
            (keystatus[28]  == 0) &&
            (keystatus[57]  == 0) &&
            (keystatus[38]  == 0) &&
            (keystatus[203] == 0) &&
            (keystatus[205] == 0) &&
            (keystatus[200] == 0) &&
            (keystatus[31]  == 0) &&
            (keystatus[35]  == 0) &&
            (keystatus[208] == 0)  ) {
          if( (totalclock - helpclock) > 1024 ) {
               keystatus[35]=1;
          }
     };

     if( keystatus[203] != 0 ) {         // LF
          map--;
          if( map < 0 ) {
               map=8;
          }
          playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
     }
     else if( keystatus[205] != 0 ) {    // RT
          map++;
          if( map > 8 ) {
               map=0;
          }
          playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
     }
     else if( keystatus[200] != 0 ) {    // UP
          switch( map ) {
          case 0: map=6; break;
          case 3: map=0; break;
          case 6: map=3; break;
          case 1: map=7; break;
          case 4: map=1; break;
          case 7: map=4; break;
          case 2: map=8; break;
          case 5: map=2; break;
          case 8: map=5; break;
          }
          playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
     }
     else if( keystatus[208] != 0 ) {    // DN
          switch( map ) {
          case 0: map=3; break;
          case 3: map=6; break;
          case 6: map=0; break;
          case 1: map=4; break;
          case 4: map=7; break;
          case 7: map=1; break;
          case 2: map=5; break;
          case 5: map=8; break;
          case 8: map=2; break;
          }
          playsound(S_MENUSOUND1,0,0,0,ST_IMMEDIATE);
     }

     keystatus[203]=0;
     keystatus[205]=0;
     keystatus[200]=0;
     keystatus[208]=0;

     if( keystatus[1] != 0 ) {
          playsound(S_PICKUP_BONUS,0,0,0,ST_IMMEDIATE);
          smkmenuframe(77);
          smkshowmenu();
          keystatus[1]=0;
          while( (keystatus[1]  == 0) &&
                 (keystatus[21] == 0) &&
                 (keystatus[49] == 0) ) {
          }
          if( (keystatus[49] == 1) || (keystatus[1] == 1) ) {
               keystatus[49]=0;
               keystatus[1]=0;
               switch( set ) {
               case 0: smkmenuframe(1); break;
               case 1: smkmenuframe(3); break;
               }
               smkshowmenu();
          }
          else {
               fadeout(0,255,0,0,0,50);
               return(0);
          }
     }

     if( keystatus[35] != 0 ) {
          playsound(S_PICKUP_BONUS,0,0,0,ST_IMMEDIATE);
          keystatus[35]=0;
          smkmenuframe(79);
          smkshowmenu();
          keystatus[1]=0;
          helpclock=totalclock;
          while( (keystatus[1]  == 0) &&
                 (keystatus[28] == 0) &&
                 (keystatus[57] == 0) ) {
               if( (totalclock-helpclock) > 2048 ) {
                    keystatus[1]=1;
               }
          }
          keystatus[28]=0;
          keystatus[57]=0;
          keystatus[1]=0;
          switch( set ) {
          case 0: smkmenuframe(1); break;
          case 1: smkmenuframe(3); break;
          }
          smkshowmenu();
     }

     if( (keystatus[1] == 0)  && (keystatus[28] == 0) &&
         (keystatus[57] == 0) && (keystatus[38] == 0) ) {
          goto choosingmap;
     }
     if( (keystatus[57] != 0)  || (keystatus[28] != 0) ) {
          switch( map ) {
          case 4:
               stall=totalclock;
               while( (totalclock-stall) < 32 )
                    ;
               if( set == 0 ) {
                    set=1;
                    smkmenuframe(3);
                    smkshowmenu();
               }
               else {
                    set=0;
                    smkmenuframe(1);
                    smkshowmenu();
               }
               goto choosingmap;
          default:
               break;
          }
     }

     smkclosemenu();

     gameover=0;
     numlives=0;
     civillianskilled=0;
     mission_accomplished=0;

     fadeout(0,255,0,0,0,50);

     clearview(0);

     if( set == 0 ) {
          switch( map ) {
          case  0: newgame("level1.map");  break;
          case  1: newgame("level2.map");  break;
          case  2: newgame("city1.map");   break;
          case  3: newgame("beach1.map");  break;

          case  5: newgame("park1.map");   break;
          case  6: newgame("mid1.map");    break;
          case  7: newgame("mid2.map");    break;
          case  8: newgame("mid3.map");    break;
          default: crash("chsmp: bad map num");
          }
     }
     else {
          switch( map ) {
          case  0: newgame("sewer1.map");  break;
          case  1: newgame("sewer2.map");  break;
          case  2: newgame("inds1.map");   break;
          case  3: newgame("free1.map");   break;

          case  5: newgame("free2.map");   break;
          case  6: newgame("ware1.map");   break;
          case  7: newgame("ware2.map");   break;
          case  8: newgame("ware3.map");   break;
          default: crash("chsmp: bad map num");
          }
     }

     clearview(0);
     dofadein=32;
     initpaletteshifts();

     musicfade();
     startmusic(rand()%7);

     return(1);
}

void
missionaccomplished(int  sn)
{
     int       ext;
     char      results=0;

     if( option[4] != 0 ) {
          return;
     }

     ext=sprptr[sn]->extra;
     if( !validext(ext) ) {
          return;
     }

     if( sprXTptr[ext]->class == CLASS_CIVILLIAN ) {
          civillianskilled++;
     }
     if( sprXTptr[ext]->class != CLASS_TEKLORD ) {
          return;
     }

     switch( sprXTptr[ext]->deathpic ) {
     case WINGDEATHPIC:
          symbols[0]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case DIDEATHPIC:
          symbols[1]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case SFRODEATHPIC:
          symbols[2]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case ANTDEATHPIC:
          symbols[3]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case SGOLDEATHPIC:
          symbols[4]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case SUNGDEATHPIC:
          symbols[5]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case REDHDEATHPIC:
          symbols[6]=1;
          mission_accomplished=1;
          gameover=1;
          break;
     case SSALDEATHPIC:
          killedsonny=1;
          mission_accomplished=1;
          gameover=1;
          break;
     }
}

void
depositsymbol(int snum)
{
     int       i,findpic;
     int       sym=sector[cursectnum[snum]].hitag;

     switch( sym ) {
     case 0: findpic=3600; break;
     case 1: findpic=3604; break;
     case 2: findpic=3608; break;
     case 3: findpic=3612; break;
     case 4: findpic=3592; break;
     case 5: findpic=3596; break;
     case 6: findpic=3616; break;
     }

     if( symbols[sym] ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( sprptr[i]->picnum == findpic ) {
                    sprptr[i]->picnum=findpic+1;
                    symbolsdeposited[sym]=1;
                    break;
               }
          }
     }

     if( symbolsdeposited[0] &&
         symbolsdeposited[1] &&
         symbolsdeposited[2] &&
         symbolsdeposited[3] &&
         symbolsdeposited[4] &&
         symbolsdeposited[5] &&
         symbolsdeposited[6] ) {
         allsymsdeposited=1;
         gameover=1;
     }
}

int
missionfailed()
{
     if( option[4] != 0 ) {
          return(0);
     }

     numlives++;

     switch( difficulty ) {
     case 0:
     case 1:
          if( numlives < 6 )
               return(0);
          break;
     case 2:
          if( numlives < 4 )
               return(0);
          break;
     default:
          if( numlives < 2 )
               return(0);
          break;
     }

     mission_accomplished=0;
     gameover=1;
     return(1);
}

void
debriefing()
{
     if( mission_accomplished ) {
          if( civillianskilled == 0 ) {
               switch( mission ) {
               case 2:
                    smkplayseq("ROSSI2");
                    break;
               case 1:
                    smkplayseq("DIMARCO2");
                    break;
               case 5:
                    smkplayseq("CONNOR2");
                    break;
               case 4:
                    smkplayseq("SONNY2");
                    break;
               case 6:
                    smkplayseq("JANUS2");
                    break;
               case 3:
                    smkplayseq("LOWELL2");
                    break;
               case 0:
                    smkplayseq("DOLLAR2");
                    break;
               }
          }
          else {
               switch( mission ) {
               case 2:
                    smkplayseq("ROSSI3");
                    break;
               case 1:
                    smkplayseq("DIMARCO3");
                    break;
               case 5:
                    smkplayseq("CONNOR3");
                    break;
               case 4:
                    smkplayseq("SONNY3");
                    break;
               case 6:
                    smkplayseq("JANUS3");
                    break;
               case 3:
                    smkplayseq("LOWELL3");
                    break;
               case 0:
                    smkplayseq("DOLLAR3");
                    break;
               }
          }
     }
     else {
          if( civillianskilled == 0 ) {
               switch( mission ) {
               case 2:
                    smkplayseq("ROSSI4");
                    break;
               case 1:
                    smkplayseq("DIMARCO4");
                    break;
               case 5:
                    smkplayseq("CONNOR4");
                    break;
               case 4:
                    smkplayseq("SONNY4");
                    break;
               case 6:
                    smkplayseq("JANUS4");
                    break;
               case 3:
                    smkplayseq("LOWELL4");
                    break;
               case 0:
                    smkplayseq("DOLLAR4");
                    break;
               }
          }
          else {
               switch( mission ) {
               case 2:
                    smkplayseq("ROSSI5");
                    break;
               case 1:
                    smkplayseq("DIMARCO5");
                    break;
               case 5:
                    smkplayseq("CONNOR5");
                    break;
               case 4:
                    smkplayseq("SONNY5");
                    break;
               case 6:
                    smkplayseq("JANUS5");
                    break;
               case 3:
                    smkplayseq("LOWELL5");
                    break;
               case 0:
                    smkplayseq("DOLLAR5");
                    break;
               }
          }
     }
}


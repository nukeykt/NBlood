/***************************************************************************
 *   TEKSTAT.C  -  sprite status and interactiob code for Tekwar           *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include  "build.h"
#include  "names.h"
#include  "pragmas.h"
#include  "cache1d.h"
#include  "mmulti.h"

#include  "tekwar.h"


//#define   NEWSTAT_DEBUG
//#define   NETDEBUG
FILE      *dbgfp2;

#define   MAXFRAMES                          8

#define   SPR_LOTAG_SPAWNCHASE               2000
#define   SPR_LOTAG_PLAYBACK                 2001
#define   SPR_LOTAG_PUSHABLE                 2002
#define   SPR_LOTAG_MORPH                    2003
#define   SPR_LOTAG_PICKUP                   2004
#define   SPR_LOTAG_FLAMMABLE                2005
#define   SPR_LOTAG_PICKUP_FLAMMABLE         2006
#define   SPR_LOTAG_PUSHABLE_FLAMMABLE       2007

#define   SECT_LOTAG_TRIGGERSPRITE           5020
#define   SECT_LOTAG_SHOWMESSAGE             5030
#define   SECT_LOTAG_NOSTANDING              5040
#define   SECT_LOTAG_OFFLIMITS_CIVILLIAN     5050
#define   SECT_LOTAG_OFFLIMITS_ALL           5055
#define   SECT_LOTAG_CLIMB                   5060
#define   SECT_LOTAG_SUNKEN                  5065

#define   DONTBOTHERDISTANCE       20480
#define   HEARGUNSHOTDIST          10240

#define   INANIMATE      0       
#define   PLAYER         8
#define   INACTIVE       100     
#define   STANDING       201     
#define   AMBUSH         202     
#define   GUARD          203     
#define   STALK          204     
#define   FLEE           205     
#define   CHASE          206     
#define   PATROL         207     
#define   CRAWL          208     
#define   STROLL         209     
#define   VIRUS          250
#define   PLRVIRUS       255
#define   ATTACK         300
#define   DEATH          301
#define   PAIN           302
#define   TWITCH         303
#define   MORPH          304
#define   SQUAT          305
#define   UNSQUAT        306
#define   JUMP           307
#define   LEAP           308
#define   DODGE          309
#define   UNDODGE        310
#define   HIDE           311
#define   UNHIDE         312
#define   DELAYEDATTACK  313
#define   MIRRORMAN1     320
#define   MIRRORMAN2     321
#define   FLOATING       322  
#define   PROJHIT        400
#define   PROJECTILE     401
#define   TOSS           402
#define   PINBALL        403     
#define   KINDLING       405
#define   DROPSIES       406    
#define   RUNTHRU        407
#define   BLOODFLOW      408
#define   FLY            500     
#define   RODENT         502
#define   TIMEBOMB       602
#define   STACKED        610
#define   FALL           611
#define   GENEXPLODE1    800
#define   GENEXPLODE2    801
#define   VANISH         999
#define   KAPUT          MAXSTATUS

#define   FORCEPROJECTILESTAT 710
#define   DARTPROJECTILESTAT  712
#define   BOMBPROJECTILESTAT  714
#define   BOMBPROJECTILESTAT2 716
#define   MOVEBODYPARTSSTAT   900

#define   ENEMYCRITICALCONDITION   25

#define   FX_NULL             0x0000
#define   FX_HASREDCARD       0x0001
#define   FX_HASBLUECARD      0x0002
#define   FX_ANDROID          0x0004
#define   FX_HOLOGRAM         0x0008

#define   FX_NXTSTTVANISH     0x0010
#define   FX_NXTSTTPAIN       0x0020
#define   FX_NXTSTTDEATH      0x0040

#define   NO_PIC              0    

#define   MINATTACKDIST  8192L
#define   CHASEATTDIST   8192L
#define   GUARDATTDIST   6144L
#define   STALKATTDIST   8192L
#define   SCARECIVILLIANDISTANCE   102400

#define   RMOD2(s)  ( ((krand_intercept(s))>0x00008000) )
#define   RMOD3(s)  ( ((krand_intercept(s))>0x00008000)+(((krand_intercept(s))>>1)&1L) )
#define   RMOD4(s)  ( (((krand_intercept(s))>>3)&0x0000000F)&3L )
#define   RMOD10(s) ( (krand_intercept(s))&9L )
#define   RMOD16(s) ( (krand_intercept(s))&15L )

int           pickupclock;
spritetype     pickup;
struct    picattribtype  picinfo;
struct    picattribtype  *picinfoptr=&picinfo;

#define   MAXBOBS   32
int      bobbing[MAXBOBS] = { 
     0,  2,  4,  6,  8, 10, 12, 14,
    16, 14, 12, 10,  8,  6,  4,  2, 
     0, -2, -4, -6, -8,-10,-12,-14,
   -16,-14,-12,-10, -8, -6, -4, -2 
};

// angles, 0 east start, 22.5deg(==128scaled) resolution
short  leftof[17]  = { 1920,    0,  128,  256,
                        384,  512,  640,  768,
                        896, 1024, 1152, 1280, 
                       1408, 1536, 1664, 1792, 1920 };

short  rightof[17] = {  128,  256,  384,  512,
                        640,  768,  896, 1024,
                       1152, 1280, 1408, 1536,
                       1664, 1792, 1920,    0,  128 };

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

struct TPACK    XTsavetype {
     short     XTnum;
     struct    spriteextension      sprXT;
};
struct XTsavetype XTsave;
struct TPACK    XTtrailertype {
     int      numXTs;          
     int      start;
     char      mapname[13];
     char      ID[13];
};
struct    XTtrailertype  XTtrailer;
#define   TRAILERID      "**MAP_EXTS**"

#ifdef _MSC_VER
#pragma pack()
#endif

#ifdef __WATCOMC__
#pragma pack(pop)
#endif

#undef TPACK

struct    sectflashtype {
     short     sectnum;
     int       step;
     unsigned char      ovis;
};
struct    sectflashtype  sectflash;

int       vadd;     
char      ensfirsttime=1;
int      stackedcheck;

#define STATLISTDEBUG
#ifdef  STATLISTDEBUG
short *headspritesectptr[MAXSECTORS+1],*headspritestatptr[MAXSTATUS+1];
short *prevspritesectptr[MAXSPRITES],  *prevspritestatptr[MAXSPRITES];
short *nextspritesectptr[MAXSPRITES],  *nextspritestatptr[MAXSPRITES];
#endif

#define   PLRSPRDEBUG
#define   VERIFYSTATS

char      vbad;



short
jsinsertsprite(short sect, short stat)
{
     short     j;

     j=insertsprite(sect,stat);
     if( j != -1 ) {
          sprite[j].x=0L;
          sprite[j].y=0L;
          sprite[j].z=0L;
          sprite[j].cstat=0;
          sprite[j].shade=0;
          sprite[j].pal=0;
          sprite[j].clipdist=32;
          sprite[j].xrepeat=0;
          sprite[j].yrepeat=0;
          sprite[j].xoffset=0;
          sprite[j].yoffset=0;
          sprite[j].picnum=0;
          sprite[j].ang=0;
          sprite[j].xvel=0;
          sprite[j].yvel=0;
          sprite[j].zvel=0;
          sprite[j].owner=-1;
          sprite[j].lotag=0;
          sprite[j].hitag=0;
          sprite[j].extra=-1;
     }
     return(j);
}

short
jsdeletesprite(short spritenum)
{
     int       ext;

     ext=sprptr[spritenum]->extra;

     if( validext(ext) ) {
          memset(sprXTptr[ext], 0, sizeof(struct spriteextension));
          sprXTptr[ext]->lock=0x00;
     }

     deletesprite(spritenum);
     sprite[spritenum].extra=-1;

     return(0);
}

int
isaplayersprite(int sprnum)
{
     int       j;

     for( j=connecthead ; j >= 0 ; j=connectpoint2[j] ) {
          if( playersprite[j] == sprnum ) {
               return(1);
          }
     }
     if( sprptr[sprnum]->statnum == 8 ) {
          crash("isplrspr: non plr has statnm 8");
     }
     return(0);
}

int
validext(int ext)
{
     if( (ext >= 0) && (ext < MAXSPRITES) ) {
          return(1);
     }
     return(0);
}

void
clearXTpics(short spriteno)
{
     short     extno=sprptr[spriteno]->extra;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(spriteno) ) {
          crash("messing w plrsprite at 2");
     }
    #endif

     if( extno != -1 ) {
          sprXTptr[extno]->basepic=sprptr[spriteno]->picnum;
          sprXTptr[extno]->standpic=NO_PIC;
          sprXTptr[extno]->walkpic=NO_PIC;
          sprXTptr[extno]->runpic=NO_PIC;
          sprXTptr[extno]->attackpic=NO_PIC;
          sprXTptr[extno]->deathpic=NO_PIC;
          sprXTptr[extno]->painpic=NO_PIC;
          sprXTptr[extno]->squatpic=NO_PIC;
          sprXTptr[extno]->morphpic=NO_PIC;
          sprXTptr[extno]->specialpic=NO_PIC;
     }
}

int
mapXT(int sn)
{
     short    i;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 0");
     }
    #endif

     for ( i=0; i<MAXSPRITES; i++ ) {
          if( sprXTptr[i]->lock == 0x00 ) {
               sprptr[sn]->extra=i;
               memset(sprXTptr[i], 0, sizeof(struct spriteextension));
               sprXTptr[i]->lock=0xFF;
               return(i);
          }
     }

     sprptr[sn]->extra=-1;
     return(-1);   // no free spot found
}

void
noextcrash(int i, int loc)
{
     crash("sprite at %d,%d no extension from  %d", sprite[i].x,sprite[i].y, loc);
}

#ifdef  VERIFYSTATS
void
verifystatus(int i, int stat)
{
     if( i == -1 ) {
          return;
     }
     if( sprptr[i]->statnum != stat ) {
          if( nextspritestat[i] != -1 ) {
               crash("verifystatus fail for %d", stat);
          }
          vbad++;
     }
}
#endif

int
initsprites()
{
     int       i;

    #ifdef   NETDEBUG
     dbgfp2=fopen("debug.sts","wt");
    #endif

     for( i=0; i< MAXSPRITES; i++ ) {
          memset(&sprite[i], 0, sizeof(spritetype));
          sprite[i].sectnum=MAXSECTORS;
          sprite[i].statnum=MAXSTATUS;
          sprite[i].extra=-1;
     }

     // initialize blast sector flashes
     sectflash.sectnum=0;
     sectflash.ovis=0;
     sectflash.step=0;

    #ifdef  STATLISTDEBUG
     for( i=0; i < (MAXSECTORS+1); i++ ) {
          headspritesectptr[i]=&headspritesect[i];
     }
     for( i=0; i < (MAXSTATUS+1); i++ ) {
          headspritestatptr[i]=&headspritestat[i];
     }
     for( i=0; i < MAXSPRITES; i ++ ) {
          prevspritesectptr[i]=&prevspritesect[i];
          prevspritestatptr[i]=&prevspritestat[i];
          nextspritesectptr[i]=&nextspritesect[i];
          nextspritestatptr[i]=&nextspritestat[i];
     }
    #endif

     return(1);
}

int
initspriteXTs()
{
     int       fh,i,nr,ext;

     memset(&pickup, 0, sizeof(pickup));
     pickup.extra=-1;

     for (i=0; i<MAXSPRITES; i++) {
          memset(&spriteXT[i], 0, sizeof(struct spriteextension));
     }

     fh=kopen4load(boardfilename, 0);
     if( fh == -1 ) {
          return 0;
     }

     // read in XTtrailer 
     klseek(fh, -((int)sizeof(struct XTtrailertype)), SEEK_END);
     memset(&XTtrailer, 0, sizeof(struct XTtrailertype));
     kread(fh, &XTtrailer, sizeof(struct XTtrailertype));

     // if no previous extension info then continue
     if( strcmp(XTtrailer.ID, TRAILERID) != 0 ) {
          goto noext;
     }

     // load and intialize spriteXT array members
     klseek(fh, XTtrailer.start, SEEK_SET);
     for( i=0; i<XTtrailer.numXTs; i++ ) {
          nr=kread(fh, &XTsave, sizeof(struct XTsavetype));
          if( nr != sizeof(struct XTsavetype) )
               break;
          spriteXT[XTsave.XTnum]=XTsave.sprXT;  // struct assign
     }

noext:

     kclose(fh);

     switch( difficulty ) {
     case 0:
     case 1:
          vadd=-1;
          break;
     case 2:
          vadd=0;
          break;
     case 3:
          vadd=3;
          break;
     }

     // adjust speed for difficulty
     for( i=0; i<MAXSPRITES; i++ ) { 
          if( sprite[i].extra != -1 ) {
              #ifdef  NOSHOWSPRITES
	          show2dsprite[i>>3] &= ~(1<<(i&7));
              #endif
               switch( spriteXT[sprite[i].extra].basestat ) {
               case GUARD:
               case CHASE:
               case STALK:
                    sprite[i].xvel+=vadd;
                    if( sprite[i].xvel < 0 )
                         sprite[i].xvel=0;
                    sprite[i].yvel+=vadd;
                    if( sprite[i].yvel < 0 )
                         sprite[i].yvel=0;
                    sprite[i].zvel+=vadd;
                    if( sprite[i].zvel < 0 )
                         sprite[i].zvel=0;
                    break;
               }
          }
     }

     // only CHASE in multiplayer
     if( option[4] != 0 ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( (ext=sprite[i].extra) != -1 ) {
                    if( sprite[i].statnum != CHASE ) {
                         jsdeletesprite(i);
                    }
                    else if( spriteXT[ext].basestat != CHASE ) {
                         jsdeletesprite(i);
                    }
               }
          }
     }

     if( noenemiesflag != 0 ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( sprptr[i]->extra != -1 ) {
                    jsdeletesprite(i);
               }
          }
     }

     if( noguardflag != 0 ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( sprptr[i]->statnum == GUARD ) {
                    jsdeletesprite(i);
               }
               else {
                    if( sprptr[i]->extra != -1 ) {
                         if( sprXTptr[sprptr[i]->extra]->basestat == GUARD ) {
                              jsdeletesprite(i);
                         }
                    }
               }
          }
     }

     if( nostalkflag != 0 ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( sprptr[i]->statnum == STALK ) {
                    jsdeletesprite(i);
               }
               else {
                    if( sprptr[i]->extra != -1 ) {
                         if( sprXTptr[sprptr[i]->extra]->basestat == STALK ) {
                              jsdeletesprite(i);
                         }
                    }
               }
          }
     }

     if( nochaseflag != 0 ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( sprptr[i]->statnum == CHASE ) {
                    jsdeletesprite(i);
               }
               else {
                    if( sprptr[i]->extra != -1 ) {
                         if( sprXTptr[sprptr[i]->extra]->basestat == CHASE ) {
                              jsdeletesprite(i);
                         }
                    }
               }
          }
     }

     if( nostrollflag != 0 ) {
          for( i=0; i<MAXSPRITES; i++ ) {
               if( sprptr[i]->statnum == STROLL ) {
                    jsdeletesprite(i);
               }
               else {
                    if( sprptr[i]->extra != -1 ) {
                         if( sprXTptr[sprptr[i]->extra]->basestat == STROLL ) {
                              jsdeletesprite(i);
                         }
                    }
               }
          }
     }

     return(1);
}

int
isvisible(short i, short target)
{

     if( !validplayer(target) ) {
          crash("isvisible: bad targetnum");
     }

	if( sintable[(sprptr[i]->ang+2560)&2047]*(posx[target]-sprptr[i]->x) + sintable[(sprptr[i]->ang+2048)&2047]*(posy[target]-sprptr[i]->y) >= 0) {
	     if( cansee(posx[target],posy[target],(posz[target])>>1,cursectnum[target],
                     sprptr[i]->x,sprptr[i]->y,sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<7),sprptr[i]->sectnum) == 1) {
          return(1);
          }
     }

     return(0);
}

void
getpicinfo(short picnum)
{
     int      amask=picanm[picnum];

     picinfoptr->numframes=amask&0x0000003F;     
     picinfoptr->animtype =( amask&0x000000C0 )>>6;
     picinfoptr->xcenteroffset=( amask&0x0000FF00 )>>8;
     picinfoptr->ycenteroffset=( amask&0x00FF0000 )>>16;
     picinfoptr->animspeed=( amask&0x0F000000 )>>24;
}

short
wallangle(int wn)
{
     int      w1x,w1y, w2x,w2y;
     short     wang;

     w1x=wallptr[wn]->x; w1y=wallptr[wn]->y;
     wn=wallptr[wn]->point2;
     w2x=wallptr[wn]->x; w2y=wallptr[wn]->y;
     wang=getangle(w2x-w1x, w2y-w1y); 

 return(wang);
}

short
arbitraryangle(void)
{        
     switch( RMOD4("STAT642 ") ) {
     case 0:
     case 1:
          return(leftof[RMOD16("STAT645 ")]);
          break;
     default:
          return(rightof[RMOD16("STAT648 ")]);
          break;
     }
}

short
wallnormal(int wn)
{
     int      w1x,w1y, w2x,w2y;
     short     wnorm;

     w1x=wallptr[wn]->x; w1y=wallptr[wn]->y;
     wn=wallptr[wn]->point2;
     w2x=wallptr[wn]->x; w2y=wallptr[wn]->y;
     wnorm=getangle(w2x-w1x, w2y-w1y); 
     wnorm=(wnorm+512L)&2047;

 return(wnorm);
}

short
walldeflect(int wn, short angin)
{
     short     wnorm,refract,delta,angout;
     
     wnorm=wallnormal(wn);

     refract=(angin+1024)&2047;

     delta=wnorm-refract;

     angout = wnorm+delta;

     if ( angout < 0  )
          angout+=2048;

 return(angout);
}

short
spritedeflect(int sn, short angin)
{ 
     short     angout;

     switch ( sprptr[sn]->statnum ) {
     case INANIMATE:
     case INACTIVE:
     case AMBUSH:
          angout=(angin+1024)&2047;
          switch( (int)RMOD2("STAT697 ") ) {
          case 0:
               angout=leftof[angout];
               break;
          case 1:
          default:
               angout=rightof[angout];
               break;
          }
          break;
     default:
          angout=sprptr[sn]->ang;
          break;
     }

 return(angout);
}

void
tweakdeathdist(short i)
{
	int      dax,day;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 1");
     }
    #endif

     sprptr[i]->clipdist<<=1;
     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
	day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
	movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);

     if( sprptr[i]->extra == -1 ) {
          sprptr[i]->lotag=SPR_LOTAG_PICKUP;
     }
}

void
splash(int i)
{
     int       j;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 3");
     }
    #endif

     if( option[4] != 0 ) {
          return;
     }

     j=jsinsertsprite(sprptr[i]->sectnum, RUNTHRU);
     if( j != -1 ) {
          fillsprite(j,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z,
                     2,-13,0,32,64,64,0,0,BLUESPLASH,sprptr[i]->ang,
		     	 0,0,0,i+4096,sprptr[i]->sectnum,
                     RUNTHRU,0,0,-1);
          getpicinfo(BLUESPLASH);
          sprptr[j]->lotag=picinfoptr->numframes;
          if( sprptr[j]->lotag > MAXFRAMES ) {
               sprptr[j]->lotag=MAXFRAMES;
          }
          if( sprptr[j]->lotag < 0 ) {
               sprptr[j]->lotag=0;
          }
          sprptr[j]->hitag=0;
          playsound(S_SPLASH,sprptr[i]->x,sprptr[i]->y,0,ST_NOUPDATE);
     }
}

int
pickupsprite(short sn)
{
    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 4");
     }
    #endif

     // can only carry one at a time of these items
     if( (pickup.picnum != 0) || (option[4] != 0) ) {
          return(0);
     }

     pickup.picnum=sprptr[sn]->picnum;
     pickup.cstat=sprptr[sn]->cstat;
     pickup.shade=sprptr[sn]->shade;
     pickup.pal=sprptr[sn]->pal;
     pickup.clipdist=sprptr[sn]->clipdist;
     pickup.xrepeat=sprptr[sn]->xrepeat;
     pickup.yrepeat=sprptr[sn]->yrepeat;
	pickup.xoffset=sprptr[sn]->xoffset;
	pickup.yoffset=sprptr[sn]->yoffset;
     // cant set lotag/hitag as TOSS needs to use them
     pickup.extra=sprptr[sn]->extra;

     jsdeletesprite(sn);

     pickupclock=lockclock;
     return(1);
}

void
toss(short snum)
{
     int       j;

     if( !validplayer(snum) ) {
          crash("toss: bad plrnum");
     }
     if( pickup.picnum == 0 ) {
          return;
     }
     j=jsinsertsprite(cursectnum[snum], TOSS);
     if( j != -1 ) {
          if( drawweap[snum] == 0 ) {
               sprptr[j]->x=posx[snum]+(sintable[(ang[snum]+2560+256)&2047]>>6);
               sprptr[j]->y=posy[snum]+(sintable[(ang[snum]+2048+256)&2047]>>6);
          }
          else {
               sprptr[j]->x=posx[snum]-(sintable[(ang[snum]+2560+256)&2047]>>6);
               sprptr[j]->y=posy[snum]-(sintable[(ang[snum]+2048+256)&2047]>>6);
          }
	     sprptr[j]->z=posz[snum]+(4<<8);
	     sprptr[j]->cstat=pickup.cstat;
	     sprptr[j]->shade=pickup.shade;
	     sprptr[j]->pal=pickup.pal;
          sprptr[j]->clipdist=pickup.clipdist;
	     sprptr[j]->xrepeat=pickup.xrepeat;
	     sprptr[j]->yrepeat=pickup.yrepeat;
	     sprptr[j]->xoffset=pickup.xoffset;
	     sprptr[j]->yoffset=pickup.yoffset;
          sprptr[j]->ang=ang[snum];
          switch( pickup.picnum ) {
          case RATPIC:
               sprptr[j]->picnum=RATTHROWPIC;
               break;
          case TUBEBOMB+1:
               sprptr[j]->picnum=TUBEBOMB;
               break;               
          case DARTBOMB+1:
               sprptr[j]->picnum=DARTBOMB;
               break;               
          default:
               sprptr[j]->picnum=pickup.picnum;
               break;
          }
          sprptr[j]->xvel=sintable[(ang[snum]+2560)&2047]>>6;
          sprptr[j]->yvel=sintable[(ang[snum]+2048)&2047]>>6;
          sprptr[j]->zvel=(80-horiz[snum])<<6;
          sprptr[j]->owner=snum+4096;
          sprptr[j]->sectnum=cursectnum[snum];
          // TOSS will be usin hi/lo tag
          sprptr[j]->lotag=0;
          sprptr[j]->hitag=0;
          sprptr[j]->extra=pickup.extra;
     }
     memset(&pickup, 0, sizeof(pickup));
     pickup.extra=-1;
}

void
triggersprite(short sn)
{
     int      datag;
     short     j;
     short     newext;

     if( option[4] != 0 ) {
          return;
     }

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 5");
     }
    #endif

     if( sprptr[sn]->extra == -1 ) {
          return;
     }

	datag = sprptr[sn]->lotag;

     switch( datag ) {

          case SPR_LOTAG_MORPH:
               break;
          case SPR_LOTAG_SPAWNCHASE:  
	          j = jsinsertsprite(sprptr[sn]->sectnum,  sprXTptr[ sprptr[sn]->extra ]->basestat);
               if ( j == -1 ) {
                    break;
               }
               newext=mapXT(j);
               if( !validext(newext) ) {
                    jsdeletesprite(j);
                    break;
               }
		     sprptr[j]->x = sprptr[sn]->x+( sintable[(sprptr[sn]->ang+512)&2047]>>6 );
		     sprptr[j]->y = sprptr[sn]->y+( sintable[sprptr[sn]->ang&2047]>>6 );
		     sprptr[j]->z = sprptr[sn]->z;
		     sprptr[j]->cstat = 0x101; 
               sprptr[j]->picnum = sprXTptr[ sprptr[sn]->extra ]->basepic;
		     sprptr[j]->shade = sprptr[sn]->shade;
		     sprptr[j]->sectnum = sprptr[sn]->sectnum;
		     sprptr[j]->xrepeat = sprptr[sn]->xrepeat;
		     sprptr[j]->yrepeat = sprptr[sn]->yrepeat;
		     sprptr[j]->ang = sprptr[sn]->ang;
		     sprptr[j]->xvel = sprptr[sn]->xvel;
		     sprptr[j]->yvel = sprptr[sn]->yvel;
		     sprptr[j]->zvel = sprptr[sn]->zvel;
		     sprptr[j]->owner = -1;
		     sprptr[j]->lotag = 0;
		     sprptr[j]->hitag = 0;
               sprXTptr[ sprptr[j]->extra ]->basestat  = sprXTptr[ sprptr[sn]->extra ]->basestat;
               sprXTptr[ sprptr[j]->extra ]->basepic   = sprXTptr[ sprptr[sn]->extra ]->basepic;
               sprXTptr[ sprptr[j]->extra ]->walkpic   = sprXTptr[ sprptr[sn]->extra ]->walkpic;
               sprXTptr[ sprptr[j]->extra ]->standpic  = sprXTptr[ sprptr[sn]->extra ]->standpic;
               sprXTptr[ sprptr[j]->extra ]->runpic    = sprXTptr[ sprptr[sn]->extra ]->runpic;
               sprXTptr[ sprptr[j]->extra ]->attackpic = sprXTptr[ sprptr[sn]->extra ]->attackpic;
               sprXTptr[ sprptr[j]->extra ]->deathpic  = sprXTptr[ sprptr[sn]->extra ]->deathpic;
               sprXTptr[ sprptr[j]->extra ]->morphpic  = sprXTptr[ sprptr[sn]->extra ]->morphpic;
               sprXTptr[ sprptr[j]->extra ]->specialpic = sprXTptr[ sprptr[sn]->extra ]->specialpic;
               // delete trigger tag from old sprites
               sprptr[sn]->lotag=0;
               sprptr[sn]->hitag=0;
               clearXTpics(sn);
			break;   
          default:
               newstatus(sn, sprXTptr[ sprptr[sn]->extra ]->basestat);
               break;
      }
}

void
sectortriggersprites(short snum)
{
     short     i,nexti,ext;
     char      triggered=0;

     if( option[4] != 0 ) {
          return;
     }

     if( !validplayer(snum) ) {
          crash("sectrtrgrsprts: bad plrnum");
     }

     if( sectptr[cursectnum[snum]]->lotag == SECT_LOTAG_SHOWMESSAGE ) {
           switch( sectptr[cursectnum[snum]]->hitag ) {
           case 0:
                 showmessage("AREA IS OFF-LIMITS");
                 break;
           }
           return;
     }
     if( sectptr[cursectnum[snum]]->lotag != SECT_LOTAG_TRIGGERSPRITE ) {
          return;
     }

     i=headspritestat[INANIMATE];
     while (i != -1) {
          nexti=nextspritestat[i];
          if (sprptr[i]->hitag == sectptr[cursectnum[snum]]->hitag) {
               triggersprite(i);
               triggered=1;
          }
          i=nexti;
     }

     i=headspritestat[AMBUSH];
     while (i != -1) {
          nexti=nextspritestat[i];
          if( sprptr[i]->hitag == sectptr[cursectnum[snum]]->hitag ) {
               ext=sprptr[i]->extra;
               if( !validext(ext) ) {
                    noextcrash(i, 300);
               }
               sprXTptr[i]->aimask|=AI_JUSTSHOTAT;
               ambushyell(i, ext);
               triggered=1;
          }
          i=nexti;
     }

     i=headspritestat[GUARD];
     while (i != -1) {
          nexti=nextspritestat[i];
          if( sprptr[i]->hitag == sectptr[cursectnum[snum]]->hitag ) {
               if( (ext=sprptr[i]->extra) != -1 ) {
                    sprXTptr[ext]->aimask&=~AI_GAVEWARNING;
                    givewarning(i, ext);
                    sprXTptr[ext]->aimask|=AI_ENCROACHMENT;
                    triggered=1;
               }
          }
          i=nexti;
     }

     i=headspritestat[STANDING];   // check GUARDS who are standing
     while (i != -1) {
          nexti=nextspritestat[i];
          ext=sprptr[i]->extra;
          if( (!validext(ext)) || (sprXTptr[ext]->basestat == GUARD) ) {
               if( sprptr[i]->hitag == sectptr[cursectnum[snum]]->hitag ) 
                    sprXTptr[ext]->aimask&=~AI_GAVEWARNING;
                    givewarning(i, ext);
                    sprXTptr[ext]->aimask|=AI_ENCROACHMENT;
                    triggered=1;
          }
          i=nexti;
     }

     if( triggered ) {
          sectptr[cursectnum[snum]]->hitag=0;
     }
}

void
bloodonwall(int wn, int x,int y,int z, short sect, short daang, int hitx, int hity, int hitz)
{
     int  j;

     if( wallptr[wn]->lotag != 0 ) {
          return;
     }

     switch( wallptr[wn]->picnum ) {
     case 15:
     case 71:
     case 72:
     case 87:
     case 93:  
     case 92:  
     case 95:  
     case 97:  
     case 98:  
     case 99:  
     case 124:  
     case 126: 
     case 147:  
     case 152:  
     case 214:  
     case 215:  
     case 235:  
     case 236:  
     case 636:  
     case 855:
     case 1457:  
          return;
     default:
          break;
     }

     if( wallptr[wn]->lotag == 0 ) {
          wallptr[wn]->lotag=1;
		neartag(x,y,z,sect,daang,
                  &neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,512L,1);
          if( neartagwall != -1 ) {
               j=jsinsertsprite(sect, BLOODFLOW);
               if( j != -1 ) {
                    sprptr[j]->picnum=WALLBLOOD;
                    sprptr[j]->x=hitx;
                    sprptr[j]->y=hity;
                    sprptr[j]->z=hitz+2048;
                    if( sprptr[j]->z > sectptr[sect]->floorz ) 
                         sprptr[j]->z=sectptr[sect]->floorz-2048;
                    sprptr[j]->xrepeat=16;
                    sprptr[j]->yrepeat=16;
                    sprptr[j]->xvel=0;
                    sprptr[j]->yvel=0;
                    sprptr[j]->zvel=0;
                    sprptr[j]->cstat=0x0090;
                    sprptr[j]->ang=wallnormal(wn);
                    sprptr[j]->shade=0; 
                    sprptr[j]->extra=-1;
                    sprptr[j]->lotag=0;
                    sprptr[j]->hitag=0;
               }
          }
          wallptr[wn]->lotag=0;
     }
}

int
spewblood(int sprnum, int hitz, short UNUSED(daang))
{
     int       j,ext=sprptr[sprnum]->extra;

     if( option[4] != 0 ) {
          return(0);
     }
     if( !validext(ext) ) {
          return(0);
     }
     if( isanandroid(sprnum) || isahologram(sprnum) ) {
          return(0);
     }

     switch( sprXTptr[ext]->basepic ) {
     case RUBWALKPIC:
     case FRGWALKPIC:
     case COP1WALKPIC:
     case ANTWALKPIC:
     case SARAHWALKPIC:
     case MAWALKPIC:
     case DIWALKPIC:
     case ERWALKPIC:
     case SAMWALKPIC:
          j=jsinsertsprite(sprptr[sprnum]->sectnum, RUNTHRU);
          if( j != -1 ) {
     	     fillsprite(j,sprptr[sprnum]->x,sprptr[sprnum]->y,sprptr[sprnum]->z-(tilesizy[sprptr[sprnum]->picnum]<<6),128,
                          0,0,12,16,16,0,0, BLOODSPLAT,sprptr[sprnum]->ang,
		                sintable[(sprptr[sprnum]->ang+2560)&2047]>>6,sintable[(sprptr[sprnum]->ang+2048)&2047]>>6,
		                30L,sprnum+4096,sprptr[sprnum]->sectnum, RUNTHRU,0,0,0);
               sprptr[j]->z=hitz;
               getpicinfo(BLOODSPLAT);
               sprptr[j]->lotag=picinfoptr->numframes;
               if( sprptr[j]->lotag > MAXFRAMES ) {
                    sprptr[j]->lotag=MAXFRAMES;
               }
               if( sprptr[j]->lotag < 0 ) {
                    sprptr[j]->lotag=0;
               }
               sprptr[j]->hitag=0;
               return(1);
          }
          break;
     default:
          break;
     }

     return(0);
}

int
playervirus(short pnum, int pic)
{
     int       j,nextj;

     if( !validplayer(pnum) ) {
          crash("plrvrus: bad plrnum");
     }

	j=headspritestat[PLRVIRUS];
	while( j >= 0 ) {
		nextj = nextspritestat[j];
          // already hosting ?
          if( sprptr[j]->owner == pnum ) {
               return(0);
          }
		j = nextj;
	}

     j=jsinsertsprite(cursectnum[pnum], PLRVIRUS);
     if( j == -1 ) {
          return(0);
     }

     sprptr[j]->extra=-1;
     sprptr[j]->x=posx[pnum];
     sprptr[j]->y=posy[pnum];
     sprptr[j]->z=posz[pnum];
     sprptr[j]->xrepeat=18;
     sprptr[j]->yrepeat=40;
     sprptr[j]->cstat=0x0000;
     sprptr[j]->shade=-28;
     sprptr[j]->picnum=pic;
     sprptr[j]->lotag=(krand_intercept("STAT1172")&512)+128;
     sprptr[j]->hitag=0;
     sprptr[j]->owner=pnum;  // host

     playsound(S_FIRELOOP,sprptr[j]->x,sprptr[j]->y,0,ST_UPDATE);
     
     return(1);
}

int
attachvirus(short i, int pic)
{
     int       j,ext,nextj;

     if( option[4] != 0 ) {
          return(0);
     }
     if( (sprptr[i]->statnum == VIRUS) || (sprptr[i]->statnum == PINBALL) ) {
          return(0);
     }
     if( isanandroid(i) || isahologram(i) ) {
          return(0);
     }

	j=headspritestat[VIRUS];
	while( j >= 0 ) {
		nextj = nextspritestat[j];
          // already hosting ?
          if( sprptr[j]->owner == i ) {
               return(0);
          }
		j = nextj;
	}

     j=jsinsertsprite(sprptr[i]->sectnum, VIRUS);
     if( j == -1 ) {
          return(0);
     }

     sprptr[j]->extra=-1;
     sprptr[j]->x=sprptr[i]->x;
     sprptr[j]->y=sprptr[i]->y;
     sprptr[j]->z=sprptr[i]->z;
     sprptr[j]->xrepeat=20;
     sprptr[j]->yrepeat=20;
     sprptr[j]->cstat=0x0000;
     sprptr[j]->shade=-28;
     sprptr[j]->picnum=pic;
     sprptr[j]->lotag=(krand_intercept("STAT1216")&512)+128;
     sprptr[j]->hitag=0;
     sprptr[j]->owner=i;  // host

     if( pic == FIREPIC ) {
          sprptr[j]->xrepeat=sprptr[i]->xrepeat;
          sprptr[j]->yrepeat=sprptr[i]->yrepeat+8;
          ext=sprptr[i]->extra;
          playsound(S_FIRELOOP,sprptr[j]->x,sprptr[j]->y,0,ST_UPDATE);
          if( validext(ext) ) {
               newstatus(i, PINBALL);
               if( sprptr[i]->statnum == PINBALL ) {
                    sprptr[i]->xvel+=4;
                    sprptr[i]->yvel+=4;
               }
          }
     }
     else {
          playsound(S_FORCEFIELDHUMLOOP, sprptr[i]->x,sprptr[i]->y,0,ST_UNIQUE);
     }

     return(1);
}

void
deathdropitem(short sprnum)
{
     short       j,ext;
     int         pic;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sprnum) ) {
          crash("messing w plrsprite at 7");
     }
    #endif

     if( option[4] != 0 ) {
          return;
     }

     ext=sprptr[sprnum]->extra;
     if( !validext(ext) ) {
          return;
     }

     if( sprXTptr[ext]->fxmask&FX_HASREDCARD ) {
           pic=RED_KEYCARD;
           sprXTptr[ext]->fxmask&=(~FX_HASREDCARD);
     }
     else if( sprXTptr[ext]->fxmask&FX_HASBLUECARD ) {
           pic=BLUE_KEYCARD;
           sprXTptr[ext]->fxmask&=(~FX_HASBLUECARD);
     }
     else if( sprXTptr[ext]->fxmask&FX_ANDROID ) {
          showmessage("WAS AN ANDROID");
          return;
     }
     else {
           switch( sprXTptr[ext]->weapon ) {
           case 4:
           case 5:
                 pic=KLIPPIC;
                 break;
           default:
                 return;
           }
     }

     j=jsinsertsprite(sprptr[sprnum]->sectnum, DROPSIES);
     if( j != -1 ) {
	     fillsprite(j,sprptr[sprnum]->x,sprptr[sprnum]->y,sprptr[sprnum]->z-(tilesizy[sprptr[sprnum]->picnum]<<6),128,
                     0,0,12,16,16,0,0,pic,sprptr[sprnum]->ang,
		           sintable[(sprptr[sprnum]->ang+2560)&2047]>>6,sintable[(sprptr[sprnum]->ang+2048)&2047]>>6,
		           30L,sprnum+4096,sprptr[sprnum]->sectnum,DROPSIES,0,0,0);
     }
     else {
           return;
     }

     // tweak the size of the pic
     switch( sprptr[j]->picnum ) {
     case KLIPPIC: 
           sprptr[j]->xrepeat-=2;
           sprptr[j]->yrepeat-=2;
           break;
     case RED_KEYCARD:
     case BLUE_KEYCARD:
           sprptr[j]->xrepeat>>=1;
           sprptr[j]->yrepeat>>=1;
           break;
     default:
           break;
     }
}

void
deathsounds(int pic, int x,int y)
{
     switch( pic ) {
     case ANTDEATHPIC:   //anton        boss
     case DIDEATHPIC:    //dimarco      boss
     case 2165:          //miles        boss
     case 2978:          //sonny hokuri boss
     case 2850:          //carlyle      boss
     case 2662:          //janus        boss
     case 2550:          //marty dollar boss
     case RUBDEATHPIC:
     case FRGDEATHPIC:
     case JAKEDEATHPIC:
     case COP1DEATHPIC:
     case ERDEATHPIC:
     case 2415:     //rebreather
     case 2295:     //black cop
     case 2455:     //trenchcoat
     case 2792:     //blacksmith
     case 2712:     //orange guy
     case 3041:     //swat guy
     case 2910:     //saline suit
          playsound(S_MANDIE1+RMOD4("STAT1378")+RMOD3("STAT1378")+RMOD3("STAT1378"),x,y,0,ST_NOUPDATE);
          break;
     case SARAHDEATHPIC:
     case SAMDEATHPIC:
     case MADEATHPIC:
     case 2340:     //nika
     case 2607:     //halter top
     case 2227:     //cowgirl
          playsound(S_GIRLDIE1+RMOD4("STAT1389")+RMOD3("STAT1389"),x,y,0,ST_NOUPDATE);
          break;
     case GLASSDEATHPIC:
          playsound(S_GLASSBREAK1+RMOD2("STAT1392"),x,y,0,ST_NOUPDATE);
          break;
     case 570:     // autogun
          playsound(S_AUTOGUNEXPLODE,x,y,0,ST_NOUPDATE);
          break;
     case 609:      //bathroom glass
          playsound(S_SMALLGLASS1+RMOD2("STAT1398"),0,0,0,ST_IMMEDIATE);
          break;
     case 3060:
     case 3064:
     case 3068:
     case 3072:
     case 3076:
     case 3080:
     case 3084:
          playsound(S_HOLOGRAMDIE,x,y,0,ST_NOUPDATE);
          break;      
     case 3973:          //matrix character death 
          if( krand_intercept("STAT1364") < 32767 ) {
               playsound(S_MATRIX_DIE1,x,y,0,ST_NOUPDATE);
          }
          else {
               playsound(S_MATRIX_DIE2,x,y,0,ST_NOUPDATE);
          }
          break;               

     }
}

void
hideplea(short sn, short ext)
{
    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 8");
     }
    #endif

     if( (sprXTptr[ext]->aimask&AI_DIDHIDEPLEA) && (krand_intercept("STAT1383")&1) )
          return;

     switch( sprXTptr[ext]->basepic ) {
     case ERWALKPIC:
          playsound(S_MALE_DONTHURT+(krand_intercept("STAT1390")&5),sprptr[sn]->x,sprptr[sn]->y,0,ST_UNIQUE);
          if(krand_intercept("STAT1391")&1) {
               sprXTptr[ext]->aimask|=AI_DIDHIDEPLEA;
          }                    
          break;
     case SARAHWALKPIC:
     case SAMWALKPIC:
          playsound(S_DIANE_DONTSHOOTP+(krand_intercept("STAT1397")&5),sprptr[sn]->x,sprptr[sn]->y,0,ST_UNIQUE);
          if(krand_intercept("STAT1398")&1) {
               sprXTptr[ext]->aimask|=AI_DIDHIDEPLEA;
          }
          break;
     }
}

void
fleescream(short sn, short ext)
{
    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 9");
     }
    #endif

     if( !validext(ext) ) {
          return;
     }

     if( (sprXTptr[ext]->aimask&AI_DIDFLEESCREAM) && (krand_intercept("STAT1417")&1) )
          return;

     switch( sprXTptr[ext]->basepic ) {
     case ERWALKPIC:
          playsound(S_MALE_OHMYGOD+(krand_intercept("STAT1424")&5),sprptr[sn]->x,sprptr[sn]->y,0,ST_UNIQUE);
          if(krand_intercept("STAT1425")&1) {
               sprXTptr[ext]->aimask|=AI_DIDFLEESCREAM;
          }
          break;
     case SARAHWALKPIC:
     case SAMWALKPIC:
          switch( RMOD4("STAT1478") ) {
		case 0:
          case 2:
               playsound(S_FEM_RUNHEGOTGUN+(krand_intercept("STAT1434")&11),sprptr[sn]->x,sprptr[sn]->y,0,ST_UNIQUE);
               break;
          default:
              playsound(S_SCREAM1+RMOD3("STAT1484"),sprptr[sn]->x,sprptr[sn]->y,0,ST_UNIQUE);
               break;
          }
          if(krand_intercept("STAT1440")&1) {
               sprXTptr[ext]->aimask|=AI_DIDFLEESCREAM;
          }
          break;
     }
}

void
rubitinsound(int UNUSED(p), int sn)
{
     int       ext=sprptr[sn]->extra;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 10");
     }
    #endif

     if( !validext(ext) ) {
          return;
     }

     if( sprXTptr[ext]->aimask&AI_DIDHIDEPLEA )
          return;

     switch( sprXTptr[ext]->basepic ) {
     case MAWALKPIC:
          playsound(S_MAR_THINKYOUTAKE+RMOD3("STAT1513"),sprptr[sn]->x,sprptr[sn]->y,0,ST_UPDATE);
     case DIWALKPIC:
          playsound(S_DIM_LAUGHTER+RMOD3("STAT1515"),sprptr[sn]->x,sprptr[sn]->y,0,ST_UPDATE);
          break;
     case ANTWALKPIC:
          playsound(S_MALE_YOULOSER+RMOD3("STAT1518"),sprptr[sn]->x,sprptr[sn]->y,0,ST_UPDATE);
          break;
     }
}

void
newstatus(short sn, int  seq)
{
     short     newpic;
     short     ext;
     int      zoffs;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 11");
     }
    #else
     if( isaplayersprite(sn) ) {
          return;
     }
    #endif
     
     ext=sprptr[sn]->extra;
     if( !validext(ext) ) {  
          noextcrash(sn, 0);     
     }

     switch( seq ) {
          case INANIMATE:
               changespritestat(sn, INANIMATE);
              #ifdef NEWSTAT_DEBUG
               showmessage("INANIMATE");
              #endif
               break;
          case INACTIVE:
               changespritestat(sn, INACTIVE);
              #ifdef NEWSTAT_DEBUG
               showmessage("INACTIVE");
              #endif
               break;
          case FLOATING:
               if( sectptr[sprptr[sn]->sectnum]->lotag  == 4 ) {
          	     if( (sprptr[sn]->cstat&128) == 0 ) {
		               zoffs = -((tilesizy[sprptr[sn]->picnum]*sprptr[sn]->yrepeat)<<1);
                    }
          	     else {
		               zoffs = 0;
                    }
                    sprptr[sn]->z=sectptr[sprptr[sn]->sectnum]->floorz-zoffs;
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->hitag=0;
                    sprptr[sn]->xvel=1;
                    sprptr[sn]->yvel=1;
                    changespritestat(sn, FLOATING);
               }
               else {
                    sprptr[sn]->z=sectptr[sprptr[sn]->sectnum]->floorz;
                    changespritestat(sn, INACTIVE);
               }
               break;
          case GUARD:
               newpic=sprXTptr[ext]->walkpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, GUARD);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=GUARD;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("GUARD");
                   #endif
               }
               break;
          case PATROL:
               newpic=sprXTptr[ext]->walkpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, PATROL);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=PATROL;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("PATROL");
                   #endif
               }
               break;
          case PINBALL:
               newpic=sprXTptr[ext]->walkpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, PINBALL);
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=PINBALL;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("PINBALL");
                   #endif
               }
               break;
          case STROLL:
               newpic=sprXTptr[ext]->walkpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, STROLL);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=STROLL;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("STROLL");
                   #endif
               }
               break;
          case CHASE:
               newpic=sprXTptr[ext]->runpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, CHASE);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=CHASE;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("CHASE");
                   #endif
               }
               break;
          case FLEE:
               newpic=sprXTptr[ext]->runpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, FLEE);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("FLEE");
                   #endif
               }
               break;
          case ATTACK:    
               // standard 5 angles, 2 frames/angle
               newpic=sprXTptr[ext]->attackpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn,ATTACK);
                    sprptr[sn]->picnum=newpic;
                    switch( sprXTptr[ext]->weapon ) {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                         sprptr[sn]->lotag=32;  
                         sprptr[sn]->hitag=32;
                         break;
                    default:
                         sprptr[sn]->lotag=64;  
                         sprptr[sn]->hitag=64;
                         break;                         
                    }
                   #ifdef NEWSTAT_DEBUG
                    showmessage("ATTACK");
                   #endif
               }
               break;
          case DELAYEDATTACK:    
               // standard 5 angles, 2 frames/angle
               sprptr[sn]->lotag=95;
               changespritestat(sn, DELAYEDATTACK);
              #ifdef NEWSTAT_DEBUG
               showmessage("DELAYED ATTACK");
              #endif
               break;
          case STALK:
               newpic=sprXTptr[ext]->runpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, STALK);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=STALK;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("STALK");
                   #endif
               }
               break;
          case SQUAT:
               newpic=sprXTptr[ext]->squatpic;
               if ( newpic != NO_PIC ) { 
                    changespritestat(sn, SQUAT);
                    // standard 3 frames
                    // stay squat for 4 TICSPERFRAME
                    sprptr[sn]->lotag=47;   
                    sprptr[sn]->hitag=64;   
                   #ifdef NEWSTAT_DEBUG
                    showmessage("SQUAT");
                   #endif
               }
               break;
          case UNSQUAT:
               newpic=sprXTptr[ext]->squatpic;
               if ( newpic != NO_PIC ) { 
                    changespritestat(sn, UNSQUAT);
                    sprptr[sn]->lotag=47;
                    sprptr[sn]->hitag=0;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("UNSQUAT");
                   #endif
               }
               break;
          case DODGE:
               newpic=sprXTptr[ext]->squatpic;
               if ( newpic != NO_PIC ) { 
                    changespritestat(sn, SQUAT);
                    // standard 2 frames
                    sprptr[sn]->lotag=31;   
                    sprptr[sn]->hitag=0;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("DODGE");
                   #endif
               }
               break;
          case UNDODGE:
               newpic=sprXTptr[ext]->squatpic;
               if ( newpic != NO_PIC ) { 
                    changespritestat(sn, UNSQUAT);
                    sprptr[sn]->lotag=31;
                    sprptr[sn]->hitag=0;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("UNDODGE");
                   #endif
               }
               break;
          case HIDE:
               newpic=sprXTptr[ext]->squatpic;
               if ( newpic != NO_PIC ) { 
                    changespritestat(sn, HIDE);
                    // standard 2 frames
                    sprptr[sn]->lotag=31; 
                    sprptr[sn]->hitag=256;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("HIDE");
                   #endif
               }
               break;
          case UNHIDE:
               newpic=sprXTptr[ext]->squatpic;
               if ( newpic != NO_PIC ) { 
                    changespritestat(sn, UNHIDE);
                    sprptr[sn]->lotag=31;
                    sprptr[sn]->hitag=0;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("UNHIDE");
                   #endif
               }
               break;
          case PAIN:
               newpic=sprXTptr[ext]->painpic;
               if ( newpic != 0 ) { 
                    changespritestat(sn,PAIN);
                    sprptr[sn]->picnum=newpic;
                    sprptr[sn]->lotag=16;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("PAIN");
                   #endif
               }
               break;
          case STANDING:
               newpic=sprXTptr[ext]->standpic;
               if( newpic != 0 ) {
                    sprptr[sn]->picnum=newpic;
                    if( sprptr[sn]->lotag <= 0 ) {
                         sprptr[sn]->lotag=((krand_intercept("STAT1732"))&1024);
                    }
                    changespritestat(sn, STANDING);
                   #ifdef NEWSTAT_DEBUG
                    showmessage("STANDING");
                   #endif
               }
               break;
          case FLY:
               newpic=sprXTptr[ext]->runpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, FLY);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=FLY;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("FLY");
                   #endif
               }
               break;
          case RODENT:
               newpic=sprXTptr[ext]->runpic;
               if( newpic != NO_PIC ) {
                    changespritestat(sn, RODENT);
                    sprptr[sn]->lotag=0;
                    sprptr[sn]->picnum=newpic;
                    sprXTptr[ext]->basestat=RODENT;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("RODENT");
                   #endif
               }
               break;
          case MORPH:
               if ( sprXTptr[ext]->morphpic != 0 ) {
                    changespritestat(sn,MORPH);
                    sprptr[sn]->lotag=95;
                   #ifdef NEWSTAT_DEBUG
                    showmessage("MORPH");
                   #endif
               }
               break;
          case DEATH:
               newpic=sprXTptr[ext]->deathpic;
               sprptr[sn]->cstat&=~257;
               if( newpic != 0 ) {  
                    changespritestat(sn,DEATH);
                    deathdropitem(sn);
                    getpicinfo(newpic);
                    sprptr[sn]->lotag=((picinfoptr->numframes)<<4)-1;
                    sprptr[sn]->hitag=sprptr[sn]->lotag;
                    deathsounds(newpic,sprptr[sn]->x,sprptr[sn]->y);
                    sprptr[sn]->picnum=newpic;
               }
               else {
                    changespritestat(sn, INACTIVE);
               }
              #ifdef NEWSTAT_DEBUG
               showmessage("DEATH");
              #endif
               break;
     }

}

int
damagesprite(int hitsprite, int points)
{
     short     ext;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(hitsprite) ) {
          crash("messing w plrsprite at 12");
     }
    #else
     if( isaplayersprite(hitsprite) ) {
          return(0);
     }
    #endif

     ext=sprptr[hitsprite]->extra;

     switch( sprptr[hitsprite]->statnum ) {
     case INANIMATE:
          if( validext(ext) ) { 
               newstatus(hitsprite, DEATH); 
          }
          return(0);
     case GENEXPLODE1:
          changespritestat(hitsprite,INACTIVE);
          genexplosion1(hitsprite);
          //changespritestat(hitsprite,VANISH);
          jsdeletesprite(hitsprite);
          return(0);
     case GENEXPLODE2:
          changespritestat(hitsprite,INACTIVE);
          genexplosion2(hitsprite);
          //changespritestat(hitsprite,VANISH);
          jsdeletesprite(hitsprite);
          return(0);
     case INACTIVE:
     case FLOATING:
          return(0);
     case PROJHIT:
     case PROJECTILE:
     case TOSS:
     case PAIN:
     case RUNTHRU:
     case TWITCH:
     case DEATH:
          return(0);
     default:
          break;
     }

     if( difficulty == 0 ) {
          points<<=1;
     }

     if( validext(ext) ) {
          if( (labs(points)) > sprXTptr[ext]->hitpoints ) {
               sprXTptr[ext]->hitpoints=0;
          }
          else {
               sprXTptr[ext]->hitpoints-=points;
          }
          sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask|=AI_TIMETODODGE;
          sprXTptr[ext]->aimask|=AI_WASATTACKED;
          if( sprXTptr[ext]->hitpoints < ENEMYCRITICALCONDITION )
               sprXTptr[ext]->aimask|=AI_CRITICAL;
          if( sprXTptr[ext]->hitpoints <= 0 ) {
               sprXTptr[ext]->hitpoints=0;
               //newstatus(hitsprite,DEATH);
               sprXTptr[ext]->fxmask|=FX_NXTSTTDEATH;
               return(1);
          }
          else {
               //newstatus(hitsprite,PAIN);
               sprXTptr[ext]->fxmask|=FX_NXTSTTPAIN;
          }
     }

     return(0);
}

void
attackifclose(short i, int snum, int dist)
{
     int      mindist;
     short     ext;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 13");
     }
    #endif

     if( !validplayer(snum) ) {
          crash("atakifclse: bad plrnum");
     }
     if( health[snum] <= 0 ) {
          return;
     }
     ext=sprptr[i]->extra;
     if( !validext(ext) ) {
          return;
     }

     switch( sprptr[i]->statnum ) {
     case CHASE:
          mindist=CHASEATTDIST;
          break;
     case PATROL:
     case GUARD:
          mindist=GUARDATTDIST;
          break;
     case STALK:
          mindist=STALKATTDIST;
          break;
     default:
          mindist=MINATTACKDIST;
          break;
     }

     switch( sprptr[i]->picnum ) {
     case RATPIC:
          mindist=1024L;
          break;
     case BATPIC:
          if( RMOD4("STAT1968") != 0 ) {
               return;
          }
          mindist=1024;
          if( dist <= 1024 ) {
               sprptr[i]->z=posz[snum]-4096;
          }

          break;
     default:
          break;
     }

	if ( (dist < mindist) ) {
          switch( sprptr[i]->statnum ) {
          default:
               sprptr[i]->ang=
                 getangle(posx[snum]-sprptr[i]->x,posy[snum]-sprptr[i]->y);               
               newstatus(i, ATTACK);
               break;
          }
     }

 return;
}

void
enemygunshot(int  sn) 
{
     int       j,nextj,ext;
     int      dist;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 14");
     }
    #endif

	j=headspritestat[STROLL];
    	while( j != -1 ) {
		nextj = nextspritestat[j];

          ext=sprptr[j]->extra;
          if( !validext(ext) ) {
               noextcrash(sn,100);
          }
     	dist = labs(sprptr[sn]->x-sprptr[j]->x)+labs(sprptr[sn]->y-sprptr[j]->y);
 		if( dist < HEARGUNSHOTDIST ) {
               sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
          }

		j = nextj;
	}

	j=headspritestat[STANDING];
    	while( j != -1 ) {
		nextj = nextspritestat[j];

          ext=sprptr[j]->extra;
          if( !validext(ext) ) {
               noextcrash(sn,101);
          }

          // guards not woken by enemies gun shot
          if( sprXTptr[ext]->basestat != GUARD ) {
     	     dist = labs(sprptr[sn]->x-sprptr[j]->x)+labs(sprptr[sn]->y-sprptr[j]->y);
 		     if( dist < HEARGUNSHOTDIST ) {
                    sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
               }
          }

		j = nextj;
	}

    #ifdef WAKEUPGUARDSBYENEMYGUNSHOT
	j=headspritestat[GUARD];
    	while( j != -1 ) {
		nextj = nextspritestat[j];

          if( j == sn ) {
               j=nextj;
               continue;
          }

          ext=sprptr[j]->extra;
          if( !validext(ext) ) {
               noextcrash(sn,102);
          }
     	dist = labs(sprptr[sn]->x-sprptr[j]->x)+labs(sprptr[sn]->y-sprptr[j]->y);
 		if( dist < HEARGUNSHOTDIST ) {
               sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
          }

		j = nextj;
	}
    #endif
}

void
playergunshot(int snum)
{
     int       j,nextj,ext;
     int      dist;

     if( !validplayer(snum) ) {
          crash("plgunsht: bad plrnum");
     }

	j=headspritestat[STROLL];
    	while( j != -1 ) {
		nextj = nextspritestat[j];

          ext=sprptr[j]->extra;
          if( !validext(ext) ) {
               noextcrash(j,103);
          }
     	dist = labs(sprptr[j]->x-posx[snum])+labs(sprptr[j]->y-posy[snum]);
 		if( dist < HEARGUNSHOTDIST ) {
               sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
          }

		j = nextj;
	}

	j=headspritestat[STANDING];
    	while( j != -1 ) {
		nextj = nextspritestat[j];

          ext=sprptr[j]->extra;
          if( !validext(ext) ) {
               noextcrash(j,105);
          }
     	dist = labs(sprptr[j]->x-posx[snum])+labs(sprptr[j]->y-posy[snum]);
 		if( dist < HEARGUNSHOTDIST ) {
               sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
          }

		j = nextj;
	}

	j=headspritestat[GUARD];
    	while( j != -1 ) {
		nextj = nextspritestat[j];

          ext=sprptr[j]->extra;
          if( !validext(ext) ) {
               noextcrash(j,106);
          }
     	dist = labs(sprptr[j]->x-posx[snum])+labs(sprptr[j]->y-posy[snum]);
 		if( dist < HEARGUNSHOTDIST ) {
               sprXTptr[ext]->aimask|=AI_JUSTSHOTAT;
          }

		j = nextj;
	}
}

void
enemywoundplayer(short plrhit, short sprnum, char guntype)
{
     int       damage=0;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sprnum) ) {
          crash("messing w plrsprite at 15");
     }
    #endif

     if( !validplayer(plrhit) ) {
          return;
     }
                               
     switch( guntype ) {
     case 0:
          damage=64;
          break;
     case 1:  
     case 2:
     case 3:
     case 4:
     case 5:
          damage=128;
          break; 
     case 6:
     default: 
          damage=256;   
          break;
     }
     switch( difficulty ) {
     case 0:
     case 1:
          damage>>=4;
          break;
     case 2:
          damage>>=3;
          break;
     case 3:
          damage>>=2;
          break;
     }

     changehealth(plrhit,-damage);
}

void                
enemyshootgun(short sprnum,int x,int y,int z,short daang,int dahoriz,
              short dasectnum,char guntype)
{
     short     hitsect,hitwall,hitsprite,daang2;
     int       j,daz2,hitx,hity,hitz,discrim;
     int       ext,target,pnum;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sprnum) ) {
          crash("messing w plrsprite at 16");
     }
    #endif

     ext=sprptr[sprnum]->extra;
     if( !validext(ext) ) {
          crash("enemyshootgun: bad ext");
     }
     target=sprXTptr[sprnum]->target;
     if( !validplayer(target) ) {
          return;
     }

     // enemy gun fire sounds
     switch (guntype) {
     case 0:
          switch( sprXTptr[ext]->attackpic ) {
          case RATATTACKPIC:
               break;
          case 3910:
               playsound(S_WH_8,x,y,0,ST_NOUPDATE);
               break;
          default:
               playsound(S_MATRIX_ATTACK2,x,y,0,ST_NOUPDATE);
               break;
          }
          break;
     case 1:
     case 2:
     case 3:
     case 4:
          switch( sprXTptr[ext]->attackpic ) {
          case 3773:
          case 3800:
          case 3805:
          case 3810:
          case 3818:
               playsound(S_MATRIX_ATTACK,x,y,0,ST_NOUPDATE);
               break;
          case 3850:
          case 3860:
          case 3890:
          case 3909:
          case 3952:
          case 4001:
               playsound(S_MATRIX_ATTACK2,x,y,0,ST_NOUPDATE);
               break;
          case COPATTACKPIC:
               playsound(S_ENEMYGUN1,x,y,0,ST_NOUPDATE);
               break;
          case ANTATTACKPIC:
               playsound(S_ENEMYGUN2,x,y,0,ST_NOUPDATE);
               break;
          case MAATTACKPIC:
               playsound(S_ENEMYGUN3,x,y,0,ST_NOUPDATE);
               break;
          case SAMATTACKPIC:
               playsound(S_ENEMYGUN4,x,y,0,ST_NOUPDATE);
               break;
          default:
               playsound(S_ENEMYGUN1,x,y,0,ST_NOUPDATE);
               break;
          }
          break;
     case 5:
     case 6:
     case 7:
     case 8:
          switch( sprXTptr[ext]->attackpic ) {
          case 3773:
          case 3800:
          case 3805:
          case 3810:
          case 3818:
               playsound(S_MATRIX_ATTACK,x,y,0,ST_NOUPDATE);
               break;
          case 3850:
          case 3860:
          case 3890:
          case 3909:
          case 3952:
          case 4001:
               playsound(S_MATRIX_ATTACK2,x,y,0,ST_NOUPDATE);
               break;
          default:
               playsound(S_AUTOGUN,x,y,0,ST_NOUPDATE);
          }
          break;
     default:
          break;
     }

     // gun shot code
     switch (guntype) {
     case 0:
          if( (labs(x-posx[target])+labs(y-posy[target])) < 1024 ) {
               if( labs(z-posz[target]) < 12288 ) {
                    playerpainsound(target);
                    if( ((sprXTptr[ext]->fxmask)&FX_HOLOGRAM) != 0 ) {
                         playsound(S_MATRIX_ATTACK,x,y,0,ST_NOUPDATE);
                         enemywoundplayer(target,sprnum,0);
                    }
                    if( ((sprXTptr[ext]->fxmask)&FX_ANDROID) != 0 ) {
                         androidexplosion(sprnum);
                         enemywoundplayer(target,sprnum,6);
                         //changespritestat(sprnum, VANISH);
                         jsdeletesprite(sprnum);
                    }
                    else {
                         enemywoundplayer(target,sprnum,0);
                    }
               }
          }
          return;
     case GUN3FLAG:               
     case GUN4FLAG:               
     case GUN5FLAG:               
          daang2=daang;
          daz2=((100-dahoriz)<<11);
          z=posz[target];     // instead of calculating a dot product angle
          hitscan(x,y,z,dasectnum,sintable[(daang2+2560)&2047],
               sintable[(daang2+2048)&2047],daz2,
               &hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);
          if( hitsprite > 0 ) {
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    enemywoundplayer(pnum,sprnum,3);
               }
               else {
                    damagesprite(hitsprite,tekgundamage(guntype,x,y,z,hitsprite));
               }
          }
          else {                   
               j=jsinsertsprite(hitsect, 3);
               if( j != -1 ) {
                    fillsprite(j,hitx,hity,hitz+(8<<8),2,-4,0,32,16,16,0,0,
                               EXPLOSION,daang,0,0,0,sprnum+MAXSPRITES,hitsect,3,63,0,0);
               }
          }
          break;
     case GUN6FLAG:               
	     j = jsinsertsprite(sprptr[sprnum]->sectnum, PROJECTILE);
          if ( j ==  -1 ) {
               break;
          }
	     sprptr[j]->x=sprptr[sprnum]->x;
	     sprptr[j]->y=sprptr[sprnum]->y;
          switch( sprXTptr[ext]->basestat ) {
          case FLY:
	          sprptr[j]->z=sprptr[sprnum]->z;
               break;
          default:
	          sprptr[j]->z=sectptr[sprptr[sprnum]->sectnum]->floorz-8192;
               break;
          }
	     sprptr[j]->cstat=0x01;      
	     sprptr[j]->picnum=PULSARPIC;
	     sprptr[j]->shade=-32;
	     sprptr[j]->xrepeat=32;
	     sprptr[j]->yrepeat=32;
	     sprptr[j]->ang=sprptr[sprnum]->ang;
	     sprptr[j]->xvel=(sintable[(sprptr[j]->ang+2560)&2047]>>4);
	     sprptr[j]->yvel=(sintable[(sprptr[j]->ang+2048)&2047]>>4);
          discrim=ksqrt((posx[target]-sprptr[j]->x)*(posx[target]-sprptr[j]->x) +
				    (posy[target]-sprptr[j]->y)*(posy[target]-sprptr[j]->y));
          if ( discrim == 0 ) {
               discrim=1;
          }
	     sprptr[j]->zvel=((posz[target]+(8<<8)-sprptr[j]->z)<<9)/discrim;
          sprptr[j]->owner=sprnum;
	     sprptr[j]->clipdist=16L;
	     sprptr[j]->lotag=0;
	     sprptr[j]->hitag=0;
          break;
     default:
          break;
     }

     if( guntype != 0 ) {
          enemygunshot(sprnum);
     }
}

int
isahologram(int i)
{
	int	ext=sprptr[i]->extra;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 21");
     }
    #endif

     if( validext(ext) ) {
          if( ((sprXTptr[ext]->fxmask)&FX_HOLOGRAM) != 0 ) {
               return(1);
          }
     }

     return(0);
}

int
isanandroid(int i)
{
	int	ext=sprptr[i]->extra;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 22");
     }
    #endif

     if( validext(ext) ) {
          if( ((sprXTptr[ext]->fxmask)&FX_ANDROID) != 0 ) {
               return(1);
          }
     }

     return(0);
}

void
enemynewsector(int i)
{
	int	ext=sprptr[i]->extra;

     if( option[4] != 0 ) {
          return;
     }

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 23");
     }
    #endif
     
     if( ensfirsttime == 1 ) {
          ensfirsttime=0;
          return;          
     }

     switch( sectptr[sprptr[i]->sectnum]->lotag ) {
     case 4:
          playsound(S_SPLASH,sprptr[i]->x,sprptr[i]->y,0,ST_UPDATE);
          break;
     default: 
          break;
     }
     switch( sectptr[sprptr[i]->sectnum]->hitag ) {
     case 1010:
     case 1011:
     case 1012:
     case 1013:
     case 1014:
     case 1015:
     case 1016:
     case 1017:
     case 1018:
     case 1019:
          if( isahologram(i) ) {
               jsdeletesprite(i);
               return;
          }
          if( tekexplodebody(i) ) {
               missionaccomplished(i);
          }
          playsound(S_BUSHIT, sprptr[i]->x,sprptr[i]->y, 0, ST_UPDATE);
          if( validext(ext) ) {
         	     switch( sprXTptr[ext]->basepic ) {
			case ERWALKPIC:
                 playsound(S_MANDIE1+RMOD4("STAT2615")+RMOD3("STAT2615"),sprptr[i]->x,sprptr[i]->y,0,ST_UPDATE);
          		break;
			case SARAHWALKPIC:
                 playsound(S_GIRLDIE1+RMOD4("STAT2618")+RMOD3("STAT2618"),sprptr[i]->x,sprptr[i]->y,0,ST_UPDATE);
          	     break;
			case SAMWALKPIC:
                 playsound(S_MANDIE1+RMOD4("STAT2621")+RMOD3("STAT2621"),sprptr[i]->x,sprptr[i]->y,0,ST_UPDATE);
          	     break;
         		}
    		}
          jsdeletesprite(i);
          break;
     default: 
          operatesector(sprptr[i]->sectnum);  // JEFF TEST
          break;
     }
}

void
ambushyell(short sn, short ext)
{
    #ifdef PLRSPRDEBUG
     if( isaplayersprite(sn) ) {
          crash("messing w plrsprite at 24");
     }
    #endif

     if( !validext(ext) ) {
          return;
     }
     if( sprXTptr[ext]->aimask&AI_DIDAMBUSHYELL )
          return;

     switch( sprXTptr[ext]->basepic ) {
     case ANTWALKPIC:
     case RUBWALKPIC:
          playsound(S_MALE_COMEONYOU+(krand_intercept("STAT2603")&6),sprptr[sn]->x,sprptr[sn]->y,0,ST_IMMEDIATE);
          sprXTptr[ext]->aimask|=AI_DIDAMBUSHYELL;
          break;
     case DIWALKPIC:
          playsound(S_DIM_WANTSOMETHIS+(krand_intercept("STAT2607")&2),sprptr[sn]->x,sprptr[sn]->y,0,ST_IMMEDIATE);
          sprXTptr[ext]->aimask|=AI_DIDAMBUSHYELL;
          break;
     }
}

void
givewarning(short i, short ext)
{
     int dist = 0L;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 25");
     }
    #endif

     if( option[4] != 0 ) {
          return;
     }
     if( !validext(ext) ) {
          return;
     }
     if( sprXTptr[ext]->aimask&AI_GAVEWARNING )
          return;

     dist=labs(posx[screenpeek]-sprptr[i]->x)+labs(posy[screenpeek]-sprptr[i]->y);
     if( dist > 5000 ) {  
          return;
     }

     switch( sprXTptr[ext]->basepic ) {
     case COP1WALKPIC:
          playsound(S_GRD_WHATDOINGHERE+(krand_intercept("STAT2636")&4),sprptr[i]->x,sprptr[i]->y,0,ST_UNIQUE);
          sprXTptr[ext]->aimask|=AI_GAVEWARNING;
          break;
     }
}

void
statuslistcode()
{
	short     p, target, hitobject, daang, osectnum, movestat, hitsprite,ext;
	int      i, nexti, dax, day, daz, dist, mindist;
     int      prevx,prevy,prevz;
     short     prevsect=0;
     int       seecan;
     int      targx,targy,targz;
     short     targang,targsect,host,tempshort;
     int       pnum;
     int      px,py,pz,deltapy,zoffs;
     spritetype     *spr;

     dosectorflash();

    #ifdef DEBUGBREAKHERE
     if( keystatus[37] != 0 ) {
          keystatus[37]=0;
          i=1;
     }
    #endif

     if( (lockclock-stackedcheck) > 30 ) {
          stackedcheck=lockclock;
	     i = headspritestat[STACKED]; 
	     while (i >= 0)
	     {
		     nexti = nextspritestat[i];
               
               if( sprptr[i]->z != sectptr[sprptr[i]->sectnum]->floorz ) { 
                    spr=sprptr[i];
	               tempshort=spr->cstat; 
                    spr->cstat&=~1;
               	getzrange(spr->x,spr->y,spr->z-1,spr->sectnum,
     				     &globhiz,&globhihit,&globloz,&globlohit,
     				     ((int)spr->clipdist)<<2,CLIPMASK1);
               	spr->cstat=tempshort;
                    if( spr->z != globloz ) {
                         spr->hitag=0;
                         changespritestat(i, FALL); 
                    }
               }
              #ifdef  VERIFYSTATS
               verifystatus(nexti,STACKED);
              #endif
		     i = nexti;
	     }
     }

	i = headspritestat[VANISH];
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          jsdeletesprite(i);

		i = nexti;
	}

	i = headspritestat[FALL];
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          spr=sprptr[i];

          spr->z+=(TICSPERFRAME<<11);

          tempshort=spr->cstat; 
          spr->cstat&=~1;
         	getzrange(spr->x,spr->y,spr->z-1,spr->sectnum,
			     &globhiz,&globhihit,&globloz,&globlohit,
     	          ((int)spr->clipdist)<<2,CLIPMASK1);
         	spr->cstat=tempshort;
          if( spr->z >= globloz ) {
               spr->z=globloz;
               if( globloz < sectptr[spr->sectnum]->floorz ) {
                    changespritestat(i, STACKED);
               }
               else {
                    changespritestat(i, INANIMATE);
               }
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,FALL);
         #endif
		i = nexti;
	}

	i = headspritestat[3];     //Go through smoke sprites
	while (i >= 0)
	{
		nexti = nextspritestat[i];

		sprite[i].z -= (TICSPERFRAME<<3);
		sprite[i].lotag -= TICSPERFRAME;
		if (sprite[i].lotag < 0)
			jsdeletesprite(i);

         #ifdef  VERIFYSTATS
          verifystatus(nexti,3);
         #endif
		i = nexti;
	}

	i = headspritestat[5];     //Go through explosion sprites
	while (i >= 0)
	{
		nexti = nextspritestat[i];

		sprite[i].lotag -= TICSPERFRAME;
		if (sprite[i].lotag < 0)
			jsdeletesprite(i);

         #ifdef  VERIFYSTATS
          verifystatus(nexti,5);
         #endif
		i = nexti;
	}

	i=headspritestat[RUNTHRU];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

		sprptr[i]->hitag++;

          if( sprptr[i]->hitag >= TICSPERFRAME ) {
               sprptr[i]->picnum++;
               sprptr[i]->hitag=0;
               sprptr[i]->lotag--;
          }
		if( sprptr[i]->lotag <= 0 ) {
               jsdeletesprite(i);
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,RUNTHRU);
         #endif
		i = nexti;
	}

	i=headspritestat[FLOATING];
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

          // bob on surface
          sprptr[i]->hitag++;
          if( sprptr[i]->hitag >= (TICSPERFRAME) ) {
               sprptr[i]->lotag++;
               if( sprptr[i]->lotag >= MAXBOBS ) {
                    sprptr[i]->lotag=0;
               }
               sprptr[i]->z+=(bobbing[sprptr[i]->lotag]<<4);
               sprptr[i]->hitag=0;
          }
	     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
	     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
          movestat=floatmovesprite(i,dax,day,0L,1024L,1024L,NORMALCLIP);
          if( (movestat&0xC000) == 32768 ) {
               setsprite(i, prevx,prevy,prevz);
               sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
          }
          else if( (movestat&0xC000) == 49152 ) {
               setsprite(i, prevx,prevy,prevz);
               sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
          }
          else if( prevsect != sprptr[i]->sectnum ) {
               setsprite(i, prevx,prevy,prevz);
               sprptr[i]->ang=arbitraryangle();
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,FLOATING);
         #endif
		i = nexti;
	}

	i=headspritestat[PINBALL];
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,19);
          }
          sprXTptr[ext]->target=0;

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

	     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
	     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
          movestat=flymovesprite(i,dax,day,0L,1024L,1024L,NORMALCLIP);

          if( (movestat&0xC000) == 32768 ) {
               setsprite(i, prevx,prevy,prevz);
               sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
               movestat=flymovesprite(i,dax,day,0L,1024L,1024L,NORMALCLIP);
          }
          else if( (movestat&0xC000) == 49152 ) {
               hitsprite=(movestat&0x0FFF);
               setsprite(i, prevx,prevy,prevz);
               if( playerhit(hitsprite, &pnum) ) {
                    sprXTptr[ext]->target=pnum;
                    newstatus(i, ATTACK);
               }
               else {
                    sprptr[i]->ang=spritedeflect(hitsprite,sprptr[i]->ang);
                    movestat=flymovesprite(i,dax,day,0L,1024L,1024L,NORMALCLIP);
               }
          }
          if( movestat != 0 ) {
               sprptr[i]->ang=arbitraryangle();
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,PINBALL);
         #endif
		i = nexti;
	}

	i=headspritestat[TIMEBOMB];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

		sprptr[i]->hitag--;
          if( sprptr[i]->hitag <= 0 ) {
               if( sectptr[sprptr[i]->sectnum]->lotag == 4 ) {
                    splash(i);
               }
               changespritestat(i,INACTIVE);
               genexplosion2(i);
               jsdeletesprite(i);
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,TIMEBOMB);
         #endif
		i = nexti;
	}

	i=headspritestat[BLOODFLOW];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

		sprptr[i]->hitag++;

          if( sprptr[i]->hitag >= (TICSPERFRAME*6) ) {
               sprptr[i]->picnum++;
               sprptr[i]->hitag=0;
               sprptr[i]->lotag++;
          }
		if( sprptr[i]->lotag == 5 ) {
               changespritestat(i, INANIMATE);
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,BLOODFLOW);
         #endif
		i = nexti;
	}

	i = headspritestat[DROPSIES]; 
	while (i != -1)
	{
		nexti = nextspritestat[i];

		dax = ((((int)sprite[i].xvel)*TICSPERFRAME)<<7);
		day = ((((int)sprite[i].yvel)*TICSPERFRAME)<<7);
		daz = 0;

		osectnum = sprite[i].sectnum;
		hitobject = movesprite((short)i,dax,day,daz,4L<<8,4L<<8,1);

		sprite[i].z += sprite[i].zvel;
		sprite[i].zvel += (TICSPERFRAME<<2);
		if (sprite[i].z < globhiz+(tilesizy[KLIPPIC]<<6)) {
			sprite[i].z = globhiz+(tilesizy[KLIPPIC]<<6);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
		if (sprite[i].z > globloz-(tilesizy[KLIPPIC]<<6)) {
			sprite[i].z = globloz-(tilesizy[KLIPPIC]<<6);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}

		dax = sprite[i].xvel; day = sprite[i].yvel;
		dist = dax*dax+day*day;
		if (dist < 46000) {
               if( sectptr[sprptr[i]->sectnum]->lotag == 4 ) {
          	     if( (sprptr[i]->cstat&128) == 0 ) {
		               zoffs = -((tilesizy[sprptr[i]->picnum]*sprptr[i]->yrepeat)<<1);
                    }
          	     else {
		               zoffs = 0;
                    }
                    sprptr[i]->z=sectptr[sprptr[i]->sectnum]->floorz-zoffs;
                    changespritestat(i, FLOATING);
                    sprptr[i]->lotag=0;
                    sprptr[i]->hitag=0;
                    sprptr[i]->xvel=1;
                    sprptr[i]->yvel=1;
               }
               else {
     			changespritestat(i, INANIMATE);
               }
               sprite[i].z=sectptr[sprite[i].sectnum]->floorz;
               switch( sprptr[i]->picnum ) {
               case KLIPPIC:
                    sprptr[i]->z-=2048;
                    break;
               default:
                    break;
               }
			goto dropsiescontinue;
		}
        if (mulscale(krand_intercept("STAT2934"),dist,30) == 0) {
			sprite[i].xvel -= ksgn(sprite[i].xvel);
			sprite[i].yvel -= ksgn(sprite[i].yvel);
			sprite[i].zvel -= ksgn(sprite[i].zvel);
		}

dropsiescontinue:
         #ifdef  VERIFYSTATS
          verifystatus(nexti,DROPSIES);
         #endif
		i = nexti;
	}

	i = headspritestat[7];     //Go through bomb spriral-explosion sprites
	while (i >= 0)
	{
		nexti = nextspritestat[i];

		sprite[i].x += ((sprite[i].xvel*TICSPERFRAME)>>4);
		sprite[i].y += ((sprite[i].yvel*TICSPERFRAME)>>4);
		sprite[i].z += ((sprite[i].zvel*TICSPERFRAME)>>4);

		sprite[i].zvel += (TICSPERFRAME<<7);
		if (sprite[i].z < sector[sprite[i].sectnum].ceilingz+(4<<8))
		{
			sprite[i].z = sector[sprite[i].sectnum].ceilingz+(4<<8);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
		if (sprite[i].z > sector[sprite[i].sectnum].floorz-(4<<8))
		{
			sprite[i].z = sector[sprite[i].sectnum].floorz-(4<<8);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}

		sprite[i].xrepeat = (sprite[i].lotag>>2);
		sprite[i].yrepeat = (sprite[i].lotag>>2);

		sprite[i].lotag -= TICSPERFRAME;
		if (sprite[i].lotag < 0)
			jsdeletesprite(i);

         #ifdef  VERIFYSTATS
          verifystatus(nexti,7);
         #endif
		i = nexti;
	}

	i = headspritestat[TOSS];    
	while (i != -1)
	{
		nexti = nextspritestat[i];

		dax = ((((int)sprite[i].xvel)*TICSPERFRAME)<<11);
		day = ((((int)sprite[i].yvel)*TICSPERFRAME)<<11);
		daz = 0;
		movestat=kenmovesprite((short)i,dax,day,daz,4L<<8,4L<<8,1);
          switch( movestat&0xC000 ) {
          // break stuff, but dont hurt enemies unless direct hit on head
          case 49152:
               hitsprite=(movestat&0x0FFF);
               ext=sprptr[hitsprite]->extra;
               if( validext(ext) && (sprXTptr[ext]->deathpic != 0) ) {
                    switch( sprptr[hitsprite]->statnum ) {
                    case INANIMATE:
                         damagesprite(hitsprite, 20);
                         break;
                    }
               }
               break;
          }

		sprite[i].z += sprite[i].zvel;
		sprite[i].zvel += (TICSPERFRAME<<5);
		if( sprite[i].z < globhiz+(tilesizy[BOMB]<<6) ) {
			sprite[i].z = globhiz+(tilesizy[BOMB]<<6);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
		if( sprite[i].z > globloz-(tilesizy[BOMB]<<4) ) {
               switch( globlohit&0xC000 ) {
               case 49152: 
                    // direct hit on head
                    hitsprite=globlohit&0x0FFF;
                    if( playerhit(hitsprite, &pnum) ) {
                         changehealth(pnum, -40);                    
                    }
                    else if( sprptr[hitsprite]->extra != -1 ) {
                         playsound(S_BUSHIT,sprptr[hitsprite]->x,sprptr[hitsprite]->y,0,ST_NOUPDATE);
                         damagesprite(hitsprite, 10);
                    }
                    break;
		     case 16384:
                    if( (sectptr[globlohit&0x0FFF]->lotag == 4) && (sprptr[i]->picnum != RATTHROWPIC) ) {
          	          if( (sprptr[i]->cstat&128) == 0 ) {
		                    zoffs = -((tilesizy[sprptr[i]->picnum]*sprptr[i]->yrepeat)<<1);
                         }
          	          else {
		                    zoffs = 0;
                         }
                         sprptr[i]->z=sectptr[sprptr[i]->sectnum]->floorz-zoffs;
                         splash(( int)i);
                         switch( sprptr[i]->picnum ) {
                         case TUBEBOMB:
          	               sprptr[i]->picnum=TUBEBOMB+1;
                              sprptr[i]->hitag=(krand_intercept("STAT3031")&255);
                              changespritestat(i, TIMEBOMB);
                              break;
                         case DARTBOMB:
                              sprptr[i]->hitag=(krand_intercept("STAT3035")&255);
          	               sprptr[i]->picnum=DARTBOMB+1;
                              changespritestat(i, TIMEBOMB);
                              break;
                         default:
                              sprptr[i]->lotag=0;
                              sprptr[i]->hitag=0;
                              sprptr[i]->xvel=1;
                              sprptr[i]->yvel=1;
                              changespritestat(i, FLOATING);
                              break;
                         }
                         goto tosscontinue;
                    }
                    break;
               }
			sprite[i].z = globloz-(tilesizy[BOMB]<<4);
			sprite[i].zvel = -(sprite[i].zvel>>1);
               sprptr[i]->hitag++;
		}
		dax = sprite[i].xvel; day = sprite[i].yvel;
		dist = dax*dax+day*day;
          if( mulscale(krand_intercept("STAT3057"),dist,30) == 0 ) {
			sprite[i].xvel -= ksgn(sprite[i].xvel);
			sprite[i].yvel -= ksgn(sprite[i].yvel);
			sprite[i].zvel -= ksgn(sprite[i].zvel);
		}
          if( sprptr[i]->hitag >= 3 ) {
               switch( sprptr[i]->picnum ) {
               case RATTHROWPIC:
                    ext=sprptr[i]->extra;
                    if( !validext(ext) ) {
                         jsdeletesprite(i);
                         break;
                    }
          	     sprptr[i]->z=sectptr[sprptr[i]->sectnum]->floorz;
                    sprptr[i]->ang=arbitraryangle();
                    sprptr[i]->picnum=RATPIC;
                    sprXTptr[ext]->basestat=RODENT;
                    newstatus(i, RODENT);                    
                    sprptr[i]->xvel=4;
                    sprptr[i]->yvel=4;
                    sprptr[i]->zvel=0;
                    sprptr[i]->lotag=2004;
                    sprptr[i]->hitag=0;
                    break;
               case TUBEBOMB:
          	     sprptr[i]->picnum=TUBEBOMB+1;
                    sprptr[i]->hitag=(krand_intercept("STAT3083")&255);
                    changespritestat(i, TIMEBOMB);
                    break;
               case DARTBOMB:
          	     sprptr[i]->picnum=DARTBOMB+1;
                    sprptr[i]->hitag=(krand_intercept("STAT3088")&255);
                    changespritestat(i, TIMEBOMB);
                    break;
               default:
                    sprptr[i]->xvel=0;
                    sprptr[i]->yvel=0;
                    sprptr[i]->zvel=0;
                    sprptr[i]->lotag=2004;
                    if( globloz != sectptr[sprptr[i]->sectnum]->floorz ) {
                         sprptr[i]->z=globloz;
                         changespritestat(i, STACKED);
                    }
                    else {
                         changespritestat(i, INANIMATE);
                    }
                    break;
               }
          }

tosscontinue:
         #ifdef  VERIFYSTATS
          verifystatus(nexti,TOSS);
         #endif
          i = nexti;
     }

     if( (activemenu != 0) && (option[4] ==0) ) {
          goto menuison;
     }

	i = headspritestat[AMBUSH];
	while (i >= 0) 
     {
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,1);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto ambushcontinue;
          }

          dist=mindist;
          sprXTptr[ext]->target=target;

          if( (sprXTptr[i]->aimask&AI_JUSTSHOTAT) || isvisible(i, target) ) {
               if( sprXTptr[ext]->morphpic != 0 ) {
                   newstatus(i, MORPH);
               }
               else {
                    ambushyell(i, sprptr[i]->extra);
                    newstatus(i, sprXTptr[ext]->basestat);
               }
          }

ambushcontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,AMBUSH);
         #endif
          i = nexti;
	}

     i = headspritestat[STALK];   
	while (i >= 0)
	{    
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,2);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto stalkcontinue;
          }
          dist=mindist;
          sprXTptr[ext]->target=target;

          targx=posx[target];
          targy=posy[target];
          targz=posz[target];
          targang=ang[target];
          targsect=cursectnum[target];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

          daang=(getangle(targx-sprptr[i]->x,targy-sprptr[i]->y)&2047);

          // USES SPRITESORTLIST - NOT MULTIPLAYER COMPATIBLE !

		if( ((sprXTptr[ext]->aimask)&AI_WASDRAWN) != 0 ) {
		     if( ((sprptr[i]->ang+2048-daang)&2047) < 1024 ) {
			     sprptr[i]->ang=((sprptr[i]->ang+2048-(TICSPERFRAME<<1))&2047);
               }
			else {
			     sprptr[i]->ang=((sprptr[i]->ang+(TICSPERFRAME<<1))&2047);
               }
               if( RMOD16("STAT3291") == 0 )
                    attackifclose(i, target, dist); 
               if( sprptr[i]->statnum != ATTACK ) {
                    daang=((daang+1024)&2047);
			     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
			     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			     movestat=movesprite(( short)i,dax,day,0L,1024L,1024L,CLIFFCLIP);
                    switch( movestat&0xC000 ) {
                    case 32768:              // blocked by a wall
                         sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
                         break;
                    case 49152:              // blocked by a sprite
                         sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
                         break;
			     case 16384:
                         sprptr[i]->ang=arbitraryangle();
                         break;
                    }
                    if( movestat != 0 ) {
			          dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
			          day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			          movestat=movesprite(( short)i,dax,day,0L,1024L,1024L,CLIFFCLIP);
                         if( movestat != 0 )
                              sprptr[i]->ang=arbitraryangle();
                    }
               }
          }
          else { 
			if( ((sprptr[i]->ang+2048-daang)&2047) < 1024 ) {
			     sprptr[i]->ang=((sprptr[i]->ang+2048-(TICSPERFRAME<<1))&2047);
               }
			else {
			     sprptr[i]->ang=((sprptr[i]->ang+(TICSPERFRAME<<1))&2047);
               }
		     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<4 );
		     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<4 );
			movestat=movesprite(( short)i,dax,day,0L,1024L,1024L,CLIFFCLIP);
               switch( movestat&0xC000 ) {
               case 32768:              // blocked by a wall
                    newstatus(i, SQUAT);
                    sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
                    break;
               case 49152:              // blocked by a sprite
                    newstatus(i, SQUAT);
                    sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
                    break;
		     case 16384:
                    sprptr[i]->ang=arbitraryangle();
                    break;
               }
               if( movestat != 0 ) {
			     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
			     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			     movestat=movesprite(( short)i,dax,day,0L,1024L,1024L,CLIFFCLIP);
                    if( movestat != 0 )
                         sprptr[i]->ang=arbitraryangle();
               }
          }

          if( sprptr[i]->sectnum != prevsect ) {
               if( sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_OFFLIMITS_ALL ) {
                    setsprite(i,prevx,prevy,prevz);
                    sprptr[i]->ang=arbitraryangle();
               }
               else {
                    enemynewsector(i);
               }
          }

stalkcontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,STALK);
         #endif
		i = nexti;
	}

     i = headspritestat[CHASE];   
	while (i >= 0)
	{    
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,3);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto chasecontinue;
          }
          dist=mindist;
          sprXTptr[ext]->target=target;

          targx=posx[target];
          targy=posy[target];
          targz=posz[target];
          targang=ang[target];
          targsect=cursectnum[target];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

          daang=(getangle(targx-sprptr[i]->x,targy-sprptr[i]->y)&2047);

          if( (option[4] == 0) && (sprXTptr[ext]->aimask)&AI_CRITICAL ) {
               if( ((sprXTptr[ext]->fxmask)&(FX_ANDROID|FX_HOLOGRAM)) == 0 ) {
                    sprptr[i]->ang=((daang+1024)&2047);
                    newstatus(i, FLEE);
                    if( sprptr[i]->statnum == FLEE )
                         goto chasecontinue;
               }
          }

          // can player see target if they squat ?
          seecan=0;
		if( cansee(targx,targy,targz,targsect,
                     sprptr[i]->x,sprptr[i]->y,sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<6),sprptr[i]->sectnum) == 1) {
               seecan=1;
          }

		if( seecan == 1 ) {
			if( ((sprptr[i]->ang+2048-daang)&2047) < 1024 ) {
			     sprptr[i]->ang=((sprptr[i]->ang+2048-(TICSPERFRAME<<1))&2047);
               }
			else {
			     sprptr[i]->ang=((sprptr[i]->ang+(TICSPERFRAME<<1))&2047);
               }
               if( RMOD4("STAT3427") == 0 ) {
                    attackifclose(i, target, dist); 
               }
               if( sprptr[i]->statnum != ATTACK ) {
			     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
			     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			     movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
                    switch( movestat&0xC000 ) {
                    case 32768:              // blocked by a wall
                         sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
                         break;
                    case 49152:              // blocked by a sprite
                         sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
                         break;
			     case 16384:
                         sprptr[i]->ang=arbitraryangle();
                         break;
                    }
                    if( movestat != 0 ) {
			          dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
			          day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			          movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
                         if( movestat != 0 )
                              sprptr[i]->ang=arbitraryangle();
                    }
               }
          }
          else { 
			if( ((sprptr[i]->ang+2048-daang)&2047) < 1024 ) {
			     sprptr[i]->ang=((sprptr[i]->ang+2048-(TICSPERFRAME<<1))&2047);
               }
			else {
			     sprptr[i]->ang=((sprptr[i]->ang+(TICSPERFRAME<<1))&2047);
               }
		     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
		     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
               switch( movestat&0xC000 ) {
               case 32768:              // blocked by a wall
                    newstatus(i, SQUAT);
                    sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
                    break;
               case 49152:              // blocked by a sprite
                    newstatus(i, SQUAT);
                    sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
                    break;
		     case 16384:
                    sprptr[i]->ang=arbitraryangle();
                    break;
               }
               if( movestat != 0 ) {
			     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
			     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			     movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
                    if( movestat != 0 )
                         sprptr[i]->ang=arbitraryangle();
               }
          }
          if( sprptr[i]->sectnum != prevsect ) {
               if( sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_OFFLIMITS_ALL ) {
                    setsprite(i,prevx,prevy,prevz);
                    sprptr[i]->ang=arbitraryangle();
               }
               else {
                    enemynewsector(i);
               }
          }

chasecontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,CHASE);
         #endif
		i = nexti;
	}
     
     i=headspritestat[GUARD];
	while (i >= 0)
	{    
          nexti=nextspritestat[i];
            
          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,4);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto guardcontinue;
          }
          dist=mindist;
          sprXTptr[ext]->target=target;

          targx=posx[target];
          targy=posy[target];
          targz=posz[target];
          targang=ang[target];
          targsect=cursectnum[target];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

          if( ((sprXTptr[ext]->aimask)&AI_JUSTSHOTAT) ) { 
	          if( cansee(targx,targy,targz,targsect, sprptr[i]->x,sprptr[i]->y,
                          sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<7),sprptr[i]->sectnum) == 1 ) {
                    sprptr[i]->ang=getangle(targx-sprptr[i]->x,targy-sprptr[i]->y);               
                    sprXTptr[ext]->aimask|=AI_WASATTACKED;  // guard needs to take action
               }
               goto guardcontinue;
          }
                                            
          if( (drawweap[target]) && isvisible(i, target) && !((sprXTptr[ext]->aimask)&(AI_WASATTACKED|AI_ENCROACHMENT)) ) {
                 givewarning(i, ext);
                 sprptr[i]->ang=getangle(targx-sprptr[i]->x,targy-sprptr[i]->y);               
                 sprptr[i]->picnum=sprXTptr[ext]->attackpic+1; 
                 if( dist < 1024 )
                    newstatus(i, ATTACK);
                 goto guardcontinue;
          }
          else {
                 sprptr[i]->picnum=sprXTptr[ext]->basepic;
          }

          switch( (sprXTptr[ext]->aimask)&(AI_WASATTACKED|AI_ENCROACHMENT) ) {
          case 0:
               sprXTptr[ext]->aimask&=~AI_GAVEWARNING;
               if( RMOD16("STAT3561") == 0 ) {
                    sprptr[i]->ang=getangle(targx-sprptr[i]->x,targy-sprptr[i]->y);               
                    newstatus(i, STANDING);
               }
               else {
	               dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
	               day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
			     movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
                    if( movestat != 0 ) {
                         sprptr[i]->ang=arbitraryangle();
                    }
               }
               break;
          default:
              if( (RMOD4("STAT3575") == 0) && (cansee(targx,targy,targz,targsect, sprptr[i]->x,sprptr[i]->y,
                          sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<7),sprptr[i]->sectnum) == 1) ) {
                    sprptr[i]->ang=getangle(targx-sprptr[i]->x,targy-sprptr[i]->y);               
                    newstatus(i, ATTACK);
               }
               else {
	               dax=( ((sintable[(sprptr[i]->ang+512)&2047])*(sprptr[i]->xvel+2)) <<3 );
	               day=( ((sintable[sprptr[i]->ang])*(sprptr[i]->yvel+2)) <<3 );
			     movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
                    if( movestat != 0 ) {
                         sprptr[i]->ang=arbitraryangle();
                    }
               }
               break;
          }

          if( (prevsect != sprptr[i]->sectnum) ) {
               setsprite(i,prevx,prevy,prevz);
               sprptr[i]->ang=arbitraryangle();
          }

guardcontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,GUARD);
         #endif
		i = nexti;
     }

     i = headspritestat[FLEE];   
	while (i >= 0)
	{    
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,5);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( (drawweap[target] == 0) || (mindist > DONTBOTHERDISTANCE) ) {
               newstatus(i, sprXTptr[ext]->basestat);
               goto fleecontinue;
          }

          dist=mindist;
          sprXTptr[ext]->target=target;

          targx=posx[target];
          targy=posy[target];
          targz=posz[target];
          targang=ang[target];
          targsect=cursectnum[target];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

         #define FLEESPEED 5
          dax=( ((sintable[(sprptr[i]->ang+512)&2047])*FLEESPEED) <<3 );
          day=( ((sintable[sprptr[i]->ang])*FLEESPEED) <<3 );
          movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);

          if( movestat != 0 ) {
               switch( movestat&0xC000 ) {
               case 32768:              // blocked by a wall
                    sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
                    break;
               case 49152:              // blocked by a sprite
                    sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
                    break;
		     case 16384:
                    sprptr[i]->ang=arbitraryangle();
                    break;
               }
	          if( cansee(targx,targy,targz,targsect,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<7),sprptr[i]->sectnum) == 1 ) {
                    attackifclose(i, target, dist);
                    if( sprptr[i]->statnum != ATTACK ) {
                         daang=getangle(targx-sprptr[i]->x,targy-sprptr[i]->y);               
                         sprptr[i]->ang=((daang+1024)&2047);
                         movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
                         if( (movestat != 0) && RMOD2("STAT3663") )
                              newstatus(i, HIDE);
                    }
               }
               else {
                    if( RMOD3("STAT3668") == 0 )
                         newstatus(i, HIDE);
               }
          }
           
          if( sprptr[i]->sectnum != prevsect ) {
               if( sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_OFFLIMITS_ALL ) {
                    setsprite(i,prevx,prevy,prevz);
                    sprptr[i]->ang=arbitraryangle();
               }
               else {
                    enemynewsector(i);
               }
          }

fleecontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,FLEE);
         #endif
		i = nexti;
	}

     i=headspritestat[STROLL];
     while( i >= 0 ) 
     {
          nexti=nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,6);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto strollcontinue;
          }
          dist=mindist;
          sprXTptr[ext]->target=target;

          targx=posx[target];
          targy=posy[target];
          targz=posz[target];
          targang=ang[target];
          targsect=cursectnum[target];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

          if( ((sprXTptr[ext]->aimask)&AI_JUSTSHOTAT) != 0 ) {
               attackifclose(i, target, dist);
               if( sprptr[i]->statnum != ATTACK ) {
                    daang=(getangle(posx[target]-sprptr[i]->x,posy[target]-sprptr[i]->y)&2047);
                    sprptr[i]->ang=((daang+1024)&2047);
                    fleescream(i, ext);
                    newstatus(i, FLEE);
               }
               goto strollcontinue;
          }                    

          if( (drawweap[target]) && isvisible(i, target) ) {
               daang=(getangle(posx[target]-sprptr[i]->x,posy[target]-sprptr[i]->y)&2047);
               sprptr[i]->ang=((daang+1024)&2047);
              #define HIDEDISTANCE      4096 
               if( dist < HIDEDISTANCE ) {
                    newstatus(i, HIDE);
               }
               if( sprptr[i]->statnum == HIDE ) {
                    hideplea(i, ext);
               }
               else {
                    fleescream(i, ext);
                    newstatus(i, FLEE);
               }
               goto strollcontinue;
          }   

	     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
	     day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );
          movestat=movesprite(( short)i,dax,day,0L,1024,1024,CLIFFCLIP);
          if( movestat != 0 ) {
               if( (RMOD10("STAT3757") == 0) && (sectptr[sprptr[i]->sectnum]->lotag != SECT_LOTAG_NOSTANDING) ) {
                    sprptr[i]->ang=((sprptr[i]->ang+1024)&2047);
                    newstatus(i, STANDING);
               }
               else {
                    switch( movestat&0xC000 ) {
                    case 32768:              // blocked by a wall
                         sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
                         break;
                    case 49152:              // blocked by a sprite
                         sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
                         break;
			     case 16384:
                         sprptr[i]->ang=arbitraryangle();
                         break;
                    }
               }
          }

          if( sprptr[i]->sectnum != prevsect ) {
               if( (sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_OFFLIMITS_CIVILLIAN) ||
                   (sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_OFFLIMITS_ALL) ) {  
                    setsprite(i,prevx,prevy,prevz);
                    sprptr[i]->ang=arbitraryangle();
               }
               else {
                    enemynewsector(i);
               }
          }

strollcontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,STROLL);
         #endif
		i = nexti;
     }

     i = headspritestat[FLY];   
     while( i >= 0 ) 
     {
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,7);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto flycontinue;
          }
          dist=mindist;
          sprXTptr[ext]->target=target;

          if( sprXTptr[ext]->basepic != AUTOGUN ) {
		     dax=( ((sintable[(sprptr[i]->ang+512)&2047])*((sprptr[i]->xvel)<<1)) <<3 );
		     day=( ((sintable[sprptr[i]->ang])*((sprptr[i]->yvel)<<1)) <<3 );
               daz=0L;
		     movestat = flymovesprite(i,dax,day,daz,1024,1024,NORMALCLIP);
               if( movestat != 0 ) {
                    sprptr[i]->ang=arbitraryangle();
               }
          }
          if( cansee(posx[target],posy[target],posz[target],cursectnum[target],
              sprptr[i]->x,sprptr[i]->y,sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<7),sprptr[i]->sectnum) ) {
               if( sprXTptr[ext]->weapon == 0 ) {
                    if( dist < 5120 ) {
                         sprptr[i]->ang=getangle(posx[target]-sprptr[i]->x,posy[target]-sprptr[i]->y);               
                    }
                    if( dist < 1024 ) {
                         sprptr[i]->ang=getangle(posx[target]-sprptr[i]->x,posy[target]-sprptr[i]->y);               
                         if( RMOD4("STAT3835") == 0 ) {
                              newstatus(i, ATTACK);
                         }
                    }
               }
               else {
                    attackifclose(i, target, dist);
               }
          }

flycontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,FLY);
         #endif
		i = nexti;
	}

     i = headspritestat[RODENT];   
	while (i >= 0)
	{    
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,3);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( mindist > DONTBOTHERDISTANCE ) {
               goto rodentcontinue;
          }
          dist=mindist;
          sprXTptr[ext]->target=target;

          targx=posx[target];
          targy=posy[target];
          targz=posz[target];
          targang=ang[target];
          targsect=cursectnum[target];

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;
          prevsect=sprptr[i]->sectnum;

		if( ((sprptr[i]->ang+2048-daang)&2047) < 1024 ) {
		     sprptr[i]->ang=((sprptr[i]->ang+2048-(TICSPERFRAME<<1))&2047);
          }
		else {
		     sprptr[i]->ang=((sprptr[i]->ang+(TICSPERFRAME<<1))&2047);
          }
		dax=( ((sintable[(sprptr[i]->ang+512)&2047])*sprptr[i]->xvel) <<3 );
		day=( ((sintable[sprptr[i]->ang])*sprptr[i]->yvel) <<3 );

		movestat=movesprite(( short)i,dax,day,0L,1024L,1024L,2);

          switch( movestat&0xC000 ) {
          case 32768:              // blocked by a wall
               sprptr[i]->ang=walldeflect(movestat&0x0FFF,sprptr[i]->ang);
               break;
          case 49152:              // blocked by a sprite
               sprptr[i]->ang=spritedeflect(movestat&0x0FFF,sprptr[i]->ang);
               break;
          case 16384:
               sprptr[i]->ang=arbitraryangle();
               break;
          }

rodentcontinue:
          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,RODENT);
         #endif
		i = nexti;
	}

	i=headspritestat[STANDING];    
	while( i >= 0 ) 
     {
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,8);
          }

		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = labs(sprite[i].x-posx[p])+labs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}
          if( sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_NOSTANDING ) {
               sprptr[i]->lotag=0;
               sprptr[i]->hitag=0;
               sprptr[i]->picnum=sprXTptr[ext]->basepic;
               newstatus(i, sprXTptr[ext]->basestat);
          }

          if( ((sprXTptr[ext]->aimask)&AI_JUSTSHOTAT) != 0 ) {
               sprptr[i]->lotag=0;
          }   
          if( ((sprXTptr[ext]->aimask)&AI_ENCROACHMENT) != 0 ) {
               sprptr[i]->lotag=0;
          }   
          if( (drawweap[target]) ) {
                 if( isvisible(i, target) ) 
                       sprptr[i]->lotag=0;
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
			newstatus(i, sprXTptr[ext]->basestat);
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,STANDING);
         #endif
		i = nexti;
	}

	i=headspritestat[ATTACK];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,9);
          }

          if( sprptr[i]->lotag == sprptr[i]->hitag ) {   // fire instance
               enemyshootgun(i, sprptr[i]->x,sprptr[i]->y,sprptr[i]->z,
                             sprptr[i]->ang, 100, sprptr[i]->sectnum,
                             sprXTptr[ext]->weapon);
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);

		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
               if( ((sprXTptr[ext]->aimask)&AI_TIMETODODGE) ){
                      sprXTptr[ext]->aimask&=~AI_TIMETODODGE; 
			     newstatus(i, DODGE);
               }
               else {
			     newstatus(i, sprXTptr[ext]->basestat);
               }
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,ATTACK);
         #endif
		i = nexti;
	}

	i=headspritestat[DELAYEDATTACK];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,10);
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);

		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
			newstatus(i, ATTACK);
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,DELAYEDATTACK);
         #endif
		i = nexti;
	}

	i=headspritestat[SQUAT];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,11);
          }

          if( sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_NOSTANDING ) {
               sprptr[i]->lotag=0;
               sprptr[i]->hitag=0;
               sprptr[i]->picnum=sprXTptr[ext]->basepic;
               newstatus(i, sprXTptr[ext]->basestat);
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->hitag -= ((int)TICSPERFRAME);
               if( sprptr[i]->hitag < 0 ) {
                    sprptr[i]->lotag=0;
                    sprptr[i]->hitag=0;
                    newstatus(i, UNSQUAT);
               }
          }
          else {
		     sprptr[i]->picnum = sprXTptr[ext]->squatpic + ((47-sprptr[i]->lotag)>>4);
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,SQUAT);
         #endif
		i = nexti;
	}

	i=headspritestat[UNSQUAT];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,12);
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
               sprptr[i]->hitag=0;
               sprptr[i]->picnum=sprXTptr[ext]->basepic;
               newstatus(i, sprXTptr[ext]->basestat);
          }
          else {
		     sprptr[i]->picnum = sprXTptr[ext]->squatpic + (((47)>>4)) - ((47-sprptr[i]->lotag)>>4);
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,UNSQUAT);
         #endif
		i = nexti;
	}

	i=headspritestat[HIDE];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,13);
          }

          if( sectptr[sprptr[i]->sectnum]->lotag == SECT_LOTAG_NOSTANDING ) {
               sprptr[i]->lotag=0;
               sprptr[i]->hitag=0;
               sprptr[i]->picnum=sprXTptr[ext]->basepic;
               newstatus(i, sprXTptr[ext]->basestat);
          }

          if( ((sprXTptr[ext]->aimask)&AI_JUSTSHOTAT) != 0 ) {
               sprptr[i]->lotag=0;
               sprptr[i]->hitag=0;
          }                    

		sprptr[i]->lotag -= ((int)TICSPERFRAME);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->hitag -= ((int)TICSPERFRAME);
               if( sprptr[i]->hitag < 0 ) {
                    sprptr[i]->lotag=0;
                    sprptr[i]->hitag=0;
                    newstatus(i, UNHIDE);
               }
          }
          else {
		     sprptr[i]->picnum = sprXTptr[ext]->squatpic + ((47-sprptr[i]->lotag)>>4);
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,HIDE);
         #endif
		i = nexti;
	}

	i=headspritestat[UNHIDE];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,14);
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
               sprptr[i]->hitag=0;
               sprptr[i]->picnum=sprXTptr[ext]->basepic;
               newstatus(i, FLEE);
          }
          else {
		     sprptr[i]->picnum = sprXTptr[ext]->squatpic + (((47)>>4)) - ((47-sprptr[i]->lotag)>>4);
          }

          sprXTptr[ext]->aimask&=~AI_JUSTSHOTAT;
          sprXTptr[ext]->aimask&=~AI_WASDRAWN;
         #ifdef  VERIFYSTATS
          verifystatus(nexti,UNHIDE);
         #endif
		i = nexti;
	}

	i=headspritestat[PAIN];    
	while( i >= 0 ) 
     {
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,15);
          }

		sprptr[i]->lotag -= ((int)TICSPERFRAME);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
               newstatus(i, sprXTptr[ext]->basestat);
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,PAIN);
         #endif
		i = nexti;
	}

	i=headspritestat[DEATH];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          ext=sprptr[i]->extra;
          if( !validext(ext) ) {
               noextcrash(i,16);
          }

          if( isanandroid(i) ) {
               androidexplosion(( int)i);
               jsdeletesprite(i);
               showmessage("WAS AN ANDROID");
               goto deathcontinue;
          }

          targsect=sprptr[i]->sectnum;
          if( sectptr[targsect]->lotag != 4 ) {
               if( sprptr[i]->z < sectptr[targsect]->floorz ) {
                    sprptr[i]->z+=1024;
               }
               else {
                    sprptr[i]->z=sectptr[targsect]->floorz;
               }
          }

		sprptr[i]->lotag-=((int)TICSPERFRAME<<1);
		if (sprptr[i]->lotag < 0) {
               sprptr[i]->lotag=0;
               tweakdeathdist(i);
               sprptr[i]->cstat&=0xFFFE;
               if( isahologram(i) ) {
                    showmessage("WAS A HOLOGRAM");
                    jsdeletesprite(i);
               }
               else {
                    missionaccomplished(i);
                    newstatus(i, FLOATING);
               }
          }
          else {
		     sprptr[i]->picnum = sprXTptr[ext]->deathpic + ((sprptr[i]->hitag-sprptr[i]->lotag)>>4);
          }

deathcontinue:
         #ifdef  VERIFYSTATS
          verifystatus(nexti,DEATH);
         #endif
		i = nexti;
	}

	i=headspritestat[PLRVIRUS];    
	while( i >= 0 ) 
     {
		nexti = nextspritestat[i];

          host=sprptr[i]->owner;
          if( !validplayer(host) ) {
               crash("plrvirus lost host");
          }

          sprptr[i]->x=posx[host];
          sprptr[i]->y=posy[host];
          sprptr[i]->z=posz[host]+(8<<8);

   		sprptr[i]->lotag -= ((int)TICSPERFRAME);
          if( (sprptr[i]->lotag > 0) && ((sprptr[i]->lotag&3) == 0) ) {
               if( changehealth(host, -64) != 0 ) {
                    sprptr[i]->lotag=0;
               }
              #ifdef NETDEBUG
               fprintf(dbgfp2, "%ld %ld %d %d\n", lockclock,randomseed,host,health[host]);
              #endif
          }
		if( sprptr[i]->lotag <= 0 ) {
               changehealth(host, -8192);
               if( host == screenpeek ) {
                    showmessage("BURNED");
               }
               jsdeletesprite(i);
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,PLRVIRUS);
         #endif
		i = nexti;
	}

	i=headspritestat[VIRUS];    
	while( i >= 0 ) 
     {
		nexti = nextspritestat[i];

          host=sprptr[i]->owner;
          if( (host < 0) || (host >=MAXSPRITES) || (sprptr[host]->statnum >= MAXSTATUS) ) {
               jsdeletesprite(i);
               i=nexti;
               continue;
          }

          sprptr[i]->x=sprptr[host]->x;
          sprptr[i]->y=sprptr[host]->y;
          if( sprptr[i]->picnum == FIREPIC ) {
               sprptr[i]->z=sprptr[host]->z-(tilesizy[sprptr[host]->picnum]<<4);
          }
          else {
               sprptr[i]->z=sprptr[host]->z;
          }

          if( sprptr[i]->picnum == FIREPIC ) {
               sprptr[i]->hitag+=4;
               if( sprptr[i]->hitag >= (TICSPERFRAME<<3) ) {
                    sprptr[host]->shade++;
                    sprptr[i]->hitag=0;
               }
               if( sprptr[host]->shade > 12 ) {
                    damagesprite(host, 1024);
                    sprptr[i]->lotag=0;
               }
          }
          else {
     		sprptr[i]->lotag -= ((int)TICSPERFRAME);
               if( damagesprite(host, 4) == 1 ) {  // killed 'em
                    // NOT NETWORK COMPATIBLE
                    killscore(host, screenpeek, 0);
                    sprptr[i]->lotag=0;
               }
          }

		if (sprptr[i]->lotag <= 0) {
               jsdeletesprite(i);
          }

         #ifdef  VERIFYSTATS
          verifystatus(nexti,VIRUS);
         #endif
		i = nexti;
	}

menuison:
      
	i=headspritestat[MIRRORMAN1];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          px=posx[screenpeek];
          py=posy[screenpeek];
          pz=posz[screenpeek];

          if( (px < -55326) || (px > -52873) || (py > 40521) || (py < 36596) ) {
               i=nexti;
               continue;
          }
          deltapy=py-36596;

          if( drawweap[screenpeek] ) 
               sprptr[i]->picnum=1079;
          else
               sprptr[i]->picnum=1074;

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;

          sprptr[i]->x=px;
          sprptr[i]->y=36596-deltapy;
          sprptr[i]->z=pz+(42<<8);

          sprptr[i]->ang=(ang[screenpeek]+1024)&2047;
          
         #ifdef  VERIFYSTATS
          verifystatus(nexti,MIRRORMAN1);
         #endif
		i = nexti;
	}

	i=headspritestat[MIRRORMAN2];    
	while (i >= 0)
	{
		nexti = nextspritestat[i];

          px=posx[screenpeek];
          py=posy[screenpeek];
          pz=posz[screenpeek];

          if( (px < -34792) || (px > -32404) || (py > 38980) || (py < 35074) ) {
               i=nexti;
               continue;
          }
          deltapy=38980-py;

          if( drawweap[screenpeek] ) 
               sprptr[i]->picnum=1079;
          else
               sprptr[i]->picnum=1074;

          prevx=sprptr[i]->x;
          prevy=sprptr[i]->y;
          prevz=sprptr[i]->z;

          sprptr[i]->x=px;
          sprptr[i]->y=38980+deltapy;
          sprptr[i]->z=pz+(42<<8);

          sprptr[i]->ang=(ang[screenpeek]+1024)&2047;
          
         #ifdef  VERIFYSTATS
          verifystatus(nexti,MIRRORMAN2);
         #endif
		i = nexti;
	}

	i = headspritestat[PROJECTILE]; 
	while (i != -1)
	{
		nexti = nextspritestat[i];

		dax = ((((int)sprptr[i]->xvel)*TICSPERFRAME)<<11);
		day = ((((int)sprptr[i]->yvel)*TICSPERFRAME)<<11);
		daz = ((((int)sprptr[i]->zvel)*TICSPERFRAME)>>2); // was 3

		hitobject = movesprite((short)i,dax,day,daz,4L<<8,4L<<8,1);

		if( hitobject != 0 ) {
			if( (hitobject&0xc000) == 16384 ) {          // hit a ceiling or floor
			}
			else if( (hitobject&0xc000) == 32768 ) {     // hit a wall
                    //playsound( ??? , sprptr[i]->x,sprptr[i]->y, 0,ST_UPDATE);
			}
			else if( (hitobject&0xc000) == 49152 ) {     // hit a sprite
                    hitsprite=(hitobject&4095);
				if( playerhit(hitsprite, &pnum) ) {
                         playerpainsound(pnum);
                         enemywoundplayer(pnum,sprptr[i]->owner,6);
                    }
                    else {
                         damagesprite(hitsprite,
                         tekgundamage(6,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z,hitsprite));               
                    }
               }
               jsdeletesprite(i);
		}

         #ifdef  VERIFYSTATS
          verifystatus(nexti,PROJECTILE);
         #endif
		i = nexti;
	}

     for( i=0; i<MAXSPRITES; i++ ) {
          ext=sprptr[i]->extra;
          if( validext(ext) ) {
               if( ((sprXTptr[ext]->fxmask)&FX_NXTSTTDEATH) != 0 ) {
                    sprXTptr[ext]->fxmask&=(~FX_NXTSTTDEATH);
                    newstatus(i, DEATH);
               }
               else if( ((sprXTptr[ext]->fxmask)&FX_NXTSTTPAIN) != 0 ) {
                    sprXTptr[ext]->fxmask&=(~FX_NXTSTTPAIN);
                    newstatus(i, PAIN);
               }
          }
     }

     gunstatuslistcode();      
}

int
playerhit(int hitsprite, int *pnum)
{
     int       j;

     for( j=connecthead ; j >= 0 ; j=connectpoint2[j] ) {
          if( playersprite[j] == hitsprite ) {
               if( sprptr[hitsprite]->statnum != 8 ) {
                    crash("plrhit: plrsprt lost sttnm 8");
               }
               *pnum=j;
               return(1);
          }
     }

     return(0);
}

void
checkblastarea(int spr)
{
     int      sect,i,j,nexti,xydist,zdist;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(spr) ) {
          crash("messing w plrsprite at 27");
     }
    #endif

     sect=sprptr[spr]->sectnum;

     for( j=connecthead ; j >= 0 ; j=connectpoint2[j] ) {
	     xydist=klabs(sprptr[spr]->x-posx[j])+klabs(sprptr[spr]->y-posy[j]);
          zdist=klabs(sprptr[spr]->z-posz[j]);
          if( (xydist < 512) && (zdist < 10240) ) {
               changehealth(j, -5000);
          }
          else if( (xydist < 2048) && (zdist < 20480) ) {
               changehealth(j, -800);
          }
          else if( (xydist < 4096) && (zdist < 40960) ) {
               changehealth(j, -200);
          }
     }

    if( option[4] == 0 ) {
	for( i=headspritesect[sect]; i>=0; i=nexti ) {
		nexti = nextspritesect[i];
          if( (i != spr) && (!isaplayersprite(i)) ) {
               switch( sprptr[i]->statnum ) {
               case PLAYER:            
               case BOMBPROJECTILESTAT:
               case BOMBPROJECTILESTAT2:
               case RUNTHRU:
               case INACTIVE:
               case DEATH:
                    break;
               default:
		          xydist=klabs(sprptr[spr]->x-sprptr[i]->x)+klabs(sprptr[spr]->y-sprptr[i]->y);
                    zdist=klabs(sprptr[spr]->z - sprptr[i]->z);
                    if( (xydist < 2560) && (zdist < 16384) ) {
                         damagesprite(i, -500);
                    }
                    break;
               }
          }
     }
    }
}

void
genexplosion1(int i)
{
     int       j;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 30");
     }
    #endif

     j=jsinsertsprite(sprptr[i]->sectnum, 5);
     if( j != -1 ) {
          fillsprite(j,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z,0,
                     -16,0,0,64,64,0,0,GENEXP1PIC+1,sprptr[i]->ang,
	                sintable[(sprptr[i]->ang+2560)&2047]>>6,sintable[(sprptr[i]->ang+2048)&2047]>>6,
	                30L,i+4096,sprptr[i]->sectnum, 5,24,0,-1);
          playsound(S_SMALLGLASS1+RMOD2("STAT4534"), sprptr[i]->x,sprptr[i]->y,0,ST_NOUPDATE);
          return;
     }
}

void
genexplosion2(int i)
{
     int       j;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 31");
     }
    #endif

     sectorflash(sprptr[i]->sectnum);
     checkblastarea(i);

     j=jsinsertsprite(sprptr[i]->sectnum, 5);
     if( j != -1 ) {
          fillsprite(j,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z-(tilesizy[sprptr[i]->picnum]<<3),0,
                     -16,0,0,64,64,0,0,GENEXP2PIC,sprptr[i]->ang,
	                sintable[(sprptr[i]->ang+2560)&2047]>>6,sintable[(sprptr[i]->ang+2048)&2047]>>6,
	                30L,i+4096,sprptr[i]->sectnum, 5,32,0,-1);
          playsound(S_EXPLODE1+RMOD2("STAT4559"), sprptr[i]->x,sprptr[i]->y,0,ST_NOUPDATE);
     }
}

void
bombexplosion(int i)
{
     int       j;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 32");
     }
    #endif

     j=jsinsertsprite(sprptr[i]->sectnum, 5);
     if( j != -1 ) {
          fillsprite(j,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z,0,
                     -16,0,0,34,34,0,0,BOMBEXP1PIC,sprptr[i]->ang,
	                sintable[(sprptr[i]->ang+2560)&2047]>>6,sintable[(sprptr[i]->ang+2048)&2047]>>6,
	                30L,i+4096,sprptr[i]->sectnum, 5,32,0,-1);
          playsound(S_RIC2, sprptr[i]->x,sprptr[i]->y,0,ST_NOUPDATE); 
     }
}

void
androidexplosion(int i)
{
     int       j;

     j=jsinsertsprite(sprptr[i]->sectnum, 5);
     if( j != -1 ) {
          fillsprite(j,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z,0,
                     -16,0,0,34,34,0,0,456,sprptr[i]->ang,
	                sintable[(sprptr[i]->ang+2560)&2047]>>6,sintable[(sprptr[i]->ang+2048)&2047]>>6,
	                30L,i+4096,sprptr[i]->sectnum, 5,32,0,-1);
     }
     playsound(S_ANDROID_DIE, sprptr[i]->x,sprptr[i]->y,0,ST_NOUPDATE); 
}

void
blastmark(int i)
{
     int       j;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 33");
     }
    #endif

     switch( sprptr[i]->statnum ) {
     case GENEXPLODE2:
          break;
     default:
          return;
     }
     switch( sprptr[i]->picnum ) {
     case BARRELL:
     case 175:
          break;
     default:
          return;
     }

     j=jsinsertsprite(sprptr[i]->sectnum, 100);
     if( j != -1 ) {
          fillsprite(j,sprptr[i]->x,sprptr[i]->y,sectptr[sprptr[i]->sectnum]->floorz,0x00A2,
                     4,0,0,34,34,0,0,465,sprptr[i]->ang,0,0,
                 30L,i+4096,sprptr[i]->sectnum,100,0,0,-1);
     }
}

void
forceexplosion(int i)
{
	int      j,k;

    #ifdef PLRSPRDEBUG
     if( isaplayersprite(i) ) {
          crash("messing w plrsprite at 34");
     }
    #endif

     j=jsinsertsprite(sprite[i].sectnum, 5);
     if( j != -1 ) {
	     fillsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,0,-4,0,
		           32,34,34,0,0,FORCEBALLPIC,sprite[i].ang,
              	      0,0,0,sprite[i].owner,sprite[i].sectnum,5,31,0,-1);
     }

	for(k=0;k<6;k++)
	{
          j=jsinsertsprite(sprite[i].sectnum, 7);
          if( j != -1 ) {
		     fillsprite(j,sprite[i].x,sprite[i].y,sprite[i].z+(8<<10),2,-4,0,
			           32,24,24,0,0,FORCEBALLPIC,sprite[i].ang,
                       (krand_intercept("STAT4497")&511)-256,(krand_intercept("STAT4497")&511)-256,(krand_intercept("STAT4497")&16384)-8192,
			           sprite[i].owner,sprite[i].sectnum,7,96,0,-1);
          }
	}

     playsound(S_FORCEFIELD2, sprptr[i]->x,sprptr[i]->y,0, ST_UPDATE);
}

void
sectorflash(short s)
{
     if(  sectflash.step != 0 ) {
          return;
     } 
     sectflash.sectnum=s;
     sectflash.step=1;
     sectflash.ovis=sectptr[s]->visibility;
}

void
dosectorflash()
{
     switch( sectflash.step ) {
     case 0:
          break;
     case 1:
          sectptr[sectflash.sectnum]->visibility=0;
          sectflash.step=2;
          break;
     case 2:
          sectptr[sectflash.sectnum]->visibility=128;
          sectflash.step=3;
          break;
     case 3:
          sectptr[sectflash.sectnum]->visibility=0;
          sectflash.step=4;
          break;
     case 4:
          sectptr[sectflash.sectnum]->visibility=sectflash.ovis;
          sectflash.sectnum=0;
          sectflash.ovis=0;
          sectflash.step=0;
     }
}

void
tekstatsave(int fh)
{
     int  i;

     for (i=0 ; i < MAXSPRITES ; i++) {
          write(fh,&spriteXT[i],sizeof(struct spriteextension));
     }
}

void
tekstatload(int fh)
{
     int  i;

     for (i=0 ; i < MAXSPRITES ; i++) {
          read(fh,&spriteXT[i],sizeof(struct spriteextension));
     }
}

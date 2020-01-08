/***************************************************************************
 *   TEKTAG.C  - Tag specific code for TEKWAR game                         *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "names.h"
#include "pragmas.h"
#include "mmulti.h"

#include "tekwar.h"

#define   AMBUPDATEDIST  4000L

#define   BOBBMAX        512
#define   BOBBDELTA      128

int       headbobon=1;
int       headbob,bobstep=BOBBDELTA;

#define   VEHICLEHEIGHT  -13312

#define   MAXANIMPICS         32
#define   MAXDELAYFUNCTIONS   32

#define   BLUEKEYITEM         0
#define   REDKEYITEM          1
#define   ELEVACTIVATED       31

#define   DOORUPTAG           6
#define   DOORDOWNTAG         7
#define   DOORSPLITHOR        8
#define   DOORSPLITVER        9
#define   DOORSWINGTAG        13
#define   DOORREVOLVETAG      14        // sector tags

#define   PLATFORMELEVTAG     1000      // additional sector tags
#define   BOXELEVTAG          1003
#define   PLATFORMDELAYTAG    1004
#define   BOXDELAYTAG         1005
#define   DOORFLOORTAG        1006
#define   PLATFORMDROPTAG     1007

#define   DST_BAYDOOR         10
#define   DST_HYDRAULICDOOR   20
#define   DST_ELEVATORDOOR    30
#define   DST_MATRIXDOOR1     40
#define   DST_MATRIXDOOR2     50
#define   DST_MATRIXDOOR3     60
#define   DST_MATRIXDOOR4     70

#define   SPRITEELEVATORTAG   1500

#define   PULSELIGHT          0         // sector effect tags flags32[]
#define   FLICKERLIGHT        1
#define   DELAYEFFECT         2
#define   WPANNING            3
#define   FPANNING            4
#define   CPANNING            5
#define   FLICKERDELAY        6
#define   BLINKDELAY          7
#define   STEADYLIGHT         8
#define   WARPSECTOR          9
#define   KILLSECTOR          10
#define   DOORSPEEDEFFECT     11
#define   QUICKCLOSE          12
#define   SOUNDON             13
#define   SOUNDOFF            14
#define   DOORSUBTYPE         15

#define   DOORDELAY      (CLKIPS*4)
#define   DOORSPEED      128L
#define   ELEVSPEED      256L

char onelev[MAXPLAYERS];

struct sectoreffect {
     unsigned int sectorflags;
     int animate;
     int  hi,lo;
     int  delay,delayreset;
     int  ang;
     int  triggerable;
     int  warpto;
     int warpx,warpy,warpz;
     int sin,cos;
     short damage;
};
struct    sectoreffect   sectoreffect[MAXSECTORS];
struct    sectoreffect   *septrlist[MAXSECTORS];
int       secnt,sexref[MAXSECTORS];

#define   MAXDOORS       200
enum {
     D_NOTHING=0,
     D_OPENDOOR,
     D_CLOSEDOOR,
     D_OPENING,
     D_CLOSING,
     D_WAITING,
     D_SHUTSOUND,
     D_OPENSOUND
};
enum {
     DOORCLOSED=0,
     DOOROPENING,
     DOOROPENED,
     DOORCLOSING
};
struct doortype {
     int  type;
     int  state;
     int  sector;
     int  step;
     int  delay;
     int goalz[4];
     int points[4];
     int  walls[8];
     int centx;
     int centy;
     int centz;
     int  subtype;   // jeff added 9-20
};
struct    doortype  doortype[MAXDOORS];
struct    doortype  *doorptr[MAXDOORS];
int       doorxref[MAXSECTORS],numdoors;

#define   MAXFLOORDOORS  25
struct floordoor {
     int  state;
     int  sectnum;
     int  wall1,wall2;
     int  dist1,dist2;
     int  dir;
};
struct    floordoor      floordoor[MAXFLOORDOORS];
struct    floordoor      *floordoorptr[MAXFLOORDOORS];
int       fdxref[MAXSECTORS],numfloordoors;

#define   MAXSECTORVEHICLES   10
#define   MAXVEHICLEPOINTS    200
#define   MAXVEHICLETRACKS    50
#define   MAXVEHICLESECTORS   30                                
#define   SECTORVEHICLETAG    1010
struct sectorvehicle {
     short acceleration,accelto;
     short speed,speedto,movespeed;                              
     short angle,angleto;
     int pivotx,pivoty;
     short numpoints;
     short point[MAXVEHICLEPOINTS];
     int pointx[MAXVEHICLEPOINTS];
     int pointy[MAXVEHICLEPOINTS];
     short track;
     short tracknum;
     int trackx[MAXVEHICLETRACKS];
     int tracky[MAXVEHICLETRACKS];
     char stop[MAXVEHICLETRACKS];                                
     int distx,disty;
     short sector[MAXVEHICLESECTORS];                            
     short numsectors;                                           
     int waittics,waitdelay;                                    
     short stoptrack;
     short killw[4];
     int  soundindex;          
};
struct    sectorvehicle  sectorvehicle[MAXSECTORVEHICLES];
struct    sectorvehicle  *sectvehptr[MAXSECTORVEHICLES];
int       numvehicles;

#define   MAXSPRITEELEVS 25
#define   MAXPARTS       20
#define   MAXELEVFLOORS  20
#define   MAXELEVDOORS   4
enum {
     E_OPENINGDOOR=0,
     E_CLOSINGDOOR,
     E_WAITING,
     E_MOVING,
     E_NEXTFLOOR
};
enum {
     E_GOINGUP=0,
     E_GOINGDOWN
};
#define   E_WAITDELAY    CLKIPS*4
#define   E_DOOROPENPOS  15360
struct elevatortype {
     int hilevel;
     int lolevel;
};
struct    elevatortype   elevator[MAXSECTORS];
struct    elevatortype   *evptrlist[MAXSECTORS];

struct spriteelev {
     int  state;
     int  parts;
     int  sprnum[MAXPARTS];
     int  door[MAXELEVDOORS];
     int doorpos;
     int startz[MAXPARTS];
     int floorz[MAXELEVFLOORS];
     int  curfloor;
     int  curdir;
     int  delay;
     int  floors;
     int floorpos;
     int  doors;
};
struct    spriteelev     spriteelev[MAXSPRITEELEVS];
struct    spriteelev     *sprelevptr[MAXSPRITEELEVS];
int       sprelevcnt;

int flags32[32]={
     0x80000000,0x40000000,0x20000000,0x10000000,
     0x08000000,0x04000000,0x02000000,0x01000000,
     0x00800000,0x00400000,0x00200000,0x00100000,
     0x00080000,0x00040000,0x00020000,0x00010000,
     0x00008000,0x00004000,0x00002000,0x00001000,
     0x00000800,0x00000400,0x00000200,0x00000100,
     0x00000080,0x00000040,0x00000020,0x00000010,
     0x00000008,0x00000004,0x00000002,0x00000001
};

#define   MAXMAPSOUNDFX            32
#define   MAP_SFX_AMBIENT          0
#define   MAP_SFX_SECTOR           1
#define   MAP_SFX_TOGGLED          2
#define   MAP_SFX_TURN_ON          3
#define   MAP_SFX_TURN_OFF         4
struct mapsndfxtype {
     int      x,y;
     short     sector;
     int       snum;
     int       loops;
     int       type;
     int       id;
};
struct    mapsndfxtype    mapsndfx[MAXMAPSOUNDFX];
struct    mapsndfxtype    *mapsndfxptr[MAXMAPSOUNDFX];
int       totalmapsndfx=0;

int      ambupdateclock;

struct animpic {
     short frames;
     short *pic;
     short tics;
     int nextclock;
} animpic[MAXANIMPICS],*animpicptr[MAXANIMPICS];

struct delayfunc {
     void (*func)(short);
     int  tics;
     short parm;
} delayfunc[MAXDELAYFUNCTIONS],*delayfuncptr[MAXDELAYFUNCTIONS];

int      subwaystopdir[4] = { 1L, 1L, 1L, 1L };
void      checktoggledmapsndfx(short dasect) ;
int       numanimates;
short     numdelayfuncs;
int       loopinsound=-1;
int       baydoorloop=-1;
int       ambsubloop=-1;

void
operatesector(short dasector)
{     //Door code
     int i, j, datag;
     int dax2, day2, centx, centy;
     short startwall, endwall, wallfind[2];

     datag = sector[dasector].lotag;

     // lights out / on
     if( datag == 33 ) {
          if( sectptr[dasector]->visibility >= 212 ) {
               sectptr[dasector]->visibility=0;
          }
          else {
               sectptr[dasector]->visibility=212;
          }
          return;
     }

     startwall = sector[dasector].wallptr;
     endwall = startwall + sector[dasector].wallnum - 1;
     centx = 0L, centy = 0L;
     for(i=startwall;i<=endwall;i++)
     {
          centx += wall[i].x;
          centy += wall[i].y;
     }
     centx /= (endwall-startwall+1);
     centy /= (endwall-startwall+1);

     // kens swinging door
     if( datag == 13 ) {  
          for( i=0; i<swingcnt; i++ ) {
               if( swingsector[i] == dasector ) {
                    if( swinganginc[i] == 0 ) {
                         if( swingang[i] == swingangclosed[i] ) {
                              swinganginc[i] = swingangopendir[i];
                         }
                         else {
                              swinganginc[i] = -swingangopendir[i];
                         }
                    }
                    else {
                         swinganginc[i] = -swinganginc[i];
                    }
               }
          }
     }

     // kens true sideways double-sliding door
     if( datag == 16 ) {
          // get 2 closest line segments to center (dax, day)
          wallfind[0] = -1;
          wallfind[1] = -1;
          for( i=startwall;i<=endwall;i++ )
               if (wall[i].lotag == 6)
               {
                    if (wallfind[0] == -1)
                         wallfind[0] = i;
                    else
                         wallfind[1] = i;
               }

          for(j=0;j<2;j++)
          {
               if ((((wall[wallfind[j]].x+wall[wall[wallfind[j]].point2].x)>>1) == centx) && (((wall[wallfind[j]].y+wall[wall[wallfind[j]].point2].y)>>1) == centy))
               {     //door was closed
                         //find what direction door should open
                    i = wallfind[j]-1; if (i < startwall) i = endwall;
                    dax2 = wall[i].x-wall[wallfind[j]].x;
                    day2 = wall[i].y-wall[wallfind[j]].y;
                    if (dax2 != 0)
                    {
                         dax2 = wall[wall[wall[wall[wallfind[j]].point2].point2].point2].x;
                         dax2 -= wall[wall[wall[wallfind[j]].point2].point2].x;
                         setanimation(&wall[wallfind[j]].x,wall[wallfind[j]].x+dax2,4L,0L);
                         setanimation(&wall[i].x,wall[i].x+dax2,4L,0L);
                         setanimation(&wall[wall[wallfind[j]].point2].x,wall[wall[wallfind[j]].point2].x+dax2,4L,0L);
                         setanimation(&wall[wall[wall[wallfind[j]].point2].point2].x,wall[wall[wall[wallfind[j]].point2].point2].x+dax2,4L,0L);
                    }
                    else if (day2 != 0)
                    {
                         day2 = wall[wall[wall[wall[wallfind[j]].point2].point2].point2].y;
                         day2 -= wall[wall[wall[wallfind[j]].point2].point2].y;
                         setanimation(&wall[wallfind[j]].y,wall[wallfind[j]].y+day2,4L,0L);
                         setanimation(&wall[i].y,wall[i].y+day2,4L,0L);
                         setanimation(&wall[wall[wallfind[j]].point2].y,wall[wall[wallfind[j]].point2].y+day2,4L,0L);
                         setanimation(&wall[wall[wall[wallfind[j]].point2].point2].y,wall[wall[wall[wallfind[j]].point2].point2].y+day2,4L,0L);
                    }
               }
               else
               {    //door was not closed
                    i = wallfind[j]-1; if (i < startwall) i = endwall;
                    dax2 = wall[i].x-wall[wallfind[j]].x;
                    day2 = wall[i].y-wall[wallfind[j]].y;
                    if (dax2 != 0)
                    {
                         setanimation(&wall[wallfind[j]].x,centx,4L,0L);
                         setanimation(&wall[i].x,centx+dax2,4L,0L);
                         setanimation(&wall[wall[wallfind[j]].point2].x,centx,4L,0L);
                         setanimation(&wall[wall[wall[wallfind[j]].point2].point2].x,centx+dax2,4L,0L);
                    }
                    else if (day2 != 0)
                    {
                         setanimation(&wall[wallfind[j]].y,centy,4L,0L);
                         setanimation(&wall[i].y,centy+day2,4L,0L);
                         setanimation(&wall[wall[wallfind[j]].point2].y,centy,4L,0L);
                         setanimation(&wall[wall[wall[wallfind[j]].point2].point2].y,centy+day2,4L,0L);
                    }
               }
          }
     }

     tekoperatesector(dasector);
}


void
tagcode()
{
     int      i, j, k, l, s, dax, day, cnt, good;
     short     startwall, endwall, dasector, p, oldang;

     for(i=0;i<warpsectorcnt;i++)
     {
          dasector = warpsectorlist[i];
          j = ((lockclock&127)>>2);
          if (j >= 16) j = 31-j;
          {
               sector[dasector].ceilingshade = j;
               sector[dasector].floorshade = j;
               startwall = sector[dasector].wallptr;
               endwall = startwall+sector[dasector].wallnum-1;
               for(s=startwall;s<=endwall;s++)
                    wall[s].shade = j;
          }
     }

     for(p=connecthead;p>=0;p=connectpoint2[p])
          if (sector[cursectnum[p]].lotag == 10)  //warp sector
          {
               if (cursectnum[p] != ocursectnum[p])
               {
                    warpsprite(playersprite[p]);
                    posx[p] = sprite[playersprite[p]].x;
                    posy[p] = sprite[playersprite[p]].y;
                    posz[p] = sprite[playersprite[p]].z;
                    ang[p] = sprite[playersprite[p]].ang;
                    cursectnum[p] = sprite[playersprite[p]].sectnum;
                    sprite[playersprite[p]].z += (KENSPLAYERHEIGHT<<8);
               }
          }


     for( i=0; i<xpanningsectorcnt; i++ ) {
          dasector = xpanningsectorlist[i];
          startwall = sector[dasector].wallptr;
          endwall = startwall+sector[dasector].wallnum-1;
          for( s=startwall; s<=endwall; s++ ) {
               wall[s].xpanning = ((lockclock>>2)&255);
          }
     }

     for( i=0; i<ypanningwallcnt; i++ ) {
          wall[ypanningwalllist[i]].ypanning = ~(lockclock&255);
     }

     for( i=0; i<rotatespritecnt; i++ ) {
          sprite[rotatespritelist[i]].ang += (TICSPERFRAME<<2);
          sprite[rotatespritelist[i]].ang &= 2047;
     }

     // kens slime floor
     for( i=0; i<floorpanningcnt; i++ ) {
          sector[floorpanninglist[i]].floorxpanning = ((lockclock>>2)&255);
          sector[floorpanninglist[i]].floorypanning = ((lockclock>>2)&255);
     }

     for( i=0; i<dragsectorcnt; i++ ) {
          dasector = dragsectorlist[i];
          startwall = sector[dasector].wallptr;
          endwall = startwall+sector[dasector].wallnum-1;
          if (wall[startwall].x+dragxdir[i] < dragx1[i]) dragxdir[i] = 16;
          if (wall[startwall].y+dragydir[i] < dragy1[i]) dragydir[i] = 16;
          if (wall[startwall].x+dragxdir[i] > dragx2[i]) dragxdir[i] = -16;
          if (wall[startwall].y+dragydir[i] > dragy2[i]) dragydir[i] = -16;
          for( j=startwall; j<=endwall; j++) {
               dragpoint(j,wall[j].x+dragxdir[i],wall[j].y+dragydir[i]);
          }
          j = sector[dasector].floorz;
          sector[dasector].floorz = dragfloorz[i]+(sintable[(lockclock<<4)&2047]>>3);
          for( p=connecthead; p>=0; p=connectpoint2[p] ) {
               if( cursectnum[p] == dasector ) {
                    posx[p] += dragxdir[i];
                    posy[p] += dragydir[i];
                    posz[p] += (sector[dasector].floorz-j);
                    setsprite(playersprite[p],posx[p],posy[p],posz[p]+(KENSPLAYERHEIGHT<<8));
                    sprite[playersprite[p]].ang = ang[p];
                    frameinterpolate = 0;
               }
          }
     }

     for(i=0;i<swingcnt;i++)
     {
          if (swinganginc[i] != 0)
          {
               oldang = swingang[i];
               for(j=0;j<(TICSPERFRAME<<2);j++)
               {
                    swingang[i] = ((swingang[i]+2048+swinganginc[i])&2047);
                    if (swingang[i] == swingangclosed[i])
                    {
                         if( j == ((TICSPERFRAME<<2)-1) ) {
                              playsound(S_JUMP,swingx[i][0],swingy[i][0],0,ST_UPDATE);
                         }
                         swinganginc[i] = 0;
                    }
                    if (swingang[i] == swingangopen[i]) { 
                         if( j == ((TICSPERFRAME<<2)-1) ) {
                              playsound(S_JUMP,swingx[i][0],swingy[i][0],0,ST_UPDATE);
                         }
                         swinganginc[i] = 0;
                    }
               }
               for(k=1;k<=3;k++)
                    rotatepoint(swingx[i][0],swingy[i][0],swingx[i][k],swingy[i][k],swingang[i],&wall[swingwall[i][k]].x,&wall[swingwall[i][k]].y);

               if (swinganginc[i] != 0)
               {
                    for(p=connecthead;p>=0;p=connectpoint2[p])
                         if ((cursectnum[p] == swingsector[i]) || (testneighborsectors(cursectnum[p],swingsector[i]) == 1))
                         {
                              cnt = 256;
                              do
                              {
                                   good = 1;

                                        //swingangopendir is -1 if forwards, 1 is backwards
                                   l = (swingangopendir[i] > 0);
                                   for(k=l+3;k>=l;k--)
                                        if (clipinsidebox(posx[p],posy[p],swingwall[i][k],128L) != 0)
                                        {
                                             good = 0;
                                             break;
                                        }

                                   if (good == 0)
                                   {
                                        if (cnt == 256)
                                        {
                                             swinganginc[i] = -swinganginc[i];
                                             swingang[i] = oldang;
                                        }
                                        else
                                        {
                                             swingang[i] = ((swingang[i]+2048-swinganginc[i])&2047);
                                        }
                                        for(k=1;k<=3;k++)
                                             rotatepoint(swingx[i][0],swingy[i][0],swingx[i][k],swingy[i][k],swingang[i],&wall[swingwall[i][k]].x,&wall[swingwall[i][k]].y);
                                        if (swingang[i] == swingangclosed[i])
                                        {
                                             swinganginc[i] = 0;
                                             break;
                                        }
                                        if (swingang[i] == swingangopen[i])
                                        {
                                             swinganginc[i] = 0;
                                             break;
                                        }
                                        cnt--;
                                   }
                              } while ((good == 0) && (cnt > 0));
                         }
               }

          }
     }

     for(i=0;i<revolvecnt;i++)
     {
          startwall = sector[revolvesector[i]].wallptr;
          endwall = startwall + sector[revolvesector[i]].wallnum - 1;

          revolveang[i] = ((revolveang[i]+2048-(TICSPERFRAME<<2))&2047);
          for(k=startwall;k<=endwall;k++)
          {
               rotatepoint(revolvepivotx[i],revolvepivoty[i],revolvex[i][k-startwall],revolvey[i][k-startwall],revolveang[i],&dax,&day);
               dragpoint(k,dax,day);
          }
     }

     for(i=0;i<subwaytrackcnt;i++)
     {
          if( subwaysound[i] == -1 ) {
               subwaysound[i]=playsound(S_SUBWAYLOOP,subwayx[i],subwaytracky1[i],-1,ST_VEHUPDATE);
          }
          else {
               updatevehiclesnds(subwaysound[i],subwayx[i],subwaytracky1[i]);
          }

          dasector = subwaytracksector[i][0];
          startwall = sector[dasector].wallptr;
          endwall = startwall+sector[dasector].wallnum-1;
          for(k=startwall;k<=endwall;k++)
          {
               if (wall[k].x > subwaytrackx1[i])
                    if (wall[k].y > subwaytracky1[i])
                         if (wall[k].x < subwaytrackx2[i])
                              if (wall[k].y < subwaytracky2[i])
                                   wall[k].x += (subwayvel[i]&0xfffffffc);
          }

          for(j=1;j<subwaynumsectors[i];j++)
          {
               dasector = subwaytracksector[i][j];

               startwall = sector[dasector].wallptr;
               endwall = startwall+sector[dasector].wallnum-1;
               for(k=startwall;k<=endwall;k++)
                    wall[k].x += (subwayvel[i]&0xfffffffc);

               s = headspritesect[dasector];
               while (s != -1)
               {
                    k = nextspritesect[s];
                    sprite[s].x += (subwayvel[i]&0xfffffffc);
                    s = k;
               }
          }

          for(p=connecthead;p>=0;p=connectpoint2[p])
               if (cursectnum[p] != subwaytracksector[i][0])
                    if (sector[cursectnum[p]].floorz != sector[subwaytracksector[i][0]].floorz)
                         if (posx[p] > subwaytrackx1[i])
                              if (posy[p] > subwaytracky1[i])
                                   if (posx[p] < subwaytrackx2[i])
                                        if (posy[p] < subwaytracky2[i])
                                        {
                                             posx[p] += (subwayvel[i]&0xfffffffc);

                                                  //Update sprite representation of player
                                             setsprite(playersprite[p],posx[p],posy[p],posz[p]+(KENSPLAYERHEIGHT<<8));
                                             sprite[playersprite[p]].ang = ang[p];
                                             frameinterpolate = 0;
                                        }

          subwayx[i] += (subwayvel[i]&0xfffffffc);

          k = subwaystop[i][subwaygoalstop[i]] - subwayx[i];
          if( k > 0 ) { 
               if( k > 2048 ) {
                    if( subwayvel[i] == 12 ) {
                         playsound(S_SUBWAYSTART,subwayx[0],subwaytracky1[0],0,ST_UNIQUE | ST_NOUPDATE);
                    }
                    if (subwayvel[i] < 128) subwayvel[i]++;
               }
               else {
                    if( subwayvel[i] == 32 ) {
                         playsound(S_SUBWAYSTOP,subwayx[0],subwaytracky1[0],0,ST_UNIQUE | ST_NOUPDATE);
                    }
                    subwayvel[i] = (k>>4)+1;
               }
          }
          else if( k < 0 ) {
               if( k < -2048 ) {
                    if( subwayvel[i] == -12 ) {
                         playsound(S_SUBWAYSTART,subwayx[0],subwaytracky1[0],0,ST_UNIQUE | ST_NOUPDATE);
                    }
                    if (subwayvel[i] > -128) subwayvel[i]--;
               }
               else {
                    if( subwayvel[i] == -32) {
                         playsound(S_SUBWAYSTOP,subwayx[0],subwaytracky1[0],0,ST_UNIQUE | ST_NOUPDATE);
                    }
                    subwayvel[i] = ((k>>4)-1);
               }
          }

          if (((subwayvel[i]>>2) == 0) && (labs(k) < 2048))
          {
               if (subwaypausetime[i] == 720)
               {
                    for(j=1;j<subwaynumsectors[i];j++)   //Open all subway doors
                    {
                         dasector = subwaytracksector[i][j];
                         if (sector[dasector].lotag == 17)
                         {
                              sector[dasector].lotag = 16;
                              playsound(S_BIGSWINGCL,subwayx[i],subwaytracky1[i],0,ST_NOUPDATE | ST_UNIQUE);
                              operatesector(dasector);
                              sector[dasector].lotag = 17;
                         }
                    }
               }
               if ((subwaypausetime[i] >= 120) && (subwaypausetime[i]-TICSPERFRAME < 120))
               {
                    for(j=1;j<subwaynumsectors[i];j++)   //Close all subway doors
                    {
                         dasector = subwaytracksector[i][j];
                         if (sector[dasector].lotag == 17)
                         {
                              sector[dasector].lotag = 16;
                              playsound(S_BIGSWINGCL,subwayx[i],subwaytracky1[i],0,ST_NOUPDATE | ST_UNIQUE);
                              operatesector(dasector);
                              sector[dasector].lotag = 17;
                         }
                    }
               }

               subwaypausetime[i] -= TICSPERFRAME;
               if (subwaypausetime[i] < 0)
               {
                    subwaypausetime[i] = 720;
                    if( subwaygoalstop[i] == (subwaystopcnt[i]-1) ) {
                         subwaystopdir[i]=-1;
                         subwaygoalstop[i]=subwaystopcnt[i]-2;
                    }
                    else if( subwaygoalstop[i] == 0 ) {
                         subwaygoalstop[i]=1;
                         subwaystopdir[i]= 1;
                    }
                    else {
                         subwaygoalstop[i]+=subwaystopdir[i];
                    }
               }
          }
     }

     tektagcode();
}

int
testneighborsectors(short sect1, short sect2)
{
     short i, startwall, num1, num2;

     num1 = sector[sect1].wallnum;
     num2 = sector[sect2].wallnum;

     // traverse walls of sector with fewest walls (for speed)
     if( num1 < num2 ) {
          startwall = sector[sect1].wallptr;
          for(i=num1-1;i>=0;i--)
               if (wall[i+startwall].nextsector == sect2)
                    return(1);
     }
     else {
          startwall = sector[sect2].wallptr;
          for(i=num2-1;i>=0;i--)
               if (wall[i+startwall].nextsector == sect1)
                    return(1);
     }
     return(0);
}

void
tekpreptags()
{
     int       angle,i,j,k,n,s,w1,w2,w3,w4;
     int      dax,day,endwall,startwall,x1,x2,y1,y2;
     short     killwcnt;
     unsigned  int      effect;
     spritetype *spr;

     totalmapsndfx=0;
     secnt=0;                                    
     memset(sexref,0,sizeof(int)*MAXSECTORS);    

     for (j=0 ; j < MAXSECTORS ; j++) {
          memset(&sectoreffect[j],0,sizeof(struct sectoreffect));
          sectoreffect[j].warpto=-1;
          if (septrlist[j] != NULL) {
               septrlist[j]=NULL;
          }
          if (evptrlist[j] != NULL) {
               memset(evptrlist[j],0,sizeof(struct elevatortype));
               evptrlist[j]=NULL;
          }
     }
     for (j=0 ; j < MAXDOORS ; j++) {
          doorptr[j]=&doortype[j];
          memset(doorptr[j],0,sizeof(struct doortype));
     }
     numfloordoors=0;
     for (j=0 ; j < MAXSECTORS ; j++) {
          fdxref[j]=-1;
     }
     for (j=0 ; j < MAXFLOORDOORS ; j++) {
          floordoorptr[j]=&floordoor[j];
          memset(floordoorptr[j],0,sizeof(struct floordoor));
     }
     for (j=0 ; j < MAXSPRITEELEVS ; j++) {
          sprelevptr[j]=&spriteelev[j];
          memset(&spriteelev[j],0,sizeof(struct spriteelev));
     }
     for (j=0 ; j < MAXMAPSOUNDFX ; j++) {   
          mapsndfxptr[j]=&mapsndfx[j];
          memset(mapsndfxptr[j], 0, sizeof(struct mapsndfxtype));
     }
     numvehicles=0;
     for (j=0 ; j < MAXSECTORVEHICLES ; j++) {
          sectvehptr[j]=&sectorvehicle[j];
          memset(sectvehptr[j],0,sizeof(struct sectorvehicle));
          sectvehptr[j]->soundindex=-1;
     }

     numdoors=0;
     for (j=0 ; j < numsectors ; j++) {
          if (sectptr[j]->ceilingpal != 0) {
               startwall=sectptr[j]->wallptr;
               endwall=startwall+sectptr[j]->wallnum-1;
               for (i=startwall ; i <= endwall ; i++) {
                    wallptr[i]->pal=sectptr[j]->ceilingpal;
               }
          }
          switch (sectptr[j]->lotag) {
          case DOORUPTAG:
          case DOORDOWNTAG:
          case DOORSPLITHOR:
          case DOORSPLITVER:
          case PLATFORMELEVTAG:
          case PLATFORMDELAYTAG:
          case BOXELEVTAG:
          case BOXDELAYTAG:
          case PLATFORMDROPTAG:
               if (sectptr[j]->lotag == BOXDELAYTAG
                         || sectptr[j]->lotag == PLATFORMDELAYTAG) {
                    evptrlist[j]=&elevator[j];
                    k=nextsectorneighborz(j,sectptr[j]->floorz,1,1);
                    evptrlist[j]->hilevel=sectptr[j]->floorz;
                    evptrlist[j]->lolevel=sectptr[k]->floorz;
               }
               n=numdoors++;
               if (numdoors >= MAXDOORS) {
                    break;
               }
               switch (sectptr[j]->lotag) {
               case DOORUPTAG:
               case DOORDOWNTAG:
               case DOORSPLITHOR:
                    doorptr[n]->step=DOORSPEED;
                    break;
               case DOORSPLITVER:
                    doorptr[n]->step=4L;
                    break;
               case PLATFORMELEVTAG:
               case PLATFORMDELAYTAG:
               case BOXELEVTAG:
               case BOXDELAYTAG:
               case PLATFORMDROPTAG:
                    doorptr[n]->step=ELEVSPEED;
                    break;
               }
               doorptr[n]->centx=doorptr[n]->centy=0L;
               startwall=sectptr[j]->wallptr;
               endwall=startwall+sectptr[j]->wallnum-1;
               for (k=0,i=startwall ; i <= endwall ; i++) {
                    if (wallptr[i]->lotag == 6 && k < 8) {
                         doorptr[n]->walls[k]=i;
                         doorptr[n]->walls[k+1]=i+1;
                         doorptr[n]->walls[k+2]=i+2;
                         doorptr[n]->walls[k+3]=i-1;
                         dax=wallptr[doorptr[n]->walls[k]]->x;
                         if (wallptr[doorptr[n]->walls[k+3]]->x == dax) {
                              day=wallptr[i+2]->y;
                              doorptr[n]->points[k/2]=day;
                              day=wallptr[wallptr[i+2]->point2]->y;
                              doorptr[n]->points[(k/2)+1]=day;
                         }
                         else {
                              dax=wallptr[i+2]->x;
                              doorptr[n]->points[k/2]=dax;
                              dax=wallptr[wallptr[i+2]->point2]->x;
                              doorptr[n]->points[(k/2)+1]=dax;
                         }
                         k+=4;
                    }
                    doorptr[n]->centx+=wallptr[i]->x;
                    doorptr[n]->centy+=wallptr[i]->y;
               }
               doorptr[n]->centx/=(endwall-startwall+1);
               doorptr[n]->centy/=(endwall-startwall+1);
               doorptr[n]->centz=(sectptr[j]->ceilingz+sectptr[j]->floorz)/2;
               doorptr[n]->type=sectptr[j]->lotag;
               doorptr[n]->sector=j;
               doorxref[j]=n;  
               break;
          case DOORFLOORTAG:
               k=fdxref[j]=numfloordoors++;
               floordoorptr[k]->state=DOORCLOSED;
               floordoorptr[k]->sectnum=i;
               startwall=sectptr[j]->wallptr;
               endwall=startwall+sectptr[j]->wallnum-1;
               for (i=startwall ; i <= endwall ; i++) {
                    if (wallptr[i]->lotag == 6) {
                         if (floordoorptr[k]->wall1 == 0) {
                              w1=floordoorptr[k]->wall1=i;
                              w2=wallptr[w1]->point2;
                         }
                         else {
                              w3=floordoorptr[k]->wall2=i;
                              w4=wallptr[w3]->point2;
                         }
                    }
               }
               x1=wallptr[w1]->x;       // close the doors all the way
               y1=wallptr[w1]->y;
               x2=wallptr[w4]->x;
               y2=wallptr[w4]->y;
               dragpoint(w1,(x1+x2)/2,(y1+y2)/2);
               if (x1 != x2) {
                    if (x1 < x2) {
                         dragpoint(w4,(x1+x2)/2+1,(y1+y2)/2);
                         floordoorptr[k]->dir=3;
                    }
                    else {
                         dragpoint(w4,(x1+x2)/2-1,(y1+y2)/2);
                         floordoorptr[k]->dir=1;
                    }
                    x1=wallptr[w1]->x;
                    w1=wallptr[wallptr[wallptr[w1]->nextwall]->point2]->point2;
                    floordoorptr[k]->dist1=abs(x1-wallptr[w1]->x)-50;
               }
               else if (y1 != y2) {
                    if (y1 < y2) {
                         dragpoint(w4,(x1+x2)/2,(y1+y2)/2+1);
                         floordoorptr[k]->dir=0;
                    }
                    else {
                         dragpoint(w4,(x1+x2)/2,(y1+y2)/2-1);
                         floordoorptr[k]->dir=2;
                    }
                    y1=wallptr[w1]->y;
                    w1=wallptr[wallptr[wallptr[w1]->nextwall]->point2]->point2;
                    floordoorptr[k]->dist1=abs(y1-wallptr[w1]->y)-50;
               }
               x1=wallptr[w2]->x;
               y1=wallptr[w2]->y;
               x2=wallptr[w3]->x;
               y2=wallptr[w3]->y;
               dragpoint(w2,(x1+x2)/2,(y1+y2)/2);
               if (x1 != x2) {
                    if (x1 < x2) {
                         dragpoint(w3,(x1+x2)/2+1,(y1+y2)/2);
                    }
                    else {
                         dragpoint(w3,(x1+x2)/2-1,(y1+y2)/2);
                    }
                    x1=wallptr[w2]->x;
                    w2=wallptr[wallptr[wallptr[w2]->nextwall]->point2]->point2;
                    floordoorptr[k]->dist2=abs(x1-wallptr[w2]->x)-50;
               }
               else if (y1 != y2) {
                    if (y1 < y2) {
                         dragpoint(w3,(x1+x2)/2,(y1+y2)/2+1);
                    }
                    else {
                         dragpoint(w3,(x1+x2)/2,(y1+y2)/2-1);
                    }
                    y1=wallptr[w2]->y;
                    w2=wallptr[wallptr[wallptr[w2]->nextwall]->point2]->point2;
                    floordoorptr[k]->dist2=abs(y1-wallptr[w2]->y)-50;
               }
               break;
          default:
               break;
          }
          if (sectptr[j]->hitag >= SECTORVEHICLETAG &&
             sectptr[j]->hitag < SECTORVEHICLETAG+MAXSECTORVEHICLES) {
               k=sectptr[j]->hitag-SECTORVEHICLETAG;
               if (sectvehptr[k]->pivotx == 0 && sectvehptr[k]->pivoty == 0) {
                    numvehicles++;
                    for (i=0 ; i < MAXSPRITES ; i++) {
                         spr=sprptr[i];
                         if (spr->lotag == sectptr[j]->hitag
                            && spr->hitag == sectptr[j]->hitag) {
                              sectvehptr[k]->pivotx=spr->x;
                              sectvehptr[k]->pivoty=spr->y;
                              jsdeletesprite(i);
                              break;
                         }
                    }
               }
               x1=sectvehptr[k]->pivotx;
               y1=sectvehptr[k]->pivoty;
               n=sectvehptr[k]->numpoints;
               startwall=sectptr[j]->wallptr;
               endwall=startwall+sectptr[j]->wallnum-1;
               killwcnt=0;
               for (i=startwall ; i <= endwall ; i++) {
                    if (wallptr[i]->lotag == 6) {
                         sectvehptr[k]->killw[killwcnt++]=i;
                    }
                    sectvehptr[k]->point[n]=i;
                    sectvehptr[k]->pointx[n]=x1-wallptr[i]->x;
                    sectvehptr[k]->pointy[n]=y1-wallptr[i]->y;
                    n++;
                    if (n >= MAXVEHICLEPOINTS) {
                         crash("tekprepareboard: vehicle #%d has too "
                              "many points",sectptr[j]->hitag);
                    }
               }
               sectvehptr[k]->numpoints=n;
               n=sectvehptr[k]->numsectors++;    
               sectvehptr[k]->sector[n]=j;       
               sectvehptr[k]->soundindex=-1; 
          }
     }

     sprelevcnt=0;
     for (i=0 ; i < MAXSPRITES ; i++) {
          spr=sprptr[i];
          if (spr->statnum < MAXSTATUS) {
               switch (spr->picnum) {
               case SNDFX_SECTOR:
               case SNDFX_AMBIENT:
               case SNDFX_TOGGLED:
                    if( totalmapsndfx == MAXMAPSOUNDFX ) {
                         jsdeletesprite(i);
                         break;
                    }
                    mapsndfxptr[totalmapsndfx]->x=sprite[i].x;
                    mapsndfxptr[totalmapsndfx]->y=sprite[i].y;
                    mapsndfxptr[totalmapsndfx]->sector=sprite[i].sectnum;
                    mapsndfxptr[totalmapsndfx]->snum=sprite[i].lotag;
                    mapsndfxptr[totalmapsndfx]->id=-1;
                    if( mapsndfxptr[totalmapsndfx]->snum > TOTALSOUNDS ) {
                         mapsndfxptr[totalmapsndfx]->snum=(TOTALSOUNDS-1);
                    }
                    mapsndfxptr[totalmapsndfx]->loops=sprite[i].hitag-1;
                    switch( spr->picnum ) {
                    case SNDFX_AMBIENT:
                         mapsndfxptr[totalmapsndfx]->type=MAP_SFX_AMBIENT;
                         break;
                    case SNDFX_SECTOR:
                         mapsndfxptr[totalmapsndfx]->type=MAP_SFX_SECTOR;
                         break;
                    case SNDFX_TOGGLED:
                         mapsndfxptr[totalmapsndfx]->type=MAP_SFX_TOGGLED;
                         break;
                    }
                    totalmapsndfx++;
                    jsdeletesprite(i);
                    break;
               case SECTOREFFECT:
                    if (spr->lotag < 1000) {
                         continue;
                    }
                    s=spr->sectnum;
                    septrlist[s]=&sectoreffect[s];
                    if (septrlist[s]->sectorflags == 0) {
                         s=sexref[secnt++]=spr->sectnum;
                         if (secnt == MAXSECTORS) {
                              crash("setupboard: Sector Effector limit exceeded");
                         }
                         septrlist[s]->warpto=-1;
                    }
                    effect=flags32[spr->lotag-1000];
                    septrlist[s]->sectorflags|=effect;
                    if( (effect&flags32[QUICKCLOSE]) != 0 ) {
                    }
                    else if( (effect&flags32[SOUNDON]) != 0 ) {
                         // match mapsndfx with same hitag
                         septrlist[s]->hi=spr->hitag;
                    }
                    else if( (effect&flags32[SOUNDOFF]) != 0 ) {
                         // match mapsndfx with same hitag
                         septrlist[s]->hi=spr->hitag;
                    }
                    else if ((effect&flags32[PULSELIGHT]) != 0
                       || (effect&flags32[FLICKERLIGHT]) != 0
                       || (effect&flags32[FLICKERDELAY]) != 0
                       || (effect&flags32[BLINKDELAY]) != 0
                       || (effect&flags32[STEADYLIGHT]) != 0) {
                         septrlist[s]->lo=sectptr[s]->floorshade;
                         septrlist[s]->hi=sectptr[s]->ceilingshade;
                         septrlist[s]->animate=1;
                         if (spr->hitag != 0) {
                              septrlist[s]->triggerable=spr->hitag;
                         }
                         else {
                              septrlist[s]->triggerable=0;
                         }
                         startwall=sectptr[s]->wallptr;
                         endwall=startwall+sectptr[s]->wallnum-1;
                         for (j=startwall ; j <= endwall ; j++) {
                              wallptr[j]->shade=sectptr[s]->floorshade;
                         }
                         sectptr[s]->ceilingshade=sectptr[s]->floorshade;
                    }
                    else if ((effect&flags32[DELAYEFFECT]) != 0) {
                         septrlist[s]->delay=spr->hitag;
                         septrlist[s]->delayreset=septrlist[s]->delay;
                    }
                    if ((effect&flags32[WPANNING]) != 0) {
                         angle=septrlist[s]->ang=spr->ang;
                         septrlist[s]->sin=(int)sintable[((angle+2048)&2047)];
                         septrlist[s]->cos=(int)sintable[((angle+2560)&2047)];
                         startwall=sectptr[s]->wallptr;
                         endwall=startwall+sectptr[s]->wallnum-1;
                         for (j=startwall ; j <= endwall ; j++) {
                              if (wallptr[j]->lotag == 0) {
                                   wallptr[j]->lotag=spr->lotag;
                              }
                         }
                    }
                    if ((effect&flags32[FPANNING]) != 0 ||
                                        (effect&flags32[CPANNING]) != 0) {
                         angle=septrlist[s]->ang=spr->ang;
                         septrlist[s]->sin=(int)sintable[((angle+2048)&2047)];
                         septrlist[s]->cos=(int)sintable[((angle+2560)&2047)];
                    }
                    if ((effect&flags32[WARPSECTOR]) != 0) {
                         for( j=0; j<MAXSECTORS; j++ ) {
                              if( sector[j].hitag == spr->hitag ) {
                                   septrlist[s]->warpto=j;
                                   j=MAXSECTORS;
                              }
                         }
                    }
                    if ((effect&flags32[KILLSECTOR]) != 0) {
                         if (spr->hitag == 0) {
                              septrlist[s]->damage=9999;
                         }
                         else {
                              septrlist[s]->damage=spr->hitag;
                         }
                    }
                    if ((effect&flags32[DOORSPEEDEFFECT]) != 0) {
                         doorptr[doorxref[spr->sectnum]]->step=spr->hitag;
                    }
                    // jeff added 9-20
                    if ((effect&flags32[DOORSUBTYPE]) != 0) {
                         doorptr[doorxref[spr->sectnum]]->subtype=spr->hitag;
                    }
                    jsdeletesprite(i);
                    break;
               default:
                    if (spr->lotag >= SECTORVEHICLETAG
                       && spr->lotag < SECTORVEHICLETAG+MAXSECTORVEHICLES) {
                         k=spr->lotag-SECTORVEHICLETAG;
                         n=spr->hitag;
                         sectvehptr[k]->trackx[n]=spr->x;
                         sectvehptr[k]->tracky[n]=spr->y;
                         if (spr->picnum == STOPPIC) {
                              sectvehptr[k]->stop[n]=1;
                              sectvehptr[k]->waitdelay=CLKIPS*8;
                         }
                         else {
                              sectvehptr[k]->stop[n]=0;
                         }
                         if (spr->picnum == SPEEDPIC) {
                              if (sectvehptr[k]->speedto == 0 && spr->hitag > 0) {
                                   sectvehptr[k]->speedto=spr->hitag;
                                   sectvehptr[k]->movespeed=sectvehptr[k]->speedto;
                              }
                              jsdeletesprite(i);
                              break;
                         }
                         sectvehptr[k]->tracknum++;
                         jsdeletesprite(i);
                    }
                    if ((spr->lotag < 2000) && (spr->lotag >= SPRITEELEVATORTAG)) {
                         j=spr->lotag-SPRITEELEVATORTAG;
                         if (spr->hitag >= 100) {
                              k=(spr->hitag-100)+1;
                              if (k >= MAXELEVFLOORS) {
                                   crash("setupboard: Only %d levels allowed "
                                        "for sprite elevators",MAXELEVFLOORS);
                              }
                              sprelevptr[j]->floorz[k]=spr->z;
                              sprelevptr[j]->floors++;
                         }
                         else {
                              k=sprelevptr[j]->parts;
                              sprelevptr[j]->sprnum[k]=i;
                              if (spr->hitag == 6 || spr->hitag == 7) {
                                   if (sprelevptr[j]->floorpos == 0) {
                                        sprelevcnt++;
                                   }
                                   sprelevptr[j]->door[sprelevptr[j]->doors]=i;
                                   sprelevptr[j]->floorz[0]=spr->z;
                                   sprelevptr[j]->floorpos=spr->z;
                                   sprelevptr[j]->doors++;
                              }
                              sprelevptr[j]->startz[k]=spr->z;
                              sprelevptr[j]->parts++;
                         }
                    }
                    break;
               }
          }

     }

     numanimates=0;
     for (j=0 ; j < MAXANIMPICS ; j++) {
          animpicptr[j]=&animpic[j];
          memset(animpicptr[j],0,sizeof(struct animpic));
     }
     numdelayfuncs=0;
     for (j=0 ; j < MAXDELAYFUNCTIONS ; j++) {
          delayfuncptr[j]=&delayfunc[j];
          memset(delayfuncptr[j],0,sizeof(struct delayfunc));
     }
}

void
tekoperatesector(short dasector)
{
     short          s;
     int            datag,i;

     s=dasector;
     datag=sectptr[s]->lotag;

     switch (datag) {
     case BOXELEVTAG:
     case PLATFORMELEVTAG:
     case BOXDELAYTAG:
     case PLATFORMDELAYTAG:
     case DOORUPTAG:          // a door that opens up
     case DOORDOWNTAG:
     case DOORSPLITHOR:
     case DOORSPLITVER:
     case PLATFORMDROPTAG:
          i=doorxref[s];
          if (i == -1) {
               crash("operatesector: invalid door reference for sector %d",s);
          }
          switch (doorptr[i]->state) {
          case D_NOTHING:
          case D_CLOSING:
          case D_CLOSEDOOR:
          case D_SHUTSOUND:
               doorptr[i]->state=D_OPENDOOR;
               break;
          default:
               if (datag != PLATFORMDROPTAG) {
                    doorptr[i]->state=D_CLOSEDOOR;
               }
               break;
          }
          break;
     case DOORFLOORTAG:
          floordoorptr[fdxref[s]]->state=DOOROPENING;
          playsound(S_FLOOROPEN,0,0,0,ST_IMMEDIATE);
          break;
     }
}

void
warp(int *x, int *y, int *z, short * UNUSED(daang), short *dasector)
{
     short          startwall, endwall, s;
     int           i, j, dax, day, ox, oy;

     ox = *x; oy = *y;

     for( i=0; i<warpsectorcnt; i++ ) {
          if( warpsectorlist[i] == *dasector ) {
               j = sector[*dasector].hitag;
               do {
                    i++;
                    if (i >= warpsectorcnt) i = 0;
               } while( sector[warpsectorlist[i]].hitag != j );
               *dasector = warpsectorlist[i];
               break;
          }
     }

     // find center of sector
     startwall = sector[*dasector].wallptr;
     endwall = startwall+sector[*dasector].wallnum-1;
     dax = 0L, day = 0L;
     for( s=startwall; s<=endwall; s++ ) {
          dax += wall[s].x, day += wall[s].y;
          if( wall[s].nextsector >= 0 ) {
               i = s;
          }
     }
     *x = dax / (endwall-startwall+1);
     *y = day / (endwall-startwall+1);
     *z = sector[*dasector].floorz-(42<<8);  

     updatesector(*x,*y,dasector);
}

void
tekwarp(int *x, int *y, int *z, short *dasector)
{
     short          startwall, endwall, s;
     int           i, dax, day;

     // find center of sector
     startwall = sector[*dasector].wallptr;
     endwall = startwall+sector[*dasector].wallnum-1;
     dax = 0L, day = 0L;
     for( s=startwall; s<=endwall; s++ ) {
          dax += wall[s].x, day += wall[s].y;
          if( wall[s].nextsector >= 0 ) {
               i = s;
          }
     }
     *x = dax / (endwall-startwall+1);
     *y = day / (endwall-startwall+1);
     *z = sector[*dasector].floorz-(42<<8);  

     updatesector(*x,*y,dasector);
}

void
warpsprite(short spritenum)
{
     short dasectnum;

     dasectnum = sprite[spritenum].sectnum;
     warp(&sprite[spritenum].x,&sprite[spritenum].y,&sprite[spritenum].z,
          &sprite[spritenum].ang,&dasectnum);
     copybuf(&sprite[spritenum].x,&osprite[spritenum].x,3);
     changespritesect(spritenum,dasectnum);
}

void
teknewsector(short p)
{
     int           i,n,nexti,s;
     int           sn;
     struct    sectoreffect   *septr;

     s=cursectnum[p];
     septr=septrlist[s];

     #define NEWMAPLOTAG 25
     if( (sector[s].lotag == NEWMAPLOTAG) && (option[4] == 0) ) {
          newmap(sector[s].hitag);
          return;
     }

     if( sectptr[cursectnum[p]]->lotag == 4 ) {
          playsound(S_SPLASH,posx[p],posy[p],0,ST_UPDATE);
     }

     if( septr != NULL ) {
          if( septr->warpto >= 0 ) {
               sn=playersprite[p];
               tekwarp(&sprite[sn].x,&sprite[sn].y,&sprite[sn].z,( short *)&(septr->warpto));
               copybuf(&sprite[sn].x,&osprite[sn].x,3);
               changespritesect(sn,septr->warpto);
               posx[p] = sprite[playersprite[p]].x;
               posy[p] = sprite[playersprite[p]].y;
               posz[p] = sprite[playersprite[p]].z;
               ang[p] = sprite[playersprite[p]].ang;
               cursectnum[p] = sprite[playersprite[p]].sectnum;
               sprite[playersprite[p]].z += (KENSPLAYERHEIGHT<<8);
          }
     }
 
     for( n=connecthead; n >= 0; n=connectpoint2[n] ) {
          if( (sectptr[cursectnum[n]]->lotag == 1) || (sectptr[cursectnum[n]]->lotag == 2) ) {
               for( i=0; i < numsectors; i++ ) {
                    if( sectptr[i]->hitag == sectptr[cursectnum[n]]->hitag ) {
                         if( (sectptr[i]->lotag != 1) && (sectptr[i]->lotag != 2) ) {
                              operatesector(i);
                          }
                   }
               }
               i=headspritestat[0];
               while( i != -1 ) {
                    nexti=nextspritestat[i];
                    if( sprptr[i]->hitag == sectptr[cursectnum[n]]->hitag ) {
                         operatesprite(i);
                    }
               i=nexti;
               }
          }
     }

     checkmapsndfx(p);
     sectortriggersprites(p);
}


void
tektagcode(void)
{
     int            floorz,hi,i,j,k,lo,oldang,p,r,s,tics;
     int           dax,dax2,day,day2,endwall,startwall;
     unsigned int  effect;
     sectortype     *sect;
     struct    sectoreffect   *septr;

     for( p=connecthead; p >= 0; p=connectpoint2[p] ) {    
          tekanimweap((syncbits[p]>>13)&15,p);
          tekhealstun(p);
     }   

     for( i=0; i < numdoors; i++ ) {
          movedoors(i);
     }

     if( option[4] == 0 ) {
          for( i=0; i < secnt; i++ ) {
               s=sexref[i];
              if (s < 0 || s >= numsectors) {
                   crash("tag1402: Invalid sector effect index (%d,e=%d)",s,i);
              }
               sect=sectptr[s];
               septr=septrlist[s];
               if( septr->triggerable != 0 ) {
                    continue;
               }
               effect=septr->sectorflags;
               if( (effect&flags32[WPANNING]) != 0 ) {
                    oldang=septr->ang;
                    tics=TICSPERFRAME;
                    startwall=sect->wallptr;
                    endwall=startwall+sect->wallnum-1;
                    dax=(tics*septr->cos)>>15;
                    day=(tics*septr->sin)>>13;
                    for( j=startwall; j <= endwall; j++ ) {
                         wallptr[j]->xpanning+=(unsigned char)dax;
                         wallptr[j]->ypanning-=(unsigned char)day;
                    }
               }
               if( (effect&flags32[FPANNING]) != 0 ) {
                    tics=TICSPERFRAME;
                    dax=(tics*septr->cos);
                    day=(tics*septr->sin);
                    j=headspritesect[s];
                    while( j != -1 ) {
                         k=nextspritesect[j];
                         if( sprptr[j]->owner < MAXSPRITES ) {
                              dax2=dax>>10;
                              day2=day>>10;
                              movesprite(j,dax2,day2,0,4<<8,4<<8,0);
                         }
                         j=k;
                    }
                    for( p=connecthead; p >= 0; p=connectpoint2[p] ) {
                         if( cursectnum[p] == s ) {
                              if( posz[p] >= (sect->floorz-(42<<8)) ) {
                                   clipmove(&posx[p],&posy[p],&posz[p],
                                            &cursectnum[p],dax<<4,day<<4,128L,4<<8,4<<8,CLIPMASK0);
                                   setsprite(playersprite[p],posx[p],posy[p],posz[p]+(42<<8));
                                   revolvedoorstat[p]=1;
                              }
                         }
                    }
                    dax>>=12;
                    day>>=12;
                    sect->floorxpanning-=(unsigned char)dax;
                    sect->floorypanning+=(unsigned char)day;
               }
               if( (effect&flags32[CPANNING]) != 0 ) {
                    tics=TICSPERFRAME;
                    dax=(tics*septr->cos)>>12;
                    day=(tics*septr->sin)>>12;
                    sect->ceilingxpanning-=(unsigned char)dax;
                    sect->ceilingypanning+=(unsigned char)day;
               }
               if( (septr->delay-=TICSPERFRAME) > 0 ) {
                    continue;
               }
               // negative overflow here without this - jeffy
               if( septr->delay < 0 ) {
                    septr->delay=0;
               }
               septr->delay+=septr->delayreset;
               if( (effect&flags32[PULSELIGHT]) != 0 ) {
                    sect->ceilingshade+=septr->animate;
                    if( septr->hi > septr->lo ) {
                         hi=septr->hi;
                         lo=septr->lo;
                    }
                    else {
                         hi=septr->lo;
                         lo=septr->hi;
                    }
                    if( septr->animate < 0 ) {
                         if( sect->ceilingshade <= lo ) {
                              septr->animate=abs(septr->animate);
                         }
                    }
                    else {
                         if( sect->ceilingshade >= hi ) {
                              septr->animate=-septr->animate;
                         }
                    }
                    sect->floorshade=sect->ceilingshade;
                    startwall=sect->wallptr;
                    endwall=startwall+sect->wallnum-1;
                    for( j=startwall; j <= endwall; j++ ) {
                         wallptr[j]->shade=sect->ceilingshade;
                    }
               }
               else if( (effect&flags32[FLICKERLIGHT]) != 0 ) {
                   r=krand_intercept("TAG 1491");
                    if( r < 16384 ) {
                         sect->ceilingshade=septr->hi;
                    }
                    else if( r > 16384 ) {
                         sect->ceilingshade=septr->lo;
                    }
                    sect->floorshade=sect->ceilingshade;
                    startwall=sect->wallptr;
                    endwall=startwall+sect->wallnum-1;
                    for( j=startwall; j <= endwall; j++ ) {
                         wallptr[j]->shade=sect->ceilingshade;
                    }
               }
               else if( (effect&flags32[FLICKERDELAY]) != 0 ) {
                    if( sect->ceilingshade == septr->lo ) {
                         sect->ceilingshade=septr->hi;
                    }
                    else {
                         sect->ceilingshade=septr->lo;
                         septr->delay>>=2;
                    }
                    sect->floorshade=sect->ceilingshade;
                    startwall=sect->wallptr;
                    endwall=startwall+sect->wallnum-1;
                    for( j=startwall; j <= endwall; j++ ) {
                         wallptr[j]->shade=sect->ceilingshade;
                    }
               }
               else if( (effect&flags32[BLINKDELAY]) != 0 ) {
                    if( sect->ceilingshade == septr->lo ) {
                         sect->ceilingshade=septr->hi;
                    }
                    else {
                         sect->ceilingshade=septr->lo;
                    }
                    sect->floorshade=sect->ceilingshade;
                    startwall=sect->wallptr;
                    endwall=startwall+sect->wallnum-1;
                    for( j=startwall; j <= endwall; j++ ) {
                         wallptr[j]->shade=sect->ceilingshade;
                    }
               }
               if( (effect&flags32[KILLSECTOR]) != 0 ) {
                    floorz=sectptr[s]->floorz;
                    for( p=connecthead ; p >= 0 ; p=connectpoint2[p] ) {
                         if( cursectnum[p] == s ) {
                              // matrix specific check here
                              if( (klabs(posz[p] - floorz) < 10240) || (mission == 7) ) {
                                   if( (k=fdxref[s]) != -1 ) {
                                        if( floordoorptr[k]->state != DOORCLOSED ) {
                                             changehealth(p,-septr->damage);
                                             changescore(p,-10);
                                        }
                                   }
                                   else {
                                        if( septr->delay == septr->delayreset ) {
                                             changehealth(p,-septr->damage);
                                             changescore(p,-10);
                                        }
                                   }
                              }
                         }
                    }
               }
          }
     }

     if( option[4] == 0 ) {
          for( i=0; i < sprelevcnt; i++ ) {
               movesprelevs(i);
          }
          for( i=0; i < numfloordoors; i++ ) {
               movefloordoor(i);
          }
          for( i=0; i < numvehicles; i++ ) {
               movevehicles(i);
          }
     }  

     tekdoanimpic();
     tekdodelayfuncs();
     if( (lockclock-ambupdateclock) > 120 ) {
          checkmapsndfx(screenpeek);
     }
}

int
stepdoor(int z,int z2,struct doortype *door,int newstate)
{
     if (z < z2) {
       z+=(door->step*TICSPERFRAME);
       if (z >= z2) {
            door->delay=DOORDELAY;
            door->state=newstate;
            z=z2;
       }
     }
     else if (z > z2) {
       z-=(door->step*TICSPERFRAME);
       if (z <= z2) {
            door->delay=DOORDELAY;
            door->state=newstate;
            z=z2;
       }
     }
     return(z);
}

void
showwall2d(int w,int onoff)
{
     if (onoff) {
          show2dwall[w>>3]|=(1<<(w&7));
     }
     else {
          show2dwall[w>>3]&=~(1<<(w&7));
     }
}

void
showsect2d(int s,int z)
{
     int  endwall,i,startwall;

     startwall=sectptr[s]->wallptr;
     endwall=startwall+sectptr[s]->wallnum-1;
     for (i=startwall ; i <= endwall ; i++) {
          if (wallptr[i]->nextwall != -1) {
               if (sectptr[wallptr[i]->nextsector]->floorz == z) {
                    showwall2d(i,0);
                    showwall2d(wallptr[i]->nextwall,0);
               }
               else {
                    showwall2d(i,1);
                    showwall2d(wallptr[i]->nextwall,1);
               }
          }
     }
}

void
showsect2dtoggle(int s,int onoff)
{
     int  endwall,i,startwall;

     startwall=sectptr[s]->wallptr;
     endwall=startwall+sectptr[s]->wallnum-1;
     for (i=startwall ; i <= endwall ; i++) {
          if (wallptr[i]->nextwall != -1) {
               showwall2d(i,onoff);
               showwall2d(wallptr[i]->nextwall,onoff);
          }
     }
}

int
sectorblocked(int   s)
{
     int       i,rv;

     rv=0;

     for( i=connecthead ; i >= 0 ; i=connectpoint2[i] ) {
          if( cursectnum[i] == s )
               rv=1;
          if( testneighborsectors(cursectnum[i], s) == 1 ) 
               rv=1;
     }
     if( headspritesect[s] != -1 )
          rv=1;

     return(rv);
}

void
movedoors(int d)
{
     int            hitag,i,j,s,sx;
     int           size,z;
     struct    doortype       *door;
     spritetype     *spr;
     walltype       *wall;
     char           stayopen;

     door=doorptr[d];
     s=door->sector;

     switch (door->state) {

     case D_NOTHING:
          break;

     case D_WAITING:
          stayopen=0;
          for( i=0 ; i < secnt ; i++ ) {
               sx=sexref[i];
               if( sx == door->sector ) {
                    if( ((septrlist[sx]->sectorflags)&flags32[QUICKCLOSE]) != 0) {   
                         if( mission == 7 ) {
                              stayopen=1;
                         }
                         else {
                              door->delay=0;
                         }
                    }
               }
          }
          if( stayopen == 0 ) {
               door->delay-=TICSPERFRAME;
          }
          if( door->delay <= 0 ) {
               door->delay=0;
               if( door->type < PLATFORMELEVTAG ) {
                    for( i=connecthead ; i >= 0 ; i=connectpoint2[i] ) {
                         if( cursectnum[i] == s ) {
                              door->delay=DOORDELAY;
                              break;
                         }
                    }
               }
               if( door->delay == 0 ) {
                    door->state=D_CLOSEDOOR;
               }
          }
          break;

     case D_OPENDOOR:
          switch (door->type) {
          case DOORUPTAG:
               switch( door->subtype ) {
               case DST_BAYDOOR:
                    playsound(S_BAYDOOR_OPEN,door->centx,door->centy,0,ST_UPDATE);
                    if( baydoorloop == -1 ) {
                         baydoorloop=playsound(S_BAYDOORLOOP,door->centx,door->centy,20,ST_UNIQUE);
                    }                   
                    break;
               case DST_HYDRAULICDOOR:
                    playsound(S_AIRDOOR_OPEN,door->centx,door->centy,0,ST_UPDATE);
                    playsound(S_AIRDOOR,door->centx,door->centy,0,ST_UPDATE);
                    break;
                    
               case DST_ELEVATORDOOR:
                    playsound(S_ELEVATOR_DOOR,door->centx,door->centy,0,ST_UPDATE);
                    break;

               case DST_MATRIXDOOR1:
                    playsound(S_MATRIX1,door->centx,door->centy,0,ST_UPDATE);
                    break;

               case DST_MATRIXDOOR2:
                    playsound(S_MATRIX2,door->centx,door->centy,0,ST_UPDATE);
                    break;
               case DST_MATRIXDOOR3:
                    playsound(S_MATRIX3,door->centx,door->centy,0,ST_UPDATE);
                    break;
               case DST_MATRIXDOOR4:
                    playsound(S_MATRIX4,door->centx,door->centy,0,ST_UPDATE);
                    break;

               default:
                    if( mission == 7 ) {
                         playsound(S_MATRIXDOOR2,door->centx,door->centy,0,ST_UPDATE);
                    }
                    else {
                         playsound(S_UPDOWNDR2_OP,door->centx,door->centy,0,ST_UPDATE);
                    }
                    break;
               }
               door->goalz[0]=sectptr[nextsectorneighborz(s,sectptr[s]->floorz,-1,-1)]->ceilingz;
               break;
          case DOORDOWNTAG:
               playsound(S_BIGSWINGOP,door->centx,door->centy,0,ST_UPDATE);
               
               door->goalz[0]=sectptr[nextsectorneighborz(s,sectptr[s]->ceilingz,1,1)]->floorz;
               
               break;


          case PLATFORMDROPTAG:
               door->goalz[0]=sectptr[nextsectorneighborz(s,sectptr[s]->ceilingz,1,1)]->floorz;
               break;
          case DOORSPLITHOR:
               if( mission == 7 ) {
                    playsound(S_MATRIXDOOR1,door->centx,door->centy,0,ST_UPDATE);
               }
               else {
                    playsound(S_WH_7,door->centx,door->centy,0,ST_UPDATE);
               }
               door->goalz[0]=sectptr[nextsectorneighborz(s,sectptr[s]->ceilingz,-1,-1)]->ceilingz;
               door->goalz[2]=sectptr[nextsectorneighborz(s,sectptr[s]->floorz,1,1)]->floorz;
               break;
          case DOORSPLITVER:
               playsound(S_SIDEDOOR1,door->centx,door->centy,0,ST_UPDATE);
               door->goalz[0]=door->points[0];
               door->goalz[1]=door->points[1];
               door->goalz[2]=door->points[2];
               door->goalz[3]=door->points[3];
               break;
          case BOXDELAYTAG:
          case PLATFORMDELAYTAG:
               playsound(S_PLATFORMSTART,door->centx,door->centy,0,ST_UPDATE);
               if( loopinsound == -1 ) {
                    loopinsound=playsound(S_PLATFORMLOOP,door->centx,door->centy,20,ST_UNIQUE);
               }
               door->goalz[0]=evptrlist[s]->lolevel;
               break;
          default:
               break;
          }
          door->state=D_OPENING;
          break;

     case D_CLOSEDOOR:
          switch (door->type) {
          case DOORUPTAG:
               switch( door->subtype ) {
               case DST_BAYDOOR:
                    playsound(S_BAYDOOR_OPEN,door->centx,door->centy,0,ST_UPDATE);
                    if( baydoorloop == -1 ) {
                         baydoorloop = playsound(S_BAYDOORLOOP,door->centx,door->centy,20,ST_UNIQUE);
                    }
                    break;

               case DST_HYDRAULICDOOR:
                    playsound(S_AIRDOOR_OPEN,door->centx,door->centy,0,ST_UPDATE);
                    playsound(S_AIRDOOR,door->centx,door->centy,0,ST_UPDATE);
                    break;
                    
               case DST_ELEVATORDOOR:
                    playsound(S_ELEVATOR_DOOR,door->centx,door->centy,0,ST_UPDATE);
                    break;

               case DST_MATRIXDOOR1:
                    playsound(S_MATRIX1,door->centx,door->centy,0,ST_UPDATE);
                    break;

               case DST_MATRIXDOOR2:
                    playsound(S_MATRIX2,door->centx,door->centy,0,ST_UPDATE);
                    break;
               case DST_MATRIXDOOR3:
                    playsound(S_MATRIX3,door->centx,door->centy,0,ST_UPDATE);
                    break;
               case DST_MATRIXDOOR4:
                    playsound(S_MATRIX4,door->centx,door->centy,0,ST_UPDATE);
                    break;

               default:
                    if( mission == 7 ) {
                         playsound(S_MATRIXDOOR2,door->centx,door->centy,0,ST_UPDATE);
                    }
                    else {
                         playsound(S_UPDOWNDR2_CL,door->centx,door->centy,0,ST_UPDATE);
                    }
                    break;
               }
               door->goalz[0]=sectptr[nextsectorneighborz(s,sectptr[s]->ceilingz,1,1)]->floorz;
               break;
          case DOORDOWNTAG:
               playsound(S_BIGSWINGOP,door->centx,door->centy,0,ST_UPDATE);
               door->goalz[0]=sectptr[s]->ceilingz;
               break;
          case DOORSPLITHOR:
               if( mission == 7 ) {
                    playsound(S_MATRIXDOOR1,door->centx,door->centy,0,ST_UPDATE);
               }
               else {
                    playsound(S_WH_7,door->centx,door->centy,0,ST_UPDATE);
               }
               door->goalz[0]=door->centz;
               door->goalz[2]=door->centz;
               break;
          case DOORSPLITVER:
               playsound(S_SIDEDOOR2,door->centx,door->centy,0,ST_UPDATE);
               if( wallptr[door->walls[0]]->x == wallptr[door->walls[3]]->x ) {
                    door->goalz[0]=door->centy;
                    door->goalz[2]=door->centy;
               }
               else {
                    door->goalz[0]=door->centx;
                    door->goalz[2]=door->centx;
               }
               door->goalz[1]=door->points[0];
               door->goalz[3]=door->points[2];
               break;
          case BOXELEVTAG:
          case PLATFORMELEVTAG:
               door->state=D_NOTHING;
               break;
          case BOXDELAYTAG:
          case PLATFORMDELAYTAG:
               playsound(S_PLATFORMSTART,door->centx,door->centy,0,ST_UPDATE);
               if( loopinsound == -1 ) {
                    loopinsound=playsound(S_PLATFORMLOOP,door->centx,door->centy,20,ST_UNIQUE);
               }
               door->goalz[0]=evptrlist[s]->hilevel;
               break;
          default:
               break;
          }
          door->state=D_CLOSING;
          if( (hitag=sectptr[s]->hitag) > 0 ) {
               for( i=0 ; i < MAXSPRITES ; i++ ) {
                    spr=sprptr[i];
                    if( spr->hitag == hitag ) {
                         switch (spr->picnum) {
                         case SWITCH2ON:
                              spr->picnum=SWITCH2OFF;
                              break;
                         case SWITCH3ON:
                              spr->picnum=SWITCH3OFF;
                              break;
                         }
                    }
               }
               for( i=0 ; i < numwalls ; i++ ) {
                    wall=wallptr[i];
                    if( wall->hitag == hitag ) {
                         switch (wall->picnum) {
                         case SWITCH2ON:
                              wall->picnum=SWITCH2OFF;
                              break;
                         case SWITCH3ON:
                              wall->picnum=SWITCH3OFF;
                              break;
                         }
                    }
               }
          }
          break;

     case D_OPENING:
          switch (door->type) {
          case DOORUPTAG:
          case DOORDOWNTAG:
          case PLATFORMDROPTAG:
               if( door->type == DOORUPTAG ) {
                    z=sectptr[s]->ceilingz;
               }
               else {
                    z=sectptr[s]->floorz;
               }
               z=stepdoor(z,door->goalz[0],door,D_OPENSOUND);
               if( door->type == DOORUPTAG ) {
                    sectptr[s]->ceilingz=z;
               }
               else {
                    sectptr[s]->floorz=z;
               }
               break;
          case DOORSPLITHOR:
               z=sectptr[s]->ceilingz;
               z=stepdoor(z,door->goalz[0],door,D_OPENSOUND);
               sectptr[s]->ceilingz=z;
               z=sectptr[s]->floorz;
               z=stepdoor(z,door->goalz[2],door,D_OPENSOUND);
               sectptr[s]->floorz=z;
               break;
          case DOORSPLITVER:
               if( wallptr[door->walls[0]]->x == wallptr[door->walls[3]]->x ) {
                    for( i=0 ; i < 8 ; i++ ) {
                         j=door->walls[i];
                         z=wallptr[j]->y;
                         z=stepdoor(z,door->goalz[i>>1],door,D_OPENSOUND);
                         dragpoint(j,wallptr[j]->x,z);
                    }
               }
               else {
                    for( i=0 ; i < 8 ; i++ ) {
                         j=door->walls[i];
                         z=wallptr[j]->x;
                         z=stepdoor(z,door->goalz[i>>1],door,D_OPENSOUND);
                         dragpoint(j,z,wallptr[j]->y);
                    }
               }
               break;
          case BOXELEVTAG:
          case PLATFORMELEVTAG:
          case BOXDELAYTAG:
          case PLATFORMDELAYTAG:
               size=sectptr[s]->ceilingz-sectptr[s]->floorz;
               z=sectptr[s]->floorz;
               z=stepdoor(z,door->goalz[0],door,D_OPENSOUND);
               sectptr[s]->floorz=z;
               if( door->type == BOXDELAYTAG || door->type == BOXELEVTAG ) {
                    sectptr[s]->ceilingz=sectptr[s]->floorz+size;
               }
               break;
          default:
               break;
          }
          break;

     case D_CLOSING:
          switch (door->type) {
          case DOORUPTAG:
          case DOORDOWNTAG:
               if( door->type == DOORUPTAG ) {
                    z=sectptr[s]->ceilingz;
               }
               else {
                    z=sectptr[s]->floorz;
               }  
               z=stepdoor(z,door->goalz[0],door,D_SHUTSOUND);
               if( door->type == DOORUPTAG ) {
                    sectptr[s]->ceilingz=z;
               }
               else {
                    sectptr[s]->floorz=z;
               }
               break;
          case DOORSPLITHOR:
               z=sectptr[s]->ceilingz;
               z=stepdoor(z,door->goalz[0],door,D_SHUTSOUND);
               sectptr[s]->ceilingz=z;
               z=sectptr[s]->floorz;
               z=stepdoor(z,door->goalz[2],door,D_SHUTSOUND);
               sectptr[s]->floorz=z;
               break;
          case DOORSPLITVER:
              i=headspritesect[s];
              if (i != -1) {
                   door->state=D_OPENDOOR;
              }
              for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
                   if (inside(posx[i],posy[i],s)) {
                       door->state=D_OPENDOOR;
                   }
              }
               if( wallptr[door->walls[0]]->x == wallptr[door->walls[3]]->x ) {
                    for( i=0 ; i < 8 ; i++ ) {
                         j=door->walls[i];
                         z=wallptr[j]->y;
                         z=stepdoor(z,door->goalz[i>>1],door,D_SHUTSOUND);
                         dragpoint(j,wallptr[j]->x,z);
                    }
               }
               else {
                    for( i=0 ; i < 8 ; i++ ) {
                         j=door->walls[i];
                         z=wallptr[j]->x;
                         z=stepdoor(z,door->goalz[i>>1],door,D_SHUTSOUND);
                         dragpoint(j,z,wallptr[j]->y);
                    }
               }
               break;
          case BOXDELAYTAG:
          case PLATFORMDELAYTAG:
               size=sectptr[s]->ceilingz-sectptr[s]->floorz;
               z=sectptr[s]->floorz;
               z=stepdoor(z,door->goalz[0],door,D_SHUTSOUND);
               sectptr[s]->floorz=z;
               if( door->type == BOXDELAYTAG ) {
                    sectptr[s]->ceilingz=sectptr[s]->floorz+size;
               }
               break;
          default:
               break;
          }
          break;

     case D_OPENSOUND:
          switch (door->type) {
          case DOORUPTAG:
               switch( door->subtype ) {
               case DST_BAYDOOR:
                    playsound(S_BAYDOOR_CLOSE,door->centx,door->centy,0,ST_UPDATE);
                    if( baydoorloop>=0 ) {  
                         stopsound(baydoorloop);
                         baydoorloop=-1;
                    }             
                    break;
               case DST_HYDRAULICDOOR:
                    playsound(S_AIRDOOR_CLOSE,door->centx,door->centy,0,ST_UPDATE);
                    break;


               case DST_ELEVATORDOOR:
               case DST_MATRIXDOOR1:
               case DST_MATRIXDOOR2:
               case DST_MATRIXDOOR3:
               case DST_MATRIXDOOR4:
                    break;
               default:
                    if( mission != 7 ) {
                         playsound(S_DOORKLUNK,door->centx,door->centy,0,ST_UPDATE);
                    }
                    break;
               }
               door->state=D_WAITING;
               showsect2d(door->sector,door->goalz[0]);
               break;

          case DOORDOWNTAG:
               playsound(S_WH_6,door->centx,door->centy,0,ST_UPDATE);
               showsect2dtoggle(door->sector,0);
               door->state=D_WAITING;
               break;

          case BOXELEVTAG:
          case PLATFORMELEVTAG:
          case PLATFORMDROPTAG:
               door->state=D_WAITING;
               showsect2d(door->sector,door->goalz[0]);
               break;
          case PLATFORMDELAYTAG:
          default:
               if( door->type == BOXDELAYTAG || door->type == PLATFORMDELAYTAG ) {
                    playsound(S_PLATFORMSTOP,door->centx,door->centy,0,ST_UPDATE);
                    if( loopinsound >= 0 ) {  
                         stopsound(loopinsound);
                         loopinsound=-1;
                    }             
                    showsect2d(door->sector,door->goalz[0]);
               }
               else {
                    showsect2dtoggle(door->sector,0);
               }
               door->state=D_WAITING;
               break;
          }
          break;

     case D_SHUTSOUND:
          switch (door->type) {
          case DOORUPTAG:
               switch( door->subtype ) {
               case DST_BAYDOOR:
                    playsound(S_BAYDOOR_CLOSE,door->centx,door->centy,0,ST_UPDATE);
                    if( baydoorloop>=0 ) {  
                         stopsound(baydoorloop);
                         baydoorloop=-1;
                    }             
                    break;

               case DST_HYDRAULICDOOR:
                    playsound(S_AIRDOOR_CLOSE,door->centx,door->centy,0,ST_UPDATE);
                    break;

               case DST_ELEVATORDOOR:
               case DST_MATRIXDOOR1:
               case DST_MATRIXDOOR2:
               case DST_MATRIXDOOR3:
               case DST_MATRIXDOOR4:
                    break;
               default:
                    if( mission != 7 ) {
                         playsound(S_DOORKLUNK,door->centx,door->centy,0,ST_UPDATE);
                    }
                    break;
               }
               door->state=D_NOTHING;
               showsect2d(door->sector,door->goalz[0]);
               break;

          case DOORDOWNTAG:
               showsect2dtoggle(door->sector,1);
               playsound(S_WH_6,door->centx,door->centy,0,ST_UPDATE);
               break;

          case BOXELEVTAG:
          case PLATFORMELEVTAG:
          case BOXDELAYTAG:
          case PLATFORMDELAYTAG:
               playsound(S_PLATFORMSTOP,door->centx,door->centy,0,ST_UPDATE);
               if( loopinsound>=0 ) {
                    stopsound(loopinsound);
                    loopinsound=-1;
               }
               showsect2d(door->sector,door->goalz[0]);
               break;
          default:
               showsect2dtoggle(door->sector,1);
               break;
          }
          door->state=D_NOTHING;
          break;
     }

}

void
movesprelevs(int e)
{
     int            i,j,n,tics;
     int           goalz;
     struct    spriteelev     *s;
     spritetype     *spr;

     s=sprelevptr[e];
     tics=TICSPERFRAME<<6;
     switch (s->state) {
     case E_WAITING:
          s->delay-=TICSPERFRAME;
          if (s->delay <= 0) {
               s->state=E_CLOSINGDOOR;
          }
          return;
     case E_CLOSINGDOOR:
          s->doorpos-=tics;
          if (s->doorpos <= 0) {
               s->doorpos=0;
               s->state=E_NEXTFLOOR;
          }
          break;
     case E_OPENINGDOOR:
          s->doorpos+=tics;
          if (s->doorpos >= E_DOOROPENPOS) {
               s->doorpos=E_DOOROPENPOS;
               s->state=E_WAITING;
               s->delay=E_WAITDELAY;
          }
       break;
     case E_MOVING:
          goalz=s->floorz[s->curfloor];
          if (s->curdir == E_GOINGUP) {
               s->floorpos-=tics;
               if (s->floorpos <= goalz) {
                    s->floorpos+=labs(s->floorpos-goalz);
                    s->state=E_OPENINGDOOR;
               }
          }
          else {
               s->floorpos+=tics;
               if (s->floorpos >= goalz) {
                    s->floorpos-=labs(s->floorpos-goalz);
                    s->state=E_OPENINGDOOR;
               }
          }
          break;
     case E_NEXTFLOOR:
          if (s->curdir == E_GOINGUP) {
               s->curfloor++;
               if (s->curfloor > s->floors) {
                    s->curfloor-=2;
                    s->curdir=E_GOINGDOWN;
                    //playsound(S_COPSEE1,sprptr[s->sprnum[0]]->x,sprptr[s->sprnum[0]]->y,0,ST_UNIQUE);
               }
          }
          else if (s->curdir == E_GOINGDOWN) {
               s->curfloor--;
               if (s->curfloor < 0) {
                    s->curfloor+=2;
                    s->curdir=E_GOINGUP;
                    //playsound(S_COPSEE2,sprptr[s->sprnum[0]]->x,sprptr[s->sprnum[0]]->y,0,ST_UNIQUE);
               }
          }
          s->state=E_MOVING;
          break;
     }
     for (i=0 ; i < s->parts ; i++) {
          j=s->sprnum[i];
          spr=sprptr[j];
          spr->z=s->startz[i]+s->floorpos-s->floorz[0];
          for (n=0 ; n < s->doors ; n++) {
               if (j == s->door[n]) {
                    spr->z-=s->doorpos;
               }
          }
     }
}

void
movefloordoor(int d)
{
     int  j,s,tics;
     struct floordoor *dptr;

     tics=TICSPERFRAME<<2;
     dptr=floordoorptr[d];
     switch (dptr->state) {
     case DOOROPENING:
          if (dptr->dist1 > 0) {
               s=tics;
               dptr->dist1-=s;
               if (dptr->dist1 < 0) {
                    s+=dptr->dist1;
                    dptr->dist1=0;
               }
               switch (dptr->dir) {
               case 0:
                    j=dptr->wall1;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y-s);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y-s);
                    break;
               case 1:
                    j=dptr->wall1;
                    dragpoint(j,wallptr[j]->x+s,wallptr[j]->y);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x+s,wallptr[j]->y);
                    break;
               case 2:
                    j=dptr->wall1;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y+s);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y+s);
                    break;
               case 3:
                    j=dptr->wall1;
                    dragpoint(j,wallptr[j]->x-s,wallptr[j]->y);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x-s,wallptr[j]->y);
                    break;
               }
          }
          if (dptr->dist2 > 0) {
               s=tics;
               dptr->dist2-=s;
               if (dptr->dist2 < 0) {
                    s+=dptr->dist2;
                    dptr->dist2=0;
               }
               switch (dptr->dir) {
               case 0:
                    j=dptr->wall2;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y+s);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y+s);
                    break;
               case 1:
                    j=dptr->wall2;
                    dragpoint(j,wallptr[j]->x-s,wallptr[j]->y);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x-s,wallptr[j]->y);
                    break;
               case 2:
                    j=dptr->wall2;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y-s);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x,wallptr[j]->y-s);
                    break;
               case 3:
                    j=dptr->wall2;
                    dragpoint(j,wallptr[j]->x+s,wallptr[j]->y);
                    j=wallptr[j]->point2;
                    dragpoint(j,wallptr[j]->x+s,wallptr[j]->y);
                    break;
               }
          }
          if (dptr->dist1 <= 0 && dptr->dist2 <= 0) {
               dptr->state=DOOROPENED;
          }
          break;
     case DOORCLOSING:
     case DOOROPENED:
     case DOORCLOSED:
          break;
     }
}

void
clearvehiclesoundindexes()
{
     int       i;

     for( i=0; i < MAXSECTORVEHICLES; i++ ) {
          sectvehptr[i]->soundindex=-1;
     }
}

void
movevehicles(int v)
{
     short     a,angv,ato,angto,curang,i,n,p,rotang,s,sto,stoptrack,track;
     int      distx,disty,px,py,x,y;
     int      xvect,xvect2,yvect,yvect2;
     int      lox,loy,hix,hiy;
     short     onveh[MAXPLAYERS];
     struct    sectorvehicle  *vptr;

     vptr=sectvehptr[v];

     if( vptr->soundindex == -1 ) {
                                        
          for( i=0; i<32; i++) {        //find mapno using names array
               if( !(strcasecmp(boardfilename,mapnames[i]) ) )
                    break;
          }
          switch( v ) {
          case 0:      
               switch( i ) {
                    case 4:   //level1.map
                         vptr->soundindex=playsound(S_TRAMBUSLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);      
                         break;
                    case 8:   //city1.map
                         vptr->soundindex=playsound(S_TRUCKLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);        
                         break;
                    case 11:  //beach1.map
                         vptr->soundindex=playsound(S_FORKLIFTLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);          
                         break;
                    case 17:  //mid3.map
                         vptr->soundindex=playsound(S_TRAMBUSLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);      
                         break;
                    case 19:  //sewer2.map
                         vptr->soundindex=playsound(S_FORKLIFTLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);          
                         break;
                    case 20:   //inds1.map
                         vptr->soundindex=playsound(S_FORKLIFTLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);          
                         break;
                    case 25:  //ware1.map
                         vptr->soundindex=playsound(S_FORKLIFTLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);          
                         break;
                    case 26:   //ware2.map
                         vptr->soundindex=playsound(S_TRUCKLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);        
                         break;
                    default:
                         break;
               }
               break;
          case 1:
               switch( i ) {
                    case 4:   //level1.map
                         vptr->soundindex=playsound(S_TRAMBUSLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);      
                         break;
                    case 11:   //beach1.map
                         vptr->soundindex=playsound(S_BOATLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);         
                         break;
                    case 26:   //ware2.map
                         vptr->soundindex=playsound(S_CARTLOOP,vptr->pivotx,vptr->pivoty,-1,ST_VEHUPDATE);         
                         break;
                    default:
                         break;
               }
               break;
          default:
               break;
          }
     }
     
     if( vptr->waittics > 0 ) {
          vptr->waittics-=TICSPERFRAME;
          if( vptr->soundindex != -1 ) {
               updatevehiclesnds(vptr->soundindex, vptr->pivotx, vptr->pivoty);
          }
          return;
     }

     px=vptr->pivotx;
     py=vptr->pivoty;

     track=vptr->track;
     distx=vptr->distx;
     disty=vptr->disty;
     stoptrack=vptr->stoptrack;
     if( vptr->stop[track] && (x=distx+disty) > 0L && x < 8192L ) {
          vptr->accelto=2;
          vptr->speedto=32;
     }
     else if( vptr->accelto != 8 ) {
          vptr->accelto=8;
          vptr->speedto=vptr->movespeed;
     }
     if( distx == 0L && disty == 0L ) {
          if( vptr->stop[stoptrack] ) {
               for( i=0 ; i < vptr->numsectors ; i++ ) {
                    s=vptr->sector[i];
                    if( sectptr[s]->lotag != 0 ) {
                         operatesector(s);
                    }
               }
               vptr->waittics=vptr->waitdelay;
               vptr->acceleration=0;
               vptr->speed=0;

          }
          distx=vptr->trackx[track]-px;
          disty=vptr->tracky[track]-py;
          vptr->angleto=getangle(distx,disty);
          vptr->accelto=8;
          vptr->distx=labs(distx);
          vptr->disty=labs(disty);
          distx=vptr->distx;
          disty=vptr->disty;
     }
     a=vptr->acceleration;
     ato=vptr->accelto;
     if( a < ato ) {
          a+=TICSPERFRAME;
          if( a > ato ) {
               a=ato;
          }
          vptr->acceleration=a;
     }
     else if( a > ato ) {
          a-=TICSPERFRAME;
          if( a < ato ) {
               a=ato;
          }
          vptr->acceleration=a;
     }
     s=vptr->speed;
     sto=vptr->speedto;
     if( s > sto ) {
          s-=a;
          if( s <= sto ) {
               s=sto;
          }
          vptr->speed=s;
     }
     else if( s < sto ) {
          s+=a;
          if( s > sto ) {
               s=sto;
          }
          vptr->speed=s;
     }
     rotang=curang=vptr->angle;                                 
     if( curang != vptr->angleto ) {
          vptr->angle=vptr->angleto;
          curang=vptr->angle;
     }
     xvect=(s*(int)TICSPERFRAME*(int)sintable[((curang+2560)&2047)])>>3;
     xvect2=xvect>>13;
     yvect=(s*(int)TICSPERFRAME*(int)sintable[((curang+2048)&2047)])>>3;
     yvect2=yvect>>13;
     distx-=labs(xvect2);
     if( distx < 0L ) {
          if( xvect2 < 0L ) {
               xvect2-=distx;
          }
          else {
               xvect2+=distx;
          }
          distx=0L;
          vptr->angleto=getangle(vptr->trackx[track]-px,vptr->tracky[track]-py);
     }
     disty-=labs(yvect2);
     if( disty < 0L ) {
          if( yvect2 < 0L ) {
               yvect2-=disty;
          }
          else {
               yvect2+=disty;
          }
          disty=0L;
          vptr->angleto=getangle(vptr->trackx[track]-px,vptr->tracky[track]-py);
     }
     if( distx == 0L && disty == 0L ) {
          vptr->stoptrack=track;
          track=(track+1)%vptr->tracknum;
          vptr->track=track;
          switch( v ) {
          //jsa vehicles
          case 0:
               if( !(strcasecmp(boardfilename,"CITY1.MAP")) || !(strcasecmp(boardfilename,"WARE2.MAP")))
                    playsound(S_TRUCKSTOP,vptr->pivotx,vptr->pivoty,0,ST_AMBUPDATE);      
               break;
          default:
               break;
          }
     }
     vptr->distx=distx;
     vptr->disty=disty;
     px+=xvect2;
     py+=yvect2;
     n=vptr->numpoints;
     for( i=0 ; i < n ; i++ ) {
          p=vptr->point[i];
          x=vptr->pointx[i];
          y=vptr->pointy[i];
          rotatepoint(px,py,px-x,py-y,curang,&x,&y);
          dragpoint(p,x,y);
     }
     vptr->pivotx=px;
     vptr->pivoty=py;
     rotang=((curang-rotang)+2048)&2047;
     n=vptr->numsectors;
     lox=loy=0x7FFFFFFF;
     hix=hiy=-(0x7FFFFFFF);
     for( i=0 ; i < 4 ; i++ ) {
          a=vptr->killw[i];
          if( wallptr[a]->x < lox ) {
               lox=wallptr[a]->x;
          }
          else if( wallptr[a]->x > hix ) {
               hix=wallptr[a]->x;
          }
          if( wallptr[a]->y < loy ) {
               loy=wallptr[a]->y;
          }
          else if (wallptr[a]->y > hiy) {
               hiy=wallptr[a]->y;
          }
     }
     memset(onveh,0,sizeof(short)*MAXPLAYERS);
     for( i=0 ; i < n ; i++ ) {
          p=headspritesect[vptr->sector[i]];
          while (p >= 0) {
               s=nextspritesect[p];
               x=sprptr[p]->x;
               y=sprptr[p]->y;
               x+=xvect2;
               y+=yvect2;
               rotatepoint(px,py,x,y,rotang,&sprptr[p]->x,&sprptr[p]->y);
               sprptr[p]->ang+=rotang;
               p=s;
          }
          for( p=connecthead ; p >= 0 ; p=connectpoint2[p] ) {
               x=posx[p];
               y=posy[p];
               if( cursectnum[p] == vptr->sector[i] ) {
                    x+=xvect2;
                    y+=yvect2;
                    rotatepoint(px,py,x,y,rotang,&posx[p],&posy[p]);
                    ang[p]+=rotang;
                    onveh[p]=1;
               }
          }
     }
     for( p=connecthead ; p >= 0 ; p=connectpoint2[p] ) {
          if( onveh[p] ) { 
               continue;
          }
          x=posx[p];
          y=posy[p];
          if( x > lox && x < hix && y > loy && y < hiy ) {
               if( (health[p] > 0) &&  (posz[p] > VEHICLEHEIGHT) ) {
                    changehealth(p,-9999);
                    changescore(p,-100);
                    if( goreflag ) {
                         tekexplodebody(playersprite[p]);
                    }
               }
          }
     }
     if( vptr->soundindex != -1 ) {
          updatevehiclesnds(vptr->soundindex, vptr->pivotx, vptr->pivoty);
     }
}

void                          // kick off function after specified tics elapse
teksetdelayfunc(void (*delayfunc)(short),int tics,short parm)
{
     int  i,n;

     if (delayfunc == NULL) {
       return;
     }
     n=numdelayfuncs;
     for (i=0 ; i < n ; i++) {
       if (delayfuncptr[i]->func == delayfunc) {
            if (tics == 0) {
              memmove(delayfuncptr[i],delayfuncptr[numdelayfuncs-1],
                sizeof(struct delayfunc));
              memset(delayfuncptr[numdelayfuncs-1],0,
                sizeof(struct delayfunc));
              numdelayfuncs--;
              return;
            }
            else {
              delayfuncptr[i]->tics=tics;
              delayfuncptr[i]->parm=parm;
              return;
            }
       }
     }
     delayfuncptr[numdelayfuncs]->func=delayfunc;
     delayfuncptr[numdelayfuncs]->tics=tics;
     delayfuncptr[numdelayfuncs]->parm=parm;
     numdelayfuncs++;
}

void
tekdodelayfuncs(void)
{
     int  i,n,p;

     n=numdelayfuncs;
     for (i=0 ; i < n ; i++) {
          if (delayfuncptr[i]->func == NULL) {
               continue;
          }
          delayfuncptr[i]->tics-=TICSPERFRAME;
          if (delayfuncptr[i]->tics <= 0) {
               p=delayfuncptr[i]->parm;
               (*delayfuncptr[i]->func)(p);
               memmove(delayfuncptr[i],delayfuncptr[numdelayfuncs-1],
                    sizeof(struct delayfunc));
                    memset(delayfuncptr[numdelayfuncs-1],0,
                    sizeof(struct delayfunc));
               numdelayfuncs--;
          }
     }
}

void
setanimpic(short *pic,short tics,short frames)
{
     int  i;

     for (i=0 ; i < numanimates ; i++) {
          if (animpicptr[i]->pic == pic) {
               return;
          }
     }
     if (numanimates+1 < MAXANIMPICS) {
          animpicptr[numanimates]->pic=pic;
          animpicptr[numanimates]->tics=tics;
          animpicptr[numanimates]->frames=frames;
          animpicptr[numanimates]->nextclock=lockclock+tics;
          numanimates++;
     }
}

void
tekdoanimpic(void)
{
     short pic;
     int  i,n;

     n=numanimates;
     for (i=0 ; i < n ; i++) {
          if (lockclock < animpicptr[i]->nextclock) {
               continue;
          }
          if (animpicptr[i]->frames > 0) {
               if (--animpicptr[i]->frames > 0) {
                    pic=*animpicptr[i]->pic;
                    pic++;
                    *animpicptr[i]->pic=pic;
                    animpicptr[i]->nextclock=lockclock+animpicptr[i]->tics;
               }
          }
          else if (animpicptr[i]->frames < 0) {
               if (++animpicptr[i]->frames < 0) {
                    pic=*animpicptr[i]->pic;
                    pic--;
                    *animpicptr[i]->pic=pic;
                    animpicptr[i]->nextclock=lockclock+animpicptr[i]->tics;
               }
          }
          else {
               numanimates--;
               if (numanimates > 0) {
                    memmove(animpicptr[i],animpicptr[numanimates],
                         sizeof(struct animpic));
                    memset(animpicptr[numanimates],0,sizeof(struct animpic));
               }
          }
     }
}

void 
checkmapsndfx(short p)
{

     int       i,s;
     int       dist;
     unsigned  int      effect;
     struct    sectoreffect   *septr;

     s=cursectnum[p];
     septr=septrlist[s];

     for( i=0; i<totalmapsndfx; i++ ) {
          switch(mapsndfxptr[i]->type) {
          case MAP_SFX_AMBIENT:
               dist=labs(posx[p]-mapsndfxptr[i]->x)+labs(posy[p]-mapsndfxptr[i]->y);
               if( (dist > AMBUPDATEDIST) && (mapsndfxptr[i]->id!=-1) ) {
                    stopsound(mapsndfxptr[i]->id);
                    mapsndfxptr[i]->id=-1;
               }
               else if( (dist < AMBUPDATEDIST) && (mapsndfxptr[i]->id==-1) )  {
                    mapsndfxptr[i]->id=playsound(mapsndfxptr[i]->snum, mapsndfxptr[i]->x,mapsndfxptr[i]->y, mapsndfxptr[i]->loops, ST_AMBUPDATE);
               }
               break;
          case MAP_SFX_SECTOR:
               if((cursectnum[p] != ocursectnum[p]) && (cursectnum[p] == mapsndfxptr[i]->sector) ) {
                   mapsndfxptr[i]->id=playsound(mapsndfxptr[i]->snum, mapsndfxptr[i]->x,mapsndfxptr[i]->y, mapsndfxptr[i]->loops, ST_UNIQUE);
               }                    
               break;
          default:
               break;
          }
     }

     if( !strncasecmp("SUBWAY",boardfilename,6) ) {
          if( ambsubloop == -1 ) {
               ambsubloop=playsound(S_SUBSTATIONLOOP, 0, 0, -1, ST_IMMEDIATE);
          }
     }
     else {
          if( ambsubloop != -1 ) {
               stopsound(ambsubloop);
               ambsubloop=-1;
          }
     }

     if( septr != NULL ) {
          effect=septr->sectorflags;
          if( (effect&flags32[SOUNDON]) != 0 ) {
               for( i=0; i<totalmapsndfx; i++ ) {
                    if( mapsndfxptr[i]->type == MAP_SFX_TOGGLED ) {
                         if( sectptr[mapsndfxptr[i]->sector]->hitag == septr->hi ) {
                              if( mapsndfxptr[i]->id == -1 ) {
                                   mapsndfxptr[i]->id=playsound(mapsndfxptr[i]->snum, mapsndfxptr[i]->x,mapsndfxptr[i]->y, mapsndfxptr[i]->loops,ST_UNIQUE | ST_IMMEDIATE);;
                              }
                         }
                    }
               }
          }
          if( (effect&flags32[SOUNDOFF]) != 0 ) {
               for( i=0; i<totalmapsndfx; i++ ) {
                    if( mapsndfxptr[i]->type == MAP_SFX_TOGGLED ) {
                         if( sectptr[mapsndfxptr[i]->sector]->hitag == septr->hi ) {
                              if( mapsndfxptr[i]->id != -1 ) {
                                   stopsound(mapsndfxptr[i]->id);
                                   mapsndfxptr[i]->id=-1;
                              }
                         }
                    }
               }
          }
     }

//     ambupdateclock=totalclock;
     ambupdateclock=lockclock;
}

void
tektagsave(int fil)
{
     int  i,rv;

     rv=write(fil,&numanimates,sizeof(int));
     for (i=0 ; i < numanimates ; i++) {
          write(fil,&animpic[i],sizeof(struct animpic));
     }
     rv=write(fil,&numdelayfuncs,sizeof(short));
     for (i=0 ; i < numdelayfuncs ; i++) {
          write(fil,&delayfunc[i],sizeof(struct delayfunc));
     }
     rv=write(fil,onelev,MAXPLAYERS);
     rv=write(fil,&secnt,sizeof(int));
     for (i=0 ; i < secnt ; i++) {
          write(fil,&sectoreffect[i],sizeof(struct sectoreffect));
     }
     rv=write(fil,sexref,MAXSECTORS*sizeof(int));
     rv=write(fil,&numdoors,sizeof(int));
     for (i=0 ; i < numdoors ; i++) {
          write(fil,&doortype[i],sizeof(struct doortype));
     }
     write(fil,&numfloordoors,sizeof(int));
     for (i=0 ; i < numfloordoors ; i++) {
          write(fil,&floordoor[i],sizeof(struct floordoor));
     }
     write(fil,fdxref,MAXSECTORS*sizeof(int));
     write(fil,&numvehicles,sizeof(int));
     for (i=0 ; i < numvehicles ; i++) {
          write(fil,&sectorvehicle[i],sizeof(struct sectorvehicle));
     }
     write(fil,elevator,MAXSECTORS*sizeof(struct elevatortype));
     write(fil,&sprelevcnt,sizeof(int));
     for (i=0 ; i < sprelevcnt ; i++) {
          write(fil,&spriteelev[i],sizeof(struct spriteelev));
     }
     write(fil,&totalmapsndfx,sizeof(int));
     for (i=0 ; i < totalmapsndfx ; i++) {
          write(fil,&mapsndfx[i],sizeof(struct mapsndfxtype));
     }
}

void
tektagload(int fil)
{
     int  i,rv;

     rv=read(fil,&numanimates,sizeof(int));
     for (i=0 ; i < numanimates ; i++) {
          read(fil,&animpic[i],sizeof(struct animpic));
     }
     rv=read(fil,&numdelayfuncs,sizeof(short));
     for (i=0 ; i < numdelayfuncs ; i++) {
          read(fil,&delayfunc[i],sizeof(struct delayfunc));
     }
     rv=read(fil,onelev,MAXPLAYERS);
     rv=read(fil,&secnt,sizeof(int));
     for (i=0 ; i < secnt ; i++) {
          read(fil,&sectoreffect[i],sizeof(struct sectoreffect));
     }
     rv=read(fil,sexref,MAXSECTORS*sizeof(int));
     rv=read(fil,&numdoors,sizeof(int));
     for (i=0 ; i < numdoors ; i++) {
          read(fil,&doortype[i],sizeof(struct doortype));
     }
     read(fil,&numfloordoors,sizeof(int));
     for (i=0 ; i < numfloordoors ; i++) {
          read(fil,&floordoor[i],sizeof(struct floordoor));
     }
     read(fil,fdxref,MAXSECTORS*sizeof(int));
     read(fil,&numvehicles,sizeof(int));
     for (i=0 ; i < numvehicles ; i++) {
          read(fil,&sectorvehicle[i],sizeof(struct sectorvehicle));
     }

     // must reinvoke vehicle sounds since all sounds were stopped
     // else updatevehiclesounds will update whatever is using
     // dsoundptr[vptr->soundindex]
     clearvehiclesoundindexes();

     read(fil,elevator,MAXSECTORS*sizeof(struct elevatortype));
     read(fil,&sprelevcnt,sizeof(int));
     for (i=0 ; i < sprelevcnt ; i++) {
          read(fil,&spriteelev[i],sizeof(struct spriteelev));
     }
     read(fil,&totalmapsndfx,sizeof(int));
     for (i=0 ; i < totalmapsndfx ; i++) {
          read(fil,&mapsndfx[i],sizeof(struct mapsndfxtype));
          // did we leave with a TOGGLED sound playong ?
          if( (mapsndfx[i].type == MAP_SFX_TOGGLED) && (mapsndfx[i].id != -1) ) {
               mapsndfxptr[i]->id=playsound(mapsndfxptr[i]->snum, mapsndfxptr[i]->x,mapsndfxptr[i]->y, mapsndfxptr[i]->loops, ST_UNIQUE);
          }
     }
}

void
tekheadbob(void)
{
     if( headbobon && (activemenu == 0) ) {
          headbob+=bobstep;
          if( headbob < -BOBBMAX || headbob > BOBBMAX ) {
               bobstep=-bobstep;
          }
     }
}

void
tekswitchtrigger(short snum)
{
     int       i,j;
     int      nexti;
     int      dax,day;

     j=sprite[neartagsprite].picnum;

     switch( j ) {
     case SWITCH2OFF:
          if( invredcards[snum] == 0 ) {
               showmessage("PASSAGE REQUIRES RED KEYCARD");
               return;
          }
          break;
     case SWITCH4OFF:
          if( invbluecards[snum] == 0 ) {
               showmessage("PASSAGE REQUIRES BLUE KEYCARD");
               return;
          }
          break;
     }

     switch( j ) {
     case SWITCH2ON:
     case SWITCH2OFF:
     case SWITCH3ON:
     case SWITCH3OFF:
     case SWITCH4ON:
     case SWITCH4OFF:
          dax = sprite[neartagsprite].x;
          day = sprite[neartagsprite].y;
          playsound(S_KEYCARDBLIP, dax,day,0, ST_UPDATE);
          break;
     default:
          break;
     }

     if (j == SWITCH2ON) sprite[neartagsprite].picnum = SWITCH2OFF;
     if (j == SWITCH2OFF) sprite[neartagsprite].picnum = SWITCH2ON;
     if (j == SWITCH3ON) sprite[neartagsprite].picnum = SWITCH3OFF;
     if (j == SWITCH3OFF) sprite[neartagsprite].picnum = SWITCH3ON;
     if (j == SWITCH4ON) sprite[neartagsprite].picnum = SWITCH4OFF;
     if (j == SWITCH4OFF) sprite[neartagsprite].picnum = SWITCH4ON;

     if (j == 3708) sprite[neartagsprite].picnum = 3709;
     if (j == 3709) sprite[neartagsprite].picnum = 3708;

     for(i=0;i<numsectors;i++)
          if (sector[i].hitag == sprite[neartagsprite].hitag)
               if (sector[i].lotag != 1)
                    operatesector(i);

     i = headspritestat[0];
     while (i != -1)
     {
          nexti = nextspritestat[i];
          if (sprite[i].hitag == sprite[neartagsprite].hitag)
               operatesprite(i);
          i = nexti;
     }


}

int
krand_intercept(char *stg)
{
     if (dbgflag) {
          if (dbgcolumn > 80) {
               fprintf(dbgfp,"\n");
               dbgcolumn=0;
          }
          else {
               fprintf(dbgfp,"%s(%-9d) ",stg,randomseed);
               dbgcolumn+=20;
          }
     }
     return(krand());
}

